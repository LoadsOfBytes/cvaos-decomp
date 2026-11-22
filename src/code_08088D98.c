/* EnemyPeepingEyeCreateEyeball. */
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern struct EwramData_EntityData *Entity_CreateObject(u32 *updateFunc);
extern void PeepingEyeEyeball_Update(struct EwramData_EntityData *entity);
extern u8 sPeepingEyeGfx[];
extern u8 sPeepingEyeSpriteSheet[];

/**
 * @brief 88D98 | Create the eyeball (PeepingEyeEyeball_Update) at the Peeping Eye's
 * position, one layer above, with its palette, frame 0xF and the parent in
 * unk_500; returns it (NULL when no slot is free).
 */
struct EwramData_EntityData *EnemyPeepingEyeCreateEyeball(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = Entity_CreateObject((u32 *)PeepingEyeEyeball_Update);

    if (child != NULL)
    {
        s32 gfx;

        child->posX.whole = entity->posX.whole;
        child->posY.whole = entity->posY.whole;
        child->spriteLayer = entity->spriteLayer + 1;
        gfx = ObjGfx_Find(sPeepingEyeGfx);
        Sprite_Attach(child, sPeepingEyeSpriteSheet, gfx, entity->unk_510.part8.basePalette);
        child->animFrame = 0xF;
        child->unk_500.unk_500_entity = entity;
        child->scrollWithBg = 0;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
    }
    return child;
}
