/* BlueSoul_CheckEndCondition. */
#include "code_08009A0.h"
#include "structs/ewram.h"

/* Blue soul info, indexed by equippedBlueSoul (as in code_08031D6C.c). */
struct BlueSoulInfo {
    void *updateFunc;
    u8 unk_4;
    u8 activationFlags;
    u8 pad_6[0xC - 0x6];
};

extern const struct BlueSoulInfo sBlueSoulInfo[];

/**
 * @brief Advance the MP interval counter and check depletion, guardian input and interaction flags; return the end decision without releasing active resources.
 */
s32 BlueSoul_CheckEndCondition(struct EwramData_EntityData *entity)
{
    struct EwramData *ew = gEwramData;
    const struct BlueSoulInfo *info = &sBlueSoulInfo[ew->playerStatus.equippedBlueSoul];
    s32 rem;

    entity->roomActive = 1;
    entity->unk_504.part16.unk_506[0]++;
    rem = (u16)entity->unk_504.part16.unk_506[0] % entity->unk_504.part8.unk_504;
    if (rem == 0 && --ew->playerStatus.currentMP <= 0)
    {
        ew->playerStatus.currentMP = rem;
        return 1;
    }
    if (!(info->activationFlags & 2))
    {
        if (!(gEwramData->inputData.playerHeldInput & gEwramData->playerStatus.guardianButton))
            return 1;
    }
    else if (entity->state != 0 && (gEwramData->inputData.playerNewInput & gEwramData->playerStatus.guardianButton))
        return 1;
    {
        u32 flags = gEwramData->gameState.stateFlags;
        if (!(flags & 0x100))
        {
            flags &= 1;
            if (flags)
                return 1;
        }
    }
    return 0;
}
