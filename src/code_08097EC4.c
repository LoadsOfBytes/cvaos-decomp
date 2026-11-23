/* CreakingSkull_AttachJoint (chain joint set-up) and CreakingSkull_ApplyResolvedDamage. */
#include "syscalls.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void Link_MarkEnemyDamaged(void);
extern void Player_SetSelectedTarget(struct EwramData_EntityData *entity);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);
extern void BossDefeatEffect_Spawn(s32 x, s32 y);
extern void BossRecoveryPickup_Spawn(s32 x, s32 y, s32 event, s32 delay, s32 special);

/**
 * @brief Bind a joint to its parent with supplied or measured length; mirror the target angle by facing, treating absolute values above 0xFFFF in magnitude as absolute angles.
 */
void CreakingSkull_AttachJoint(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length)
{
    struct EntityJointPart *parts = entity->ext.joint.parts;
    // &parts[partIndex], written as integer arithmetic (offset + base) as in
    // the original
    struct EntityJointPart *part = (struct EntityJointPart *)(partIndex * sizeof(struct EntityJointPart) + (u32)parts);
    s32 dx;
    s32 dy;
    s32 baseAngle;
    s32 angle;
    struct EntityJointPart *anglePart;

    part->anchorXOrSentinel = 0x80000000;
    part->anchorYOrPartIndex = anchorIndex;
    if (length != 0)
    {
        part->linkLength = length;
    }
    else
    {
        dx = parts[anchorIndex].offsetX - part->offsetX;
        dy = parts[anchorIndex].offsetY - part->offsetY;
        dx = FixedMultiply(dx, dx);
        dy = FixedMultiply(dy, dy);
        part->linkLength = (u32)(Sqrt(dx + dy) << 16) >> 8;
    }
    dx = parts[partIndex].offsetX - parts[anchorIndex].offsetX;
    dy = parts[partIndex].offsetY - parts[anchorIndex].offsetY;
    angle = ArcTan2(dx < 0 ? -(-dx >> 16) : dx >> 16, dy < 0 ? -(-dy >> 16) : dy >> 16);
    anglePart = (struct EntityJointPart *)(partIndex * sizeof(struct EntityJointPart) + (u32)parts);
    anglePart->swingAngle = baseAngle = (u16)angle + 0x10000;
    if ((angleSpec < 0 ? -angleSpec : angleSpec) > 0xFFFF)
    {
        if (entity->flipX)
        {
            if (angleSpec > 0x18000)
                anglePart->targetSwingAngle = 0x38000 - angleSpec;
            else
                anglePart->targetSwingAngle = 0x28000 - angleSpec;
        }
        else
        {
            anglePart->targetSwingAngle = angleSpec;
        }
    }
    else if (entity->flipX)
    {
        anglePart->targetSwingAngle = baseAngle - angleSpec;
    }
    else
    {
        anglePart->targetSwingAngle = baseAngle + angleSpec;
    }
}

/**
 * @brief Apply already-resolved damage and hit flash to the root. On
 * defeat drop rewards, enter death state 2 and optionally start boss
 * defeat effects and clear selected-target tracking.
 */
void CreakingSkull_ApplyResolvedDamage(struct EwramData_EntityData *entity, s32 unused1, s32 unused2, s32 damage)
{
    entity->hitFlashTimer = 8;
    entity->hp -= damage;
    Link_MarkEnemyDamaged();
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x6B);
        entity->palette = entity->basePalette;
        entity->timer = 0x64;
        entity->state = 2;
        entity->subState = 0;
        if (entity->spawnParam0 != 0)
        {
            Player_SetSelectedTarget(NULL);
            BossDefeatEffect_Spawn(entity->posX.whole, entity->posY.whole - 0x200000);
            BossRecoveryPickup_Spawn((s16)entity->posX.part16.integer, 0x20, 0xB, 0x60, 0);
        }
    }
}
