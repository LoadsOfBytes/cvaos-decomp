#include "code_08009A0.h"
#include "code_08032444.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

/*
 * Soul container: palette of the soul variant and the soul sprite entity it
 * shows inside. Token-identical expansion.
 */
#define SOUL_CONTAINER_PALETTE(e) e->unk_500.part8.unk_500
#define SOUL_CONTAINER_SOUL(e) e->unk_4FC.unk_4FC_entity
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
        entity->unk_518.part8.enemyId = variant;
        entity->unk_514.part16.spawnParam1 = index;
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
        SOUL_CONTAINER_PALETTE(entity) = Soul_GetPickupPalette(variant);
        child = EntityCreateInRange(0x49, 0xCF, (u32 *)SoulPickup_UpdateContainerAnimation);
        if (child != NULL)
        {
            child->scrollWithBg = 1;
            child->spriteLayer = entity->spriteLayer;
            child->posX.whole = entity->posX.whole;
            child->posY.whole = entity->posY.whole;
            graphics = ObjGfx_Find(sCommonObjectGfx);
            Sprite_Attach(child, sCommonObjectSpriteSheet, graphics, (u8)SOUL_CONTAINER_PALETTE(entity));
            Anim_StartIndexed(child, 0x12, 0, 1);
            Anim_Update(child);
            child->drawFunc = (u32 *)Sprite_DrawEntity;
            SOUL_CONTAINER_SOUL(entity) = child;
        }
    }
    return entity;
}
