/* Headhunter AI (81EBC), intro (82EA0), attack states (82FFC) and defeat
 * sequence (83860). */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code_08033CAC.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "syscalls.h"

extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Entity_GetVisibilityPattern(s32);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *, s32);
extern void Headhunter_SpawnFallingPart(struct EwramData_EntityData *, s32, s32, s32);
extern void Headhunter_SpawnDefeatEffect(s32, s32);
extern void SoulPickup_SpawnFromEnemyDefinition(s32, s32, s32, s32);
extern void BossRecoveryPickup_Spawn(s32, s32, s32, s32, s32);
extern s32 Entity_MoveWithCollisionDefault(struct EwramData_EntityData *, s32, s32);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern void EntityDampVelocityY(struct EwramData_EntityData *, s32);
extern void Headhunter_SpawnWaveBurst(struct EwramData_EntityData *, s32, s32, s32);
extern void Headhunter_SpawnLightningBurst(struct EwramData_EntityData *, s32, s32, s32);
extern void Headhunter_SpawnAttackFollower(struct EwramData_EntityData *, s32);
extern void Headhunter_SpawnBurstShot(struct EwramData_EntityData *, s32, s32, s32, s32);
extern void Headhunter_SpawnExtendingAttack(struct EwramData_EntityData *, s32, s32, s32);

extern s32 Target_IsInRect(s16, s16, s16, s16);
extern void EntityMoveX(struct EwramData_EntityData *);
extern void EntityMoveY(struct EwramData_EntityData *);
extern void Effect_SpawnRisingParticleInBox(s32, s32, s32, s32, s32, s32);
s32 Headhunter_UpdateIntro(struct EwramData_EntityData *entity);

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
    Target_IsInRect((e)->posX.part16.integer - (dx), (e)->posY.part16.integer - (dy), (w), (h))

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
        BossRecoveryPickup_Spawn((s16)entity->posX.part16.integer, 0x20, 4, 0x60, 1);
        break;
    case 4:
        break;
    }
}
