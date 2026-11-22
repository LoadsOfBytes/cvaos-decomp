/* Bomber Armor effect (Bomber Armor graphics). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern u8 sBomberArmorRockArmorGfx[], sBomberArmorRockArmorPalette[], sBomberArmorSpriteSheet[];

/**
 * @brief B2FC4 | One-shot effect: state 0 loads graphics (deleting the entity
 * on failure), starts animation 4 and sets spriteLayer = 0x12 and scrollWithBg.
 * Every frame deleteRequested is set once the animation returns 3.
 */
void BomberArmorBombTrail_Update(struct EwramData_EntityData *entity)
{
    if (entity->state == 0)
    {
        if (!Entity_LoadGraphics(entity, sBomberArmorRockArmorGfx, sBomberArmorRockArmorPalette, 0, sBomberArmorSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        Anim_StartIndexed(entity, 4, 0, 0);
        entity->spriteLayer = 0x12;
        entity->scrollWithBg = 1;
        entity->state++;
    }
    if (Anim_Update(entity) == 3)
        entity->deleteRequested = 1;
}
