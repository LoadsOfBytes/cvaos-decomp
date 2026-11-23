/* Initialize object 0x03's window mask, three vertical tile positions and variant state. */
#include "code_08001194.h"
#include "structs/ewram.h"
#include "structs/main.h"
extern void Door_SetupGraphics(struct EwramData_EntityData *);
void Object03Create(struct EwramData_EntityData *entity)
{
    s32 x, y;
    struct BgLayerState *bg;
    Door_SetupGraphics(entity);
    gDisplayRegisters.winOut_L = 0x3F;
    gDisplayRegisters.winOut_H = 0x2F;
    x = (s16)entity->posX.part16.integer;
    bg = &gEwramData->bgInfo[1];
    x += (s16)bg->xPos.part16.integer;
    y = (s16)entity->posY.part16.integer;
    y += (s16)bg->yPos.part16.integer;
    Collision_SetBlock(x, y - 8);
    Collision_SetBlock(x, y - 24);
    Collision_SetBlock(x, y - 40);
    entity->ext.door.openHeight = 0;
    switch (entity->spawnParam0)
    {
    case 1: entity->state = 20; break;
    case 3: entity->state = 99; break;
    default: entity->state = 0; break;
    }
}
