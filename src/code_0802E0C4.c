/* Activate an equipped blue soul when input, state and MP checks permit it. */
#include "code_08009A0.h"
#include "player_entity.h"
#include "structs/ewram.h"

/* Blue soul info, indexed by equippedBlueSoul (as in code_08031D6C.c). */
struct BlueSoulInfo {
    void *updateFunc;
    u8 mpPerSecond;
    u8 activationFlags;
    u16 effectParam;
    u32 effectValue;
};

extern const struct BlueSoulInfo sBlueSoulInfo[];

/* Keep the post-allocation reload separate from the earlier table index. */
static inline u8 GetBlueSoulId(void)
{
    return gEwramData->playerStatus.equippedBlueSoul;
}
/**
 * @brief Activate the equipped blue soul when its button is held (or newly
 * pressed for trigger-type souls), no blue-soul entity is active, the
 * scene permits it and MP exceeds maxMP / 128. Create and register the
 * soul entity and set player flag 0x200.
 */
void Soma_HandleBlueSoulInput(struct EwramData_EntityData *entity)
{
    struct EwramData *ew = gEwramData;
    struct EwramData_EntityData *active;
    const struct BlueSoulInfo *info;
    struct EwramData_EntityData *soul;
    u16 button;

    if (ew->playerState.actionLockFlags & 0x200)
        return;
    if ((ew->gameState.gameFlagsTail[0] & 0x200000) && ew->playerStatus.equippedBlueSoul > 3)
        return;
    active = ew->playerState.activeBlueSoulEntities[0];
    if (active != NULL)
        return;
    {
        /* Preserve address calculation, held-input read, then button read.
         * The register constraint emits no inline instructions. */
        u16 *buttonPtr = &ew->playerStatus.guardianButton;
        u16 held = *(vu16 *)&ew->inputData.playerHeldInput; /* volatile read (replaces a pinned register) */
        if (!(held & (button = *buttonPtr)))
            return;
    }
    info = &sBlueSoulInfo[ew->playerStatus.equippedBlueSoul];
    if ((info->activationFlags & 2) && !(button &= ew->inputData.playerNewInput))
        return;
    if (info->updateFunc == NULL)
        return;
    if (ew->playerStatus.currentMP < (ew->playerStatus.maxMP >> 7) + 1)
        return;
    soul = EntityCreateInRange(2, 0x10, info->updateFunc);
    if (soul == NULL)
        return;
    gEwramData->playerState.activeBlueSoulEntities[0] = soul;
    soul->ext.blueSoul.owner = entity;
    soul->ext.blueSoul.id = GetBlueSoulId();
    soul->ext.blueSoul.param = info->effectParam;
    soul->ext.blueSoul.slot = (u32)active;
    soul->ext.blueSoul.value = info->effectValue;
    soul->ext.blueSoul.mpInterval = 60 / info->mpPerSecond;
    entity->ext.player.actionFlags |= 0x200;
}
