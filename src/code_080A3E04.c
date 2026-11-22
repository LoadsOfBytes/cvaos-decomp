/* ChaosMark_Update: soul halo following the player; matched. */
#include "structs/ewram.h"
#include "chaos_entity.h"

extern s32 Entity_GetVisibilityPattern(s32);

struct HaloOffset { s32 x; s32 y; };
struct HaloOffsets { struct HaloOffset o[3]; };
extern const struct HaloOffsets sChaosMarkOffsets;

#define IDX(e) (*(u8 *)&CHAOS_ORB_KIND((e)))

/**
 * @brief Mark update: stay next to the player at the offset of its orb kind,
 * step through its show sequence (states 0 / 1) and blink out (state 2).
 */
void ChaosMark_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = CHAOS_LINK(entity);
    struct EwramData *ew = gEwramData;
    struct HaloOffsets offs = sChaosMarkOffsets;

    if (entity->state == 0)
    {
        entity->posX.whole = ew->playerState.player->posX.whole + offs.o[IDX(entity)].x;
        entity->posY.whole = ew->playerState.player->posY.whole + offs.o[IDX(entity)].y;
        if (Entity_GetVisibilityPattern(entity->timer))
            entity->hidden = 0;
        else
            entity->hidden = 1;
        if ((s8)--entity->timer <= 0)
            entity->state++;
    }
    else if (entity->state == 1)
    {
        entity->posX.whole = ew->playerState.player->posX.whole + offs.o[IDX(entity)].x;
        entity->posY.whole = ew->playerState.player->posY.whole + offs.o[IDX(entity)].y;
        if (IDX(entity) == 0)
        {
            if (CHAOS_BROKEN_ORBS(parent) & 1)
            {
                entity->state++;
                entity->timer = 0x28;
            }
        }
        else if (IDX(entity) == 1)
        {
            if (CHAOS_BROKEN_ORBS(parent) & 2)
            {
                entity->state++;
                entity->timer = 0x28;
            }
        }
        else if (IDX(entity) == 2)
        {
            if (CHAOS_BROKEN_ORBS(parent) & 4)
            {
                entity->state++;
                entity->timer = 0x28;
            }
        }
    }
    else if (entity->state == 2)
    {
        entity->posX.whole = ew->playerState.player->posX.whole + offs.o[IDX(entity)].x;
        entity->posY.whole = ew->playerState.player->posY.whole + offs.o[IDX(entity)].y;
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0)
            entity->deleteRequested = 1;
    }
}
