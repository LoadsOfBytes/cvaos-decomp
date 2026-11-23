/* Stolas_SpawnBeamImpact (Stolas hit box child). */
#include "code_08009A0.h"
#include "stolas_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void StolasBeamImpact_Update(struct EwramData_EntityData *entity);
extern u8 sStolasGfx[];
extern u8 sStolasSpriteSheet[];

/**
 * @brief 9C0C8 | Spawn a StolasBeamImpact_Update child (slots 0x1B..0x48) at
 * (offsetX, offsetY) px from entity (x mirrored by its facing) with the
 * already loaded graphics sStolasGfx (deleted when they are not loaded),
 * frame 0x23, layer 0xA, entity's palette, type unk_51A and facing; the
 * parent is kept in STOLAS_EFFECT_OWNER.
 */
void Stolas_SpawnBeamImpact(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, (u32 *)StolasBeamImpact_Update);
    s32 gfx;

    if (child == NULL)
        return;
    gfx = ObjGfx_Find(sStolasGfx);
    if (gfx < 0)
    {
        EntityDelete(child);
        return;
    }
    Sprite_Attach(child, sStolasSpriteSheet, gfx, entity->unk_510.part8.basePalette);
    child->animFrame = 0x23;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    child->scrollWithBg = 1;
    child->spriteLayer = 0xA;
    child->unk_510.part8.basePalette = entity->unk_510.part8.basePalette;
    child->unk_518.part8.enemyId = entity->unk_518.part8.enemyId;
    STOLAS_EFFECT_OWNER(child) = entity;
    child->flipX = entity->flipX;
    if (entity->flipX)
        child->posX.whole = entity->posX.whole + (offsetX << 16);
    else
        child->posX.whole = entity->posX.whole - (offsetX << 16);
    child->posY.whole = entity->posY.whole + (offsetY << 16);
}
