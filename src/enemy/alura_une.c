/*
 * Alura Une: body, thorns and root waves.
 */
#include "core.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "entity_util.h"
#include "structs/entity_render_flags.h"
#include "player_core.h"
#include "sound.h"
#include "gameplay.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "map_flags.h"
#include "math_anim.h"
#include "music.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"

/* Initialize AluraUne's graphics, collisions and initial facing. */
extern u8 AluraUne_LoadGraphics(struct EwramData_EntityData *);
extern void Hit_EnableContact(struct EwramData_EntityData *, s32, void *, void *);
extern void AluraUne_DamagePlayerOnContact(struct EwramData_EntityData *);
void AluraUne_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
void EnemyAluraUneCreate(struct EwramData_EntityData *entity)
{
    if (!AluraUne_LoadGraphics(entity))
    {
        EntityDelete(entity);
        goto finished;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        goto finished;
    }
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, AluraUne_DamagePlayerOnContact, AluraUne_OnHit);
    else
        Hit_Enable(entity, 8, AluraUne_DamagePlayerOnContact, (s32)AluraUne_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->state = 0;
    entity->flipX = Entity_IsTargetRight(entity);
finished:
    return;
}

extern s32 Target_IsInRect(s16, s16, s16, s16);
extern void AluraUne_SpawnThorn(struct EwramData_EntityData *, s32, s32, s32);

/**
 * @brief 7C150 | Boss AI (subState), always facing the player: wakes on the
 * player box 0xB4 x 0x60 (first time straight to the entrance, state 10),
 * then picks at random between an animation 1 / 3 / 4 sequence with a
 * random 60-119 frame wait (unk_505 cuts it short), attack mode
 * (state = 1) or the roar (state 0xB): states 10-13 spawn eight debris
 * pieces (AluraUne_SpawnThorn), shake the screen (ScreenShake_RequestOffset) and play sound
 * 0x105 (Sound_PlayTracked / Sound_PlayTrackedIfAbsent / Sound_StopTracked).
 */
void AluraUne_UpdateIdle(struct EwramData_EntityData *entity)
{
    u32 r;
    s32 i;

    entity->flipX = Entity_IsTargetRight(entity);
    switch (entity->subState)
    {
    case 0:
        if (Target_IsInRect(entity->posX.part16.integer - 0x5A, entity->posY.part16.integer - 0x30, 0xB4, 0x60) || Enemy_IsDamaged(entity))
        {
            if (!*(s8 *)&entity->ext.aluraUne.awake)
            {
                entity->ext.aluraUne.awake = 1;
                entity->subState = 10;
                break;
            }
            r = RandomNumberGenerator() % 3;
            if (r == 0)
            {
                entity->subState++;
                Anim_StartIndexed(entity, 1, 0, 0);
            }
            else if (r == 1)
            {
                entity->state = r;
                entity->subState = 0;
            }
            else
            {
                entity->state = 1;
                entity->subState = 10;
            }
        }
        break;
    case 1:
        if (entity->animDone)
        {
            entity->subState++;
            Anim_StartIndexed(entity, 3, 0, 0);
        }
        break;
    case 2:
        if (entity->animDone)
        {
            entity->subState++;
            entity->timer = RandomNumberGenerator() % 60 + 60;
            Anim_StartIndexed(entity, 4, 0, 1);
        }
        break;
    case 3:
        if (*(s8 *)&entity->ext.aluraUne.hit)
        {
            entity->timer = 0;
            entity->ext.aluraUne.hit = 0;
        }
        if ((s8)entity->timer-- > 0)
            break;
        r = RandomNumberGenerator() % 3;
        if (r == 0)
        {
            entity->subState++;
            Anim_StartIndexed(entity, 0xB, 0, 0);
        }
        else if (r == 1)
        {
            entity->state = r;
            entity->subState = r;
        }
        else
        {
            entity->state = 1;
            entity->subState = 0xB;
            entity->animDone = 1;
        }
        break;
    case 4:
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 10, 0, 0);
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 5:
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 6:
        if (entity->timer++ > 0x3B)
        {
            entity->subState = 0;
            entity->timer = 0;
        }
        break;
    case 10:
        entity->subState++;
        entity->timer = 0;
        for (i = 0; i <= 3; i++)
        {
            AluraUne_SpawnThorn(entity, (RandomNumberGenerator() & 0xF) + i * 16, 0, 0x28);
            AluraUne_SpawnThorn(entity, -(RandomNumberGenerator() & 0xF) - i * 16, 0, 0x28);
        }
        break;
    case 11:
        if (!(entity->timer & 7))
            ScreenShake_RequestOffset(0, 0, 1);
        else
            ScreenShake_RequestOffset(0, 0, 0);
        entity->subState++;
        entity->timer = 0;
        Anim_StartIndexed(entity, 1, 0, 0);
        Sound_PlayTracked(0x105);
        break;
    case 12:
        entity->timer++;
        if ((entity->timer & 3) == 0)
        {
            s32 a = (RandomNumberGenerator() & 1) + 1;
            s32 b = (RandomNumberGenerator() & 1) + 1;
            ScreenShake_RequestOffset(0, a, b);
            Sound_PlayTrackedIfAbsent(0x105);
        }
        else
        {
            ScreenShake_RequestOffset(0, 0, 0);
        }
        if (entity->animDone)
        {
            entity->subState++;
            entity->timer = 0;
            Anim_StartIndexed(entity, 2, 0, 1);
        }
        break;
    case 13:
        if ((entity->timer & 3) == 0)
        {
            s32 a = (RandomNumberGenerator() & 1) + 1;
            s32 b = (RandomNumberGenerator() & 1) + 1;
            ScreenShake_RequestOffset(0, a, b);
            Sound_PlayTrackedIfAbsent(0x105);
        }
        else
        {
            ScreenShake_RequestOffset(0, 0, 0);
        }
        if (entity->timer++ > 0x1D)
        {
            entity->state = 0;
            entity->subState = 2;
            Anim_StartIndexed(entity, 3, 0, 0);
            Sound_StopTracked(0x105);
        }
        break;
    }
}

/* AluraUne_UpdateAttack: enemy behaviour state machine (14 states). */
extern void AluraUne_SpawnRootWave(struct EwramData_EntityData *parent, s32 index, s32 facing);

/**
 * @brief Attack state machine (state 1): animation 1, then rows of thorns at
 * random gaps on both sides (AluraUne_SpawnThorn) or two root waves along
 * the floor in both directions (AluraUne_SpawnRootWave); then back to
 * state 0.
 */
void AluraUne_UpdateAttack(struct EwramData_EntityData *entity)
{
    u32 r;
    u32 m;
    s32 i;
    s32 skip1, skip2;

    entity->flipX = Entity_IsTargetRight(entity);
    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->subState++;
        break;
    case 1:
        if (entity->animDone)
        {
            skip1 = RandomNumberGenerator() % 6;
            skip2 = RandomNumberGenerator() % 6;
            for (i = 0; i <= 6; i++)
            {
                if (i != skip1 && i != skip2)
                    AluraUne_SpawnThorn(entity, i * 16 + 0x28, -1, 0x6E);
            }
            skip1 = RandomNumberGenerator() % 6;
            skip2 = RandomNumberGenerator() % 6;
            for (i = 0; i <= 6; i++)
            {
                if (i != skip1 && i != skip2)
                    AluraUne_SpawnThorn(entity, -(i * 16 + 0x28), -1, 0x6E);
            }
            entity->ext.aluraUne.hit = 0;
            entity->subState++;
            entity->timer = 0;
            if (entity->animScriptIndex != 4)
                Anim_StartIndexed(entity, 3, 0, 0);
            else
                entity->subState = entity->animScriptIndex;
            Sound_PlayTracked(0x105);
        }
        break;
    case 2:
        entity->timer++;
        if ((entity->timer & 3) == 0)
            Sound_PlayTrackedIfAbsent(0x105);
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 4, 0, 1);
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 3:
        entity->subState++;
    case 4:
        if ((entity->timer & 3) == 0)
            Sound_PlayTrackedIfAbsent(0x105);
        if (entity->timer > 0x1D)
        {
            if (entity->timer <= 0x5A)
            {
                if (!(entity->timer & 7))
                    ScreenShake_RequestOffset(0, 0, 1);
                else
                    ScreenShake_RequestOffset(0, 0, 0);
            }
            else
            {
                if (entity->timer == 0x6E)
                    Anim_StartIndexed(entity, 2, 0, 1);
                if ((entity->timer & 3) == 0)
                {
                    s32 a = (RandomNumberGenerator() & 1) + 1;
                    s32 b = (RandomNumberGenerator() & 1) + 1;
                    ScreenShake_RequestOffset(0, a, b);
                }
                else
                {
                    ScreenShake_RequestOffset(0, 0, 0);
                }
            }
        }
        if (entity->timer++ <= 0xFC)
            break;
        Sound_StopTracked(0x105);
        if (EntityIsOutsideScreen(entity, 4))
        {
            entity->state = 0;
            entity->subState = 0;
            Anim_StartIndexed(entity, 0, 0, 1);
            break;
        }
        r = RandomNumberGenerator() % 3;
        if (r == 0)
        {
            entity->subState++;
            entity->timer = r;
            Anim_StartIndexed(entity, 10, 0, 0);
        }
        else if (r == 1)
        {
            entity->state = 0;
            entity->subState = 2;
            Anim_StartIndexed(entity, 3, 0, 0);
        }
        else
        {
            entity->animDone = 1;
            entity->subState = 0xB;
        }
        break;
    case 5:
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 6:
        if (entity->timer++ > 0x59)
        {
            entity->state = 0;
            entity->subState = 0;
            entity->timer = 0;
        }
        break;
    case 10:
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->subState++;
        break;
    case 11:
        if (entity->animDone)
        {
            entity->ext.aluraUne.hit = 0;
            entity->subState++;
            entity->timer = 0;
            Anim_StartIndexed(entity, 5, 0, 0);
        }
        break;
    case 12:
        if (entity->animDone)
        {
            entity->subState++;
            entity->timer = 0;
            Anim_StartIndexed(entity, 6, 0, 1);
            break;
        }
        m = 0xFFFF;
        if ((*(u16 *)&entity->animStep & m) == 3)
        {
            AluraUne_SpawnRootWave(entity, 0, entity->flipX);
            AluraUne_SpawnRootWave(entity, 0, ~(*(ENTITY_RENDER_FLAGS_ADDRESS(entity)) >> 6) & 1);
        }
        m &= *(u16 *)&entity->animStep;
        if (m == 4)
        {
            AluraUne_SpawnRootWave(entity, 1, entity->flipX);
            AluraUne_SpawnRootWave(entity, 1, ~(*(ENTITY_RENDER_FLAGS_ADDRESS(entity)) >> 6) & 1);
        }
        break;
    case 13:
        if (entity->timer++ > 0xEF)
        {
            if (EntityIsOutsideScreen(entity, 4))
            {
                entity->state = 0;
                entity->subState = 0;
                Anim_StartIndexed(entity, 0, 0, 1);
                break;
            }
            r = RandomNumberGenerator() % 3;
            if (r == 0)
            {
                entity->subState = 5;
                entity->timer = 0;
                Anim_StartIndexed(entity, 10, 0, 0);
            }
            else if (r == 1)
            {
                entity->state = 0;
                entity->subState = 2;
                Anim_StartIndexed(entity, 3, 0, 0);
            }
            else
            {
                Anim_StartIndexed(entity, 0xB, 0, 0);
                entity->subState = 1;
            }
        }
        break;
    }
}

/* Advance the display sequence through three animations. */
/**
 * @brief Bestiary preview state (state 3): every 60 frames open (animations 1
 * and 3) and close again.
 */
void AluraUne_UpdatePreview(struct EwramData_EntityData *entity)
{
    s32 state = entity->subState;
    switch (state)
    {
    case 0:
        if (entity->timer++ > 59)
        {
            entity->subState++;
            Anim_StartIndexed(entity, 1, 0, 0);
        }
        break;
    case 1:
        if (entity->animDone)
        {
            entity->subState = 2;
            Anim_StartIndexed(entity, 3, 0, 0);
        }
        break;
    case 2:
        if (entity->animDone)
        {
            entity->subState = 3;
            Anim_StartIndexed(entity, 4, 0, 1);
        }
        break;
    }
}

/* Apply damage only when the current animation permits it. */
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *, s16);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s32, s32);
/**
 * @brief Hit: no damage while closed (animation 0, sound 0x6A), else subtract and
 * show it; mark the hit (unk_505 = 1, cuts the idle wait short); at
 * HP <= 0 die.
 */
void AluraUne_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    u16 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));
    entity->hitFlashTimer = 8;
    RandomNumberGenerator();
    if (entity->animScriptIndex == 0)
    {
        damage = 0;
        PlaySong(0x6A);
    }
    {
        s32 hp = entity->hp;
        s32 applied = (s16)damage;
        entity->hp = hp - applied;
        Entity_ShowDamage(entity, applied, 0);
    }
    entity->ext.aluraUne.hit = 1;
    if ((s16)entity->hp <= 0)
    {
        Sound_StopTracked(0x105);
        Enemy_DropRewards(entity);
        PlaySong(0x6E);
        entity->palette = entity->basePalette;
        entity->timer = 40;
        entity->state = 2;
        entity->subState = 0;
    }
}

/* Create Alura Une's projectile with a timed or immediate activation. */
extern void AluraUneThorn_Update(struct EwramData_EntityData *);
extern void AluraUneAttack_DamagePlayerOnContact(struct EwramData_EntityData *);
/**
 * @brief Spawn a thorn at xOffset from parent that rises from the floor after
 * delay frames (hit box {-2, -54, 4, 54}) and sinks again.
 */
void AluraUne_SpawnThorn(struct EwramData_EntityData *parent, s32 xOffset, s32 delay, s32 type)
{
    struct EwramData_EntityData *projectile = EntityCreateInRange(0x1B, 0x48, (u32 *)AluraUneThorn_Update);
    if (projectile)
    {
        s32 graphics = ObjGfx_Find(sAluraUneGfx);
        if (graphics < 0)
        {
            EntityDelete(projectile);
            goto finished;
        }
        Sprite_Attach(projectile, sAluraUneSpriteSheet, graphics, parent->basePalette);
        Anim_StartIndexed(projectile, 12, 0, 0);
        projectile->drawFunc = (u32 *)Sprite_DrawEntity;
        Hit_Enable(projectile, 8, AluraUneAttack_DamagePlayerOnContact, 0);
        projectile->hit.flags |= 4;
        projectile->hit.flags |= 2;
        projectile->scrollWithBg = 1;
        projectile->spriteLayer = parent->spriteLayer;
        projectile->hidden = 1;
        if (delay == -1)
        {
            s32 timer = RandomNumberGenerator() & 31;
            projectile->timer = timer + 32;
        }
        else projectile->timer = delay;
        projectile->ext.aluraUneThorn.time = type;
        if (delay == 0)
        {
            Anim_StartIndexed(projectile, 7, 0, 0);
            Anim_Update(projectile);
            projectile->state = 2;
            projectile->hidden = 0;
        }
        ENTITY_OWNER(projectile) = parent;
        projectile->posX.part16.integer = parent->posX.part16.integer + xOffset;
        projectile->posY.part16.integer = parent->posY.part16.integer;
        projectile->flipX = parent->flipX;
        projectile->attack = parent->attack;
    }
finished:
    return;
}

/* Activate a timed projectile and retain it only while its parent is alive. */
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
void AluraUneThorn_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct SpriteBox box = {-2, -54, 4, 54};
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    switch (entity->state)
    {
    case 0:
        if ((s8)entity->timer-- <= 0)
        {
            Anim_StartIndexed(entity, 12, 0, 0);
            entity->hidden = 0;
            entity->state++;
        }
        break;
    case 1:
        if (entity->animDone)
        {
            if (parent && parent->updateFunc && parent->enemyId == 0x4D && parent->state != 2)
            {
                entity->state++;
                entity->timer = 0;
                Anim_StartIndexed(entity, 7, 0, 0);
            }
            else
            {
                entity->deleteRequested = 1;
                goto finished;
            }
        }
        break;
    case 2:
        if (++entity->timer == 8)
        {
            s32 vx = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
            s32 vy = -0x20000 - (RandomNumberGenerator() & 0x1FFFF);
            s32 type = RandomNumberGenerator() & 1;
            Effect_SpawnFallingPiece(entity->posX.whole, entity->posY.whole, vx, vy, entity->spriteLayer - 1, type);
        }
        if (entity->animDone)
        {
            entity->state++;
            Anim_StartIndexed(entity, 8, 0, 1);
            entity->timer = 0;
            HitBox_SetAttack(entity, &box);
            entity->hit.flags &= ~2;
            entity->hit.flags |= 4;
        }
        break;
    case 3:
        Hit_CheckWithPlayer(collision, entity);
        if (entity->timer++ >= entity->ext.aluraUneThorn.time)
        {
            entity->state++;
            Anim_StartIndexed(entity, 13, 0, 0);
        }
        if (!parent || !parent->updateFunc || parent->enemyId != 0x4D || parent->state == 2)
        {
            Anim_StartIndexed(entity, 13, 0, 0);
            entity->state++;
            goto finished;
        }
        break;
    case 4:
        if (entity->animDone)
        {
            entity->deleteRequested = 1;
            goto finished;
        }
        break;
    }
    Anim_Update(entity);
finished:
    return;
}

/* Create one segment of Alura Une's attack on the selected side. */
extern void AluraUneRootWave_Update(struct EwramData_EntityData *);
/**
 * @brief Spawn a root wave (animation 9) moving along the floor in the facing
 * direction.
 */
void AluraUne_SpawnRootWave(struct EwramData_EntityData *parent, s32 index, s32 facing)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x1B, 0x48, (u32 *)AluraUneRootWave_Update);
    if (part)
    {
        s32 graphics = ObjGfx_Find(sAluraUneGfx);
        if (graphics < 0)
        {
            EntityDelete(part);
            goto finished;
        }
        Sprite_Attach(part, sAluraUneSpriteSheet, graphics, parent->basePalette);
        Anim_StartIndexed(part, 9, 0, 0);
        Anim_Update(part);
        part->drawFunc = (u32 *)Sprite_DrawEntity;
        Entity_UpdateBoxes(part);
        Hit_Enable(part, 8, AluraUneAttack_DamagePlayerOnContact, 0);
        part->hit.flags &= ~2;
        part->hit.flags |= 4;
        part->scrollWithBg = 1;
        part->spriteLayer = parent->spriteLayer;
        ENTITY_OWNER(part) = parent;
        part->posX.part16.integer = parent->posX.part16.integer;
        part->posY.part16.integer = parent->posY.part16.integer;
        part->flipX = facing;
        part->attack = parent->attack;
        if (part->flipX) { part->posX.part16.integer -= 128; part->posX.part16.integer += index * 6; }
        else { part->posX.part16.integer += 128; part->posX.part16.integer -= index * 6; }
    }
finished:
    return;
}

/* Advance a six-step attack, turn at its midpoint and pause between steps. */
static inline s32 Flipped(s32 facing) { return facing ^ 1; }
/**
 * @brief Root wave update: after each animation (sound 0x80) jump 40 px further,
 * turning once after the third; delete after six.
 */
void AluraUneRootWave_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    switch (entity->state)
    {
    case 0:
        if (entity->animStep == 0 && entity->animTimer == 1) PlaySong(0x80);
        if (entity->animDone)
        {
            entity->state++;
            if (++entity->ext.aluraUneRootWave.step > 5) { entity->deleteRequested = 1; goto finished; }
            if (entity->ext.aluraUneRootWave.step == 3) entity->flipX ^= 1;
            else if (entity->flipX) entity->posX.part16.integer += 40;
            else entity->posX.part16.integer -= 40;
            entity->hidden = 1;
            entity->timer = 0;
            entity->hit.flags |= 2;
            if (!parent || !parent->updateFunc || parent->enemyId != 0x4D || parent->state == 2)
            {
                entity->deleteRequested = 1;
                goto finished;
            }
        }
        else
        {
            Entity_UpdateBoxes(entity);
            Hit_CheckWithPlayer(collision, entity);
        }
        break;
    case 1:
        if (entity->timer++ > 5)
        {
            Anim_StartIndexed(entity, 9, 0, 0);
            entity->hidden = 0;
            entity->state = 0;
            entity->hit.flags &= ~2;
        }
        if (!parent || !parent->updateFunc || parent->enemyId != 0x4D || parent->state == 2)
        {
            entity->deleteRequested = 1;
            goto finished;
        }
        break;
    }
    Anim_Update(entity);
finished:
    return;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   AluraUneAttack_DamagePlayerOnContact = Object28_OnPlayerContact
 *   AluraUne_LoadGraphics = Object25_LoadGraphics
 *   AluraUne_DamagePlayerOnContact = Object28_OnPlayerContact */

extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

/**
 * @brief 7CEF0 | Same as Object28_OnPlayerContact.
 */
void AluraUneAttack_DamagePlayerOnContact(struct EwramData_EntityData *entity)
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

/**
 * @brief 7CF20 | Same as Object25_LoadGraphics.
 */
u8 AluraUne_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sAluraUneGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sAluraUnePalette, 0);
    Sprite_Attach(entity, sAluraUneSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 7CF6C | Same as Object28_OnPlayerContact.
 */
void AluraUne_DamagePlayerOnContact(struct EwramData_EntityData *entity)
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

/* Update Alura Une and clear the room event when she leaves its bounds. */
void EnemyAluraUneUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    if (Enemy_PreUpdate(entity))
    {
        if (!Entity_IsInRect(entity, -128, -64, 496, 288)) Sound_StopTracked(0x105);
        goto finished;
    }
    switch (entity->state)
    {
    case 0: AluraUne_UpdateIdle(entity); break;
    case 1: AluraUne_UpdateAttack(entity); break;
    case 2: Enemy_UpdateFragmentDeath(entity); break;
    case 3:
        AluraUne_UpdatePreview(entity);
        Anim_Update(entity);
        goto finished;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
finished:
    return;
}
