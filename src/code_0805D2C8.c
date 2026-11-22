/* EventScene03_SpawnPartsA, EventScene03_SpawnPartsB. */
#include "structs/ewram.h"

/* Spawn spots of EventScene03_SpawnPartsA: room x and flags for EventScene03_SpawnRisingPart. */
struct Unk_081187AC {
    s32 x;
    s32 flags;
};

extern const struct Unk_081187AC sEventScene03SpotsA[5];
extern const struct Unk_081187AC sEventScene03SpotsB[2];
extern struct EwramData_EntityData *EventScene03_SpawnRisingPart(struct EwramData_EntityData *parent, s32 x, s32 y, s32 flags);
extern struct EwramData_EntityData *EventScene03_SpawnFlyingPart(struct EwramData_EntityData *parent, s32 x, s32 y);

/**
 * @brief 5D2C8 | Spawn the five effects of sEventScene03SpotsA (EventScene03_SpawnRisingPart at
 * room (x + 0x20, 0x29F), attack = index) and one EventScene03_SpawnFlyingPart effect at
 * room (0x14F, 0x288), all relative to the camera bgInfo[1].
 */
void EventScene03_SpawnPartsA(struct EwramData_EntityData *entity)
{
    struct Unk_081187AC spots[5];
    s32 i;

    memcpy(spots, sEventScene03SpotsA, sizeof(spots));
    for (i = 0; i <= 4; i++)
    {
        struct BgLayerState *bg = &gEwramData->bgInfo[1];

        EventScene03_SpawnRisingPart(entity, spots[i].x - (s16)bg->xPos.part16.integer + 0x20,
                     0x29F - (s16)bg->yPos.part16.integer, spots[i].flags)->attack = i;
    }
    {
        struct BgLayerState *bg = &gEwramData->bgInfo[1];

        EventScene03_SpawnFlyingPart(entity, 0x14F - (s16)bg->xPos.part16.integer, 0x288 - (s16)bg->yPos.part16.integer);
    }
}

/**
 * @brief 5D35C | As EventScene03_SpawnPartsA with the two spots of sEventScene03SpotsB
 * relative to the player's x (attack = 5 + index) and two EventScene03_SpawnFlyingPart
 * effects at room (0x7C, 0x290) and (0xC4, 0x280).
 */
void EventScene03_SpawnPartsB(struct EwramData_EntityData *entity)
{
    struct Unk_081187AC spots[2];
    s32 i;

    memcpy(spots, sEventScene03SpotsB, sizeof(spots));
    for (i = 0; i <= 1; i++)
    {
        struct BgLayerState *bg;
        s32 x = (s16)gEwramData->playerState.player->posX.part16.integer + spots[i].x;

        bg = &gEwramData->bgInfo[1];
        EventScene03_SpawnRisingPart(entity, x, 0x29F - (s16)bg->yPos.part16.integer, spots[i].flags)->attack = i + 5;
    }
    {
        struct BgLayerState *bg = &gEwramData->bgInfo[1];

        EventScene03_SpawnFlyingPart(entity, 0x7C - (s16)bg->xPos.part16.integer, 0x290 - (s16)bg->yPos.part16.integer);
    }
    {
        struct BgLayerState *bg = &gEwramData->bgInfo[1];

        EventScene03_SpawnFlyingPart(entity, 0xC4 - (s16)bg->xPos.part16.integer, 0x280 - (s16)bg->yPos.part16.integer);
    }
}
