/* Create the combat body overlay, two side parts and a linked attacker.
 * The special 0xD0 path, as in the
 * original, does not define a return value. */
#include "code_08009A0.h"
#include "graham_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern void GrahamBodyOverlay_Update(struct EwramData_EntityData *);
extern struct EwramData_EntityData *Graham_CreateSidePart(struct EwramData_EntityData *, s32);
extern struct EwramData_EntityData *Graham_CreateAttacker(struct EwramData_EntityData *);
extern u8 sGrahamPalette2[], sGrahamSpriteSheet2[], sGrahamGfx2[];

/**
 * @brief Create the body overlay with shared combat fields, create two
 * side parts and the linked attacker, and store side pointers. Preserve
 * the bestiary branch with its unspecified return value.
 */
struct EwramData_EntityData *Graham_CreateBodyOverlay(struct EwramData_EntityData *owner)
{
    u8 *gfx2 = sGrahamPalette2;
    u8 *gfx3 = sGrahamSpriteSheet2;
    u8 *gfx1 = sGrahamGfx2;
    struct EwramData_EntityData *e = Entity_CreateObject((u32 *)GrahamBodyOverlay_Update);
    if (e == NULL) return NULL;
    if (!Entity_LoadGraphics(e, gfx1, gfx2, 0, gfx3))
    {
        EntityDelete(e);
        return NULL;
    }
    ENTITY_PARENT_INDEX(e) = ENTITY_INDEX(owner);
    e->spriteLayer = owner->spriteLayer + 1;
    e->flipX = owner->flipX;
    e->posX.whole = owner->posX.whole;
    e->posY.whole = owner->posY.whole;
    e->scrollWithBg = 1;
    e->unk_518.part16.hp = owner->unk_518.part16.hp;
    e->unk_510.part16.unk_512 = owner->unk_510.part16.unk_512;
    e->attack = owner->attack;
    e->defense = owner->defense;
    e->enemyFlags |= 1;
    e->enemyFlags |= 4;
    e->unk_518.part8.enemyId = owner->unk_518.part8.enemyId;
    Anim_StartIndexed(e, 0, 0, 1);
    Anim_Update(e);
    GRAHAM_SIDE_PART_0(e) = (s32)Graham_CreateSidePart(e, 0);
    GRAHAM_SIDE_PART_1(e) = (s32)Graham_CreateSidePart(e, 1);
    Graham_CreateAttacker(e);

    if (gEwramData->gameState.bestiaryPreview)
    {
        e->state = 0xD0;
        e->stage = 0;
        e->subState = 0;
    }
    else
    {

        return e;
    }
}
