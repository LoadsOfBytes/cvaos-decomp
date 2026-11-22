/* EnemyBlueCrowCreate. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, u8 *resource, void *palette, u8 variant, void *sheet);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_EnableBoxes(struct EwramData_EntityData *entity, void *onHit, s32 onHurt);
extern void Enemy_OnContact(struct EwramData_EntityData *entity);
extern void BlueCrow_OnHit(struct EwramData_EntityData *entity);

/* Graphics of Blue Crow. */
extern u8 sBlueCrowRedCrowGfx[];
extern u8 sBlueCrowRedCrowPalette[];
extern u8 sBlueCrowRedCrowSpriteSheet[];

/**
 * @brief C9A6C | Blue Crow create: load the graphics (delete the enemy when
 * that fails), start the animation, layer 3, state 1, hit boxes; state 3
 * instead when bestiaryPreview is set.
 */
void EnemyBlueCrowCreate(struct EwramData_EntityData *entity)
{
    u8 *palette = sBlueCrowRedCrowPalette;
    u8 *sheet = sBlueCrowRedCrowSpriteSheet;

    if (!Entity_LoadGraphics(entity, sBlueCrowRedCrowGfx, palette, 0, sheet))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 3;
    entity->state = 1;
    entity->stage = 0;
    entity->subState = 0;
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        entity->stage = 0;
        entity->subState = 0;
        return;
    }
    Entity_EnableBoxes(entity, Enemy_OnContact, (s32)BlueCrow_OnHit);
}
