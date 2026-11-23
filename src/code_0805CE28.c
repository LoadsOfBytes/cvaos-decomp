/* EventScene03_Init. */
#include "code_08009A0.h"
#include "event_scene03.h"
#include "structs/ewram.h"

extern s32 GameFlag_GetMask(s32 flag);
extern struct EwramData_EntityData *Entity_CreateFromObjectTable(s32 kind);
extern void EventScene03_Start(struct EwramData_EntityData *entity);

/**
 * @brief Init of event scene variant 3 (Object 0x20 spawnParam0 = 3,
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
        entity->ext.event03.actor0 = Entity_CreateFromObjectTable(0);
        entity->ext.event03.actor1 = Entity_CreateFromObjectTable(1);
        entity->ext.event03.actor8 = Entity_CreateFromObjectTable(8);
        if (entity->ext.event03.actor0 != NULL && entity->ext.event03.actor1 != NULL
            && entity->ext.event03.actor8 != NULL)
            goto created;
        if (entity->ext.event03.actor0 != NULL)
            EntityDelete(entity->ext.event03.actor0);
        if (entity->ext.event03.actor1 != NULL)
            EntityDelete(entity->ext.event03.actor1);
        if (entity->ext.event03.actor8 != NULL)
            EntityDelete(entity->ext.event03.actor8);
    }
    entity->deleteRequested = 1;
    return;

created:
    EventScene03_Start(entity);
    gEwramData->gameState.stateFlags |= 1;
    bg = &gEwramData->bgInfo[1];
    if ((s16)bg->xPos.part16.integer > 0x1F)
        EVENT_SCENE03_CAMERA_STEP(entity) = 5;
    else
        EVENT_SCENE03_CAMERA_STEP(entity) = done;
}
