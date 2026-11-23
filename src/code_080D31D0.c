/* Create the linked combat body and its subordinate parts.
 * The special 0xD0 path, as in the
 * original, does not define a return value. */
#include "code_08009A0.h"
#include "graham_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, s32);
extern void GrahamCombatBody_OnHit(struct EwramData_EntityData *);
extern void GrahamCombatBody_Update(struct EwramData_EntityData *);
extern struct EwramData_EntityData *Graham_CreateBodyOverlay(struct EwramData_EntityData *);
extern u8 sGrahamPalette2[], sGrahamSpriteSheet2[], sGrahamGfx2[];

/**
 * @brief Create the later linked combat body, set combat fields and root
 * slot link, then create its subordinate parts. Preserve the bestiary
 * branch whose return value is unspecified in the matching source.
 */
struct EwramData_EntityData *Graham_CreateCombatBody(struct EwramData_EntityData *owner)
{
    u8 *gfx2 = sGrahamPalette2;
    u8 *gfx3 = sGrahamSpriteSheet2;
    u8 *gfx1 = sGrahamGfx2;
    struct EwramData_EntityData *e = Entity_CreateObject((u32 *)GrahamCombatBody_Update);
    if (e == NULL) return NULL;
    if (!Entity_LoadGraphics(e, gfx1, gfx2, 0, gfx3))
    {
        EntityDelete(e);
        return NULL;
    }
    ENTITY_PARENT_INDEX(e) = ENTITY_INDEX(owner);
    e->spriteLayer = owner->spriteLayer;
    e->flipX = owner->flipX;
    e->posX.whole = owner->posX.whole;
    e->posY.whole = owner->posY.whole;
    e->scrollWithBg = 1;
    e->unk_518.part8.enemyId = owner->unk_518.part8.enemyId;
    e->unk_518.part16.hp = 2000;
    e->unk_510.part16.unk_512 = owner->unk_510.part16.unk_512;
    e->attack = owner->attack;
    e->defense = 110;
    e->enemyFlags |= 4;
    owner->ext.grahamScene.childIndex = ENTITY_INDEX(e);
    Anim_StartIndexed(e, 1, 0, 1);
    Anim_Update(e);
    Graham_CreateBodyOverlay(e);

    if (gEwramData->gameState.bestiaryPreview)
    {
        e->state = 0xD0;
        e->stage = 0;
        e->subState = 0;
    }
    else
    {
        Entity_EnableBoxes(e, NULL, (s32)GrahamCombatBody_OnHit);
        e->hit.flags |= 2;
        return e;
    }
}
