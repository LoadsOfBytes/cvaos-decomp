#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void ManticoreShot_Update(struct EwramData_EntityData *entity);
extern s32 ManticoreShot_LoadGraphics(struct EwramData_EntityData *entity, struct EwramData_EntityData *shot);
extern void ManticoreShot_EnableHitBoxes(struct EwramData_EntityData *entity);

/**
 * @brief 73B54 | Create a shot ManticoreShot_Update of the Manticore code
 * (0x08073xxx): set up by ManticoreShot_LoadGraphics (deleted when that fails, but still
 * initialised afterwards as in the original) and ManticoreShot_EnableHitBoxes, layer one
 * below the owner (unk_4F4), 44 px in front and 15 px above it, velX
 * 0x40000 in its facing, hidden (hidden), owner's spawnIndex, palette and
 * attack.
 */
void Manticore_SpawnShot(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *shot = EntityCreateInRange(0x1B, 0x48, (u32 *)ManticoreShot_Update);

    if (shot == NULL)
        return;
    if ((u8)ManticoreShot_LoadGraphics(entity, shot) == 0)
        EntityDelete(shot);
    shot->scrollWithBg = 1;
    ManticoreShot_EnableHitBoxes(shot);
    shot->spriteLayer = entity->spriteLayer - 1;
    ENTITY_CREATOR(shot) = entity;
    shot->flipX = entity->flipX;
    if (entity->flipX)
    {
        shot->posX.part16.integer = entity->posX.part16.integer + 0x2C;
        shot->velX.whole = 0x40000;
    }
    else
    {
        shot->posX.part16.integer = entity->posX.part16.integer - 0x2C;
        shot->velX.whole = -0x40000;
    }
    shot->posY.part16.integer = entity->posY.part16.integer - 0xF;
    shot->velY.whole = 0;
    shot->accelX.whole = 0;
    shot->accelY.whole = 0;
    shot->hidden = 1;
    shot->spawnIndex = entity->spawnIndex;
    shot->basePalette = entity->basePalette;
    shot->attack = entity->attack;
}
