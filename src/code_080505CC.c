/* Object 0x0C: rectangular interaction regions with a callback table. */
#include "structs/ewram.h"
#include "code_08001194.h"
#include "code/code_080211F0.h"

struct Object0CTrigger {
    u16 type;
    u16 unk_2;
    s32 value;
    s16 x;
    s16 y;
    u16 width;
    u16 height;
};

extern struct Object0CTrigger *sObject0CTriggerLists[];
extern void (*sObject0CTriggerFuncs[])(struct EwramData_EntityData *, struct Object0CTrigger *);

static inline s32 TriggerContainsPoint(s16 x, s16 y, struct Object0CTrigger *trigger)
{
    if (x < trigger->x)
        return 0;
    if (x > trigger->x + trigger->width)
        return 0;
    if (y < trigger->y)
        return 0;
    if (y > trigger->y + trigger->height)
        return 0;
    return 1;
}

void Object0CUpdate(struct EwramData_EntityData *entity)
{
    struct Object0CTrigger *trigger = sObject0CTriggerLists[entity->unk_514.part16.spawnParam0];
    u16 x = GetEntityRoomXPositionInteger(gEwramData->playerState.player);
    u16 y = GetEntityRoomYPositionInteger(gEwramData->playerState.player);
    entity->velX.whole = 0;
    while (trigger->type != 3)
    {
        if (TriggerContainsPoint(x, y, trigger))
            sObject0CTriggerFuncs[trigger->type](entity, trigger);
        trigger++;
    }
    if (!entity->roomActive)
    {
        entity->deleteRequested = 1;
        Player_ClearSolidObject();
    }
}
