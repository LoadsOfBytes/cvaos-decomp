/* Short-lived blue-soul companion; cancel if its active soul slot is gone. */
#include "code_08009A0.h"
#include "structs/ewram.h"
#include "macros.h"

static inline s32 IsSlotEmpty(struct EwramData_EntityData *entity)
{
    return entity == NULL;
}

static inline struct EwramData_EntityData *GetActiveBlueSoulEntity(struct EwramData *data, u32 index)
{
    u32 scaled = index * 4;
    u32 offset = OFFSET_OF(struct EwramData, playerState.activeBlueSoulEntities);
    u8 *slots = (u8 *)data + offset;
    slots += scaled;
    return *(struct EwramData_EntityData **)slots;
}
/* Keep the separate offset lifetimes for the first and indexed slot reads.
 * They reproduce agbcc's reload and allocation. The countdown copy similarly
 * retains the original compare/decrement registers. These are compiler
 * constraints, not evidence for the original source spelling. */
/**
 * @brief Delete the afterimage when its blue soul slot is empty, else count
 * down its timer and delete it at the end.
 */
void BlueSoulAfterimage_Update(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram = gEwramData;
    s32 empty;
    {
        s8 cancelled = -1;
        ewram->playerState.blueSoulGfx = cancelled;
    }
    {
        s32 offset = OFFSET_OF(struct EwramData, playerState.activeBlueSoulEntities);
        s16 missing = IsSlotEmpty(*(struct EwramData_EntityData **)((u8 *)ewram + offset));
        empty = missing;
    }
    if (empty || GetActiveBlueSoulEntity(ewram, entity->stage) == NULL)
    {
        do { EntityDelete(entity); } while (0); /* loop note weights entity (global-alloc order) */
        return;
    }
    {
        struct EwramData_EntityData *timerEntity = entity;
        u8 timerByte = timerEntity->timer;
        s16 timer = timerByte;
        if (timer == 4) timerEntity->drawSkip = 1;
        if (timer != 0) timerEntity->timer = timerByte - 1;
        else EntityDelete(timerEntity);
    }
}
