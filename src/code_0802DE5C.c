/* BlueSoul_ShouldEnd; as
 * BlueSoul_CheckEndCondition (src/code_0802DF2C.c) plus BlueSoul_ReleaseActiveResources on ending. */
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
extern void BlueSoul_ReleaseActiveResources(struct EwramData_EntityData *);

/**
 * @brief 2DE5C | Whether an active blue soul ends this frame: every unk_504
 * frames (counter unk_506) it costs 1 MP and ends at 0 MP; a hold-type soul
 * (info activationFlags bit 1 clear) ends when the guardian button is released, a
 * toggle-type one when it is pressed again after the first state; also ends
 * when stateFlags bit 0 is set without bit 8. Sets roomActive;
 * on ending calls BlueSoul_ReleaseActiveResources (same tests as BlueSoul_CheckEndCondition).
 *
 * @return 1 when the soul should end, else 0
 */
s32 BlueSoul_ShouldEnd(struct EwramData_EntityData *entity)
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
        goto end;
    }
    if (!(info->activationFlags & 2))
    {
        if (gEwramData->inputData.playerHeldInput & gEwramData->playerStatus.guardianButton)
            goto check;
        goto end;
    }
    else if (entity->state != 0 && (gEwramData->inputData.playerNewInput & gEwramData->playerStatus.guardianButton))
        goto end;
check:
    {
        u32 flags = gEwramData->gameState.stateFlags;
        if (flags & 0x100)
            goto keep;
        flags &= 1;
        if (!flags)
            goto keep;
    }
end:
    BlueSoul_ReleaseActiveResources(entity);
    return 1;
keep:
    return 0;
}
