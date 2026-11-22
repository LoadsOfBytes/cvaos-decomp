#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern u16 Collision_GetGroundOffset(s32, s32);
/**
 * @brief Burst debris piece: start with a random velocity away from the player,
 * fall under gravity, bounce once at half speed (new frame) when the point
 * below it is solid, then keep falling; deleted outside the screen margin.
 */
void DebrisPiece_Update(struct EwramData_EntityData *entity)
{
    struct EwramData **ewram = &gEwramData;
    struct EwramData_EntityData *player = (*ewram)->playerState.player;
    switch (entity->state)
    {
    case 0:
        entity->velX.whole = ((RandomNumberGenerator() & 0x1FFF) << 4) - 0x8000;
        if (entity->posX.whole < player->posX.whole) entity->velX.whole = -entity->velX.whole;
        entity->velY.whole = -(RandomNumberGenerator() & 0x3FFF) << 4;
        entity->state++;
        break;
    case 1:
    {
        s32 x = (s16)entity->posX.part16.integer;
        struct BgLayerState *camera = &(*ewram)->bgInfo[1];
        if (Collision_GetGroundOffset(x + (s16)camera->xPos.part16.integer,
                         (s16)entity->posY.part16.integer + (s16)camera->yPos.part16.integer + 4))
        {
            entity->animFrame = (RandomNumberGenerator() & 1) + 0x40;
            entity->velY.whole = -(entity->velY.whole >> 1);
            entity->state++;
        }
        else
        {
            entity->posX.whole += entity->velX.whole;
            entity->posY.whole += entity->velY.whole;
            entity->velY.whole += 0x3000;
        }
        break;
    }
    case 2:
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += 0x3000;
        break;
    }
    if (!Entity_IsInRect(entity, -16, -16, 272, 192)) entity->deleteRequested = 1;
}
