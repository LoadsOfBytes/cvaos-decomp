/* Spawn an effect 31 px in front of and 15 px above the owner (graphics
 * 081CBF54 / 082145BC in the owner palette, animation 5), one priority
 * level above it; updated by ManticoreEffect_Update. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void ManticoreEffect_Update(struct EwramData_EntityData *);
extern u8 sManticoreGfx[], sManticoreSpriteSheet[];

/**
 * @brief Spawn the muzzle flash 31 px in front of and 15 px above owner; it
 * vanishes after its animation (ManticoreEffect_Update).
 */
void Manticore_SpawnShotFlash(struct EwramData_EntityData *owner)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)ManticoreEffect_Update);
    s32 graphics;

    if (entity == NULL)
        return;
    entity->spriteLayer = owner->spriteLayer - 1;
    entity->flipX = owner->flipX;
    if (entity->flipX)
        entity->posX.part16.integer = owner->posX.part16.integer + 0x1F;
    else
        entity->posX.part16.integer = owner->posX.part16.integer - 0x1F;
    entity->posY.part16.integer = owner->posY.part16.integer - 0xF;
    graphics = ObjGfx_Find(sManticoreGfx);
    Sprite_Attach(entity, sManticoreSpriteSheet, graphics, owner->unk_510.part8.basePalette);
    Anim_StartIndexed(entity, 5, 0, 0);
    Anim_Update(entity);
    entity->scrollWithBg = 1;
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    entity->timer = 0;
}
