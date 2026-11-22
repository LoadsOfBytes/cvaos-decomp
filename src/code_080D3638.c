/* Linked part creation.
 * The original defines its return value only on allocation failure and the
 * ordinary success path. Graphics failure and the special 0xD0 mode
 * leave the return value unspecified; preserve those original paths. */
#include "code_08009A0.h"
#include "graham_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, s32);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern void GrahamSidePart_Update(struct EwramData_EntityData *);
extern u8 sGrahamPalette2[], sGrahamSideSpriteSheet[], sGrahamGfx3[];

/**
 * @brief Create a side-selected linked part with graphics and contact
 * boxes. Preserve the unspecified return on graphics failure and
 * bestiary branches; ordinary gameplay success returns the part.
 */
struct EwramData_EntityData *Graham_CreateSidePart(struct EwramData_EntityData *owner, s32 side)
{
    u8 *gfx2 = sGrahamPalette2;
    u8 *gfx3 = sGrahamSideSpriteSheet;
    u8 *gfx1 = sGrahamGfx3;
    struct EwramData_EntityData *e = Entity_CreateObject((u32 *)GrahamSidePart_Update);
    if (e == NULL)
        return NULL;
    if (!Entity_LoadGraphics(e, gfx1, gfx2, 0, gfx3))
    {
        EntityDelete(e);
        return;
    }
    ENTITY_PARENT_INDEX(e) = ENTITY_INDEX(owner);
    e->spriteLayer = owner->spriteLayer - 3;
    e->flipX = side;
    e->posX.whole = owner->posX.whole;
    e->posY.whole = owner->posY.whole;
    e->scrollWithBg = 1;
    e->enemyFlags |= 4;
    e->attack = owner->attack;
    e->unk_518.part8.enemyId = owner->unk_518.part8.enemyId;
    GRAHAM_PART_SIDE(e) = side;
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
        Entity_EnableBoxes(e, Enemy_OnContact, 0);
        e->hit.flags |= 4;
        return e;
    }
}
