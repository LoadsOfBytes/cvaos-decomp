#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern void ManticoreTailTip_Update(struct EwramData_EntityData *);
extern void ManticoreTail_EnableHitBoxes(struct EwramData_EntityData *);
extern u8 sManticoreGfx[], sManticoreSpriteSheet[];
/**
 * @brief Create the tail tip of owner (affine sprite, contact damage through
 * ManticoreTail_EnableHitBoxes outside the bestiary preview); return it or
 * NULL when no affine slot is free.
 */
struct EwramData_EntityData *Manticore_CreateTailTip(struct EwramData_EntityData *owner)
{
    struct EwramData_EntityData *entity = Entity_CreateObject((u32 *)ManticoreTailTip_Update);
    struct EwramData **ewram = &gEwramData;
    if (entity != NULL)
    {
        s32 graphics;
        u16 slot;
        entity->spriteLayer = owner->spriteLayer + 1;
        graphics = ObjGfx_Find(sManticoreGfx);
        Sprite_Attach(entity, sManticoreSpriteSheet, graphics, owner->basePalette);
        entity->flipY = 1;
        entity->animFrame = 0;
        entity->enemyId = owner->enemyId;
        entity->hp = 1;
        entity->scrollWithBg = 0;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        ENTITY_OWNER(entity) = owner;
        entity->ext.manticoreTail.kind = 6;
        entity->ext.manticoreTail.link.value = 0x4000;
        entity->attack = owner->attack;
        slot = ObjAffine_Alloc(1);
        if ((s16)slot < 0) { EntityDelete(entity); return NULL; }
        entity->affineIndex = slot;
        entity->affine = 1;
        if (!(*ewram)->gameState.bestiaryPreview) ManticoreTail_EnableHitBoxes(entity);
        return entity;
    }
    return NULL;
}
