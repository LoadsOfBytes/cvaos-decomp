/* Create a Merman projectile and inherit the parent's visual attributes. */
#include "code_08009A0.h"
#include "structs/ewram.h"
extern void MermanShot_Update(struct EwramData_EntityData *);
extern u8 MermanShot_LoadGraphics(struct EwramData_EntityData *, struct EwramData_EntityData *);
extern void MermanShot_EnableHitBoxes(struct EwramData_EntityData *);
/**
 * @brief Spawn the horizontal shot 5 px in front of and 24 px above parent at
 * 2 px per frame in its facing direction.
 */
void Merman_SpawnShot(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *projectile = EntityCreateInRange(0x1B, 0x48, (u32 *)MermanShot_Update);
    if (projectile)
    {
        if (!MermanShot_LoadGraphics(parent, projectile)) EntityDelete(projectile);
        projectile->scrollWithBg = 1;
        MermanShot_EnableHitBoxes(projectile);
        projectile->spriteLayer = parent->spriteLayer + 1;
        ENTITY_CREATOR(projectile) = parent;
        projectile->flipX = parent->flipX;
        if (parent->flipX)
        {
            projectile->posX.part16.integer = parent->posX.part16.integer + 5;
            projectile->velX.whole = 0x20000;
        }
        else
        {
            projectile->posX.part16.integer = parent->posX.part16.integer - 5;
            projectile->velX.whole = -0x20000;
        }
        projectile->posY.part16.integer = parent->posY.part16.integer - 24;
        projectile->unk_518.part8.spawnIndex = parent->unk_518.part8.spawnIndex;
        projectile->unk_510.part8.basePalette = parent->unk_510.part8.basePalette;
        projectile->attack = parent->attack;
    }
}
