#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void StolasCastEffect_Update(struct EwramData_EntityData *entity);
extern u8 sStolasGfx[], sStolasSpriteSheet[];

/**
 * @brief 9BD5C | Create an effect StolasCastEffect_Update of the owner (file
 * 0x0809B9D4) at (dx, dy) from it: graphics sStolasSpriteSheet / sStolasGfx
 * with the owner's palette, frame 0xC, boxes off, layer one below the
 * owner, owner in unk_500, its facing; own palette with blending and an
 * affine matrix with angle unk_4F8 - 0x8000. Deleted when the graphics or
 * the palette cannot be allocated.
 */
void Stolas_SpawnCastEffect(struct EwramData_EntityData *entity, s32 dx, s32 dy)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, (u32 *)StolasCastEffect_Update);
    s32 tiles;
    u16 palette;

    if (child == NULL)
        return;
    tiles = ObjGfx_Find(sStolasGfx);
    if (tiles < 0)
        goto delete;
    Sprite_Attach(child, sStolasSpriteSheet, tiles, entity->basePalette);
    child->animFrame = 0xC;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    child->scrollWithBg = 1;
    child->spriteLayer = entity->spriteLayer - 1;
    child->basePalette = entity->basePalette;
    child->enemyId = entity->enemyId;
    child->ext.stolasEffect.owner = entity;
    child->flipX = entity->flipX;
    child->posX.whole = entity->posX.whole + dx;
    child->posY.whole = entity->posY.whole + dy;
    palette = ObjAffine_Alloc(1);
    if ((s16)palette < 0)
    {
    delete:
        EntityDelete(child);
        return;
    }
    child->affineIndex = palette;
    child->affine = 1;
    ObjAffine_Set(child->ext.stolasEffect.castAngle - 0x8000, 0x10000, 0x10000, child->flipX, child->flipY, child->affineIndex);
}
