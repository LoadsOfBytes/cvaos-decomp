/* EnemyAxeArmorCreate. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, u8 *resource, void *palette, u8 variant, void *sheet);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_EnableBoxes(struct EwramData_EntityData *entity, void *onHit, s32 onHurt);
extern void Enemy_OnContact(struct EwramData_EntityData *entity);
extern void AxeArmor_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hitData);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *entity);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);

/* Graphics of the Axe Armor (see code_080B0CCC.c). */
extern u8 sAxeArmorGfx[];
extern u8 sAxeArmorPalette[];
extern u8 sAxeArmorSpriteSheet[];

/**
 * @brief B12C4 | Axe Armor create: load the graphics (delete the enemy
 * when that fails), hit boxes, layer 0x13, sprite 0; state 3 when
 * bestiaryPreview is set, otherwise collision callbacks, boxes, one pixel lower,
 * 0x53D bit 1 and state 0.
 */
void EnemyAxeArmorCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sAxeArmorGfx, sAxeArmorPalette, 0, sAxeArmorSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x13;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        return;
    }
    Entity_EnableBoxes(entity, Enemy_OnContact, (s32)AxeArmor_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->posY.part16.integer++;
    entity->scrollWithBg = 1;
    entity->state = 0;
}
