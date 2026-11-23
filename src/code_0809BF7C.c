/* Stolas_SpawnBeamSegment (Stolas hit box child). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void StolasBeamSegment_Update(struct EwramData_EntityData *entity);
extern u8 sStolasGfx[];
extern u8 sStolasSpriteSheet[];

/**
 * @brief 9BF7C | Spawn a StolasBeamSegment_Update child (slots 0x1B..0x48) at
 * (offsetX, offsetY) px from entity (x mirrored by its facing) with the
 * already loaded graphics sStolasGfx (deleted when they are not loaded),
 * frame 0x1F, layer 9, entity's palette, type enemyId and facing; the
 * parent is kept in ext.stolasEffect.owner.
 */
void Stolas_SpawnBeamSegment(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, (u32 *)StolasBeamSegment_Update);
    s32 gfx;

    if (child == NULL)
        return;
    gfx = ObjGfx_Find(sStolasGfx);
    if (gfx < 0)
    {
        EntityDelete(child);
        return;
    }
    Sprite_Attach(child, sStolasSpriteSheet, gfx, entity->basePalette);
    child->animFrame = 0x1F;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    child->scrollWithBg = 1;
    child->spriteLayer = 9;
    child->basePalette = entity->basePalette;
    child->enemyId = entity->enemyId;
    child->ext.stolasEffect.owner = entity;
    child->flipX = entity->flipX;
    if (entity->flipX)
        child->posX.whole = entity->posX.whole + (offsetX << 16);
    else
        child->posX.whole = entity->posX.whole - (offsetX << 16);
    child->posY.whole = entity->posY.whole + (offsetY << 16);
}
