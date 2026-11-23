/* Stolas_SpawnDebris (Stolas projectile). */
#include "code_08009A0.h"
#include "stolas_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void StolasDebris_Update(struct EwramData_EntityData *entity);
extern u8 sStolasGfx[];
extern u8 sStolasSpriteSheet[];

/**
 * @brief 9C224 | Spawn a StolasDebris_Update projectile (slots 0x1B..0x48) at
 * entity's position + (offsetX, offsetY) with velocity (velocityX, velocityY),
 * gravity 0x800, timer 40, the already loaded graphics sStolasGfx (deleted
 * when they are not loaded), frame 0x1E, one layer above, entity's palette,
 * type unk_51A and facing; the parent is kept in unk_500.
 */
void Stolas_SpawnDebris(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY, s32 velocityX, s32 velocityY)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, (u32 *)StolasDebris_Update);
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
    child->animFrame = 0x1E;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    child->scrollWithBg = 1;
    child->spriteLayer = entity->spriteLayer + 1;
    child->unk_510.part8.basePalette = entity->unk_510.part8.basePalette;
    child->unk_518.part8.enemyId = entity->unk_518.part8.enemyId;
    STOLAS_EFFECT_OWNER(child) = entity;
    child->flipX = entity->flipX;
    child->posX.whole = entity->posX.whole + offsetX;
    child->posY.whole = entity->posY.whole + offsetY;
    child->velX.whole = velocityX;
    child->velY.whole = velocityY;
    child->accelY.whole = 0x800;
    child->timer = 0x28;
}
