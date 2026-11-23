/* PeepingEye_CreatePupil: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void PeepingEyePupil_Update(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *updateFunc);
extern u8 sPeepingEyeGfx[];
extern u8 sPeepingEyeSpriteSheet[];

/**
 * @brief 88D28 | Like EnemyPeepingEyeCreateEyeball, but frame 0x10.
 */
struct EwramData_EntityData *PeepingEye_CreatePupil(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = Entity_CreateObject((u32 *)PeepingEyePupil_Update);

    if (child != NULL)
    {
        s32 gfx;

        child->posX.whole = entity->posX.whole;
        child->posY.whole = entity->posY.whole;
        child->spriteLayer = entity->spriteLayer + 1;
        gfx = ObjGfx_Find(sPeepingEyeGfx);
        Sprite_Attach(child, sPeepingEyeSpriteSheet, gfx, entity->unk_510.part8.basePalette);
        child->animFrame = 0x10;
        child->ext.peepingEye.owner = entity;
        child->scrollWithBg = 0;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
    }
    return child;
}
