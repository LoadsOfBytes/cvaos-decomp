/* Falling/settling pickup update (set by Pickup_CreateMana): rise until velY >= 0,
 * then sway down (sine of timer) until the ground (Collision_GetPushUp) is hit,
 * wait 200 frames, blink 64 frames and disappear. */
#include "code_08009A0.h"
#include "code_08001194.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "entity_hit.h"

static inline s32 RoomX(s32 pos) { struct BgLayerState *bg = &gEwramData->bgInfo[1]; return pos + (s16)bg->xPos.part16.integer; }
static inline s32 RoomY(s32 pos) { struct BgLayerState *bg = &gEwramData->bgInfo[1]; return pos + (s16)bg->yPos.part16.integer; }
/**
 * @brief Check player contact, rise then drift down to the ground, wait 200 frames and request deletion after the final 64-frame draw-skip phase.
 */
void Pickup_UpdateDriftingMana(struct EwramData_EntityData *entity)
{
    struct HitResult res;
    s16 dy;

    Hit_CheckWithPlayer(&res, (u8 *)entity);
    switch (entity->state)
    {
    case 0:
        if (entity->velY.whole >= 0)
            entity->state = 1;
        else
        {
            entity->velY.whole += 0x2000;
            entity->posY.whole += entity->velY.whole;
        }
        break;
    case 1:
    {
        entity->velY.whole = 0x4000;
        entity->velX.whole = Sin((entity->timer << 9) + 0x4000) >> 1;
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->timer++;
        dy = Collision_GetPushUp(RoomX((s16)entity->posX.part16.integer), RoomY((s16)entity->posY.part16.integer) + 4);
        if (dy != 0)
        {
            entity->posY.whole -= dy;
            entity->timer = 200;
            entity->state++;
        }
        break;
    }
    case 2:
        if (--entity->timer == 0)
        {
            entity->timer = 64;
            entity->state++;
        }
        break;
    case 3:
        entity->timer--;
        entity->drawSkip = 1;
        if (entity->timer == 0)
            entity->deleteRequested = 1;
        break;
    }
}
