/* Create a Mudman projectile and inherit the parent's visual attributes. */
#include "code_08009A0.h"
#include "structs/ewram.h"
extern void MudmanShot_Update(struct EwramData_EntityData *);
extern u8 MudmanShot_LoadGraphics(struct EwramData_EntityData *, struct EwramData_EntityData *);
extern void MudmanShot_EnableHitBoxes(struct EwramData_EntityData *);
/**
 * @brief Spawn the mud shot 16 px in front of parent at 2 px per frame in its
 * facing direction.
 */
void Mudman_ThrowShot(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *projectile = EntityCreateInRange(0x1B, 0x48, (u32 *)MudmanShot_Update);
    if (projectile)
    {
        if (!MudmanShot_LoadGraphics(parent, projectile)) EntityDelete(projectile);
        projectile->scrollWithBg = 1;
        MudmanShot_EnableHitBoxes(projectile);
        projectile->spriteLayer = parent->spriteLayer + 1;
        ENTITY_CREATOR(projectile) = parent;
        projectile->flipX = parent->flipX;
        if (parent->flipX)
        {
            projectile->posX.part16.integer = parent->posX.part16.integer + 16;
            projectile->velX.whole = 0x20000;
        }
        else
        {
            projectile->posX.part16.integer = parent->posX.part16.integer - 16;
            projectile->velX.whole = -0x20000;
        }
        projectile->posY.part16.integer = parent->posY.part16.integer - 24;
        projectile->velY.whole = -0x10000;
        projectile->accelX.whole = 0;
        projectile->accelY.whole = 0x1800;
        projectile->spawnIndex = parent->spawnIndex;
        projectile->basePalette = parent->basePalette;
        projectile->attack = parent->attack;
    }
}

