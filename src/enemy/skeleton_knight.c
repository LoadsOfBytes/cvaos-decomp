/*
 * Skeleton Knight.
 */
#include "core.h"
#include "graphics.h"
#include "gameplay.h"
#include "structs/ewram.h"
#include "sound.h"
#include "player_core.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "damage_numbers.h"
#include "entity_motion.h"
#include "hitbox.h"
#include "constants/songs.h"

u8 SkeletonKnight_LoadGraphics(struct EwramData_EntityData *entity);
void SkeletonKnight_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void SkeletonKnight_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

/* EnemySkeletonKnightCreate 0x0808EC64..0x0808ECF7. */

/* States of SkeletonKnight (entity->state), named after the update function the dispatcher calls. */
enum {
    SKELETON_KNIGHT_STATE_WALK = 0,
    SKELETON_KNIGHT_STATE_ATTACK = 1,
    SKELETON_KNIGHT_STATE_FRAGMENT_DEATH = 2,
    SKELETON_KNIGHT_STATE_ANIM_ONLY = 3,
};

/**
 * @brief 8EC64 | Skeleton Knight create: load the graphics (delete the
 * enemy when that fails), start the animation and set up the hit boxes
 * (Hit_EnableContact when a box record has a value > 1 at offset 4, else
 * Hit_Enable); state 3 instead when bestiaryPreview is set.
 */
void EnemySkeletonKnightCreate(struct EwramData_EntityData *entity)
{
    if (!SkeletonKnight_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = SKELETON_KNIGHT_STATE_ANIM_ONLY;
        return;
    }
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, SkeletonKnight_DamagePlayerOnContact, SkeletonKnight_OnHit);
    else
        Hit_Enable(entity, 8, SkeletonKnight_DamagePlayerOnContact, SkeletonKnight_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->state = SKELETON_KNIGHT_STATE_WALK;
}

/**
 * @brief 8ECF8 | Enemy walk AI like DeadCrusader_UpdateWalk without the crouch: wakes
 * on the player box 0xB4 x 0x60 (or Enemy_IsDamaged), walks towards (state 1)
 * / away from (state 2) the player at half speed, turning every 60 frames
 * and on walls (Entity_ClampXToRoom); attacks (state = 1) at random when the
 * player is in the 0x50 box.
 */
void SkeletonKnight_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 state = entity->subState;
    s32 speed;
    s32 speed2;
    s32 res;
    s32 found;
    s32 m;
    s32 found2;

    switch (state)
    {
    case 0:
        if (Target_IsInRect((s16)(entity->posX.part16.integer - 0x5A), (s16)(entity->posY.part16.integer - 0x30), 0xB4, 0x60) || Enemy_IsDamaged(entity))
        {
            entity->subState++;
            entity->timer = state;
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
        }
        break;
    case 1:
        if (entity->timer++ > 0x3B)
        {
            entity->timer = 0;
            entity->flipX = Entity_IsTargetRight(entity);
        }
        if (entity->flipX)
            entity->velX.whole = 0x8000;
        else
            entity->velX.whole = -0x8000;
        speed = 0x10000;
        res = Entity_MoveWithCollision(entity, speed, -0x200000, 0x80000, 8);
        if (!(res & 4))
            break;
        found = Target_IsInRect((s16)(entity->posX.part16.integer - 0x28), (s16)(entity->posY.part16.integer - 0x28), 0x50, 0x50);
        if (found)
        {
            if ((RandomNumberGenerator() & 0x7F) <= 0x40)
            {
                entity->state = SKELETON_KNIGHT_STATE_ATTACK;
                entity->subState = 0;
                entity->timer = 0;
                break;
            }
            entity->flipX = Entity_IsTargetRight(entity);
            entity->subState = 2;
            entity->timer = 0;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x2800;
            break;
        }
        found2 = Entity_ClampXToRoom(entity, 1);
        if (found2)
        {
            entity->flipX = Entity_IsTargetRight(entity);
            entity->subState = 2;
            entity->timer = found;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x2800;
            break;
        }
        if (EntityIsOutsideScreen(entity, 4))
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->subState = found2;
        }
        break;
    case 2:
        if (entity->timer++ > 0x3B)
        {
            entity->timer = 0;
            entity->flipX = Entity_IsTargetRight(entity);
        }
        if (entity->flipX)
            entity->velX.whole = -0x8000;
        else
            entity->velX.whole = 0x8000;
        speed2 = 0x10000;
        res = Entity_MoveWithCollision(entity, speed2, -0x200000, 0x80000, 8);
        if (!(res & 4))
            break;
        if (!Target_IsInRect((s16)(entity->posX.part16.integer - 0x3C), (s16)(entity->posY.part16.integer - 0x28), 0x78, 0x50) || entity->timer > 0x3B)
        {
            entity->flipX = Entity_IsTargetRight(entity);
            entity->subState = 1;
            entity->timer = 0;
            entity->velY.whole = speed2;
            entity->accelY.whole = 0x2800;
            break;
        }
        if (Entity_ClampXToRoom(entity, 1) || (m = res & 3) != 0)
        {
            entity->flipX = Entity_IsTargetRight(entity);
            entity->subState = 1;
            entity->timer = 0;
            entity->velY.whole = speed2;
            entity->accelY.whole = 0x2800;
            break;
        }
        if (Target_IsInRect((s16)(entity->posX.part16.integer - 0x28), (s16)(entity->posY.part16.integer - 0x28), 0x50, 0x50) && entity->timer > 0x3B
            && (RandomNumberGenerator() & 0x7F) <= 0x40)
        {
            entity->state = SKELETON_KNIGHT_STATE_ATTACK;
            entity->subState = m;
            entity->timer = m;
        }
        break;
    }
}

/**
 * @brief 8EFD8 | Enemy attack sub-state (file 0x0808ECF8): faces the
 * player and plays animation 2, after 44 frames animation 3, then
 * animation 4 with song 0x85 on frame 11, after 30 frames animation 3 for
 * 20 more frames; then back to state 0 (animation 1, velY 0x10000, accelY
 * 0x2800).
 */
void SkeletonKnight_UpdateAttack(struct EwramData_EntityData *entity)
{
    u8 sub = entity->subState;

    switch (sub)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->subState++;
        entity->timer = sub;
        break;
    case 1:
        if (entity->timer++ > 0x2B)
        {
            Anim_StartIndexed(entity, 3, 0, 0);
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 2:
        if (entity->timer == 0xB)
            PlaySong(SE_SWING);
        if (entity->timer++ > 0x1D)
        {
            Anim_StartIndexed(entity, 4, 0, 0);
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 3:
        if (entity->timer++ > 0x13)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->state = SKELETON_KNIGHT_STATE_WALK;
            entity->subState = 1;
            entity->timer = 0;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
        }
        break;
    }
}

/* SkeletonKnight_OnHit (hit with HP / death), generated with a template script from the
 * template of WingedSkeleton_OnHit and checked byte for byte. */

/**
 * @brief 8F0A4 | Hit: flash (hitFlashTimer = 8), subtract the damage from the HP
 * and show it; at HP <= 0 die (Enemy_DropRewards, song 0x72, palette into
 * 0x53E bits 3-6, timer 0x28, state 2). Same template as WingedSkeleton_OnHit.
 */
void SkeletonKnight_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, (struct EwramData_EntityData *)attacker, hit, Player_CalculateAttackPower(hit[2]));

    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;

        // (s16)damage, written so that agbcc sign-extends after the HP load
        damage <<= 16;
        damage >>= 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, (s16)damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x72);
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->state = SKELETON_KNIGHT_STATE_FRAGMENT_DEATH;
        entity->subState = 0;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   SkeletonKnight_LoadGraphics = LegionWalkingCorpse_LoadGraphics
 *   SkeletonKnight_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */

/**
 * @brief 8F120 | Same as LegionWalkingCorpse_LoadGraphics.
 */
u8 SkeletonKnight_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sDeadCrusaderSkeletonKnightGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sDeadCrusaderSkeletonKnightPalette, 0);
    Sprite_Attach(entity, sDeadCrusaderSkeletonKnightSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 8F16C | Same as Enemy_DamagePlayerOnContact.
 */
void SkeletonKnight_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/* EnemySkeletonKnightUpdate. */

/**
 * @brief 8F198 | Skeleton Knight update: unless Enemy_PreUpdate handles the
 * frame, run the state (0 SkeletonKnight_UpdateWalk, 1 SkeletonKnight_UpdateAttack, 2 Enemy_UpdateFragmentDeath,
 * 3 only animate) and then Entity_AnimateWithBoxes / Enemy_CheckHitPlayer.
 */
void EnemySkeletonKnightUpdate(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (Enemy_PreUpdate(entity))
        return;
    switch (entity->state)
    {
        case SKELETON_KNIGHT_STATE_WALK:
            SkeletonKnight_UpdateWalk(entity);
            break;
        case SKELETON_KNIGHT_STATE_ATTACK:
            SkeletonKnight_UpdateAttack(entity);
            break;
        case SKELETON_KNIGHT_STATE_FRAGMENT_DEATH:
            Enemy_UpdateFragmentDeath(entity);
            break;
        case SKELETON_KNIGHT_STATE_ANIM_ONLY:
            Anim_Update(entity);
            return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}
