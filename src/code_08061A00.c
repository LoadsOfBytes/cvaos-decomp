/* EventScene19_Init. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "macros.h"
#include "structs/ewram.h"

extern s32 GameFlag_GetMask(s32 flag);
extern void Interaction_End(struct EwramData_EntityData *entity, s32 deleteEntity);
extern s32 EventScene19_LoadGraphics(struct EwramData_EntityData *entity);

/**
 * @brief Init of event scene variant 19 (Object 0x20 unk_514 = 19,
 * sEventSceneCreateFuncs[19]). Create step of a scene object: deleted when flag
 * 0x29 is set or the player is left of room x 0x80; if EventScene19_LoadGraphics
 * fails mark the event and delete; else enter from room x 0x150 moving
 * left (-0x10000) with animation 1, stateFlags bit 0. The camera address is
 * added to the loaded gEwramData in place, hence the OFFSET_OF form.
 */
void EventScene19_Init(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg;

    if (GameFlag_GetMask(0x29) == 0)
    {
        s32 x;

        bg = (struct BgLayerState *)gEwramData;
        x = (s16)((struct EwramData *)bg)->playerState.player->posX.part16.integer;
        bg = (struct BgLayerState *)((u32)bg + OFFSET_OF(struct EwramData, bgInfo[1]));
        if (x + (s16)bg->xPos.part16.integer > 0x7F)
            goto visible;
    }
    entity->deleteRequested = 1;
    return;

visible:
    if (EventScene19_LoadGraphics(entity) == 0)
    {
        Interaction_End(entity, 1);
        EntityDelete(entity);
        return;
    }
    {
        struct BgLayerState *cam = (struct BgLayerState *)gEwramData;

        cam = (struct BgLayerState *)((u32)cam + OFFSET_OF(struct EwramData, bgInfo[1]));
        entity->posX.part16.integer = 0x150 - cam->xPos.part16.integer;
    }
    entity->velX.whole = -0x10000;
    entity->flipX = 0;
    entity->hidden = 1;
    Anim_StartIndexed(entity, 1, 1, 1);
    Anim_Update(entity);
    gEwramData->gameState.stateFlags |= 1;
}
