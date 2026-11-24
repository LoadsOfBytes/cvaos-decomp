/*
 * Valkyrie.
 */
#include "valkyrie.h"
#include "core.h"
#include "graphics.h"
#include "gameplay.h"
#include "structs/ewram.h"
#include "entity_util.h"
#include "sound.h"
#include "bg_and_collision.h"
#include "player_core.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "hitbox.h"
#include "damage_numbers.h"
#include "entity_hit.h"

u8 Valkyrie_LoadGraphics(struct EwramData_EntityData *entity);
void Valkyrie_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void Valkyrie_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hitData);

/* EnemyValkyrieCreate 0x0806E714..0x0806E7A7. */

/**
 * @brief 6E714 | Valkyrie create: load the graphics (delete the
 * enemy when that fails), start the animation and set up the hit boxes
 * (Hit_EnableContact when a box record has a value > 1 at offset 4, else
 * Hit_Enable), layer 3; state 4 instead when bestiaryPreview is set.
 */
void EnemyValkyrieCreate(struct EwramData_EntityData *entity)
{
    if (!Valkyrie_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        return;
    }
    entity->spriteLayer = 3;
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, Valkyrie_DamagePlayerOnContact, Valkyrie_OnHit);
    else
        Hit_Enable(entity, 8, Valkyrie_DamagePlayerOnContact, Valkyrie_OnHit);
    Entity_UpdateBoxes(entity);
    entity->state = 0;
}

/* Valkyrie idle / approach state. */

/**
 * @brief Approach state machine (state 0): face the target and wait until it is
 * within 0x78 px or the Valkyrie is damaged, then fly to a point 0x40 px
 * beside the target and start the attack (state 1).
 */
void Valkyrie_UpdateApproach(struct EwramData_EntityData *entity)
{
    s32 x;
    s32 difference;
    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        if (Target_IsWithinDistance(entity, 0x780000) || Enemy_IsDamaged(entity))
            entity->subState = 1;
        break;
    case 1:
        entity->flipX = Entity_IsTargetRight(entity);
        if (entity->flipX)
            x = Target_GetX(entity) - 0x400000;
        else
            x = Target_GetX(entity) + 0x400000;
        if (Entity_MoveTowardPoint(entity, x, Target_GetY(entity) - 0x300000, 0x20000))
        {
            entity->state = 1;
            entity->subState = 0;
        }
        else
        {
            difference = Target_GetX(entity) - entity->posX.whole;
            if (difference < 0) difference = -difference;
            if (difference <= 0x3BFFFF && entity->posY.whole < Target_GetY(entity) - 0x200000)
            {
                difference = Target_GetY(entity) - entity->posY.whole;
                if (difference < 0) difference = -difference;
                if (difference <= 0x3BFFFF)
                {
                    entity->state = 1;
                    entity->subState = 0;
                }
            }
        }
        break;
    }
}

/**
 * @brief Attack state machine (state 1): spear thrust (animation 1) with damped
 * flight, then back to state 0.
 */
void Valkyrie_UpdateAttack(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->subState++;
        entity->accelX.whole = 0;
        entity->accelY.whole = 0;
        entity->timer = 0;
        /* fall through */
    case 1:
        EntityDampVelocityX(entity, 6);
        EntityDampVelocityY(entity, 6);
        EntityAccelerateAndMove(entity, 0x30000);
        if (entity->animStep == 2)
        {
            if (entity->timer++ > 0xF)
            {
                entity->animTimer = 0x64;
                entity->subState++;
            }
            else entity->animTimer = 0;
        }
        break;
    case 2:
        EntityDampVelocityX(entity, 6);
        EntityDampVelocityY(entity, 6);
        EntityAccelerateAndMove(entity, 0x30000);
        if (entity->animStep <= 2) break;
        entity->subState++;
        PlaySong(0x87);
        if (entity->flipX) entity->velX.whole = 0x30000;
        else entity->velX.whole = -0x30000;
        entity->velY.whole = 0x28000;
        entity->accelX.whole = 0;
        entity->accelY.whole = 0;
        /* fall through */
    case 3:
        EntityAccelerateAndMove(entity, 0x30000);
        if (entity->animStep > 7)
        {
            entity->subState++;
            entity->accelY.whole = -0x4000;
        }
        break;
    case 4:
        EntityAccelerateAndMove(entity, 0x30000);
        if (entity->animDone)
        {
            entity->subState++;
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->ext.valkyrie.attackFrame = 0;
        }
        break;
    case 5:
        EntityAccelerateAndMove(entity, 0x30000);
        EntityDampVelocityX(entity, 0x20);
        if (!Target_IsWithinDistance(entity, 0x780000))
        {
            entity->subState++;
            entity->ext.valkyrie.attackFrame = 0;
            entity->accelY.whole = 0;
        }
        if (++entity->ext.valkyrie.attackFrame > 0x13)
        {
            entity->subState++;
            entity->ext.valkyrie.attackFrame = 0;
            entity->accelY.whole = 0;
        }
        break;
    case 6:
        entity->flipX = Entity_IsTargetRight(entity);
        EntityDampVelocityX(entity, 8);
        EntityDampVelocityY(entity, 8);
        EntityAccelerateAndMove(entity, 0x30000);
        if (++entity->ext.valkyrie.attackFrame > 0x2F)
        {
            entity->state = 0;
            entity->subState = 1;
        }
        break;
    }
}

/**
 * @brief Recoil state machine (state 3): fly up and away from the target, slowing
 * down, then resume.
 */
void Valkyrie_UpdateRecoil(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity) ^ 1;
        if (entity->flipX) entity->velX.whole = 0x30000;
        else entity->velX.whole = -0x30000;
        entity->velY.whole = -0x30000;
        entity->subState++;
        break;
    case 1:
        EntityDampVelocityX(entity, 0xC);
        EntityDampVelocityY(entity, 0xC);
        EntityMove(entity);
        if (entity->velY.whole > -0x14000)
            entity->flipX = Entity_IsTargetRight(entity);
        if (entity->velY.whole > -0x2000)
        {
            entity->state = 0;
            entity->subState = 0;
        }
        break;
    }
}

u8 Valkyrie_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics = ObjGfx_LoadResource(sValkyrieGfx, 0);
    u16 x;
    s32 y;
    s32 room;
    if (graphics < 0) return 0;
    if (entity->enemyId == 0x29)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sValkyriePalette, 0);
    else
        entity->basePalette = ObjPalette_FindOrLoadVariant(sValkyriePalette, 1);
    Sprite_Attach(entity, sValkyrieSpriteSheet, graphics, entity->basePalette);
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    room = GetRoomFromMapPosition((s16)x, (s16)y);
    if (GetAreaFromMapPosition((s16)x, (s16)y) == 4 && (u32)(room - 0xC) <= 1)
        entity->drawFunc = Entity_DrawWithOffsetCopy;
    else
        entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief Hit: resolve, flash, subtract and show the damage; at HP <= 0 drop the
 * rewards and die (song 0x1CD, animation 2, state 2); type 0x29 hit by a
 * hit of type 3 / 4 during the attack recoils (state 3).
 */
void Valkyrie_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hitData)
{
    struct EntityHit *hit = &attacker->hit;
    s32 damage = Hit_ResolveDamage(entity, attacker, hitData, Player_CalculateAttackPower(hitData[2]));
    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;
        damage <<= 16;
        damage >>= 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, (s16)damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x1CD);
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->state = 2;
        entity->accelX.whole = 0;
        entity->accelY.whole = 0x1000;
    }
    else if (entity->enemyId == 0x29
             && (u8)(hit->type - 3) <= 1 && entity->state == 1)
    {
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->state = 3;
        entity->subState = 0;
    }
}

/* Valkyrie hit effect: full strength during animation 1, frames 4..6. */

/**
 * @brief Contact: Player_ApplyContactDamage with half the attack, the full attack
 * during the thrust frames (state 1, animation 1, steps 4..6).
 */
void Valkyrie_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage effect;
    effect.attack = entity->attack >> 1;
    if (entity->state == 1 && entity->animScriptIndex == 1
        && (u8)(entity->animStep - 4) <= 2)
        effect.attack = entity->attack;
    effect.attributes = 0;
    effect.unused = 0;
    effect.reaction = 0;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
}

void EnemyValkyrieUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    if (Entity_IsInRect(entity, -0x10, 0, 0x110, 0xB0))
        Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 2)
            Hit_CheckWithPlayer(&result, entity);
        return;
    }
    switch (entity->state)
    {
    case 0: Valkyrie_UpdateApproach(entity); break;
    case 1: Valkyrie_UpdateAttack(entity); break;
    case 2:
        EntityDampVelocityX(entity, 0xA);
        EntityAccelerateAndMove(entity, 0x30000);
        Enemy_UpdateFragmentDeath(entity);
        break;
    case 3: Valkyrie_UpdateRecoil(entity); break;
    case 4:
        Anim_Update(entity);
        return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}
