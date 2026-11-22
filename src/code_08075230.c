#include "code_08009A0.h"
#include "structs/ewram.h"

extern void WingedSkeletonShot_Update(struct EwramData_EntityData *entity);
extern s32 WingedSkeletonShot_LoadGraphics(struct EwramData_EntityData *entity, struct EwramData_EntityData *child);
extern void WingedSkeletonShot_EnableHitBoxes(struct EwramData_EntityData *entity);

/**
 * @brief 75230 | Spawn a WingedSkeletonShot_Update projectile (Winged Skeleton graphics) from entity (its owner in
 * unk_4F4): graphics via WingedSkeletonShot_LoadGraphics (deleted if that fails, though setup
 * continues), WingedSkeletonShot_EnableHitBoxes, layer + 1, facing of the owner, 0x15 px in
 * front and 9 px above it, velX +-0x20000, velY 0x20000, no acceleration;
 * copies unk_51B, unk_510 and attack from the owner.
 */
void WingedSkeleton_ThrowShot(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, (u32 *)WingedSkeletonShot_Update);

    if (child == NULL)
        return;
    if (!(u8)WingedSkeletonShot_LoadGraphics(entity, child))
        EntityDelete(child);
    child->scrollWithBg = 1;
    WingedSkeletonShot_EnableHitBoxes(child);
    child->spriteLayer = entity->spriteLayer + 1;
    ENTITY_CREATOR(child) = entity;
    child->flipX = entity->flipX;
    if (entity->flipX)
    {
        child->posX.part16.integer = entity->posX.part16.integer + 0x15;
        child->velX.whole = 0x20000;
    }
    else
    {
        child->posX.part16.integer = entity->posX.part16.integer - 0x15;
        child->velX.whole = -0x20000;
    }
    child->posY.part16.integer = entity->posY.part16.integer - 9;
    child->velY.whole = 0x20000;
    child->accelX.whole = 0;
    child->accelY.whole = 0;
    child->unk_518.part8.spawnIndex = entity->unk_518.part8.spawnIndex;
    child->unk_510.part8.basePalette = entity->unk_510.part8.basePalette;
    child->attack = entity->attack;
}
