/* EventScene03_Init. */
#include "code_08009A0.h"
#include "event_scene03.h"
#include "structs/ewram.h"

extern s32 GameFlag_GetMask(s32 flag);
extern struct EwramData_EntityData *Entity_CreateFromObjectTable(s32 kind);
extern void EventScene03_Start(struct EwramData_EntityData *entity);

/**
 * @brief Init of event scene variant 3 (Object 0x20 unk_514 = 3,
 * sEventSceneCreateFuncs[3]). Create step of a scene object: unless flag 0x20 is
 * set (GameFlag_GetMask), create its three helper entities
 * (Entity_CreateFromObjectTable kinds 0, 1 and 8) into
 * unk_4F8/unk_4FC/unk_500; if all exist start it (EventScene03_Start,
 * gameState.stateFlags bit 0, byte 0x51C = 5 when the camera x is past 0x1F),
 * else delete the ones created and itself.
 */
void EventScene03_Init(struct EwramData_EntityData *entity)
{
    s32 done = GameFlag_GetMask(0x20);
    struct BgLayerState *bg;

    if (done == 0)
    {
        EVENT03_ACTOR0(entity) = Entity_CreateFromObjectTable(0);
        EVENT03_ACTOR1(entity) = Entity_CreateFromObjectTable(1);
        EVENT03_ACTOR8(entity) = Entity_CreateFromObjectTable(8);
        if (EVENT03_ACTOR0(entity) != NULL && EVENT03_ACTOR1(entity) != NULL
            && EVENT03_ACTOR8(entity) != NULL)
            goto created;
        if (EVENT03_ACTOR0(entity) != NULL)
            EntityDelete(EVENT03_ACTOR0(entity));
        if (EVENT03_ACTOR1(entity) != NULL)
            EntityDelete(EVENT03_ACTOR1(entity));
        if (EVENT03_ACTOR8(entity) != NULL)
            EntityDelete(EVENT03_ACTOR8(entity));
    }
    entity->deleteRequested = 1;
    return;

created:
    EventScene03_Start(entity);
    gEwramData->gameState.stateFlags |= 1;
    bg = &gEwramData->bgInfo[1];
    if ((s16)bg->xPos.part16.integer > 0x1F)
        ((u8 *)&entity->unk_51C)[0] = 5;
    else
        ((u8 *)&entity->unk_51C)[0] = done;
}
