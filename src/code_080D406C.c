/* Create a linked part and copy the owner's position and combat fields. */
#include "code_08009A0.h"
#include "graham_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, s32);
extern void Entity_OnPlayerContactDamage(struct EwramData_EntityData *);
extern void GrahamAttackEffect_Update(struct EwramData_EntityData *);
extern u8 sGrahamPalette2[], sGrahamSpriteSheet2[], sGrahamGfx3[];

/**
 * @brief Create an animated damaging effect linked to the attacker with
 * five-quarters owner damage. Return it or NULL after failure cleanup.
 * Preserve the dead-register instruction and multiply/shift lifetime.
 */
struct EwramData_EntityData *GrahamAttacker_CreateAttackEffect(struct EwramData_EntityData *owner)
{
    u8 *palette = sGrahamPalette2;
    u8 *animation = sGrahamSpriteSheet2;
    u8 *graphics = sGrahamGfx3;
    struct EwramData_EntityData *part = Entity_CreateObject((u32 *)GrahamAttackEffect_Update);
    if (part == NULL)
        return NULL;
    if (Entity_LoadGraphics(part, graphics, palette, 1, animation))
    {
    ENTITY_PARENT_INDEX(part) = ENTITY_INDEX(owner);
    part->spriteLayer = owner->spriteLayer - 1;
    part->flipX = owner->flipX;
    part->posX.whole = owner->posX.whole;
    part->posY.whole = owner->posY.whole;
    part->scrollWithBg = 1;
    owner->ext.grahamScene.childIndex = ENTITY_INDEX(part);
    { u8 v_ = part->enemyFlags; { register u32 z_ asm("r7"); asm("" : "=r"(z_) : "0"(0)); } part->enemyFlags = v_ | 1; } /* FAKEMATCH: dead "movs r7, #0" of the original (graphics = 0) */

    part->enemyFlags |= 4;
    part->enemyId = owner->enemyId;
    { s32 m_ = owner->attack * 5; s32 s_ = m_ >> 2; asm("" :: "r"(m_)); part->attack = s_; } /* FAKEMATCH: m_ outlives the shift, so the result gets its own reg */
    Anim_StartIndexed(part, 4, 0, 1);
    Anim_Update(part);
    Entity_EnableBoxes(part, Entity_OnPlayerContactDamage, 0);
    part->hit.flags |= 4;
    return part;
    }
    else
    { EntityDelete(part); return NULL; }
}
