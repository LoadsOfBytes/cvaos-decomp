/* Red soul info and small geometry helpers 0x0802D1D8..0x0802D407.
 *
 * This file does not include code/code_080211F0.h: that header declares
 * RedSoul_FindFreeEntitySlot as returning u8, but the original returns a sign-extended
 * s8 (-1 when no slot is free). Changing the header would change callers. */
#include "structs/ewram.h"
#include "syscalls.h"

/* Same layout as struct RedSoulActivationView in code/code_080211F0.h, plus activeEntityLimit. */
struct RedSoulInfo {
    void *updateFunc; // TODO: function signature
    u16 castAnimKind;
    s16 mpCost;
    u8 activeEntityLimit;
    u8 pad_9[0xE - 0x9];
    u16 unk_E;
}; /* size = 0x10 */

extern const struct RedSoulInfo sSomaRedSoulInfo[]; // Soma (currentCharacter == 0)
extern const struct RedSoulInfo sJuliusSubweaponInfo[]; // Julius mode (defined in code_08033CAC.c)
extern const u8 sRedSoulCastAnims[][2];

const struct RedSoulInfo *RedSoul_GetInfo(u8 soul);

#define INT_TOWARD_ZERO(v) ((v) < 0 ? -((-(v)) >> 16) : (v) >> 16)

/**
 * @brief Return the cast animation for pose 0 (ground) or 1 (air) from the
 * sRedSoulCastAnims pair selected by the equipped red soul info castAnimKind.
 */
u8 RedSoul_GetCastAnim(s32 pose)
{
    const struct RedSoulInfo *info = RedSoul_GetInfo(gEwramData->playerStatus.equippedRedSoul);

    return sRedSoulCastAnims[info->castAnimKind][pose];
}

/**
 * @brief Return the first NULL active-red-soul slot among eight, or signed -1; preserve existing caller declaration differences.
 */
s8 RedSoul_FindFreeEntitySlot(void)
{
    s8 i;

    for (i = 0; i < 8; i++)
    {
        if (gEwramData->playerState.activeRedSoulEntities[i] == NULL)
            return i;
    }
    return -1;
}

/**
 * @brief Return the last-used red-soul record from the current-character table without adding index validation.
 */
const struct RedSoulInfo *RedSoul_GetLastUsedInfo(void)
{
    if (gEwramData->playerStatus.currentCharacter == 0)
        return &sSomaRedSoulInfo[gEwramData->playerState.lastUsedRedSoul];
    return &sJuliusSubweaponInfo[gEwramData->playerState.lastUsedRedSoul];
}

/**
 * @brief Return the indexed red-soul record from the current-character table, retaining the byte argument and unchecked index.
 */
const struct RedSoulInfo *RedSoul_GetInfo(u8 soul)
{
    if (gEwramData->playerStatus.currentCharacter == 0)
        return &sSomaRedSoulInfo[soul];
    return &sJuliusSubweaponInfo[soul];
}

/**
 * @brief Return the active-entity limit (unk_8) of lastUsedRedSoul from the
 * current character's table (Soma sSomaRedSoulInfo, Julius sJuliusSubweaponInfo).
 */
u8 RedSoul_GetActiveLimit(void)
{
    const struct RedSoulInfo *info;

    if (gEwramData->playerStatus.currentCharacter == 0)
        info = &sSomaRedSoulInfo[gEwramData->playerState.lastUsedRedSoul];
    else
        info = &sJuliusSubweaponInfo[gEwramData->playerState.lastUsedRedSoul];
    return info->activeEntityLimit;
}

/**
 * @brief 2D300 | EWRAM byte 0x1307D.
 */
u8 Hit_GetTimerOverride1(void)
{
    return gEwramData->hitTimerOverride[1];
}

/**
 * @brief Return ArcTan2 of target minus source integer positions; preserve the existing integer-field conversion behavior and separate matching copy.
 */
u16 Entity_AngleToEntity(struct EwramData_EntityData *entity, struct EwramData_EntityData *target)
{
    return ArcTan2(target->posX.part16.integer - entity->posX.part16.integer,
                   target->posY.part16.integer - entity->posY.part16.integer);
}

/**
 * @brief Return the angle to a 16.16 point, truncating negative fixed-point coordinates toward zero and preserving source integer-field conversions.
 */
u16 Entity_AngleToPoint(struct EwramData_EntityData *entity, s32 x, s32 y)
{
    return ArcTan2(INT_TOWARD_ZERO(x) - (s16)entity->posX.part16.integer,
                   INT_TOWARD_ZERO(y) - entity->posY.part16.integer);
}

/**
 * @brief Return squared integer-pixel distance to the target, retaining signed X conversion and existing Y arithmetic.
 */
s32 Entity_DistanceSquaredToEntity(struct EwramData_EntityData *entity, struct EwramData_EntityData *target)
{
    s32 dx = (s16)target->posX.part16.integer - (s16)entity->posX.part16.integer;
    s32 dy = target->posY.part16.integer - entity->posY.part16.integer;

    return dx * dx + dy * dy;
}

/**
 * @brief Return squared distance to a 16.16 point after truncation toward zero, preserving the original coordinate conversions.
 */
s32 Entity_DistanceSquaredToPoint(struct EwramData_EntityData *entity, s32 x, s32 y)
{
    s32 dx = INT_TOWARD_ZERO(x) - (s16)entity->posX.part16.integer;
    s32 dy = INT_TOWARD_ZERO(y) - entity->posY.part16.integer;

    return dx * dx + dy * dy;
}
