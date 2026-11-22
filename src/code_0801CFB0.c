#include "code_08009A0.h"

#include "macros.h"
static inline struct EwramData_EntityData *ReadPlayer(struct EwramData *ew)
{
    register s32 offset asm("r1") = OFFSET_OF(struct EwramData, playerState.player); /* FAKEMATCH: pinned register */
    u8 *address = (u8 *)ew + offset;
    return *(struct EwramData_EntityData **)address;
}
/* Return a player-relative Y coordinate selected by the observed flags.
 * The getter's short address/offset bindings preserve the separate slot
 * loads and address calculations in the original instructions. They are
 * compiler constraints, not evidence for original source organization. */
/**
 * @brief Return the y position above the player head for damage numbers and
 * status icons (lower in the small and crouching states).
 */
s16 Player_GetOverheadY(void)
{
    struct EwramData *ew = gEwramData;
    s32 y;

    if (ew->playerStatus.soulStateFlags & 0x200)
    {
        y = (s16)ReadPlayer(ew)->posY.part16.integer - 0x10;
        goto end;
    }
    if (ReadPlayer(ew)->unk_4F4.whole & 0x400)
    {
        y = (s16)ReadPlayer(ew)->posY.part16.integer - 0x16;
        goto end;
    }
    y = (s16)ReadPlayer(ew)->posY.part16.integer - 0x22;
end:
    return y;
}
