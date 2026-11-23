#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern void PickupSource_LoadIcon(s32);
extern void PickupSource_Draw(struct EwramData_EntityData *);
extern u8 sCommonObjectGfx[], sCommonObjectSpriteSheet[];
/**
 * @brief Load the source icon, attach shared graphics and animation, configure its body box and install the source draw callback.
 */
void PickupSource_Init(struct EwramData_EntityData *entity)
{
    s32 graphics;
    PickupSource_LoadIcon(entity->spawnParam0);
    entity->spriteLayer = 0x14;
    entity->flipX = 0;
    graphics = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, 7);
    Anim_StartIndexed(entity, 3, 0, 1);
    Anim_Update(entity);
    Hit_Enable(entity, 0, NULL, 0);
    entity->hit.flags |= 2;
    HitBox_SetBodyRaw(entity, 0x140CF4FA);
    entity->drawFunc = (u32 *)PickupSource_Draw;
}
