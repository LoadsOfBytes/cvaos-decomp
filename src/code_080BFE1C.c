#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern void EvilButcher_OnHit(struct EwramData_EntityData *, s32, u16 *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, void *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern u8 sEvilButcherGfx[], sEvilButcherPalette[], sEvilButcherSpriteSheet[];

void EnemyEvilButcherCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sEvilButcherGfx, sEvilButcherPalette, (u8)entity->unk_514.part16.spawnParam1, sEvilButcherSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x13;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview)
        entity->state = 3;
    else
    {
        Entity_EnableBoxes(entity, Enemy_OnContact, EvilButcher_OnHit);
        Entity_UpdateBoxes(entity);
        Entity_SnapToGround(entity);
        entity->posY.part16.integer++;
        entity->scrollWithBg = 1;
        entity->state = 0;
    }
}
