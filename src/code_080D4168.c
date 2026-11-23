#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, s32);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern void GrahamFirstForm_OnHit(struct EwramData_EntityData *, s32, u16 *);
extern void GrahamFirstForm_Update(struct EwramData_EntityData *);
extern u8 sGrahamPalette1[], sGrahamSpriteSheet1[], sGrahamGfx1[];

/**
 * @brief Create the initial linked combat entity, copy root identity and
 * configure its HP, graphics and callbacks. Return the entity in the
 * gameplay branch or NULL on allocation/graphics failure. Preserve the
 * bestiary branch with its existing unspecified return value.
 */
struct EwramData_EntityData *GrahamFirstForm_Create(struct EwramData_EntityData *owner)
{
    u8 *gfx2 = sGrahamPalette1;
    u8 *gfx3 = sGrahamSpriteSheet1;
    u8 *gfx1 = sGrahamGfx1;
    struct EwramData_EntityData *e = Entity_CreateObject((u32 *)GrahamFirstForm_Update);
    if (e == NULL) return NULL;
    if (!Entity_LoadGraphics(e, gfx1, gfx2, 0, gfx3))
    {
        EntityDelete(e);
        return NULL;
    }
    ENTITY_PARENT_INDEX(e) = ENTITY_INDEX(owner);
    e->spriteLayer = owner->spriteLayer - 1;
    e->flipX = owner->flipX;
    e->posX.whole = owner->posX.whole;
    e->posY.whole = owner->posY.whole;
    e->scrollWithBg = 1;
    e->state = 3;
    e->stage = 0;
    e->subState = 0;
    e->enemyFlags |= 4;
    e->hp = 1500;
    e->mp = owner->mp / 2;
    e->attack = 60;
    e->defense = 100;
    e->enemyId = owner->enemyId;
    e->spawnIndex = owner->spawnIndex;
    e->enemyFlags = owner->enemyFlags | 4;
    Anim_StartIndexed(e, 0, 0, 1);
    Anim_Update(e);
    if (gEwramData->gameState.bestiaryPreview)
    {
        e->state = 0xD0;
        e->stage = 0;
        e->subState = 0;
    }
    else
    {
        Entity_SnapToGround(e);
        Entity_EnableBoxes(e, Enemy_OnContact, (s32)GrahamFirstForm_OnHit);
        return e;
    }
}
