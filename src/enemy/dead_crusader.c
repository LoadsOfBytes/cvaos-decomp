/*
 * Dead Crusader.
 */
#include "core.h"
#include "player_core.h"
#include "graphics.h"
#include "entity_util.h"
#include "structs/ewram.h"
#include "gameplay.h"
#include "sound.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "hitbox.h"
#include "damage_numbers.h"
#include "entity_motion.h"

extern s32 Target_IsInRect(s16, s16, s16, s16);
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);

void DeadCrusader_UpdateWalk(struct EwramData_EntityData *entity);
void DeadCrusader_UpdateAttack(struct EwramData_EntityData *entity);
u8 DeadCrusader_LoadGraphics(struct EwramData_EntityData *entity);
void DeadCrusader_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void DeadCrusader_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);

/**
 * @brief 8F1F4 | Dead Crusader update: clears ext.deadCrusader.guarding, wakes when on screen
 * (Player_TrackEntity; skipped entirely while off screen unless dying), hidden
 * off screen (hidden), Enemy_UpdateHitFlash unless dying, states 0 / 1
 * (DeadCrusader_UpdateWalk / DeadCrusader_UpdateAttack), 2 dying (Enemy_UpdateFragmentDeath), 3 animation only;
 * then Entity_AnimateWithBoxes and Enemy_CheckHitPlayer.
 */
void EnemyDeadCrusaderUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];

    entity->ext.deadCrusader.guarding = 0;
    if (entity->state != 2)
    {
        if (EntityIsOutsideScreen(entity, 4))
            return;
        if (Entity_IsInRect(entity, -16, 0, 272, 176))
            Player_TrackEntity(entity);
    }
    if (EntityIsOutsideScreen(entity, 4))
        entity->hidden = 1;
    else
        entity->hidden = 0;
    if (entity->state != 2)
        Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity) && entity->state != 2)
    {
        Hit_CheckWithPlayer((struct HitResult *)collision, entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        DeadCrusader_UpdateWalk(entity);
        break;
    case 1:
        DeadCrusader_UpdateAttack(entity);
        break;
    case 2:
        Enemy_UpdateFragmentDeath(entity);
        break;
    case 3:
        Anim_Update(entity);
        return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   EnemyDeadCrusaderCreate = EnemySkeletonKnightCreate */

/**
 * @brief 8F2D0 | Same as EnemySkeletonKnightCreate.
 */
void EnemyDeadCrusaderCreate(struct EwramData_EntityData *entity)
{
    if (!DeadCrusader_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        return;
    }
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, DeadCrusader_DamagePlayerOnContact, DeadCrusader_OnHit);
    else
        Hit_Enable(entity, 8, DeadCrusader_DamagePlayerOnContact, DeadCrusader_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->state = 0;
}

/**
 * @brief 8F364 | Enemy walk AI like Persephone_UpdateWalk (subState): wakes on the
 * player box 0xB4 x 0x60 (or Enemy_IsDamaged), walks towards (state 1) / away
 * from (state 2) the player at 0.75 px, turning every 60 frames; landing
 * with Player_GetFocusedActionFlags & 0x360 makes it stop and crouch (animation 7, state 5,
 * unk_505 = 1) for 60 frames. Attacks (state = 1) at random when the
 * player is in the 0x50 box.
 */
void DeadCrusader_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 speed;
    s32 res;
    s32 found;
    s32 t;
    s32 m;

    switch (entity->subState)
    {
    case 0:
        if (Target_IsInRect(entity->posX.part16.integer - 0x5A, entity->posY.part16.integer - 0x30, 0xB4, 0x60) || Enemy_IsDamaged(entity))
        {
            entity->subState++;
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
            entity->velX.whole = 0xC000;
        else
            entity->velX.whole = -0xC000;
        speed = 0x10000;
        res = Entity_MoveWithCollision(entity, speed, -0x200000, 0x80000, 8);
        if (!(res & 4))
            break;
        res = Player_GetFocusedActionFlags() & 0x360;
        if (res)
        {
            Anim_StartIndexed(entity, 7, 0, 0);
            entity->subState = 5;
            entity->timer = 0;
            entity->ext.deadCrusader.guarding = 1;
            entity->velX.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x2800;
            break;
        }
        found = Target_IsInRect(entity->posX.part16.integer - 0x28, entity->posY.part16.integer - 0x28, 0x50, 0x50);
        if (found)
        {
            if ((RandomNumberGenerator() & 0x7F) <= 0x40)
            {
                entity->state = 1;
                entity->subState = res;
                entity->timer = res;
                break;
            }
            entity->subState = 2;
            entity->timer = res;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x2800;
            break;
        }
        res = Entity_ClampXToRoom(entity, 1);
        if (res)
        {
            entity->subState = 2;
            entity->timer = found;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x2800;
            break;
        }
        if (EntityIsOutsideScreen(entity, 4))
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->subState = res;
        }
        break;
    case 2:
        if (entity->timer++ > 0x3B)
        {
            entity->timer = 0;
            entity->flipX = Entity_IsTargetRight(entity);
        }
        if (entity->flipX)
            entity->velX.whole = -0xC000;
        else
            entity->velX.whole = 0xC000;
        res = Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x80000, 8);
        if (!(res & 4))
            break;
        t = Player_GetFocusedActionFlags() & 0x360;
        if (t)
        {
            Anim_StartIndexed(entity, 7, 0, 0);
            entity->subState = 5;
            entity->timer = 0;
            entity->ext.deadCrusader.guarding = 1;
            entity->velX.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
            break;
        }
        if (!Target_IsInRect(entity->posX.part16.integer - 0x3C, entity->posY.part16.integer - 0x28, 0x78, 0x50) || entity->timer > 0x3B)
        {
            entity->flipX = Entity_IsTargetRight(entity);
            entity->subState = 1;
            entity->timer = t;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
            break;
        }
        if (Entity_ClampXToRoom(entity, 1) || (m = res & 3) != 0)
        {
            entity->flipX = Entity_IsTargetRight(entity);
            entity->subState = 1;
            entity->timer = t;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
            break;
        }
        if (Target_IsInRect(entity->posX.part16.integer - 0x28, entity->posY.part16.integer - 0x28, 0x50, 0x50) && entity->timer > 0x3B
            && (RandomNumberGenerator() & 0x7F) <= 0x40)
        {
            entity->state = 1;
            entity->subState = m;
            entity->timer = m;
        }
        break;
    case 5:
        entity->flipX = Entity_IsTargetRight(entity);
        entity->ext.deadCrusader.guarding = 1;
        speed = 0x10000;
        res = Entity_MoveWithCollision(entity, speed, -0x200000, 0x80000, 8);
        if (!(res & 4))
            break;
        if (entity->timer++ > 0x3B)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->subState = 1;
            entity->timer = 0;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x2800;
        }
        break;
    }
}

/**
 * @brief 8F6E8 | Enemy attack sub-state (as SkeletonKnight_UpdateAttack, file
 * 0x0808F364): animation 2 facing the player, after 38 frames animation 3
 * and with 1/2 chance animation 4 (song 0x85 on frame 11, 30 frames) then
 * 20 frames of animation 3, or else animation 6 repeated four times (song
 * 0x85 on frame 11, 12 frames each); then back to state 0.
 */
void DeadCrusader_UpdateAttack(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->subState++;
        entity->timer = 0;
        break;
    case 1:
        if (entity->timer++ > 0x25)
        {
            Anim_StartIndexed(entity, 3, 0, 0);
            entity->timer = 0;
            if ((RandomNumberGenerator() & 0x7F) <= 0x3F)
                entity->subState++;
            else
                entity->subState = 5;
        }
        break;
    case 2:
        if (entity->timer == 0xB)
            PlaySong(0x85);
        if (entity->timer++ > 0x1D)
        {
            Anim_StartIndexed(entity, 4, 0, 0);
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 3:
        if (entity->timer++ <= 0x13)
            break;
        goto done;
    case 4:
        break;
    case 5:
        if (entity->timer == 0xB)
            PlaySong(0x85);
        if (entity->timer++ > 0xB)
        {
            Anim_StartIndexed(entity, 6, 0, 0);
            entity->subState++;
            entity->stage = 0;
            entity->timer = 0;
        }
        break;
    case 6:
        if (entity->timer++ > 0xB)
        {
            if (entity->stage++ > 2)
            {
            done:
                Anim_StartIndexed(entity, 1, 0, 1);
                entity->state = 0;
                entity->subState = 1;
                entity->timer = 0;
                entity->velY.whole = 0x10000;
                entity->accelY.whole = 0x2800;
            }
            else
            {
                Anim_StartIndexed(entity, 6, 0, 0);
                entity->timer = 0;
            }
        }
        break;
    }
}

/**
 * @brief Hit: while crouching behind the shield (ext.deadCrusader.guarding) take no damage (sound
 * 0x69), else resolve the damage; flash, subtract and show it; at HP <= 0
 * drop the rewards and die (song 0x72, 0x28 frames, state 2).
 */
void DeadCrusader_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 signedDamage;
    u16 damage = Player_CalculateAttackPower(hit[2]);
    if ((s8)entity->ext.deadCrusader.guarding != 0)
    {
        Hit_ResolveDamage(entity, (s32)attacker, hit, 0);
        damage = 0;
        PlaySong(0x69);
    }
    else damage = Hit_ResolveDamage(entity, (s32)attacker, hit, (s16)damage);
    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;
        signedDamage = damage;
        signedDamage <<= 16;
        signedDamage >>= 16;

        entity->hp = hp - signedDamage;
    }
    Entity_ShowDamage(entity, (s16)signedDamage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x72);
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->state = 2;
        entity->subState = 0;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   DeadCrusader_LoadGraphics = Object32_LoadGraphics
 *   DeadCrusader_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */

/**
 * @brief 8F8EC | Same as Object32_LoadGraphics.
 */
u8 DeadCrusader_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sDeadCrusaderSkeletonKnightGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sDeadCrusaderSkeletonKnightPalette, 1);
    Sprite_Attach(entity, sDeadCrusaderSkeletonKnightSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 8F938 | Same as Enemy_DamagePlayerOnContact.
 */
void DeadCrusader_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}
