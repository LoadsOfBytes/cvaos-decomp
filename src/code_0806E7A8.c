/* Valkyrie idle / approach state. */
#include "structs/ewram.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "code_08001194.h"

/*
 * Valkyrie spear attack (Valkyrie_UpdateAttack): frame counter of the current
 * attack step. Token-identical expansion.
 */
#define VALKYRIE_ATTACK_FRAME(e) e->unk_4F4.whole

extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern void EntityMove(struct EwramData_EntityData *);
extern u8 sValkyrieGfx[], sValkyriePalette[], sValkyrieSpriteSheet[];
extern s32 ObjPalette_FindOrLoadVariant(void *, s32);
extern void Entity_DrawWithOffsetCopy(struct EwramData_EntityData *);
extern u16 Player_CalculateAttackPower(u16);
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s16, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);

extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_IsWithinDistance(struct EwramData_EntityData *, s32);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern s32 Entity_MoveTowardPoint(struct EwramData_EntityData *, s32, s32, s32);

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
            VALKYRIE_ATTACK_FRAME(entity) = 0;
        }
        break;
    case 5:
        EntityAccelerateAndMove(entity, 0x30000);
        EntityDampVelocityX(entity, 0x20);
        if (!Target_IsWithinDistance(entity, 0x780000))
        {
            entity->subState++;
            VALKYRIE_ATTACK_FRAME(entity) = 0;
            entity->accelY.whole = 0;
        }
        if (++VALKYRIE_ATTACK_FRAME(entity) > 0x13)
        {
            entity->subState++;
            VALKYRIE_ATTACK_FRAME(entity) = 0;
            entity->accelY.whole = 0;
        }
        break;
    case 6:
        entity->flipX = Entity_IsTargetRight(entity);
        EntityDampVelocityX(entity, 8);
        EntityDampVelocityY(entity, 8);
        EntityAccelerateAndMove(entity, 0x30000);
        if (++VALKYRIE_ATTACK_FRAME(entity) > 0x2F)
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

s32 Valkyrie_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics = ObjGfx_LoadResource(sValkyrieGfx, 0);
    u16 x;
    s32 y;
    s32 room;
    if (graphics < 0) return 0;
    if (entity->unk_518.part8.enemyId == 0x29)
        entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sValkyriePalette, 0);
    else
        entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sValkyriePalette, 1);
    Sprite_Attach(entity, sValkyrieSpriteSheet, graphics, entity->unk_510.part8.basePalette);
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    room = GetRoomFromMapPosition((s16)x, (s16)y);
    if (GetAreaFromMapPosition((s16)x, (s16)y) == 4 && (u32)(room - 0xC) <= 1)
        entity->drawFunc = (u32 *)Entity_DrawWithOffsetCopy;
    else
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
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
    s32 damage = Hit_ResolveDamage(entity, (s32)attacker, hitData, (s16)Player_CalculateAttackPower(hitData[2]));
    entity->unk_510.part8.hitFlashTimer = 8;
    {
        u16 hp = entity->unk_518.part16.hp;
        damage <<= 16;
        damage >>= 16;
        entity->unk_518.part16.hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)entity->unk_518.part16.hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x1CD);
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->timer = 0x28;
        entity->state = 2;
        entity->accelX.whole = 0;
        entity->accelY.whole = 0x1000;
    }
    else if (entity->unk_518.part8.enemyId == 0x29
             && (u8)(hit->type - 3) <= 1 && entity->state == 1)
    {
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->state = 3;
        entity->subState = 0;
    }
}
