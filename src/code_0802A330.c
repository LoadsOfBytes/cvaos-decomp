/* RedSoulHistoryTrail_Update. */
#include "code_08009A0.h"
#include "code_08001194.h"

struct Unk_0802A330 {
    s16 x;
    s16 y;
};

void RedSoulHistoryTrail_Draw(struct EwramData_EntityData *entity);

static inline s32 NoRedSouls(struct EwramData *ew)
{
    s32 empty;
    if (ew->playerState.nbrActiveRedSoulEntities == 0 && ew->playerState.activeRedSoulEntities[0] == NULL)
        empty = 1;
    else
        empty = 0;
    return empty;
}

/**
 * @brief History trail update that installs its draw callback on the first
 * update: delete when no red soul is active or its slot is empty,
 * otherwise shift the 10-entry history and record the soul position.
 */
void RedSoulHistoryTrail_Update(struct EwramData_EntityData *entity)
{
    struct Unk_0802A330 *trail = (struct Unk_0802A330 *)&entity->unk_4F4;
    struct EwramData *ew = gEwramData;
    struct EwramData_EntityData *owner = ew->playerState.activeRedSoulEntities[entity->stage];
    s32 i;

    if (NoRedSouls(ew) || gEwramData->playerState.activeRedSoulEntities[entity->stage] == NULL)
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        entity->drawFunc = (u32 *)RedSoulHistoryTrail_Draw;
        entity->state++;
    }
    for (i = 9; i > 0; i--)
    {
        trail[i].x = trail[i - 1].x;
        trail[i].y = trail[i - 1].y;
    }
    trail[0].x = GetEntityRoomXPositionInteger(owner);
    trail[0].y = GetEntityRoomYPositionInteger(owner);
}
