/*
 * Headhunter (boss): body, transformation, waves, lightning, burst shots and the extending attack.
 */
#include "core.h"
#include "music.h"
#include "player_level.h"
#include "graphics.h"
#include "gameplay.h"
#include "entity_util.h"
#include "player_core.h"
#include "structs/ewram.h"
#include "sound.h"
#include "structs/main.h"
#include "syscalls.h"
#include "boss_defeat.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "damage_numbers.h"
#include "entity_hit.h"
#include "hit_shared_history.h"

/* Headhunter (entity type 0x6A): update. */
void Headhunter_UpdateMovement(struct EwramData_EntityData *);
void Headhunter_UpdateAttack(struct EwramData_EntityData *);
void Headhunter_UpdateDeath(struct EwramData_EntityData *);
u8 Headhunter_LoadGraphics(struct EwramData_EntityData *);
void HeadhunterTransformationPart_Update(struct EwramData_EntityData *);
void Headhunter_DamagePlayerOnContact(struct EwramData_EntityData *);
void Headhunter_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

#define KIND(e) (*(s8 *)&e->ext.headhunterChild.kind)


void Headhunter_SpawnFallingPart(struct EwramData_EntityData *, s32, s32, s32);
void Headhunter_SpawnWaveBurst(struct EwramData_EntityData *, s32, s32, s32);
void Headhunter_SpawnLightningBurst(struct EwramData_EntityData *, s32, s32, s32);
void Headhunter_SpawnAttackFollower(struct EwramData_EntityData *, s32);
void Headhunter_SpawnBurstShot(struct EwramData_EntityData *, s32, s32, s32, s32);
void Headhunter_SpawnExtendingAttack(struct EwramData_EntityData *, s32, s32, s32);
s32 Headhunter_UpdateIntro(struct EwramData_EntityData *entity);
void HeadhunterTransformationArc_Update(struct EwramData_EntityData *entity);
void Headhunter_SpawnLightningBurstFlicker(struct EwramData_EntityData *, s32, s32);
void Headhunter_SpawnWave(struct EwramData_EntityData *, s32, s32, s32);
void Headhunter_SpawnLightningBolt(struct EwramData_EntityData *, s32, s32);
void Headhunter_SpawnOrbitingParticle(struct EwramData_EntityData *, struct EwramData_EntityData *, s32, s32, s32);
void HeadhunterFallingPart_Update(struct EwramData_EntityData *entity);
void HeadhunterWaveBurst_Update(struct EwramData_EntityData *entity);
void HeadhunterLightningBurst_Update(struct EwramData_EntityData *entity);
void HeadhunterLightningBurstFlicker_Update(struct EwramData_EntityData *);
void HeadhunterWave_DamagePlayerOnContact(struct EwramData_EntityData *);
void HeadhunterLightningImpact_DamagePlayerOnContact(struct EwramData_EntityData *);
void HeadhunterLightningImpactFlicker_Update(struct EwramData_EntityData *);
void HeadhunterWave_Update(struct EwramData_EntityData *entity);
void HeadhunterLightningBolt_Update(struct EwramData_EntityData *entity);
void Headhunter_SpawnLightningImpact(struct EwramData_EntityData *owner, struct EwramData_EntityData *source, s32 x, s32 y);
void HeadhunterLightningImpact_Update(struct EwramData_EntityData *entity);
void Headhunter_SpawnLightningImpactFlicker(struct EwramData_EntityData *owner, struct EwramData_EntityData *source, s32 x, s32 y);
void HeadhunterAttackFollower_Update(struct EwramData_EntityData *);
void HeadhunterBurstShot_DamagePlayerOnContact(struct EwramData_EntityData *);
void HeadhunterAttackTip_DamagePlayerOnContact(struct EwramData_EntityData *);
void HeadhunterAttackTip_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
void HeadhunterAttackSegment_DamagePlayerOnContact(struct EwramData_EntityData *);
void HeadhunterAttackSegment_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
void HeadhunterBurstShot_Update(struct EwramData_EntityData *entity);
void HeadhunterAttackTip_Update(struct EwramData_EntityData *head);
struct EwramData_EntityData *Headhunter_CreateAttackSegment(struct EwramData_EntityData *owner, struct EwramData_EntityData *parent, s32 index);
void HeadhunterAttackSegment_Update(struct EwramData_EntityData *segment);

/* Headhunter: dispatch AI (0), attacks (1), defeat (2) and the frozen
 * defeated form (3); bit 1 of unk_4FE overrides the frame with unk_4FF,
 * bit 0 disables the contact damage. */
/**
 * @brief Dispatch movement (0), attacks (1), defeat (2) and bestiary
 * animation (3). Bit 1 of unk_4FE overrides the animation frame;
 * bit 0 disables contact checks. Defeat bypasses time-stop gating.
 */
void EnemyHeadhunterUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult results[3]; /* only results[0] is written; the original reserved 48 bytes */
    struct SingleFrameAnimation animation = sHeadhunterFrameAnim;
    if (entity->state != 2 && Entity_IsInRect(entity, -16, 0, 272, 176))
        Player_TrackEntity(entity);
    *(s8 *)&entity->ext.headhunter.flags &= ~3;
    if (entity->state != 2) Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity) && entity->state != 2)
    {
        Hit_CheckWithPlayer(results, entity);
        return;
    }
    switch (entity->state)
    {
    case 0: Headhunter_UpdateMovement(entity); break;
    case 1: Headhunter_UpdateAttack(entity); break;
    case 2: Headhunter_UpdateDeath(entity); break;
    case 3:
        Anim_Update(entity);
        return;
    }
    {
        u8 *initial = &entity->animFrame;
        u8 previous = *initial;
        s32 override = entity->ext.headhunter.flags & 2;
        u8 *frame = initial;
        if (override)
        {
            Anim_Start(entity, (struct AnimScript *)&animation, 2, 1);
            animation.frame = entity->ext.headhunter.forcedFrame;
            Anim_Update(entity);
        }
        else Anim_Update(entity);
        if (previous != *frame) Entity_UpdateBoxes(entity);
    }
    if (entity->state != 2 && !(entity->ext.headhunter.flags & 1))
        Hit_CheckWithPlayer(results, entity);
}

/* Headhunter (entity type 0x6A): update and create. */
/* Create the Headhunter (unless already defeated) with its three
 * transformation parts (HeadhunterTransformationPart_Update, kinds 2..0) at fixed room positions;
 * after the fight (bestiaryPreview) only the final defeated form is shown. */
/* The final global flag update keeps the address in r2 and loaded word
 * in r0 to reproduce agbcc allocation. A byte match does not establish
 * original source spelling or organization. */
void EnemyHeadhunterCreate(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    struct EwramData_EntityData *part;
    s32 kind;
    if (entity->spawnParam0 == 0 || gEwramData->gameState.bossRushMode != 0
        || !(gEwramData->gameState.bossEventFlags & 0x10))
    {
        if (Headhunter_LoadGraphics(entity))
        {
            Enemy_ResetFromTemplate(entity);
            if (gEwramData->gameState.bossRushMode) entity->attack += 20;
            if (gEwramData->gameState.bestiaryPreview)
            {
                entity->state = 3;
                Anim_StartIndexed(entity, 9, 2, 1);
                Anim_Update(entity);
                entity->basePalette = entity->ext.headhunter.palette2;
                entity->palette = entity->basePalette;
                return;
            }
            if (Anim_GetMaxBoxCount(entity) > 1)
                Hit_EnableContact(entity, 8, Headhunter_DamagePlayerOnContact, Headhunter_OnHit);
            else
                Hit_Enable(entity, 8, Headhunter_DamagePlayerOnContact, Headhunter_OnHit);
            Entity_UpdateBoxes(entity);
            Entity_SnapToGround(entity);
            for (kind = 2; kind >= 0; kind--)
            {
                s32 graphics;
                part = Entity_CreateObject(HeadhunterTransformationPart_Update);
                if (part == NULL) goto delete;
                part->spriteLayer = entity->spriteLayer + 1;
                graphics = entity->ext.headhunter.graphics;
                switch (kind)
                {
                case 0:
                    Sprite_Attach(part, sHeadhunterSpriteSheet, graphics, *(s8 *)&entity->ext.headhunter.palette0);
                    part->basePalette = entity->ext.headhunter.palette0;
                    Anim_Start(part, &sHeadhunterAnimScript0, 2, 1);
                    part->posX.part16.integer = 0xF0 - camera->xPos.part16.integer;
                    part->posY.part16.integer = 0x7C - camera->yPos.part16.integer;
                    break;
                case 1:
                    Sprite_Attach(part, sHeadhunterSpriteSheet, graphics, *(s8 *)&entity->ext.headhunter.palette1);
                    part->basePalette = entity->ext.headhunter.palette1;
                    Anim_Start(part, &sHeadhunterAnimScript1, 2, 1);
                    part->posX.part16.integer = 0x110 - camera->xPos.part16.integer;
                    part->posY.part16.integer = 0x80 - camera->yPos.part16.integer;
                    break;
                case 2:
                    Sprite_Attach(part, sHeadhunterSpriteSheet, graphics, *(s8 *)&entity->ext.headhunter.palette2);
                    part->basePalette = entity->ext.headhunter.palette2;
                    Anim_Start(part, &sHeadhunterAnimScript2, 2, 1);
                    part->posX.part16.integer = 0x100 - camera->xPos.part16.integer;
                    part->posY.part16.integer = 0x60 - camera->yPos.part16.integer;
                    break;
                }
                part->enemyId = entity->enemyId;
                part->scrollWithBg = 1;
                part->drawFunc = Sprite_DrawEntity;
                part->flipX = entity->flipX;
                ENTITY_OWNER(part) = entity;
                KIND(part) = kind;
                Anim_Update(part);
            }
            entity->state = 0;
            Anim_StartIndexed(entity, 0x17, 2, 1);
            Anim_Update(entity);
            Player_SetSelectedTarget(entity);
            { register u32 *flags asm("r2") = &gEwramData->gameState.stateFlags; /* FAKEMATCH: pinned register */
              u32 value = *flags;
              value |= 2;
              *flags = value; }
            Music_SetCurrent(0x10);
            return;
        }
    }
delete:
    EntityDelete(entity);
}

/* Headhunter AI (81EBC), intro (82EA0), attack states (82FFC) and defeat
 * sequence (83860). */

#define PHASE(e) (e->ext.headhunter.formPhase)
#define FLAGS(e) (e->ext.headhunter.flags)
#define FLAGS_S8(e) (*(s8 *)&e->ext.headhunter.flags)
#define FRAME(e) (e->ext.headhunter.forcedFrame)
#define ACCEL_X(e) ((e)->accelX.whole)
#define ACCEL_Y(e) ((e)->accelY.whole)
#define FLASH(e)                                                          \
    if (Entity_GetVisibilityPattern((e)->timer)) (e)->palette = 4;                   \
    else (e)->palette = (e)->basePalette;
#define IN_BOX(e, dx, dy, w, h) \
    Target_IsInRect((s16)((e)->posX.part16.integer - (dx)), (s16)((e)->posY.part16.integer - (dy)), (s16)((w)), (s16)((h)))

/* Headhunter AI (state subState, timers stage/timer): 0-1 intro and
 * first form, 2-12 jumping first form and hovering second form, 20-25
 * transformation into the third form, 26-33 / 40-43 / 50-51 walking,
 * leaping and wall-clinging third form. Choices switch to the attack
 * states of Headhunter_UpdateAttack (state = 1). FLAGS bit 0 disables contact
 * damage, bit 1 overrides the frame with FRAME, bits 2/3 are the hover
 * directions. */
/**
 * @brief Movement state 0: coordinate the intro, phase transitions and
 * first-form jumps, second-form hovering and third-form walking,
 * leaping and wall movement. Preserve the original macros and timers.
 */
void Headhunter_UpdateMovement(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    s32 r;
    switch (entity->subState)
    {
    case 0:
        if (!Headhunter_UpdateIntro(entity)) break;
        entity->subState++;
        entity->stage = 0;
        PHASE(entity) = 1;
        entity->timer = 100;
        Anim_StartIndexed(entity, 0x18, 2, 0);
        break;
    case 1:
        if (entity->stage == 0)
        {
            if (entity->timer != 0)
            {
                entity->timer--;
                FLASH(entity);
                if (entity->timer == 0) entity->stage++;
            }
        }
        else entity->palette = 4;
        FLAGS(entity) |= 1;
        if (entity->animScriptIndex != 0x18) break;
        if (!entity->animDone) break;
        Anim_StartIndexed(entity, 0, 2, 0);
        break;
    case 2:
        if (entity->stage == 1)
        {
            entity->palette = entity->basePalette;
            entity->subState++;
        }
        if (entity->timer++ > 27)
        {
            entity->subState++;
            entity->timer = 60;
        }
        break;
    case 3:
        if (entity->timer++ > 59)
        {
            s32 found;
            entity->flipX = Entity_IsTargetRight(entity);
            entity->timer = 0;
            entity->stage = 0;
            ACCEL_X(entity) = 0;
            entity->velY.whole = 0x10000;
            ACCEL_Y(entity) = 0x2800;
            if (entity->flipX) entity->velX.whole = 0x6000;
            else entity->velX.whole = -0x6000;
            found = IN_BOX(entity, 8, 40, 16, 80);
            if (!found && IN_BOX(entity, 40, 40, 80, 80))
            {
                entity->state = 1;
                entity->subState = found;
                entity->stage = found;
                entity->timer = found;
                break;
            }
        }
        if (entity->animScriptIndex == 1 && IN_BOX(entity, 60, 40, 120, 80))
        {
            entity->subState++;
            entity->timer = 0;
            entity->animFlags |= 2;
            break;
        }
        if (entity->stage == 0)
        {
            if (*(s8 *)&entity->ext.headhunter.hitReact || IN_BOX(entity, 30, 40, 60, 80))
            {
                entity->stage = 1;
                entity->ext.headhunter.hitReact = 0;
                entity->flipX = Entity_IsTargetRight(entity);
                entity->timer = 0;
                ACCEL_X(entity) = 0;
                entity->velY.whole = 0x10000;
                ACCEL_Y(entity) = 0x2800;
                if (entity->flipX) entity->velX.whole = -0x8000;
                else entity->velX.whole = 0x8000;
            }
        }
        Entity_MoveWithCollisionDefault(entity, 0x40000, -0x200000);
        break;
    case 4:
        if (entity->timer++ > 17)
        {
            Anim_StartIndexed(entity, 2, 2, 1);
            entity->timer = 0;
            entity->subState++;
        }
        break;
    case 5:
        if (entity->timer++ > 7)
        {
            entity->subState = 3;
            entity->timer = 60;
        }
        break;
    case 6:
        FLAGS(entity) |= 1;
        entity->subState++;
        break;
    case 7:
        FLAGS(entity) |= 1;
        entity->timer = 0;
        entity->stage = 0;
        break;
    case 8:
        FLAGS(entity) |= 1;
        entity->stage = 0;
        if (entity->timer++ > 27)
        {
            entity->subState++;
            entity->timer = 0;
            Anim_StartIndexed(entity, 0x1A, 2, 0);
        }
        break;
    case 9:
        FLAGS(entity) |= 1;
        PHASE(entity) = 2;
        if (entity->stage == 0)
        {
            if (entity->timer++ > 19)
            {
                entity->stage++;
                entity->timer = 90;
                PlaySong(0x1CF);
            }
        }
        else if (entity->stage == 1)
        {
            if (entity->timer != 0)
            {
                entity->timer--;
                FLASH(entity);
                if (entity->timer == 0) entity->stage++;
            }
        }
        else entity->palette = 4;
        break;
    case 10:
        ACCEL_X(entity) = 0;
        entity->velX.whole = 0;
        entity->velY.whole = 0;
        ACCEL_Y(entity) = -0x1000;
        entity->flipX = Entity_IsTargetRight(entity);
        if (entity->flipX) FLAGS(entity) |= 4;
        else FLAGS_S8(entity) &= ~4;
        entity->subState++;
        entity->palette = entity->basePalette;
    case 11:
        EntityAccelerateAndMove(entity, 0x10000);
        if (entity->posY.whole + camera->yPos.whole <= 0x6FFFFF)
        {
            entity->subState++;
            Anim_StartIndexed(entity, 6, 2, 1);
        }
        break;
    case 12:
    {
        s32 x; /* shared by both branches (register allocation) */
        entity->flipX = Entity_IsTargetRight(entity);
        if ((s8)--entity->timer <= 0)
        {
            if (!(RandomNumberGenerator() & 1))
            {
                entity->state = 1;
                entity->subState = 10;
                break;
            }
            entity->timer = RandomNumberGenerator() % 30 + 60;
        }
        if (FLAGS(entity) & 8)
        {
            if (entity->posY.whole + camera->yPos.whole <= 0x71FFFF)
                ACCEL_Y(entity) = 0xC00;
            else
            {
                ACCEL_Y(entity) = -0xC00;
                FLAGS_S8(entity) &= ~8;
            }
        }
        else
        {
            if (entity->posY.whole + camera->yPos.whole > 0x6E0000)
                ACCEL_Y(entity) = -0xC00;
            else
            {
                ACCEL_Y(entity) = 0xC00;
                FLAGS(entity) |= 8;
            }
        }
        if (FLAGS(entity) & 4)
        {
            s32 posX = entity->posX.whole;
            x = posX + camera->xPos.whole;
            if (x <= 0xBFFFFF)
                FLAGS_S8(entity) &= ~4;
            else if (gEwramData->playerState.player->posX.whole - posX > 0x300000)
            {
                if (x > 0x13FFFFF) ACCEL_X(entity) = -0x600;
                else ACCEL_X(entity) = 0x600;
            }
            else
                ACCEL_X(entity) = -0x600;
        }
        else
        {
            s32 posX = entity->posX.whole;
            x = posX + camera->xPos.whole;
            if (x > 0x1400000)
                FLAGS(entity) |= 4;
            else if (gEwramData->playerState.player->posX.whole - posX < -0x300000 && x > 0xC00000)
                ACCEL_X(entity) = -0x600;
            else
                ACCEL_X(entity) = 0x600;
        }
        EntityAccelerateAndMove(entity, 0x18000);
        break;
    }
    case 20:
        FLAGS(entity) |= 1;
        entity->velX.whole = 0;
        ACCEL_X(entity) = 0;
        entity->velY.whole = 0;
        ACCEL_Y(entity) = 0x2000;
        if (entity->posX.whole + camera->xPos.whole <= 0x67FFFF) entity->velX.whole = 0x4000;
        if (entity->posX.whole + camera->xPos.whole > 0x1970000) entity->velX.whole = -0x4000;
        entity->subState++;
        entity->stage = 0;
        break;
    case 21:
        FLAGS(entity) |= 1;
        entity->timer = 0;
        EntityAccelerateAndMove(entity, 0x60000);
        if (entity->posY.whole + camera->yPos.whole <= 0xB3FFFF) break;
        entity->velX.whole = 0;
        ACCEL_X(entity) = 0;
        entity->velY.whole = 0;
        ACCEL_Y(entity) = 0;
        entity->posY.whole = 0xC00000 - camera->yPos.whole;
        if (entity->stage != 0) break;
        entity->stage++;
        Anim_StartIndexed(entity, 0x1B, 2, 0);
        Headhunter_SpawnFallingPart(entity, entity->flipX ? 0xB0000 : -0xB0000, -0x1D0000, 1);
        break;
    case 22:
        FLAGS(entity) |= 1;
        entity->stage = 0;
        if (entity->animScriptIndex == 0x1B && entity->animDone)
            Anim_StartIndexed(entity, 0x1C, 2, 0);
        if (entity->timer++ > 27)
        {
            entity->subState++;
            entity->stage = 0;
            entity->timer = 120;
            PHASE(entity) = 3;
        }
        break;
    case 23:
        FLAGS(entity) |= 1;
        if (entity->stage == 0)
        {
            if (entity->timer != 0)
            {
                entity->timer--;
                FLASH(entity);
                if (entity->timer == 0) entity->stage++;
            }
        }
        else entity->palette = 4;
        break;
    case 24:
        ACCEL_X(entity) = 0;
        entity->velX.whole = 0;
        entity->velY.whole = 0x10000;
        ACCEL_Y(entity) = 0x1000;
        entity->palette = entity->basePalette;
        entity->flipX = Entity_IsTargetRight(entity);
        if (entity->flipX) FLAGS(entity) |= 4;
        else FLAGS_S8(entity) &= ~4;
        entity->subState++;
        entity->timer = 0;
        PlaySong(0x72);
    case 25:
        entity->flipX = Entity_IsTargetRight(entity);
        Entity_MoveWithCollisionDefault(entity, 0x40000, -0x200000);
        if (entity->timer++ > 19)
        {
            entity->subState++;
            Anim_StartIndexed(entity, 10, 2, 1);
            entity->timer = 0;
        }
        break;
    case 26:
        entity->flipX = Entity_IsTargetRight(entity);
        entity->velX.whole = entity->flipX ? 0x10000 : -0x10000;
        Entity_MoveWithCollisionDefault(entity, 0x40000, -0x200000);
        if (++entity->timer > 19 && IN_BOX(entity, 60, 150, 120, 150))
        {
            entity->subState++;
            Anim_StartIndexed(entity, 11, 2, 0);
            entity->timer = 0;
            entity->velX.whole = 0;
        }
        else if (entity->timer > 119)
        {
            entity->subState++;
            Anim_StartIndexed(entity, 11, 2, 0);
            entity->timer = 0;
            entity->velX.whole = 0;
        }
        break;
    case 27:
        Entity_MoveWithCollisionDefault(entity, 0x40000, -0x200000);
        if (entity->timer++ > 29)
        {
            entity->subState++;
            entity->timer = 0;
            FLAGS(entity) |= 2;
            FRAME(entity) = 0x39;
            ACCEL_X(entity) = 0;
            entity->velX.whole = 0;
            entity->velY.whole = -0x50000;
            ACCEL_Y(entity) = 0x1000;
        }
        break;
    case 28:
        FLAGS(entity) |= 2;
        FRAME(entity) = 0x39;
        r = Entity_MoveWithCollisionDefault(entity, 0x40000, -0x380000);
        if (r & 8)
        {
            Anim_StartIndexed(entity, 14, 2, 0);
            FLAGS_S8(entity) &= ~2;
            entity->posY.whole = 0x400000 - camera->yPos.whole;
            entity->subState++;
            entity->flipY = 1;
            entity->timer = 20;
            PlaySong(0x12F);
        }
        break;
    case 29:
        if ((s8)--entity->timer <= 0) entity->subState = 30;
        break;
    case 30:
        entity->flipX = Entity_IsTargetRight(entity);
        r = RandomNumberGenerator() % 7;
        if (r <= 2)
        {
            Anim_StartIndexed(entity, 12, 2, 1);
            entity->subState = 31;
            ACCEL_X(entity) = 0;
            entity->velY.whole = 0;
            ACCEL_Y(entity) = 0;
            entity->velX.whole = entity->flipX ? 0x20000 : -0x20000;
            entity->timer = 20;
        }
        else if (r == 3)
        {
            entity->subState = 29;
            entity->timer = 30;
            Anim_StartIndexed(entity, 14, 2, 0);
        }
        else if (r == 4)
        {
            entity->subState = 32;
            entity->stage = 0;
            ACCEL_X(entity) = 0;
            entity->velX.whole = 0;
            entity->velY.whole = 0x10000;
            ACCEL_Y(entity) = 0x2000;
        }
        else
        {
            entity->state = 1;
            entity->subState = 30;
            entity->timer = 0;
        }
        break;
    case 31:
    {
        s32 x;
        EntityMoveX(entity);
        x = entity->posX.whole + camera->xPos.whole;
        if (x <= 0x66FFFF)
        {
            entity->posX.whole = 0x670000 - camera->xPos.whole;
            entity->subState = 30;
        }
        else if (x > 0x1980000)
        {
            entity->posX.whole = 0x1980000 - camera->xPos.whole;
            entity->subState = 30;
        }
        else if ((s8)--entity->timer <= 0)
            entity->subState = 30;
        break;
    }
    case 32:
        FLAGS(entity) |= 2;
        if (entity->posY.whole + camera->yPos.whole <= 0x600000)
            FRAME(entity) = 0x3B;
        else
        {
            FRAME(entity) = 0x39;
            if (entity->stage == 0)
            {
                entity->stage++;
                entity->flipY = 0;
                entity->posY.whole += 0x180000;
            }
        }
        EntityAccelerateAndMove(entity, 0x60000);
        if (entity->posY.whole + camera->yPos.whole <= 0xBEFFFF) break;
        entity->posY.whole = 0xBF0000 - camera->yPos.whole;
        entity->subState++;
        entity->stage = 0;
        entity->timer = 14;
        Effect_SpawnRisingParticleInBox(entity->posX.whole, entity->posY.whole, 0x200000, 0, entity->spriteLayer - 1, 1);
        Effect_SpawnRisingParticleInBox(entity->posX.whole, entity->posY.whole, 0x200000, 0, entity->spriteLayer - 1, 1);
        PlaySong(0x12F);
        break;
    case 33:
        FLAGS(entity) |= 2;
        FRAME(entity) = 0x3B;
        if ((s8)--entity->timer <= 0) entity->subState = 40;
        break;
    case 40:
        entity->flipX = Entity_IsTargetRight(entity);
        r = RandomNumberGenerator() & 7;
        if (r <= 2)
        {
            s32 side = RandomNumberGenerator() % 3;
            if (side == 0)
            {
                s32 x = entity->posX.whole + camera->xPos.whole;
                if (x <= 0xC6FFFF)
                {
                    entity->subState = 42;
                    ACCEL_X(entity) = 0;
                    entity->flipX = 0;
                    entity->velX.whole = -0x30000;
                    entity->velY.whole = -0x40000;
                    ACCEL_Y(entity) = 0x2000;
                    FLAGS(entity) |= 2;
                    FRAME(entity) = 0x3A;
                    break;
                }
                if (x > 0x1380000)
                {
                    entity->subState = 42;
                    ACCEL_X(entity) = 0;
                    entity->flipX = 1;
                    entity->velX.whole = entity->flipX ? 0x30000 : -0x30000;
                    entity->velY.whole = -0x40000;
                    ACCEL_Y(entity) = 0x2000;
                    FLAGS(entity) |= 2;
                    FRAME(entity) = 0x3A;
                    break;
                }
            }
            Anim_StartIndexed(entity, 12, 2, 1);
            entity->subState = 41;
            entity->flipX = Entity_IsTargetRight(entity);
            ACCEL_X(entity) = 0;
            entity->velY.whole = 0;
            ACCEL_Y(entity) = 0;
            entity->velX.whole = entity->flipX ? 0x20000 : -0x20000;
            entity->timer = 16;
        }
        else if (r == 3 || r == 4)
        {
            entity->subState = 33;
            entity->timer = 30;
            Anim_StartIndexed(entity, 14, 2, 0);
        }
        else if (r == 5)
        {
            entity->subState = 28;
            entity->timer = 0;
            ACCEL_X(entity) = 0;
            entity->velX.whole = 0;
            entity->velY.whole = -0x50000;
            ACCEL_Y(entity) = 0x1000;
            FLAGS(entity) |= 2;
            FRAME(entity) = 0x39;
        }
        else
        {
            entity->state = 1;
            entity->subState = 20;
            entity->timer = 0;
            FLAGS(entity) |= 2;
            FRAME(entity) = 0x3B;
        }
        break;
    case 41:
    {
        s32 x;
        EntityMoveX(entity);
        x = entity->posX.whole + camera->xPos.whole;
        if (x <= 0x66FFFF)
        {
            entity->posX.whole = 0x670000 - camera->xPos.whole;
            entity->subState = 40;
        }
        else if (x > 0x1980000)
        {
            entity->posX.whole = 0x1980000 - camera->xPos.whole;
            entity->subState = 40;
        }
        else if ((s8)--entity->timer <= 0)
            entity->subState = 40;
        break;
    }
    case 42:
    {
        s32 y;
        FLAGS(entity) |= 2;
        FRAME(entity) = 0x3A;
        EntityAccelerateAndMove(entity, 0x60000);
        y = entity->posY.whole + camera->yPos.whole;
        if (y > 0xBEFFFF)
        {
            entity->posY.whole = 0xBF0000 - camera->yPos.whole;
            entity->subState = 33;
            entity->stage = 0;
            entity->timer = 14;
            Effect_SpawnRisingParticleInBox(entity->posX.whole, entity->posY.whole, 0x200000, 0, entity->spriteLayer - 1, 1);
            Effect_SpawnRisingParticleInBox(entity->posX.whole, entity->posY.whole, 0x200000, 0, entity->spriteLayer - 1, 1);
            PlaySong(0x12F);
        }
        else if (y <= 0x4F0000)
        {
            entity->posY.whole = 0x400000 - camera->yPos.whole;
            Anim_StartIndexed(entity, 14, 2, 0);
            FLAGS_S8(entity) &= ~2;
            entity->subState = 29;
            entity->flipY = 1;
            entity->timer = 20;
            PlaySong(0x12F);
        }
        else
        {
            s32 x = entity->posX.whole + camera->xPos.whole;
            if (x <= 0x68FFFF)
            {
                entity->posX.whole = 0x630000 - camera->xPos.whole;
                entity->subState++;
                entity->timer = 20;
                FLAGS(entity) |= 2;
                FRAME(entity) = 0x45;
                PlaySong(0x12F);
            }
            else if (x > 0x1960000)
            {
                entity->posX.whole = 0x19C0000 - camera->xPos.whole;
                entity->subState++;
                entity->timer = 20;
                FLAGS(entity) |= 2;
                FRAME(entity) = 0x45;
                PlaySong(0x12F);
            }
        }
        break;
    }
    case 43:
        FLAGS(entity) |= 2;
        FRAME(entity) = 0x45;
        if ((s8)--entity->timer <= 0) entity->subState = 50;
        break;
    case 50:
        FLAGS(entity) |= 2;
        FRAME(entity) = 0x45;
        r = RandomNumberGenerator() % 7;
        if (r <= 2)
        {
            Anim_StartIndexed(entity, 0x11, 2, 1);
            FLAGS_S8(entity) &= ~2;
            entity->subState = 51;
            ACCEL_X(entity) = 0;
            entity->velX.whole = 0;
            ACCEL_Y(entity) = 0;
            if (entity->posY.whole + camera->yPos.whole > 0x980000) entity->velY.whole = -0x10000;
            else entity->velY.whole = 0x10000;
            entity->timer = 20;
        }
        else if (r == 3)
        {
            entity->subState = 43;
            entity->timer = 30;
            FLAGS(entity) |= 2;
            FRAME(entity) = 0x45;
        }
        else if (r == 4)
        {
            if (entity->flipX) entity->posX.whole += -0x50000;
            else entity->posX.whole += 0x50000;
            entity->flipX ^= 1;
            ACCEL_X(entity) = 0;
            entity->velX.whole = entity->flipX ? 0x30000 : -0x30000;
            entity->velY.whole = -0x38000;
            ACCEL_Y(entity) = 0x2000;
            entity->subState = 42;
            FLAGS(entity) |= 2;
            FRAME(entity) = 0x3A;
        }
        else
        {
            entity->state = 1;
            entity->subState = 40;
            entity->timer = 0;
            FLAGS(entity) |= 2;
            FRAME(entity) = 0x45;
        }
        break;
    case 51:
    {
        s32 y;
        EntityMoveY(entity);
        y = entity->posY.whole + camera->yPos.whole;
        if (y <= 0x67FFFF)
        {
            entity->posY.whole = 0x680000 - camera->yPos.whole;
            entity->subState = 50;
        }
        else if (y > 0xBF0000)
        {
            entity->posY.whole = 0xBF0000 - camera->yPos.whole;
            entity->subState = 50;
        }
        else if ((s8)--entity->timer <= 0)
            entity->subState = 50;
        break;
    }
    }
}

#define INTRO_STEP(e) (e->ext.headhunter.introStep)

/* Boss intro: after 8 frames lock the room (stateFlags bits 0/1, HUD flag
 * startPauseEnabled off), walk the player to within 0x30 px of x 0x100 (only when
 * spawnParam0 is set), wait a second, play the intro voice and hand back
 * control. Returns 1 when the intro is over. */
/**
 * @brief Run the room intro and optional forced player positioning,
 * then restore the HUD/input gate and return 1. Return 0 until
 * the sequence reaches its final step.
 */
s32 Headhunter_UpdateIntro(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    switch (INTRO_STEP(entity))
    {
    case 0:
        if (entity->timer++ > 7)
        {
            INTRO_STEP(entity)++;
            entity->timer = 0;
        }
        break;
    case 1:
        if (!(gEwramData->gameState.stateFlags & 1))
        {
            INTRO_STEP(entity)++;
            gEwramData->gameState.stateFlags |= 1;
            gEwramData->startPauseEnabled = 0;
            gEwramData->gameState.stateFlags |= 2;
        }
        break;
    case 2:
        if (entity->spawnParam0 == 0)
            INTRO_STEP(entity)++;
        else
        {
            s32 x = gEwramData->playerState.player->posX.whole + camera->xPos.whole;
            s32 distance = 0x1000000 - x;
            u16 input;
            if (distance < 0) distance = -distance;
            if (distance <= 0x2FFFFF)
            {
                INTRO_STEP(entity)++;
                entity->timer = 0;
                input = 0;
            }
            else
                input = x <= 0xFFFFFF ? 0x10 : 0x20;
            SetPlayerInput(2, input);
        }
        break;
    case 3:
        if (entity->timer++ > 59)
        {
            INTRO_STEP(entity)++;
            entity->timer = 0;
            PlaySong(0x1CE);
        }
        SetPlayerInput(2, 0);
        break;
    case 4:
        gEwramData->gameState.stateFlags &= ~1;
        gEwramData->startPauseEnabled = 1;
        return 1;
    }
    return 0;
}

#define ANIM_FRAME16(e) (*(u16 *)&(e)->animStep)
#define TO_PIXELS(d) ((d) < 0 ? -(-(d) >> 16) : (d) >> 16)
#define PIXEL_DIFF(a, b) ((a) - (b) < 0 ? -(((b) - (a)) >> 16) : ((a) - (b)) >> 16)

/* Attacks of the Headhunter (state subState, timer timer): 0-1 jump
 * back, 10-14 affine bursts launching waves (Headhunter_SpawnWaveBurst) or lightning
 * (Headhunter_SpawnLightningBurst), 20-27 / 30-37 / 40-47 three shooting forms: a spread of
 * shots (Headhunter_SpawnBurstShot, aimed at the player in 30s and 40s) or one big
 * shot (Headhunter_SpawnExtendingAttack). Each ends in AI state 0 with the next step. */
/**
 * @brief Attack state 1: select the phase-specific backward jump,
 * four-corner affine attack or projectile pattern, then return
 * to movement state 0 at its designated substate.
 */
void Headhunter_UpdateAttack(struct EwramData_EntityData *entity)
{
    s32 x, y, vx, vy;
    s32 angle;
    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        Anim_StartIndexed(entity, 3, 2, 0);
        entity->subState++;
        entity->velX.whole = 0;
    case 1:
        if (ANIM_FRAME16(entity) == 2)
        {
            entity->accelX.whole = 0;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
            if (entity->flipX) entity->velX.whole = 0x18000;
            else entity->velX.whole = -0x18000;
            PlaySong(0x85);
        }
        if (ANIM_FRAME16(entity) == 4)
            entity->velX.whole = 0;
        EntityDampVelocityX(entity, 16);
        Entity_MoveWithCollisionDefault(entity, 0x40000, -0x200000);
        if (entity->animDone || *(s8 *)&entity->ext.headhunter.hitReact)
        {
            entity->state = 0;
            entity->subState = 3;
            entity->timer = 60;
            Anim_StartIndexed(entity, 2, 2, 1);
        }
        break;
    case 10:
        if (RandomNumberGenerator() % 100 < 50)
        {
            Anim_StartIndexed(entity, 7, 2, 0);
            entity->subState++;
            entity->accelX.whole = 0;
            entity->accelY.whole = 0;
        }
        else
        {
            Anim_StartIndexed(entity, 7, 2, 0);
            entity->subState = 13;
            entity->accelX.whole = 0;
            entity->accelY.whole = 0;
        }
    case 11:
        EntityDampVelocityX(entity, 8);
        EntityDampVelocityY(entity, 8);
        EntityAccelerateAndMove(entity, 0x18000);
        if (entity->animStep > 3)
        {
            Headhunter_SpawnWaveBurst(entity, 0, 0, 0);
            Headhunter_SpawnWaveBurst(entity, 0, 0, 1);
            Headhunter_SpawnWaveBurst(entity, 0, 0, 2);
            Headhunter_SpawnWaveBurst(entity, 0, 0, 3);
            Headhunter_SpawnAttackFollower(entity, 0x65);
            entity->subState++;
            entity->timer = 0;
            PlaySong(0x1D1);
        }
        break;
    case 13:
        EntityDampVelocityX(entity, 8);
        EntityDampVelocityY(entity, 8);
        EntityAccelerateAndMove(entity, 0x18000);
        if (entity->animStep > 3)
        {
            Headhunter_SpawnLightningBurst(entity, 0, 0, 0);
            Headhunter_SpawnLightningBurst(entity, 0, 0, 1);
            Headhunter_SpawnLightningBurst(entity, 0, 0, 2);
            Headhunter_SpawnLightningBurst(entity, 0, 0, 3);
            Headhunter_SpawnAttackFollower(entity, 0x65);
            entity->subState++;
            entity->timer = 0;
            PlaySong(0x1D0);
        }
        break;
    case 12:
    case 14:
        if (entity->timer++ > 99)
        {
            entity->state = 0;
            entity->subState = 12;
            entity->velX.whole = 0;
            entity->velY.whole = 0;
            entity->accelX.whole = 0;
            entity->accelY.whole = 0;
            Anim_StartIndexed(entity, 6, 2, 1);
        }
        break;
    case 20:
        if (RandomNumberGenerator() % 100 < 4)
        {
            entity->subState++;
            Anim_StartIndexed(entity, 13, 2, 0);
        }
        else
        {
            entity->subState = 24;
            Anim_StartIndexed(entity, 13, 2, 0);
        }
        break;
    case 21:
        if (entity->animStep > 1)
        {
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 22:
        if (entity->timer > 19 && (entity->timer & 3) == 0)
        {
            x = entity->flipX ? 0x1D0000 : -0x1D0000;
            y = -0x120000;
            vx = entity->flipX ? 0x20000 : -0x20000;
            vx = vx + (-0x8000 + (RandomNumberGenerator() & 0xFFFF));
            vy = (RandomNumberGenerator() & 0xFFFF) - 0x8000;
            Headhunter_SpawnBurstShot(entity, x, y, vx, vy);
            PlaySong(0x67);
        }
        if (entity->timer++ > 45)
        {
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 23:
        if (entity->timer++ > 29)
        {
            entity->state = 0;
            entity->subState = 40;
            entity->timer = 0;
        }
        break;
    case 24:
        if (entity->animStep > 1)
        {
            entity->subState++;
            entity->timer = 0;
            x = entity->flipX ? 0x150000 : -0x150000;
            y = -0x120000;
            angle = entity->flipX ? 0x10000 : 0x18000;
            Headhunter_SpawnExtendingAttack(entity, x, y, angle);
            PlaySong(0x130);
            entity->subState++;
        }
        break;
    case 27:
        entity->state = 0;
        entity->subState = 40;
        entity->timer = 0;
        break;
    case 30:
        if (RandomNumberGenerator() % 100 < 50)
            entity->subState++;
        else
            entity->subState = 34;
        Anim_StartIndexed(entity, 15, 2, 0);
        entity->flipY = 0;
        break;
    case 31:
        if (entity->animStep > 1)
        {
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 32:
        if (entity->timer > 19 && (entity->timer & 3) == 0)
        {
            x = entity->flipX ? 0x100000 : -0x100000;
            y = 0x1C0000;
            {
                struct EwramData_EntityData *player = gEwramData->playerState.player;
                s32 px = player->posX.whole;
                s32 py;
                vx = px - (entity->posX.whole + x);
                py = player->posY.whole;
                vy = py - (entity->posY.whole + (y + 0x100000));
            }
            angle = ArcTan2(TO_PIXELS(vx), TO_PIXELS(vy));
            vx = Sin(angle + 0x4000);
            vy = Sin(angle);
            vx = FixedMultiply(vx, 0x28000);
            vy = FixedMultiply(vy, 0x28000);
            vx = vx + (-0x8000 + (RandomNumberGenerator() & 0xFFFF));
            vy = vy + (-0x8000 + (RandomNumberGenerator() & 0xFFFF));
            Headhunter_SpawnBurstShot(entity, x, y, vx, vy);
            PlaySong(0x67);
        }
        if (entity->timer++ > 45)
        {
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 33:
        if (entity->timer++ > 29)
        {
            entity->state = 0;
            entity->subState = 30;
            entity->timer = 0;
            entity->flipY = 1;
            Anim_StartIndexed(entity, 14, 2, 0);
        }
        break;
    case 34:
        if (entity->animStep > 1)
        {
            entity->subState++;
            entity->timer = 0;
            x = entity->flipX ? 0xF0000 : -0xF0000;
            y = 0x180000;
            {
                struct EwramData_EntityData *player = gEwramData->playerState.player;
                s32 px = player->posX.whole;
                s32 py;
                vx = px - (entity->posX.whole + x);
                py = player->posY.whole;
                vy = py - (entity->posY.whole + (y + 0x100000));
            }
            angle = ArcTan2(TO_PIXELS(vx), TO_PIXELS(vy));
            Headhunter_SpawnExtendingAttack(entity, x, y, angle);
            PlaySong(0x130);
            entity->subState++;
        }
        break;
    case 37:
        entity->state = 0;
        entity->subState = 30;
        entity->timer = 0;
        entity->flipY = 1;
        Anim_StartIndexed(entity, 14, 2, 0);
        break;
    case 40:
        if (RandomNumberGenerator() % 100 < 50)
            entity->subState++;
        else
            entity->subState = 44;
        Anim_StartIndexed(entity, 18, 2, 0);
        break;
    case 41:
        if (entity->animStep > 1)
        {
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 42:
        if (entity->timer > 19 && (entity->timer & 3) == 0)
        {
            x = entity->flipX ? -0x180000 : 0x180000;
            y = -0x200000;
            {
                struct EwramData_EntityData *player = gEwramData->playerState.player;
                s32 px = player->posX.whole;
                s32 py;
                vx = px - (entity->posX.whole + x);
                py = player->posY.whole;
                vy = py - (entity->posY.whole + (y + 0x100000));
            }
            angle = ArcTan2(TO_PIXELS(vx), TO_PIXELS(vy));
            vx = Sin(angle + 0x4000);
            vy = Sin(angle);
            vx = FixedMultiply(vx, 0x28000);
            vy = FixedMultiply(vy, 0x28000);
            vx = vx + (-0x8000 + (RandomNumberGenerator() & 0xFFFF));
            vy = vy + (-0x8000 + (RandomNumberGenerator() & 0xFFFF));
            Headhunter_SpawnBurstShot(entity, x, y, vx, vy);
            PlaySong(0x67);
        }
        if (entity->timer++ > 45)
        {
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 43:
        if (entity->timer++ > 29)
        {
            entity->state = 0;
            entity->subState = 50;
            entity->timer = 0;
        }
        break;
    case 44:
        if (entity->animStep > 1)
        {
            entity->subState++;
            entity->timer = 0;
            x = entity->flipX ? -0x180000 : 0x180000;
            y = -0x200000;
            {
                struct EwramData_EntityData *player = gEwramData->playerState.player;
                s32 px = player->posX.whole;
                s32 py;
                vx = px - (entity->posX.whole + x);
                py = player->posY.whole;
                vy = py - (entity->posY.whole + (y + 0x100000));
            }
            angle = ArcTan2(TO_PIXELS(vx), TO_PIXELS(vy));
            Headhunter_SpawnExtendingAttack(entity, x, y, angle);
            PlaySong(0x130);
            entity->subState++;
        }
        break;
    case 47:
        entity->state = 0;
        entity->subState = 50;
        entity->timer = 0;
        break;
    }
}

/* Defeat: face the player, shed the last part (kind 2, whose landing
 * advances the state), fall to the floor (y 0xC0), flash BG 2 for 90
 * frames, then set the boss flag, spawn the drop and the room event. */
/**
 * @brief Defeat state 2: shed the last part, fall to room Y 0xC0,
 * flash BG2, set the boss flag and create the soul/drop event.
 * Retain the deliberate case fallthrough and signed countdown.
 */
void Headhunter_UpdateDeath(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        entity->ext.headhunter.flags |= 2;
        entity->ext.headhunter.flags |= 1;
        entity->ext.headhunter.forcedFrame = 0x4C;
        if (entity->flipY)
        {
            entity->flipY = 0;
            entity->posY.whole += 0x180000;
        }
        Headhunter_SpawnFallingPart(entity, entity->flipX ? 0xE0000 : -0xE0000, -0x130000, 2);
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0x2000;
        entity->subState++;
        break;
    case 1:
        entity->ext.headhunter.flags |= 2;
        entity->ext.headhunter.flags |= 1;
        entity->ext.headhunter.forcedFrame = 0x4C;
        entity->timer = 0;
        EntityAccelerateAndMove(entity, 0x60000);
        if (entity->posY.whole + camera->yPos.whole > 0xBFFFFF)
        {
            entity->velX.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = 0;
            entity->accelY.whole = 0;
            entity->posY.whole = 0xC00000 - camera->yPos.whole;
        }
        break;
    case 2:
        Anim_StartIndexed(entity, 0x13, 2, 0);
        entity->timer = 90;
        entity->subState++;
        Headhunter_SpawnDefeatEffect(entity->posX.whole, entity->posY.whole);
    case 3:
        if (Entity_GetVisibilityPattern(entity->timer))
            gUnk_03002CB0.dispCnt &= ~0x400;
        else
            gUnk_03002CB0.dispCnt |= 0x400;
        if ((entity->timer & 7) == 0)
            PlaySong(0x76);
        if ((s8)entity->timer-- > 0)
            break;
        gEwramData->gameState.gameFlagsTail[1] |= 0x40000;
        Player_SetSelectedTarget(0);
        entity->deleteRequested = 1;
        gUnk_03002CB0.dispCnt &= ~0x400;
        entity->subState++;
        SoulPickup_SpawnFromEnemyDefinition((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer, 0x6A, 4);
        BossRecoveryPickup_SpawnWide((s16)entity->posX.part16.integer, 0x20, 4, 0x60, 1);
        break;
    case 4:
        break;
    }
}

/* Headhunter: graphics loader (83A40), damage callback (83AC4), slash arcs
 * (83C1C/83EEC) and the transformation parts (83FD0). */

/* form of the Headhunter: defeat in phases 1 and 2 starts a transformation,
 * the parts of HeadhunterTransformationPart_Update wait for phases 1-3 */

/* Load the graphics and the four palettes (unk_504..507, one per form). */
/**
 * @brief Load the graphics and four palette variants, attach the
 * initial sprite and set its draw callback. Return 0 on graphics
 * allocation failure or 1 after successful attachment.
 */
u8 Headhunter_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics = ObjGfx_LoadResource(sHeadhunterGfx, 0);
    if (graphics < 0) return 0;
    entity->ext.headhunter.palette0 = ObjPalette_FindOrLoadVariant(sHeadhunterPalette, 0);
    entity->ext.headhunter.palette1 = ObjPalette_FindOrLoadVariant(sHeadhunterPalette, 1);
    entity->ext.headhunter.palette2 = ObjPalette_FindOrLoadVariant(sHeadhunterPalette, 2);
    entity->ext.headhunter.palette3 = ObjPalette_FindOrLoadVariant(sHeadhunterPalette, 3);
    entity->ext.headhunter.graphics = graphics;
    entity->basePalette = entity->ext.headhunter.palette0;
    Sprite_Attach(entity, sHeadhunterSpriteSheet, graphics, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/* Damage: in phases 1 and 2 a defeat starts the transformation (phase 1
 * sheds its first part), otherwise the Headhunter dies. Survived hits
 * sometimes play a voice. */
/**
 * @brief Apply resolved damage and hit flash. HP exhaustion in phases
 * 1 and 2 resets the enemy template and starts the next transformation;
 * otherwise drop rewards and enter defeat state 2.
 */
void Headhunter_ApplyResolvedDamage(struct EwramData_EntityData *entity, s32 unused1, u16 *unused2, s32 damage)
{
    s32 zero;
    entity->hitFlashTimer = 8;
    entity->ext.headhunter.hitReact = 1;
    zero = 0;
    if ((s16)(entity->hp -= damage) <= 0)
    {
        if (PHASE(entity) == 1)
        {
            Enemy_ResetFromTemplate(entity);
            PlaySong(0x1C5);
            if (gEwramData->gameState.bossRushMode) entity->attack += 20;
            Anim_StartIndexed(entity, 0x19, 2, 0);
            Anim_Update(entity);
            Headhunter_SpawnFallingPart(entity, entity->flipX ? 0xE0000 : -0xE0000, -0x290000, 0);
            entity->state = zero;
            entity->subState = 6;
        }
        else if (PHASE(entity) == 2)
        {
            Enemy_ResetFromTemplate(entity);
            PlaySong(0x1D3);
            if (gEwramData->gameState.bossRushMode) entity->attack += 20;
            entity->state = zero;
            entity->subState = 20;
        }
        else
        {
            Enemy_DropRewards(entity);
            PlaySong(0x1C0);
            entity->palette = entity->basePalette;
            entity->timer = zero;
            entity->state = 2;
            entity->subState = zero;
        }
    }
    else
    {
        s32 phase = PHASE(entity);
        if (phase == 1)
        {
            if ((RandomNumberGenerator() & 0x7F) < 0x40)
            {
                PlaySong(0x1C4);
                entity->ext.headhunter.hitReact = phase;
            }
            else
                entity->ext.headhunter.hitReact = zero;
            entity->stage = 0;
        }
        else if (phase == 2)
        {
            if ((RandomNumberGenerator() & 0x7F) < 0x40)
                PlaySong(0x1D2);
        }
    }
}

#define ANGLE(e) ((e)->velX.whole)
#define ANGLE_SPEED(e) ((e)->velY.whole)
#define SCALE(e) ((e)->accelX.whole)

/* Create a swinging slash arc of kind 0-2 at the owner + (x, y) (mirrored
 * with the owner for kinds 1 and 2); big selects the larger sprite, dir 1
 * swings the other way. */
/**
 * @brief Create an affine arc for the transformation part and owner,
 * with kind-dependent mirroring, frame, swing and supplied timer.
 * Remove it if affine allocation fails; preserve allocation order.
 */
void Headhunter_SpawnTransformationArc(struct EwramData_EntityData *parent, struct EwramData_EntityData *owner, s32 x, s32 y,
                  s32 big, s32 timer, s32 kind, s32 dir)
{
    struct EwramData_EntityData *arc = EntityCreateInRange(0x49, 0xCF, HeadhunterTransformationArc_Update);
    s32 graphics;
    u16 slot;
    if (arc == NULL) return;
    arc->spriteLayer = owner->spriteLayer + 1;
    graphics = owner->ext.headhunter.graphics;
    arc->basePalette = owner->ext.headhunter.palette1;
    Sprite_Attach(arc, sHeadhunterSpriteSheet, graphics, arc->basePalette);
    if (big == 0) arc->animFrame = 0x60;
    else arc->animFrame = 0x5F;
    arc->animMode = 2;
    if (kind == 0)
    {
        arc->posX.whole = owner->posX.whole + x;
        arc->posY.whole = owner->posY.whole + y;
        if (dir == 1)
        {
            ANGLE(arc) = big == 0 ? -0x2000 : -0x2600;
            ANGLE_SPEED(arc) = -0x2AA;
            arc->flipX = 1;
        }
        else
        {
            ANGLE(arc) = big == 0 ? 0xA000 : 0xA500;
            ANGLE_SPEED(arc) = 0x2AA;
            arc->flipX = 0;
        }
    }
    else if (kind == 1)
    {
        if (owner->flipX) arc->posX.whole = owner->posX.whole - x;
        else arc->posX.whole = owner->posX.whole + x;
        arc->posY.whole = owner->posY.whole + y;
        if (owner->flipX)
        {
            if (dir == 1)
            {
                ANGLE(arc) = big == 0 ? 0xB000 : 0xB100;
                ANGLE_SPEED(arc) = 0x2AA;
                arc->flipX = 0;
            }
            else
            {
                ANGLE(arc) = big == 0 ? 0xF000 : 0xEB00;
                ANGLE_SPEED(arc) = -0x2AA;
                arc->flipX = 1;
            }
        }
        else
        {
            if (dir == 1)
            {
                ANGLE(arc) = big == 0 ? -0x3000 : -0x3100;
                ANGLE_SPEED(arc) = -0x2AA;
                arc->flipX = 1;
            }
            else
            {
                ANGLE(arc) = big == 0 ? 0x9000 : 0x9500;
                ANGLE_SPEED(arc) = 0x2AA;
                arc->flipX = 0;
            }
        }
    }
    else if (kind == 2)
    {
        if (owner->flipX) arc->posX.whole = owner->posX.whole - x;
        else arc->posX.whole = owner->posX.whole + x;
        arc->posY.whole = owner->posY.whole + y;
        if (owner->flipX)
        {
            if (dir == 1)
            {
                ANGLE(arc) = big == 0 ? 0xB000 : 0xB100;
                ANGLE_SPEED(arc) = 0x2AA;
                arc->flipX = 0;
            }
            else
            {
                ANGLE(arc) = big == 0 ? 0xF000 : 0xEB00;
                ANGLE_SPEED(arc) = -0x2AA;
                arc->flipX = 1;
            }
        }
        else
        {
            if (dir == 1)
            {
                ANGLE(arc) = big == 0 ? -0x3000 : -0x3100;
                ANGLE_SPEED(arc) = -0x2AA;
                arc->flipX = 1;
            }
            else
            {
                ANGLE(arc) = big == 0 ? 0x9000 : 0x9500;
                ANGLE_SPEED(arc) = 0x2AA;
                arc->flipX = 0;
            }
        }
    }
    if ((s32)((slot = ObjAffine_Alloc(1)) << 16) < 0)
    {
        EntityDelete(arc);
        return;
    }
    arc->affineIndex = slot;
    arc->affine = 1;
    ObjAffine_SetBiased(ANGLE(arc) - 0xC000, SCALE(arc), 0x10000, arc->flipX, arc->flipY, arc->affineIndex);
    arc->timer = timer;
    ENTITY_OWNER(arc) = parent;
    arc->ext.headhunterChild.owner = owner;
    KIND(arc) = kind;
    arc->scrollWithBg = 1;
    arc->drawFunc = Sprite_DrawEntity;
}

/* Slash arc: swing and grow for 12 frames, fall after 60 frames (kinds 1
 * and 2 also drift with the owner's facing), vanish after 90. */
/**
 * @brief Swing and grow during the first 12 updates, then drift after
 * timer 59 and free the affine slot after timer 89. The supplied
 * initial timer also contributes to the lifetime.
 */
void HeadhunterTransformationArc_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.headhunterChild.owner;
    if (entity->subState != 0) return;
    entity->timer++;
    if (entity->stage < 12)
    {
        entity->stage++;
        ANGLE(entity) += ANGLE_SPEED(entity);
        SCALE(entity) += 0x1555;
    }
    ObjAffine_SetBiased(ANGLE(entity) - 0xC000, SCALE(entity), SCALE(entity), entity->flipX, entity->flipY, entity->affineIndex);
    if (entity->timer > 59) entity->posY.whole += 0x4000;
    if (KIND(entity) == 1)
    {
        if (entity->timer > 59)
        {
            if (owner->flipX) entity->posX.whole += -0x2000;
            else entity->posX.whole += 0x2000;
        }
    }
    else if (KIND(entity) == 2)
    {
        if (entity->timer > 59)
        {
            if (owner->flipX) entity->posX.whole += -0x2000;
            else entity->posX.whole += 0x2000;
        }
    }
    if (entity->timer > 89)
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
}

#define COUNT(e) (*(s8 *)&e->ext.headhunterPart.count)
#define TARGET_X(e) (e->ext.headhunterPart.targetX)
#define TARGET_Y(e) (e->ext.headhunterPart.targetY)

/* Use palette 4 (TODO: flash?) while the owner does, else our own. */
#define SYNC_PALETTE(entity, owner)                                       \
    if ((owner)->palette == 4) (entity)->palette = 4;                \
    else (entity)->palette = (entity)->basePalette;

/* Transformation part of the Headhunter (entity type 0x6A, the owner):
 * once the owner reaches phase KIND + 1 this part jumps around (two
 * bounces), flies next to the owner, slashes four arcs (Headhunter_SpawnTransformationArc), then
 * either ends the phase-0 transformation or bursts into particles and
 * switches the owner to the next form (palette unk_505 / unk_506). */
/**
 * @brief Wait for the matching owner phase, move through two turning
 * passes and toward its attachment point, emit transformation arcs
 * and advance the owner to its next form or opening movement.
 */
void HeadhunterTransformationPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    s32 i;
    s32 unused[4]; /* only reserves 16 bytes of stack like the original */
    if (entity->state == 0)
    {
        if (owner != NULL && owner->updateFunc != NULL && owner->enemyId == 0x6A)
        {
            switch (entity->subState)
            {
            case 0:
                switch (KIND(entity))
                {
                case 0:
                    if (PHASE(owner) == 1)
                    {
                        entity->subState++;
                        COUNT(entity) = 0;
                        entity->timer = 0;
                        entity->flipX = owner->flipX;
                        entity->spriteLayer -= 2;
                    }
                    break;
                case 1:
                    if (PHASE(owner) == 2)
                    {
                        entity->subState++;
                        COUNT(entity) = 0;
                        entity->timer = 0;
                        entity->flipX = owner->flipX;
                        entity->spriteLayer -= 2;
                    }
                    break;
                case 2:
                    if (PHASE(owner) == 3)
                    {
                        entity->subState++;
                        COUNT(entity) = 0;
                        entity->timer = 0;
                        entity->flipX = owner->flipX;
                        entity->spriteLayer -= 2;
                    }
                    break;
                }
                break;
            case 1:
                entity->subState++;
            case 2:
                TARGET_X(entity) = owner->posX.whole + camera->xPos.whole
                    + (RandomModuloAtLeast((2 - COUNT(entity)) << 19, (2 - COUNT(entity)) << 18) - ((2 - COUNT(entity)) << 18));
                TARGET_Y(entity) = owner->posY.whole + camera->yPos.whole - ((2 - COUNT(entity)) * 0x14 + 0x28) * 0x10000;
                entity->subState++;
            case 3:
            {
                s32 velX;
                u8 changed;
                SYNC_PALETTE(entity, owner);
                Entity_AimAccelerationAtPoint(entity, TARGET_X(entity), TARGET_Y(entity), 0x2000);
                entity->velY.whole = entity->accelY.whole << 3;
                entity->accelY.whole = 0;
                velX = entity->velX.whole;
                EntityAccelerateAndMove(entity, 0x10000);
                changed = ((entity->velX.whole & 0x80000000) ^ (velX & 0x80000000)) != 0;
                if (changed)
                {
                    if (++COUNT(entity) <= 1)
                        entity->subState--;
                    else
                    {
                        entity->subState++;
                        entity->stage = 0;
                    }
                }
                break;
            }
            case 4:
                SYNC_PALETTE(entity, owner);
                if (entity->stage == 0)
                {
                    s32 x;
                    entity->stage++;
                    TARGET_X(entity) = x = owner->posX.whole + camera->xPos.whole;
                    switch (KIND(entity))
                    {
                    case 0:
                        TARGET_Y(entity) = owner->posY.whole + camera->yPos.whole - 0x340000;
                        if (owner->flipX) TARGET_X(entity) = x + 0x20000;
                        else TARGET_X(entity) = x - 0x20000;
                        break;
                    case 1:
                        TARGET_Y(entity) = owner->posY.whole + camera->yPos.whole - 0x2A0000;
                        if (owner->flipX) TARGET_X(entity) = x + 0x120000;
                        else TARGET_X(entity) = x - 0x120000;
                        break;
                    case 2:
                        TARGET_Y(entity) = owner->posY.whole + camera->yPos.whole - 0x280000;
                        if (owner->flipX) TARGET_X(entity) = x + 0x100000;
                        else TARGET_X(entity) = x - 0x100000;
                        break;
                    }
                }
                if (Entity_AimVelocityAtPointByDistance(entity, TARGET_X(entity), TARGET_Y(entity), 0x8000))
                {
                    entity->subState++;
                    entity->timer = 0;
                    return;
                }
                EntityAccelerateAndMove(entity, 0x10000);
                break;
            case 5:
            {
                SYNC_PALETTE(entity, owner);
                entity->timer++;
                switch (KIND(entity))
                {
                case 0:
                    if (entity->timer == 2) Headhunter_SpawnTransformationArc(entity, owner, -0x50000, -0x200000, 0, entity->timer, 0, 0);
                    else if (entity->timer == 5) Headhunter_SpawnTransformationArc(entity, owner, 0x30000, -0x220000, 0, entity->timer, 0, 1);
                    else if (entity->timer == 20) Headhunter_SpawnTransformationArc(entity, owner, -0x90000, -0x1C0000, 1, entity->timer, 0, 0);
                    else if (entity->timer == 28) Headhunter_SpawnTransformationArc(entity, owner, 0x70000, -0x1B0000, 1, entity->timer, 0, 1);
                    else if (entity->timer == 90) entity->subState++;
                    if (entity->timer > 59) entity->posY.whole += 0x4000;
                    break;
                case 1:
                    if (entity->timer == 2) Headhunter_SpawnTransformationArc(entity, owner, -0xD0000, -0x1C0000, 0, entity->timer, 1, 0);
                    else if (entity->timer == 5) Headhunter_SpawnTransformationArc(entity, owner, -0xA0000, -0x1E0000, 0, entity->timer, 1, 1);
                    else if (entity->timer == 20) Headhunter_SpawnTransformationArc(entity, owner, -0xF0000, -0x160000, 1, entity->timer, 1, 0);
                    else if (entity->timer == 28) Headhunter_SpawnTransformationArc(entity, owner, 0, -0x180000, 1, entity->timer, 1, 1);
                    else if (entity->timer == 90) entity->subState++;
                    if (entity->timer > 59)
                    {
                        entity->posY.whole += 0x4000;
                        if (entity->flipX) entity->posX.whole += -0x2000;
                        else entity->posX.whole += 0x2000;
                    }
                    break;
                case 2:
                    if (entity->timer == 2) Headhunter_SpawnTransformationArc(entity, owner, -0xB0000, -0x140000, 0, entity->timer, 2, 0);
                    else if (entity->timer == 5) Headhunter_SpawnTransformationArc(entity, owner, -0x80000, -0x160000, 0, entity->timer, 2, 1);
                    else if (entity->timer == 20) Headhunter_SpawnTransformationArc(entity, owner, -0xF0000, -0x100000, 1, entity->timer, 2, 0);
                    else if (entity->timer == 28) Headhunter_SpawnTransformationArc(entity, owner, 0x10000, -0x110000, 1, entity->timer, 2, 1);
                    else if (entity->timer == 90) entity->subState++;
                    if (entity->timer > 59)
                    {
                        entity->posY.whole += 0x4000;
                        if (entity->flipX) entity->posX.whole += -0x2000;
                        else entity->posX.whole += 0x2000;
                    }
                    break;
                }
                break;
            }
            case 6:
                SYNC_PALETTE(entity, owner);
                PlaySong(0x11D);
                switch (KIND(entity))
                {
                case 0:
                    owner->subState++;
                    Anim_StartIndexed(owner, 1, 2, 1);
                    Anim_Update(owner);
                    EntityDelete(entity);
                    return;
                case 1:
                    for (i = 0; i < 10; i++)
                        Effect_SpawnRisingParticleInBox(owner->posX.whole, owner->posY.whole - 0x100000, 0x200000, 0x200000, owner->spriteLayer - 1, 1);
                    entity->timer = 5;
                    entity->subState++;
                    break;
                case 2:
                    for (i = 0; i < 10; i++)
                        Effect_SpawnRisingParticleInBox(owner->posX.whole, owner->posY.whole - 0x80000, 0x200000, 0x200000, owner->spriteLayer - 1, 1);
                    entity->timer = 5;
                    entity->subState++;
                    break;
                }
                break;
            case 7:
                SYNC_PALETTE(entity, owner);
                switch (KIND(entity))
                {
                case 0:
                    break;
                case 1:
                    if ((s8)--entity->timer > 0) break;
                    owner->basePalette = owner->ext.headhunter.palette1;
                    owner->palette = owner->basePalette;
                    Anim_StartIndexed(owner, 5, 2, 0);
                    Anim_Update(owner);
                    owner->subState++;
                    owner->posY.whole += -0x130000;
                    EntityDelete(entity);
                    return;
                case 2:
                    if ((s8)--entity->timer > 0) break;
                    owner->basePalette = owner->ext.headhunter.palette2;
                    owner->palette = owner->basePalette;
                    Anim_StartIndexed(owner, 9, 2, 1);
                    Anim_Update(owner);
                    owner->subState++;
                    owner->posY.whole += 0xB0000;
                    EntityDelete(entity);
                    return;
                }
                break;
            }
            Anim_Update(entity);
            return;
        }
        EntityDelete(entity);
    }
    else
        entity->deleteRequested = 1;
}

/* Headhunter transformation effects: falling parts (84760/8489C) and two
 * affine bursts in four mirrored corners that launch the rolling waves
 * (84B20/84C7C) or the lightning (84DB8/84F14). */

/* bit 0 of the owner's unk_4FE ends its effects (TODO: meaning) */
#define OWNER_DONE(o) (o->ext.headhunter.flags & 1)
#undef FRAME
#define FRAME(e) (e->ext.headhunterEffect.frame)
#undef KIND
#define KIND(e) (*(s8 *)&e->ext.headhunterChild.kind) /* also the burst corner 0-3 */
#define CORNER(e) KIND(e)
#define OFFSET_X(e) ((e)->velX.whole)
#define OFFSET_Y(e) ((e)->velY.whole)
#define SCALE_X(e) ((e)->velX.whole)
#define SCALE_Y(e) ((e)->velY.whole)
#define ROTATION(e) ((e)->accelX.whole)

/* Create a falling part of kind 0-2 at the owner + (x, y). */
/**
 * @brief Create an affine falling part of kind 0..2 at an owner-relative
 * offset with its form palette and frame. Preserve the short-cast
 * affine failure check and the kind-dependent starting countdown.
 */
void Headhunter_SpawnFallingPart(struct EwramData_EntityData *owner, s32 x, s32 y, s32 kind)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, HeadhunterFallingPart_Update);
    s32 graphics;
    u16 slot;
    if (part == NULL) return;
    OFFSET_X(part) = x;
    OFFSET_Y(part) = y;
    part->flipX = owner->flipX;
    part->spriteLayer = owner->spriteLayer - 1;
    graphics = owner->ext.headhunter.graphics;
    switch (kind)
    {
    case 0:
        part->basePalette = owner->ext.headhunter.palette0;
        FRAME(part) = 1;
        part->timer = 8;
        break;
    case 1:
        part->basePalette = owner->ext.headhunter.palette1;
        FRAME(part) = 0x6C;
        part->timer = 8;
        break;
    case 2:
        part->basePalette = owner->ext.headhunter.palette2;
        FRAME(part) = 0x4F;
        part->timer = 60;
        break;
    }
    Sprite_Attach(part, sHeadhunterSpriteSheet, graphics, part->basePalette);
    if ((s32)((slot = ObjAffine_Alloc(1)) << 16) < 0)
    {
        EntityDelete(part);
        return;
    }
    part->affineIndex = slot;
    part->affine = 1;
    ObjAffine_SetBiased(0, 0x10000, 0x10000, part->flipX, part->flipY, part->affineIndex);
    ENTITY_OWNER(part) = owner;
    KIND(part) = kind;
    part->scrollWithBg = 0;
    part->drawFunc = Sprite_DrawEntity;
}

/* Falling part: stick to the owner shedding sparks, then fall, bounce on
 * the floor (y 0xB7) inside the room x range and blink out. */
/**
 * @brief Follow the owner while emitting sparks, then fall, rotate and
 * blink within the room bounds. Kind 2 advances the owner on release;
 * other kinds do so on expiry. Free the affine slot on removal.
 */
void HeadhunterFallingPart_Update(struct EwramData_EntityData *entity)
{
    struct SingleFrameAnimation animation = sHeadhunterFrameAnim;
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    Anim_Start(entity, (struct AnimScript *)&animation, 2, 1);
    animation.frame = FRAME(entity);
    Anim_Update(entity);
    if (entity->subState == 0)
    {
        entity->posX.whole = owner->posX.whole + OFFSET_X(entity);
        entity->posY.whole = owner->posY.whole + OFFSET_Y(entity);
        if ((entity->timer & 7) == 0)
        {
            s32 x = entity->posX.whole + (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
            s32 y = entity->posY.whole + (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
            s32 vx = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
            s32 vy = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
            Effect_SpawnParticleAnim13(x, y, vx, vy, entity->spriteLayer - 1, 2);
        }
        ObjAffine_SetBiased(0, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
        if ((s8)entity->timer-- <= 0)
        {
            entity->subState++;
            entity->timer = 60;
            entity->velX.whole = entity->flipX ? 0x10000 : -0x10000;
            ROTATION(entity) = 0;
            entity->velY.whole = 0;
            entity->accelY.whole = 0x1800;
            entity->scrollWithBg = 1;
            if (KIND(entity) == 2) owner->subState++;
        }
    }
    else
    {
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        EntityMoveX(entity);
        EntityAccelerateAndMoveY(entity, 0x40000);
        if (entity->posY.whole + camera->yPos.whole > 0xB70000)
        {
            entity->posY.whole = 0xB70000 - camera->yPos.whole;
            entity->velY.whole = 0x10000;
            EntityDampVelocityX(entity, 8);
        }
        if (entity->posX.whole + camera->xPos.whole < 0x670000)
            entity->posX.whole = 0x670000 - camera->xPos.whole;
        if (entity->posX.whole + camera->xPos.whole > 0x1980000)
            entity->posX.whole = 0x1980000 - camera->xPos.whole;
        if ((entity->velX.whole < 0 ? -entity->velX.whole : entity->velX.whole) > 0x2000)
        {
            if (entity->flipX) ROTATION(entity) += 0x400;
            else ROTATION(entity) -= 0x400;
        }
        ObjAffine_SetBiased(ROTATION(entity), 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
        if ((entity->timer & 7) == 0)
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x10000, 0x80000, 0x40000, entity->spriteLayer - 1);
        if ((s8)entity->timer-- <= 0)
        {
            if (KIND(entity) != 2) owner->subState++;
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
        }
    }
}

/* Create a burst of the wave attack in one of four mirrored corners. */
/**
 * @brief Create one mirrored corner of the affine wave-attack burst,
 * with countdown 0x6C, owner graphics and stored contact strength.
 * Remove the entity if affine allocation fails.
 */
void Headhunter_SpawnWaveBurst(struct EwramData_EntityData *owner, s32 x, s32 y, s32 corner)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, HeadhunterWaveBurst_Update);
    s32 graphics;
    u16 slot;
    if (effect == NULL) return;
    effect->posX.whole = owner->posX.whole + x;
    effect->posY.whole = owner->posY.whole + y;
    switch (corner)
    {
    case 0:
        effect->flipX = 0;
        effect->flipY = 0;
        break;
    case 1:
        effect->flipX = 1;
        effect->flipY = 0;
        break;
    case 2:
        effect->flipX = 0;
        effect->flipY = 1;
        break;
    case 3:
        effect->flipX = 1;
        effect->flipY = 1;
        break;
    }
    CORNER(effect) = corner;
    effect->spriteLayer = owner->spriteLayer - 1;
    graphics = owner->ext.headhunter.graphics;
    effect->ext.headhunterEffect.graphics = graphics;
    effect->basePalette = owner->ext.headhunter.palette3;
    Sprite_Attach(effect, sHeadhunterSpriteSheet, graphics, effect->basePalette);
    FRAME(effect) = 0x59;
    if ((s32)((slot = ObjAffine_Alloc(1)) << 16) < 0)
    {
        EntityDelete(effect);
        return;
    }
    effect->affineIndex = slot;
    effect->affine = 1;
    effect->objMode = 1;
    SCALE_X(effect) = 0x10000;
    SCALE_Y(effect) = 0;
    ObjAffine_SetBiased(0, 0x10000, 0, effect->flipX, effect->flipY, effect->affineIndex);
    effect->timer = 0x6C;
    effect->attack = owner->attack;
    ENTITY_OWNER(effect) = owner;
    effect->scrollWithBg = 1;
    effect->drawFunc = Sprite_DrawEntity;
}

/* Wave burst: corner 0 launches two rolling waves at 58 frames and eight
 * falling particles at 70. */
/**
 * @brief Grow and shrink the affine burst. Corner 0 creates two moving
 * waves at countdown 58 and eight orbiting particles at 70. Free
 * the affine slot on timeout or owner effect-cancel bit 0.
 */
void HeadhunterWaveBurst_Update(struct EwramData_EntityData *entity)
{
    struct SingleFrameAnimation animation = sHeadhunterFrameAnim;
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    s32 i;
    if (entity->timer > 67)
    {
        if (SCALE_Y(entity) <= 0xFFFF) SCALE_Y(entity) += 0xC00;
    }
    else if (entity->timer == 58)
    {
        if (CORNER(entity) == 0)
        {
            Headhunter_SpawnWave(owner, 0, -0x140000, 0);
            Headhunter_SpawnWave(owner, 0, 0x100000, 1);
            PlaySong(0x7A);
        }
    }
    else if (entity->timer <= 24)
    {
        if (SCALE_Y(entity) > 0xBFF) SCALE_Y(entity) -= 0xC00;
    }
    if (CORNER(entity) == 0 && entity->timer == 70)
    {
        for (i = 0; i < 8; i++)
            Headhunter_SpawnOrbitingParticle(entity, owner, 0, -0x20000, i << 13);
    }
    ObjAffine_SetBiased(0, SCALE_X(entity), SCALE_Y(entity), entity->flipX, entity->flipY, entity->affineIndex);
    Anim_Start(entity, (struct AnimScript *)&animation, 2, 1);
    animation.frame = FRAME(entity);
    Anim_Update(entity);
    if ((s8)entity->timer-- <= 0)
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
    else if (OWNER_DONE(owner))
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
}

/* Create a burst of the lightning attack in one of four mirrored corners. */
/**
 * @brief Create one mirrored corner of the affine lightning-attack
 * burst with initially zero scale and rotation. Preserve the comma
 * expression assigning the common zero and the corner.
 */
void Headhunter_SpawnLightningBurst(struct EwramData_EntityData *owner, s32 x, s32 y, s32 corner)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, HeadhunterLightningBurst_Update);
    s32 graphics;
    s32 zero;
    u16 slot;
    if (effect == NULL) return;
    effect->posX.whole = owner->posX.whole + x;
    effect->posY.whole = owner->posY.whole + y;
    switch (corner)
    {
    case 0:
        effect->flipX = 0;
        effect->flipY = 0;
        break;
    case 1:
        effect->flipX = 1;
        effect->flipY = 0;
        break;
    case 2:
        effect->flipX = 0;
        effect->flipY = 1;
        break;
    case 3:
        effect->flipX = 1;
        effect->flipY = 1;
        break;
    }
    /* the zero for the transform fields is set together with the corner */
    CORNER(effect) = (zero = 0, corner);
    effect->spriteLayer = owner->spriteLayer - 1;
    graphics = owner->ext.headhunter.graphics;
    effect->ext.headhunterEffect.graphics = graphics;
    effect->basePalette = owner->ext.headhunter.palette3;
    Sprite_Attach(effect, sHeadhunterSpriteSheet, graphics, effect->basePalette);
    FRAME(effect) = 0x59;
    if ((s32)((slot = ObjAffine_Alloc(1)) << 16) < 0)
    {
        EntityDelete(effect);
        return;
    }
    effect->affineIndex = slot;
    effect->affine = 1;
    effect->objMode = 1;
    SCALE_X(effect) = zero;
    SCALE_Y(effect) = zero;
    ROTATION(effect) = zero;
    ObjAffine_SetBiased(0, 0, 0, effect->flipX, effect->flipY, effect->affineIndex);
    effect->attack = owner->attack;
    ENTITY_OWNER(effect) = owner;
    effect->scrollWithBg = 1;
    effect->drawFunc = Sprite_DrawEntity;
}

/* Lightning burst: grow, corner 0 spawns the effect at 42 and three bolts
 * at 58 frames, then shrink. */
/**
 * @brief Grow and rotate the burst; corner 0 creates its flicker at
 * timer 42 and three bolts at 58. Shrink and free the affine slot
 * on signed timer expiry or owner effect-cancel bit 0.
 */
void HeadhunterLightningBurst_Update(struct EwramData_EntityData *entity)
{
    struct SingleFrameAnimation animation = sHeadhunterFrameAnim;
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    if (entity->timer++ <= 32)
    {
        if (SCALE_X(entity) <= 0xFFFF) SCALE_X(entity) += 0x800;
        if (SCALE_Y(entity) <= 0xFFFF) SCALE_Y(entity) += 0x800;
        if (ROTATION(entity) <= 0xFFFF) ROTATION(entity) += 0x800;
    }
    else if (entity->timer == 42)
    {
        if (CORNER(entity) == 0) Headhunter_SpawnLightningBurstFlicker(owner, 0, 0);
    }
    else if (entity->timer == 58)
    {
        if (CORNER(entity) == 0)
        {
            s32 x, y;
            x = entity->posX.whole;
            y = 0x7A0000 - camera->yPos.whole;
            Headhunter_SpawnLightningBolt(owner, x, y);
            x = entity->posX.whole - 0x400000;
            y = 0x800000 - camera->yPos.whole;
            Headhunter_SpawnLightningBolt(owner, x, y);
            x = entity->posX.whole + 0x400000;
            y = 0x800000 - camera->yPos.whole;
            Headhunter_SpawnLightningBolt(owner, x, y);
        }
    }
    else if (entity->timer > 95)
    {
        if (SCALE_X(entity) > 0x9FF) SCALE_X(entity) -= 0xA00;
        if (SCALE_Y(entity) > 0x9FF) SCALE_Y(entity) -= 0xA00;
        ROTATION(entity) += 0x800;
    }
    ObjAffine_SetBiased(ROTATION(entity), SCALE_X(entity), SCALE_Y(entity), entity->flipX, entity->flipY, entity->affineIndex);
    Anim_Start(entity, (struct AnimScript *)&animation, 2, 1);
    animation.frame = FRAME(entity);
    Anim_Update(entity);
    if ((s8)entity->timer < 0)
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
    else if (OWNER_DONE(owner))
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
}

/* Headhunter attacks: rolling wave (850A0-8524C), lightning bolt with its
 * impact (8538C-855C8) and the flicker effect (85694). */

/* bit 0 of the owner's unk_4FE ends its effects (TODO: meaning) */
#define ACCEL(e) ((e)->accelX.whole)

/* Create the effect HeadhunterLightningBurstFlicker_Update at the owner + (x, y). */
/**
 * @brief Create the lightning burst flicker at an owner-relative offset
 * using frame base 0x23 and the fourth palette. Preserve the factory
 * without an affine allocation despite the update cleanup branch.
 */
void Headhunter_SpawnLightningBurstFlicker(struct EwramData_EntityData *owner, s32 x, s32 y)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, HeadhunterLightningBurstFlicker_Update);
    if (effect != NULL)
    {
        s32 graphics;
        effect->posX.whole = owner->posX.whole + x;
        effect->posY.whole = owner->posY.whole + y;
        effect->spriteLayer = owner->spriteLayer - 2;
        graphics = owner->ext.headhunter.graphics;
        effect->ext.headhunterEffect.graphics = graphics;
        effect->basePalette = owner->ext.headhunter.palette3;
        Sprite_Attach(effect, sHeadhunterSpriteSheet, graphics, effect->basePalette);
        FRAME(effect) = 0x23;
        effect->attack = owner->attack;
        ENTITY_OWNER(effect) = owner;
        effect->scrollWithBg = 1;
        effect->drawFunc = Sprite_DrawEntity;
    }
}

/* Create the rolling wave moving left (direction 0) or right (1), falling
 * towards y = 0xB0 in the room. */
/**
 * @brief Create a left/right moving wave with inward horizontal
 * acceleration and vertical velocity toward room Y 0xB0 over 128
 * updates. Install contact damage at 125 percent of owner strength.
 */
void Headhunter_SpawnWave(struct EwramData_EntityData *owner, s32 x, s32 y, s32 direction)
{
    struct EwramData_EntityData *wave = EntityCreateInRange(0x49, 0xCF, HeadhunterWave_Update);
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    struct SpriteBox box = {-4, -4, 8, 8};
    if (wave != NULL)
    {
        s32 graphics;
        wave->posX.whole = owner->posX.whole + x;
        wave->posY.whole = owner->posY.whole + y;
        switch (direction)
        {
        case 0:
            wave->flipX = 1;
            wave->velX.whole = -0x30000;
            ACCEL(wave) = 0x2000;
            break;
        case 1:
            wave->flipX = 0;
            wave->velX.whole = 0x30000;
            ACCEL(wave) = -0x2000;
            break;
        }
        wave->velY.whole = (0xB00000 - (wave->posY.whole + camera->yPos.whole)) / 128;
        wave->spriteLayer = owner->spriteLayer - 1;
        graphics = owner->ext.headhunter.graphics;
        wave->ext.headhunterEffect.graphics = graphics;
        wave->basePalette = owner->ext.headhunter.palette3;
        Sprite_Attach(wave, sHeadhunterSpriteSheet, graphics, wave->basePalette);
        FRAME(wave) = 0x26;
        Hit_Enable(wave, 8, HeadhunterWave_DamagePlayerOnContact, 0);
        HitBox_SetAttack(wave, &box);
        wave->hit.flags |= 4;
        wave->hit.flags &= ~2;
        wave->attack = owner->attack + (owner->attack >> 2);
        ENTITY_OWNER(wave) = owner;
        wave->scrollWithBg = 1;
        wave->drawFunc = Sprite_DrawEntity;
    }
}

/* Rolling wave: shed particles every 4 frames, reverse acceleration at 65,
 * grow, then fade out; despawn after 172 frames. */
/**
 * @brief Move the wave with frame growth and particles, reverse horizontal
 * acceleration at 65 and disable contact after 147. Preserve the
 * double horizontal damping; delete after 171 or owner cancel.
 */
void HeadhunterWave_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    struct HitResult result;
    struct SingleFrameAnimation animation = sHeadhunterFrameAnim;
    if ((entity->timer & 3) == 0)
    {
        s32 x = entity->posX.whole + (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
        s32 y = entity->posY.whole + (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
        Effect_SpawnParticleAnim0(x, y, entity->velX.whole / 2, 0, entity->spriteLayer + 1);
    }
    if (++entity->timer == 65) ACCEL(entity) = -ACCEL(entity);
    if (entity->timer == 100) FRAME(entity)++;
    if (entity->timer == 106) FRAME(entity)++;
    if (entity->timer == 148) FRAME(entity) = 0x26;
    if (entity->timer == 154) FRAME(entity) = 0x28;
    if (entity->timer == 160) FRAME(entity) = 0x2A;
    if (entity->timer == 166) FRAME(entity) = 0x32;
    if (entity->timer > 147)
    {
        /* the original damps the horizontal velocity twice */
        EntityDampVelocityX(entity, 0x10);
        EntityDampVelocityX(entity, 0x10);
    }
    if (entity->timer <= 36 && (entity->timer & 3) == 0 && (s8)FRAME(entity) <= 47)
        FRAME(entity)++;
    EntityAccelerateAndMove(entity, 0x40000);
    Anim_Start(entity, (struct AnimScript *)&animation, 2, 1);
    animation.frame = FRAME(entity);
    Anim_Update(entity);
    if (entity->timer > 171)
        EntityDelete(entity);
    else
    {
        if (entity->timer <= 147) Hit_CheckWithPlayer(&result, entity);
        if (OWNER_DONE(owner)) EntityDelete(entity);
    }
}

/* Create a lightning bolt at (x, y), kept inside the room x range
 * 0x68..0x197. */
/**
 * @brief Create a bolt at the supplied position, clamping room X to
 * 0x68..0x197. Copy owner graphics, damage and palette variant 3.
 */
void Headhunter_SpawnLightningBolt(struct EwramData_EntityData *owner, s32 x, s32 y)
{
    struct EwramData_EntityData *bolt = EntityCreateInRange(0x49, 0xCF, HeadhunterLightningBolt_Update);
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    if (bolt != NULL)
    {
        s32 graphics;
        bolt->posX.whole = x;
        bolt->posY.whole = y;
        if (x + camera->xPos.whole < 0x680000)
            bolt->posX.whole = 0x680000 - camera->xPos.whole;
        if (bolt->posX.whole + camera->xPos.whole > 0x1970000)
            bolt->posX.whole = 0x1970000 - camera->xPos.whole;
        bolt->flipX = RandomNumberGenerator() % 2;
        bolt->spriteLayer = owner->spriteLayer - 3;
        graphics = owner->ext.headhunter.graphics;
        bolt->ext.headhunterEffect.graphics = graphics;
        bolt->basePalette = owner->ext.headhunter.palette3;
        Sprite_Attach(bolt, sHeadhunterSpriteSheet, graphics, bolt->basePalette);
        bolt->ext.headhunterEffect.frame = 0x1A;
        bolt->objMode = 1;
        bolt->attack = owner->attack;
        ENTITY_OWNER(bolt) = owner;
        bolt->scrollWithBg = 1;
        bolt->drawFunc = Sprite_DrawEntity;
    }
}

/* Lightning bolt: at 20 frames create the impact on the ground (y 0xBF). */
/**
 * @brief Alternate bolt frames and create a ground impact at timer 20.
 * Delete on signed timer expiry or owner effect-cancel bit 0.
 */
void HeadhunterLightningBolt_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    struct SingleFrameAnimation animation = sHeadhunterFrameAnim;
    if (entity->timer > 8) entity->objMode = 0;
    if (entity->timer == 20)
    {
        Headhunter_SpawnLightningImpact(entity, owner, entity->posX.whole, 0xBF0000 - camera->yPos.whole);
        PlaySong(0x75);
    }
    Anim_Start(entity, (struct AnimScript *)&animation, 2, 1);
    animation.frame = RandomNumberGenerator() % 2 + entity->ext.headhunterEffect.frame;
    Anim_Update(entity);
    if ((s8)entity->timer++ > 34)
        EntityDelete(entity);
    else if (OWNER_DONE(owner))
        EntityDelete(entity);
}

/* Create the bolt impact (hit box -2, -64, 4x64). */
/**
 * @brief Create the lightning ground impact at the supplied position,
 * with a 4-by-64 attack box, contact callback and 125 percent of
 * the bolt strength; retain both bolt and boss links.
 */
void Headhunter_SpawnLightningImpact(struct EwramData_EntityData *owner, struct EwramData_EntityData *source, s32 x, s32 y)
{
    struct EwramData_EntityData *impact = EntityCreateInRange(0x49, 0xCF, HeadhunterLightningImpact_Update);
    struct SpriteBox box = {-2, -64, 4, 64};
    if (impact != NULL)
    {
        s32 graphics;
        impact->posX.whole = x;
        impact->posY.whole = y;
        impact->spriteLayer = owner->spriteLayer + 1;
        graphics = owner->ext.headhunter.graphics;
        impact->ext.headhunterEffect.graphics = graphics;
        impact->basePalette = owner->basePalette;
        Sprite_Attach(impact, sHeadhunterSpriteSheet, graphics, impact->basePalette);
        impact->ext.headhunterEffect.frame = 0x1C;
        Hit_Enable(impact, 8, HeadhunterLightningImpact_DamagePlayerOnContact, 0);
        HitBox_SetAttack(impact, &box);
        impact->hit.flags |= 2;
        impact->hit.flags |= 4;
        impact->attack = owner->attack + (owner->attack >> 2);
        ENTITY_OWNER(impact) = owner;
        impact->ext.headhunterEffect.source = source;
        impact->scrollWithBg = 1;
        impact->drawFunc = Sprite_DrawEntity;
    }
}

/* Bolt impact: animate, spawn the flicker at 9 frames, vanish after 16. */
/**
 * @brief Change impact frames, adjust hit flags at timer 8 and create
 * the short flicker at 9. Check contact until removal after 15
 * or the boss effect-cancel bit; preserve the redundant limit check.
 */
void HeadhunterLightningImpact_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.headhunterEffect.source;
    struct HitResult result;
    struct SingleFrameAnimation animation = sHeadhunterFrameAnim;
    if (++entity->timer == 5)
    {
        FRAME(entity)++;
        PlaySong(0x131);
    }
    if (entity->timer == 8)
    {
        FRAME(entity)++;
        entity->hit.flags &= ~2;
    }
    if (entity->timer == 9)
        Headhunter_SpawnLightningImpactFlicker(entity, owner, entity->posX.whole, entity->posY.whole);
    if (entity->timer > 7 && RandomNumberGenerator() % 2 == 0)
    {
        if ((s8)FRAME(entity) == 30) FRAME(entity) = 31;
        else FRAME(entity) = 30;
    }
    Anim_Start(entity, (struct AnimScript *)&animation, 2, 1);
    animation.frame = FRAME(entity);
    Anim_Update(entity);
    if (entity->timer > 15)
        EntityDelete(entity);
    else
    {
        if (entity->timer <= 32) Hit_CheckWithPlayer(&result, entity);
        if (OWNER_DONE(owner)) EntityDelete(entity);
    }
}

/* Create the flicker effect HeadhunterLightningImpactFlicker_Update at an absolute position. */
/**
 * @brief Create the short impact flicker at the supplied position,
 * copying impact graphics and retaining its boss link for cancel.
 */
void Headhunter_SpawnLightningImpactFlicker(struct EwramData_EntityData *owner, struct EwramData_EntityData *source, s32 x, s32 y)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, HeadhunterLightningImpactFlicker_Update);
    if (effect != NULL)
    {
        s32 graphics;
        effect->posX.whole = x;
        effect->posY.whole = y;
        effect->flipX = RandomNumberGenerator() % 2;
        effect->spriteLayer = owner->spriteLayer - 1;
        graphics = owner->ext.headhunter.graphics;
        effect->ext.headhunterEffect.graphics = graphics;
        effect->basePalette = owner->basePalette;
        Sprite_Attach(effect, sHeadhunterSpriteSheet, graphics, effect->basePalette);
        effect->ext.headhunterEffect.frame = 0x20;
        effect->attack = owner->attack;
        ENTITY_OWNER(effect) = owner;
        effect->ext.headhunterEffect.source = source;
        effect->scrollWithBg = 1;
        effect->drawFunc = Sprite_DrawEntity;
    }
}

/* Effect circling its anchor (unk_4FC/unk_500, camera-relative) with a growing
 * radius; deleted when flagged or when its source (unk_4F4) is done. */
/**
 * @brief Orbit the stored anchor with increasing angular speed, expand
 * the radius after timer 25 and compensate for camera movement.
 * Advance animation every other update; remove on completion/cancel.
 */
void HeadhunterOrbitingParticle_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    struct EwramData_EntityData *source = entity->ext.headhunterEffect.source;
    s32 x, y, radius;
    if (entity->accelY.whole <= 0x3FF)
        entity->accelY.whole += 0x10;
    entity->accelX.whole += entity->accelY.whole;
    radius = 0x4000;
    x = Sin(entity->accelX.whole + radius);
    y = Sin(entity->accelX.whole);
    if (entity->timer <= 25)
    {
        radius = 0x200000;
        x = FixedMultiply(x, radius);
        y = FixedMultiply(y, radius);
    }
    else
    {
        s32 offset = (entity->timer - 26) << 16;
        s32 base = 0x200000;
        x = FixedMultiply(x, offset + base);
        y = FixedMultiply(y, ((entity->timer - 26) << 16) + base);
    }
    entity->posX.whole = entity->velX.whole + x;
    entity->posY.whole = entity->velY.whole + y;
    entity->posX.whole += entity->ext.headhunterOrbit.cameraX - camera->xPos.whole;
    entity->posY.whole += entity->ext.headhunterOrbit.cameraY - camera->yPos.whole;
    if (entity->timer++ & 1)
        Anim_Update(entity);
    if (entity->animDone)
        EntityDelete(entity);
    else if (OWNER_DONE(source))
        EntityDelete(entity);
}

/* Headhunter attack follower, burst shots and segmented extending attack.
 * The affine tip extends and retracts along an angle with ten linked
 * segments. */

#define BASE_X(e) (e->ext.headhunterWhip.baseX) /* room coordinates */
#define BASE_Y(e) (e->ext.headhunterWhip.baseY)
#undef OFFSET_X
#define OFFSET_X(e) (e->ext.headhunterWhip.offsetX)
#undef OFFSET_Y
#define OFFSET_Y(e) (e->ext.headhunterWhip.offsetY)
#define SPEED(e) ((e)->accelX.whole)
#define SEGMENT_INDEX(e) (*(s8 *)&e->ext.headhunterWhipSegment.index)

/* Create the follower effect HeadhunterAttackFollower_Update for `timer` frames. */
/**
 * @brief Create the animation-0x16 effect that follows the owner for
 * the supplied signed countdown, using palette variant 3.
 */
void Headhunter_SpawnAttackFollower(struct EwramData_EntityData *owner, s32 timer)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, HeadhunterAttackFollower_Update);
    if (effect != NULL)
    {
        s32 graphics;
        effect->flipX = owner->flipX;
        effect->spriteLayer = owner->spriteLayer;
        graphics = owner->ext.headhunter.graphics;
        effect->basePalette = owner->ext.headhunter.palette3;
        Sprite_Attach(effect, sHeadhunterSpriteSheet, graphics, effect->basePalette);
        Anim_StartIndexed(effect, 0x16, 2, 0);
        Anim_Update(effect);
        effect->timer = timer;
        ENTITY_OWNER(effect) = owner;
        effect->scrollWithBg = 0;
        effect->drawFunc = Sprite_DrawEntity;
    }
}

/* Create the burst shot HeadhunterBurstShot_Update at the owner + (x, y). */
/**
 * @brief Create a burst shot at the owner-relative offset with supplied
 * velocity, an 8-by-8 attack box and its distinct contact callback.
 */
void Headhunter_SpawnBurstShot(struct EwramData_EntityData *owner, s32 x, s32 y, s32 vx, s32 vy)
{
    struct EwramData_EntityData *shot = EntityCreateInRange(0x49, 0xCF, HeadhunterBurstShot_Update);
    struct SpriteBox box = {-4, -4, 8, 8};
    if (shot != NULL)
    {
        s32 graphics;
        shot->posX.whole = owner->posX.whole + x;
        shot->posY.whole = owner->posY.whole + y;
        shot->velX.whole = vx;
        shot->velY.whole = vy;
        shot->spriteLayer = owner->spriteLayer - 1;
        graphics = owner->ext.headhunter.graphics;
        shot->ext.headhunterEffect.graphics = graphics;
        shot->basePalette = owner->ext.headhunter.palette3;
        Sprite_Attach(shot, sHeadhunterSpriteSheet, graphics, shot->basePalette);
        shot->ext.headhunterEffect.frame = 0x54;
        Hit_Enable(shot, 8, HeadhunterBurstShot_DamagePlayerOnContact, 0);
        HitBox_SetAttack(shot, &box);
        shot->hit.flags |= 4;
        shot->hit.flags &= ~2;
        shot->attack = owner->attack;
        ENTITY_OWNER(shot) = owner;
        shot->scrollWithBg = 1;
        shot->drawFunc = Sprite_DrawEntity;
    }
}

/* Burst shot: slow down, grow its hit box at 20 and 40 frames, vanish after 60. */
/**
 * @brief Damp and move the shot, grow the frame and attack box at
 * timers 20 and 40 and check contact until removal after timer 59.
 */
void HeadhunterBurstShot_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    struct SingleFrameAnimation animation = sHeadhunterFrameAnim;
    struct SpriteBox box1 = {-7, -7, 14, 14};
    struct SpriteBox box2 = {-11, -11, 22, 22};
    EntityDampVelocityX(entity, 0x20);
    EntityDampVelocityY(entity, 0x20);
    entity->flipX = RandomNumberGenerator() % 2;
    entity->flipY = RandomNumberGenerator() % 2;
    EntityAccelerateAndMove(entity, 0x40000);
    if (entity->timer == 20)
    {
        entity->ext.headhunterEffect.frame = 0x55;
        HitBox_SetAttack(entity, &box1);
    }
    if (entity->timer == 40)
    {
        entity->ext.headhunterEffect.frame = 0x56;
        HitBox_SetAttack(entity, &box2);
    }
    Anim_Start(entity, (struct AnimScript *)&animation, 2, 1);
    animation.frame = entity->ext.headhunterEffect.frame;
    Anim_Update(entity);
    if (entity->timer++ > 59)
        EntityDelete(entity);
    else
        Hit_CheckWithPlayer(&result, entity);
}

/* Create the whip head at the owner + (x, y) towards `angle`, with ten segments. */
/**
 * @brief Create the affine attack tip and ten segments at the supplied
 * offset and angle. Preserve the existing partial-allocation cleanup
 * and owner substate advance if a segment cannot be created.
 */
void Headhunter_SpawnExtendingAttack(struct EwramData_EntityData *owner, s32 x, s32 y, s32 angle)
{
    struct EwramData_EntityData *head = EntityCreateInRange(0x49, 0xCF, HeadhunterAttackTip_Update);
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    struct SpriteBox box = {-1, -1, 3, 3};
    struct SingleFrameAnimation animation = sHeadhunterAttackTipFrameAnim;
    u16 slot;
    s32 graphics;
    s32 i;
    if (head == NULL) return;
    head->posX.whole = owner->posX.whole + x;
    head->posY.whole = owner->posY.whole + y;
    BASE_X(head) = owner->posX.whole + x + camera->xPos.whole;
    BASE_Y(head) = owner->posY.whole + y + camera->yPos.whole;
    OFFSET_X(head) = 0;
    OFFSET_Y(head) = 0;
    ANGLE(head) = angle;
    SPEED(head) = 0x40000;
    head->spriteLayer = owner->spriteLayer - 1;
    graphics = owner->ext.headhunter.graphics;
    head->basePalette = owner->ext.headhunter.palette2;
    Sprite_Attach(head, sHeadhunterSpriteSheet, graphics, head->basePalette);
    Anim_Start(head, (struct AnimScript *)&animation, 2, 1);
    Anim_Update(head);
    if ((s32)((slot = ObjAffine_Alloc(1)) << 16) < 0)
    {
        EntityDelete(head);
        return;
    }
    head->affineIndex = slot;
    head->affine = 1;
    ObjAffine_SetBiased(0, 0x10000, 0x10000, head->flipX, head->flipY, head->affineIndex);
    Hit_Enable(head, 8, HeadhunterAttackTip_DamagePlayerOnContact, HeadhunterAttackTip_OnHit);
    HitBox_SetAttack(head, &box);
    HitBox_SetBody(head, &box);
    head->hit.flags &= ~4;
    head->hit.flags &= ~2;
    head->attack = owner->attack;
    ENTITY_OWNER(head) = owner;
    head->scrollWithBg = 0;
    head->drawFunc = Sprite_DrawEntity;
    for (i = 0; i < 10; i++)
    {
        if (!Headhunter_CreateAttackSegment(owner, head, i))
        {
            ObjAffine_Free(head->affineIndex);
            EntityDelete(head);
            owner->subState++;
            return;
        }
    }
}

/* Whip head: wait, extend, slow down to reverse, retract, then finish. */
/**
 * @brief Wait, extend along the stored angle, decelerate through reversal
 * and retract. Advance the owner substate on return; free the affine
 * slot on finish or owner defeat. Share contact history with the boss.
 */
void HeadhunterAttackTip_Update(struct EwramData_EntityData *head)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(head);
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    struct HitResult result;
    struct SingleFrameAnimation animation = sHeadhunterAttackTipFrameAnim;
    s32 x, y;
    if (owner->state == 2)
    {
        ObjAffine_Free(head->affineIndex);
        EntityDelete(head);
        return;
    }
    if (head->subState == 0)
    {
        head->posX.whole = BASE_X(head) + OFFSET_X(head) - camera->xPos.whole;
        head->posY.whole = BASE_Y(head) + OFFSET_Y(head) - camera->yPos.whole;
        if (head->timer++ > 14)
        {
            head->subState++;
            head->timer = 0;
            PlaySong(0x85);
        }
    }
    else if (head->subState == 1)
    {
        x = Sin(ANGLE(head) + 0x4000);
        y = Sin(ANGLE(head));
        x = FixedMultiply(x, SPEED(head));
        y = FixedMultiply(y, SPEED(head));
        OFFSET_X(head) += x;
        OFFSET_Y(head) += y;
        head->posX.whole = BASE_X(head) + OFFSET_X(head) - camera->xPos.whole;
        head->posY.whole = BASE_Y(head) + OFFSET_Y(head) - camera->yPos.whole;
        if (head->timer++ > 14)
        {
            head->subState++;
            head->timer = 0;
        }
    }
    else if (head->subState == 2)
    {
        SPEED(head) -= 0x8000;
        x = Sin(ANGLE(head) + 0x4000);
        y = Sin(ANGLE(head));
        x = FixedMultiply(x, SPEED(head));
        y = FixedMultiply(y, SPEED(head));
        OFFSET_X(head) += x;
        OFFSET_Y(head) += y;
        head->posX.whole = BASE_X(head) + OFFSET_X(head) - camera->xPos.whole;
        head->posY.whole = BASE_Y(head) + OFFSET_Y(head) - camera->yPos.whole;
        if (SPEED(head) <= -0x40000) head->subState++;
    }
    else if (head->subState == 3)
    {
        x = Sin(ANGLE(head) + 0x4000);
        y = Sin(ANGLE(head));
        x = FixedMultiply(x, SPEED(head));
        y = FixedMultiply(y, SPEED(head));
        OFFSET_X(head) += x;
        OFFSET_Y(head) += y;
        head->posX.whole = BASE_X(head) + OFFSET_X(head) - camera->xPos.whole;
        head->posY.whole = BASE_Y(head) + OFFSET_Y(head) - camera->yPos.whole;
        if ((OFFSET_X(head) < 0 ? -OFFSET_X(head) : OFFSET_X(head)) < 0x40000
            && (OFFSET_Y(head) < 0 ? -OFFSET_Y(head) : OFFSET_Y(head)) < 0x40000)
            head->subState++;
    }
    else if (head->subState == 4)
    {
        head->subState++;
        owner->subState++;
        head->hidden = 1;
        return;
    }
    else if (head->subState == 5)
    {
        ObjAffine_Free(head->affineIndex);
        EntityDelete(head);
        head->subState++;
        return;
    }
    Anim_Start(head, (struct AnimScript *)&animation, 2, 1);
    Anim_Update(head);
    ObjAffine_SetBiased(ANGLE(head), 0x10000, 0x10000, head->flipX, head->flipY, head->affineIndex);
    Hit_CheckWithPlayerSharedHistory(&result, head, owner);
}

/* Create whip segment `index` of ten. */
/**
 * @brief Create one indexed affine segment linked to the attack tip
 * and boss, with both contact and forwarded-damage callbacks.
 * Return NULL on entity or affine allocation failure.
 */
struct EwramData_EntityData *Headhunter_CreateAttackSegment(struct EwramData_EntityData *owner, struct EwramData_EntityData *parent, s32 index)
{
    struct EwramData_EntityData *segment = EntityCreateInRange(0x49, 0xCF, HeadhunterAttackSegment_Update);
    struct SpriteBox box = {-1, -1, 3, 3};
    struct SingleFrameAnimation animation = sHeadhunterAttackSegmentFrameAnim;
    u16 slot;
    s32 graphics;
    if (segment == NULL) return NULL;
    segment->posX.whole = parent->posX.whole;
    segment->posY.whole = parent->posY.whole;
    SEGMENT_INDEX(segment) = index;
    segment->spriteLayer = parent->spriteLayer + 1;
    graphics = owner->ext.headhunter.graphics;
    segment->basePalette = owner->ext.headhunter.palette2;
    Sprite_Attach(segment, sHeadhunterSpriteSheet, graphics, segment->basePalette);
    Anim_Start(segment, (struct AnimScript *)&animation, 2, 1);
    Anim_Update(segment);
    if ((s32)((slot = ObjAffine_Alloc(1)) << 16) < 0)
    {
        EntityDelete(segment);
        return NULL;
    }
    segment->affineIndex = slot;
    segment->affine = 1;
    ObjAffine_SetBiased(0, 0x10000, 0x10000, segment->flipX, segment->flipY, segment->affineIndex);
    Hit_Enable(segment, 8, HeadhunterAttackSegment_DamagePlayerOnContact, HeadhunterAttackSegment_OnHit);
    HitBox_SetAttack(segment, &box);
    HitBox_SetBody(segment, &box);
    segment->hit.flags &= ~4;
    segment->hit.flags &= ~2;
    segment->attack = owner->attack;
    ENTITY_OWNER(segment) = owner;
    segment->ext.headhunterWhipSegment.tip = parent;
    segment->scrollWithBg = 0;
    segment->drawFunc = Sprite_DrawEntity;
    return segment;
    return NULL;
}

/* Whip segment: index/10 of the way from the base to the head. */
/**
 * @brief Place the segment at index/10 of the tip displacement from
 * its stored room-space base. Share boss contact history and free
 * the affine slot on boss defeat or tip substate 5.
 */
void HeadhunterAttackSegment_Update(struct EwramData_EntityData *segment)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(segment);
    struct EwramData_EntityData *parent = segment->ext.headhunterWhipSegment.tip;
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    struct HitResult result;
    struct SingleFrameAnimation animation = sHeadhunterAttackSegmentFrameAnim;
    if (owner->state == 2 || parent->subState == 5)
    {
        ObjAffine_Free(segment->affineIndex);
        EntityDelete(segment);
        return;
    }
    segment->posX.whole = parent->ext.headhunterWhip.baseX + parent->ext.headhunterWhip.offsetX * SEGMENT_INDEX(segment) / 10 - camera->xPos.whole;
    segment->posY.whole = parent->ext.headhunterWhip.baseY + parent->ext.headhunterWhip.offsetY * SEGMENT_INDEX(segment) / 10 - camera->yPos.whole;
    Anim_Start(segment, (struct AnimScript *)&animation, 2, 1);
    Anim_Update(segment);
    ObjAffine_SetBiased(parent->velX.whole, 0x10000, 0x10000, segment->flipX, segment->flipY, segment->affineIndex);
    Hit_CheckWithPlayerSharedHistory(&result, segment, owner);
}

/* Headhunter effect: follow the owner (unk_4F8) until the timer runs out. */
/* bit 0 of the owner's unk_4FE ends its effects (TODO: meaning) */

/**
 * @brief Follow and animate at the owner position until the signed
 * countdown expires or the owner sets effect-cancel bit 0.
 */
void HeadhunterAttackFollower_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    entity->posX.whole = owner->posX.whole;
    entity->posY.whole = owner->posY.whole;
    Anim_Update(entity);
    if ((s8)entity->timer-- <= 0)
        EntityDelete(entity);
    else if (OWNER_DONE(owner))
        EntityDelete(entity);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   HeadhunterWave_DamagePlayerOnContact = Object28_OnPlayerContact */

/**
 * @brief Apply stored wave damage with effect type 0 and flags 1.
 */
void HeadhunterWave_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/* Headhunter lightning-burst flicker. The creator does not allocate an
 * affine slot; retain the existing affine-free call on owner cancel. */
/* bit 0 of the owner's unk_4FE ends its effects (TODO: meaning) */

/**
 * @brief Randomize among three frames through signed timer 49, then
 * remove. Preserve the existing affine-free call only on the owner
 * cancel branch; the creator does not allocate an affine slot.
 */
void HeadhunterLightningBurstFlicker_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    struct SingleFrameAnimation animation = sHeadhunterFrameAnim;
    Anim_Start(entity, (struct AnimScript *)&animation, 2, 1);
    animation.frame = entity->ext.headhunterEffect.frame + RandomNumberGenerator() % 3;
    Anim_Update(entity);
    if ((s8)entity->timer++ > 49)
        EntityDelete(entity);
    else if (OWNER_DONE(owner))
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
}

/* HeadhunterLightningImpact_DamagePlayerOnContact: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */

/**
 * @brief Apply stored lightning impact damage with effect type 8
 * and flags 0.
 */
void HeadhunterLightningImpact_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 8;
    contactDamage.unused = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/* Headhunter lightning-impact flicker and orbiting-particle creation.
 * The particle follows a stored anchor with an expanding radius. */
/* bit 0 of the owner's unk_4FE ends its effects (TODO: meaning) */

/**
 * @brief Randomize between two frames and remove after signed timer 7
 * or when the retained boss sets effect-cancel bit 0.
 */
void HeadhunterLightningImpactFlicker_Update(struct EwramData_EntityData *entity)
{
    struct SingleFrameAnimation animation = sHeadhunterFrameAnim;
    struct EwramData_EntityData *owner = entity->ext.headhunterEffect.source;
    Anim_Start(entity, (struct AnimScript *)&animation, 2, 1);
    animation.frame = RandomNumberGenerator() % 2 + entity->ext.headhunterEffect.frame;
    Anim_Update(entity);
    if ((s8)entity->timer++ > 7)
        EntityDelete(entity);
    else if (OWNER_DONE(owner))
        EntityDelete(entity);
}
/**
 * @brief Create an orbiting particle with the supplied initial angle,
 * an owner-relative anchor and stored camera offsets. Retain the
 * separate boss link used by its effect-cancel check.
 */
void Headhunter_SpawnOrbitingParticle(struct EwramData_EntityData *owner, struct EwramData_EntityData *source, s32 x, s32 y, s32 speed)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, HeadhunterOrbitingParticle_Update);
    if (effect != NULL)
    {
        s32 graphics;
        effect->velX.whole = owner->posX.whole + x;
        effect->velY.whole = owner->posY.whole + y;
        effect->accelX.whole = speed;
        effect->accelY.whole = 0x100;
        effect->ext.headhunterOrbit.cameraX = camera->xPos.whole;
        effect->ext.headhunterOrbit.cameraY = camera->yPos.whole;
        effect->flipX = RandomNumberGenerator() % 2;
        effect->flipY = 1;
        effect->spriteLayer = owner->spriteLayer - 1;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(effect, sCommonObjectSpriteSheet, graphics, 7);
        Anim_StartIndexed(effect, 0, 0, 0);
        Anim_Update(effect);
        ENTITY_OWNER(effect) = owner;
        effect->ext.headhunterEffect.source = source;
        effect->scrollWithBg = 0;
        effect->drawFunc = Sprite_DrawEntity;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   HeadhunterBurstShot_DamagePlayerOnContact = PoisonWorm_DamagePlayerOnContact
 *   HeadhunterAttackTip_DamagePlayerOnContact = Object28_OnPlayerContact */

/**
 * @brief Apply stored burst-shot damage with effect type 0x40
 * and flags 0.
 */
void HeadhunterBurstShot_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0x40;
    contactDamage.unused = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief Apply stored attack-tip damage with effect type 0
 * and flags 1.
 */
void HeadhunterAttackTip_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   HeadhunterAttackTip_OnHit = ManticoreTail_OnHit */

/**
 * @brief Temporarily resolve boss damage at the struck tip position,
 * restore boss coordinates, display the number on the tip and
 * forward resolved damage to the boss phase handler.
 */
void HeadhunterAttackTip_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    struct EwramData_EntityData *headhunter = ENTITY_OWNER(entity);
    s32 oldX = headhunter->posX.whole;
    s32 oldY = headhunter->posY.whole;
    s16 damage;
    s16 attackPower;

    headhunter->posX.whole = entity->posX.whole;
    headhunter->posY.whole = entity->posY.whole;
    attackPower = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(ENTITY_OWNER(entity), (struct EwramData_EntityData *)attacker, hit, attackPower);
    headhunter->posX.whole = oldX;
    headhunter->posY.whole = oldY;
    Entity_ShowDamage(entity, (s16)damage, 0);
    Headhunter_ApplyResolvedDamage(ENTITY_OWNER(entity), attacker, hit, damage);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   HeadhunterAttackSegment_DamagePlayerOnContact = Object28_OnPlayerContact */

/**
 * @brief Apply stored attack-segment damage with effect type 0
 * and flags 1.
 */
void HeadhunterAttackSegment_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   HeadhunterAttackSegment_OnHit = ManticoreTail_OnHit */

/**
 * @brief Temporarily resolve boss damage at the struck segment position,
 * restore boss coordinates, display the number on that segment and
 * forward resolved damage to the boss phase handler.
 */
void HeadhunterAttackSegment_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    struct EwramData_EntityData *headhunter = ENTITY_OWNER(entity);
    s32 oldX = headhunter->posX.whole;
    s32 oldY = headhunter->posY.whole;
    s16 damage;
    s16 attackPower;

    headhunter->posX.whole = entity->posX.whole;
    headhunter->posY.whole = entity->posY.whole;
    attackPower = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(ENTITY_OWNER(entity), (struct EwramData_EntityData *)attacker, hit, attackPower);
    headhunter->posX.whole = oldX;
    headhunter->posY.whole = oldY;
    Entity_ShowDamage(entity, (s16)damage, 0);
    Headhunter_ApplyResolvedDamage(ENTITY_OWNER(entity), attacker, hit, damage);
}

/* Emit the contact effect, variant 1 when unk_4FC is 3. */
/**
 * @brief Apply stored body contact damage with effect type 0 and
 * flags 1 only for phase 3; other phases use flags 0.
 */
void Headhunter_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage effect;
    effect.attack = entity->attack;
    effect.attributes = 0;
    effect.unused = 0;
    if (entity->ext.headhunter.formPhase != 3)
        effect.reaction = 0;
    else
        effect.reaction = 1;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Headhunter_OnHit = Manticore_OnHit */

/**
 * @brief Resolve and display root body damage, then forward it to
 * the phase-aware resolved-damage helper.
 */
void Headhunter_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s16 damage = Hit_ResolveDamage(entity, (struct EwramData_EntityData *)attacker, hit, Player_CalculateAttackPower(hit[2]));

    Entity_ShowDamage(entity, (s16)damage, 0);
    Headhunter_ApplyResolvedDamage(entity, attacker, hit, damage);
}
