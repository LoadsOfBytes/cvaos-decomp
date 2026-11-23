#include "code_08009A0.h"
#include "code_08032444.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern void SoulPickup_UpdateContainer(struct EwramData_EntityData *);
extern void SoulPickup_UpdateContainerAnimation(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern u8 sCommonObjectGfx[], sCommonObjectSpriteSheet[];
/**
 * @brief Create the grounded soul container and optional colored animation child using the supplied soul kind/index; return NULL if the container allocation fails.
 */
struct EwramData_EntityData *SoulPickup_CreateContainer(s32 x, s32 y, s32 variant, s32 index)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)SoulPickup_UpdateContainer);
    if (entity != NULL)
    {
        s32 graphics;
        struct EwramData_EntityData *child;
        entity->scrollWithBg = 1;
        entity->spriteLayer = 0x13;
        entity->enemyId = variant;
        entity->spawnParam1 = index;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, 8);
        Anim_StartIndexed(entity, 0x13, 0, 1);
        Anim_Update(entity);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Hit_Enable(entity, 0, NULL, 0);
        entity->hit.flags |= 2;
        HitBox_SetBodyRaw(entity, 0x2010E0F8);
        entity->posX.part16.integer = x;
        entity->posY.part16.integer = y;
        Entity_SnapToGround(entity);
        entity->ext.soulContainer.palette = Soul_GetPickupPalette(variant);
        child = EntityCreateInRange(0x49, 0xCF, (u32 *)SoulPickup_UpdateContainerAnimation);
        if (child != NULL)
        {
            child->scrollWithBg = 1;
            child->spriteLayer = entity->spriteLayer;
            child->posX.whole = entity->posX.whole;
            child->posY.whole = entity->posY.whole;
            graphics = ObjGfx_Find(sCommonObjectGfx);
            Sprite_Attach(child, sCommonObjectSpriteSheet, graphics, (u8)entity->ext.soulContainer.palette);
            Anim_StartIndexed(child, 0x12, 0, 1);
            Anim_Update(child);
            child->drawFunc = (u32 *)Sprite_DrawEntity;
            entity->ext.soulContainer.soul = child;
        }
    }
    return entity;
}
