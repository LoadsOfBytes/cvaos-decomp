/* Link the animated overhead object back to the boss. */
#include "code_08009A0.h"
#include "graham_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *,u8 *,void *,u8,void *);
extern void GrahamTransitionPart_Update(struct EwramData_EntityData *);
extern u8 sGrahamPalette2[],sGrahamFirstFormSpriteSheet[],sGrahamFirstFormGfx[];
/**
 * @brief Create the animated transition part above the first form and
 * store both owner/child links. Preserve the existing allocation assumption.
 */
void GrahamFirstForm_CreateTransitionPart(struct EwramData_EntityData *entity)
{
    u8 *sprite=sGrahamPalette2;
    u8 *animation=sGrahamFirstFormSpriteSheet;
    u8 *graphics=sGrahamFirstFormGfx;
    struct EwramData_EntityData *effect=Entity_CreateObject((u32 *)GrahamTransitionPart_Update);
    if(!Entity_LoadGraphics(effect,graphics,sprite,5,animation))
    {
        EntityDelete(effect);
        return;
    }
    ENTITY_PARENT_INDEX(effect)=ENTITY_INDEX(entity);
    effect->spriteLayer=entity->spriteLayer-1;
    effect->flipX=entity->flipX;
    effect->posX.whole=entity->posX.whole;
    effect->posY.whole=entity->posY.whole-0x300000;
    effect->scrollWithBg=1;
    effect->hidden=1;
    GRAHAM_CHILD_INDEX(entity)=ENTITY_INDEX(effect);
    Anim_StartIndexed(effect,1,1,0);
    Anim_Update(effect);
}
