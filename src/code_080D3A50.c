/* Create the linked attacker with doubled health/parameter 512.
 * The special 0xD0 path, as in the
 * original, does not define a return value. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, s32);
extern void GrahamAttacker_OnHit(struct EwramData_EntityData *, s32, u16 *);
extern void GrahamAttacker_Update(struct EwramData_EntityData *);
extern u8 sGrahamPalette2[], sGrahamSpriteSheet2[], sGrahamGfx3[];

/**
 * @brief Create the linked attacker with doubled owner HP and parameter
 * 0x512, shared identity and its own hit callback. Preserve the
 * bestiary branch with its unspecified return value.
 */
struct EwramData_EntityData *Graham_CreateAttacker(struct EwramData_EntityData *owner)
{
    u8 *gfx2 = sGrahamPalette2;
    u8 *gfx3 = sGrahamSpriteSheet2;
    u8 *gfx1 = sGrahamGfx3;
    struct EwramData_EntityData *e = Entity_CreateObject((u32 *)GrahamAttacker_Update);
    if (e == NULL) return NULL;
    if (!Entity_LoadGraphics(e, gfx1, gfx2, 0, gfx3))
    {
        EntityDelete(e);
        return NULL;
    }
    ENTITY_PARENT_INDEX(e) = ENTITY_INDEX(owner);
    e->spriteLayer = owner->spriteLayer - 2;
    e->flipX = owner->flipX;
    e->posX.whole = owner->posX.whole;
    e->posY.whole = owner->posY.whole;
    e->scrollWithBg = 1;
    e->enemyFlags |= 1;
    e->enemyFlags |= 4;
    e->enemyId = owner->enemyId;
    e->hp = (s16)owner->hp * 2;
    e->mp = owner->mp * 2;
    e->attack = owner->attack;
    e->defense = owner->defense * 2 / 3;
    Anim_StartIndexed(e, 2, 0, 1);
    Anim_Update(e);

    if (gEwramData->gameState.bestiaryPreview)
    {
        e->state = 0xD0;
        e->stage = 0;
        e->subState = 0;
    }
    else
    {
        Entity_EnableBoxes(e, NULL, (s32)GrahamAttacker_OnHit);
        e->hit.flags |= 2;
        return e;
    }
}
