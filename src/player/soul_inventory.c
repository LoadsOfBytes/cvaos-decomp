#include "soul_inventory.h"
#include "gba.h"
#include "syscalls.h"

#include "constants/main.h"

#include "data/interrupt_table.h"
#include "data/debug_font.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"

/**
 * @brief 32444 | Get the total for the specified soul in group 1 (checks soulIndex + 1, what does this do?)
 * 
 * @param soulType Soul type (red, blue, yellow, or ability)
 * @param soulIndex Soul index (into soul inventory)
 * @return u8 Soul total
 */
u8 SoulInventory_GetFirstGroupSoulTotalDifferent(s32 soulType, s32 soulIndex)
{
    u8 *soulInventoryFirstGroup;
    u8 soulInventoryFirstGroupAmount;

    soulInventoryFirstGroup = SoulInventory_GetInventoryFirstGroupPointer(soulType);
    if (soulInventoryFirstGroup == NULL)
    {
        return 0;
    }

    soulInventoryFirstGroupAmount = soulInventoryFirstGroup[soulIndex >> 1];
    if (soulIndex & 1)
    {
        soulInventoryFirstGroupAmount = (soulInventoryFirstGroupAmount >> 4) & 0xF;
    }
    else
    {
        soulInventoryFirstGroupAmount = soulInventoryFirstGroupAmount & 0xF;
    }

    switch (soulType)
    {
        case 0:
            if ((soulIndex + 1) == gEwramData->playerStatus.equippedRedSoul)
            {
                soulInventoryFirstGroupAmount += 1;
            }
            break;

        case 1:
            if ((soulIndex + 1) == gEwramData->playerStatus.equippedBlueSoul)
            {
                soulInventoryFirstGroupAmount += 1;
            }
            break;

        case 2:
            if ((soulIndex + 1) == gEwramData->playerStatus.equippedYellowSoul)
            {
                soulInventoryFirstGroupAmount += 1;
            }
            break;
    }

    return soulInventoryFirstGroupAmount;
}

/**
 * @brief 324D0 | Add an additional amount to the first group total for the specified soul
 * 
 * @param soulType Soul type (red, blue, yellow, or ability)
 * @param soulIndex Soul index (into soul inventory)
 * @param additionalSoulAmount Additional amount to add to soul total
 */
void SoulInventory_AddAmountToFirstGroupTotal(s32 soulType, s32 soulIndex, s32 additionalSoulAmount)
{
    s32 newUpperSoulAmount;
    s32 newLowerSoulAmount;
    s8 soulFirstGroupAmount;
    u8 *soulInventoryFirstGroup;
    u8 *soulInventoryFirstGroupLower;
    u8 soulInventoryFirstGroupAmount;
    u8 *soulInventoryFirstGroupUpper;
    s8 totalFirstGroupSoulAmount;

    soulInventoryFirstGroup = SoulInventory_GetInventoryFirstGroupPointer(soulType);
    if (soulInventoryFirstGroup == NULL)
    {
        soulFirstGroupAmount = 0;
    }
    else
    {
        soulInventoryFirstGroupAmount = soulInventoryFirstGroup[soulIndex >> 1];
        if (soulIndex & 1)
        {
            soulInventoryFirstGroupAmount = (soulInventoryFirstGroupAmount >> 4) & 0xF;
        }
        else
        {
            soulInventoryFirstGroupAmount = soulInventoryFirstGroupAmount & 0xF;
        }
        soulFirstGroupAmount = soulInventoryFirstGroupAmount;
    }

    totalFirstGroupSoulAmount = soulFirstGroupAmount + additionalSoulAmount;
    if (totalFirstGroupSoulAmount < 0)
    {
        totalFirstGroupSoulAmount = 0;
    }
    else if (totalFirstGroupSoulAmount > 9)
    {
        totalFirstGroupSoulAmount = 9;
    }

    if (additionalSoulAmount > 0)
    {
        gEwramData->playerStatus.totalNbrSoulsCollected += additionalSoulAmount;
        if (gEwramData->playerStatus.totalNbrSoulsCollected > 999)
        {
            gEwramData->playerStatus.totalNbrSoulsCollected = 999;
        }
    }

    newUpperSoulAmount = totalFirstGroupSoulAmount;
    newLowerSoulAmount = newUpperSoulAmount;
    soulInventoryFirstGroupLower = SoulInventory_GetInventoryPointer(0, soulType);
    soulInventoryFirstGroupUpper = soulInventoryFirstGroupLower;
    if (soulInventoryFirstGroupLower != NULL)
    {
        if (soulIndex & 1)
        {
            // Set upper nibble
            soulInventoryFirstGroupLower[soulIndex >> 1] = (soulInventoryFirstGroupLower[soulIndex >> 1] & 0xF) | (newUpperSoulAmount << 4);
        }
        else
        {
            // Set lower nibble
            soulInventoryFirstGroupUpper[soulIndex >> 1] = (soulInventoryFirstGroupUpper[soulIndex >> 1] & 0xF0) | newLowerSoulAmount;
        }
    }
}

/**
 * @brief 32588 | Add an additional amount to the group total for the specified soul
 * 
 * @param soulGroupOffset Soul group offset (0 for first, 1 for second)
 * @param soulType Soul type (red, blue, yellow, or ability)
 * @param soulIndex Soul index (into soul inventory)
 * @param additionalSoulAmount Additional amount to add to soul total
 */
void SoulInventory_AddAmountToGroupTotal(u8 soulGroupOffset, s32 soulType, s32 soulIndex, s32 additionalSoulAmount)
{
    struct PlayerStatus *status;
    u8 inventorySoulAmount;
    s8 baseSoulAmount;
    s8 totalSoulAmount;
    s32 newSoulAmount;
    u8 *soulInventory;
    u8 *soulInventory_2;

    status = &gEwramData->playerStatus;
    soulInventory = NULL;
    switch (soulType)
    {
        case 0:
            soulInventory = status->redSoulInventory[soulGroupOffset];
            break;

        case 1:
            soulInventory = status->blueSoulInventory[soulGroupOffset];
            break;

        case 2:
            soulInventory = status->yellowSoulInventory[soulGroupOffset];
            break;

        case 3:
            soulInventory = status->abilitySoulInventory;
            break;
    }

    if (soulInventory == NULL)
    {
        baseSoulAmount = 0;
    }
    else
    {
        inventorySoulAmount = soulInventory[soulIndex >> 1];
        if (soulIndex & 1)
        {
            inventorySoulAmount = (inventorySoulAmount >> 4) & 0xF;
        }
        else
        {
            inventorySoulAmount = inventorySoulAmount & 0xF;
        }
        baseSoulAmount = inventorySoulAmount;
    }

    totalSoulAmount = baseSoulAmount + additionalSoulAmount;
    if (totalSoulAmount < 0)
    {
        totalSoulAmount = 0;
    }
    else if (totalSoulAmount > 9)
    {
        totalSoulAmount = 9;
    }
    newSoulAmount = totalSoulAmount;

    status = &gEwramData->playerStatus;
    soulInventory_2 = NULL;
    switch (soulType)
    {
        case 0:
            soulInventory_2 = status->redSoulInventory[soulGroupOffset];
            break;

        case 1:
            soulInventory_2 = status->blueSoulInventory[soulGroupOffset];
            break;

        case 2:
            soulInventory_2 = status->yellowSoulInventory[soulGroupOffset];
            break;

        case 3:
            soulInventory_2 = status->abilitySoulInventory;
            break;
    }

    if (soulInventory_2 != NULL)
    {
        if (soulIndex & 1)
        {
            // Set upper nibble
            soulInventory_2[soulIndex >> 1] = (soulInventory_2[soulIndex >> 1] & 0xF) | (newSoulAmount << 4);
        }
        else
        {
            // Set lower nibble
            soulInventory_2[soulIndex >> 1] = (soulInventory_2[soulIndex >> 1] & 0xF0) | newSoulAmount;
        }
    }
}

/**
 * @brief 326B8 | Get the total for the specified soul (sum of group 1 and group 2 totals)
 * 
 * @param soulType Soul type (red, blue, yellow, or ability)
 * @param soulIndex Soul index (into soul inventory)
 * @return u8 Soul total
 */
u8 SoulInventory_GetSoulTotal(s32 soulType, s32 soulIndex)
{
    struct PlayerStatus *status;
    u8 soulFirstGroupAmount;
    u8 soulSecondGroupAmount;
    u8 soulInventoryFirstGroupAmount;
    u8 soulInventorySecondGroupAmount;
    u8 *soulInventoryFirstGroup;
    u8 *soulInventorySecondGroup;

    status = &gEwramData->playerStatus;
    soulInventoryFirstGroup = NULL;
    switch (soulType)
    {
        case 0:
            soulInventoryFirstGroup = status->redSoulInventory[0];
            break;

        case 1:
            soulInventoryFirstGroup = status->blueSoulInventory[0];
            break;

        case 2:
            soulInventoryFirstGroup = status->yellowSoulInventory[0];
            break;

        case 3:
            soulInventoryFirstGroup = status->abilitySoulInventory;
            break;
    }

    if (soulInventoryFirstGroup == NULL)
    {
        soulFirstGroupAmount = 0;
    }
    else
    {
        soulInventoryFirstGroupAmount = soulInventoryFirstGroup[soulIndex >> 1];
        if (soulIndex & 1)
        {
            soulInventoryFirstGroupAmount = (soulInventoryFirstGroupAmount >> 4) & 0xF;
        }
        else
        {
            soulInventoryFirstGroupAmount = soulInventoryFirstGroupAmount & 0xF;
        }
        soulFirstGroupAmount = soulInventoryFirstGroupAmount;
    }

    status = &gEwramData->playerStatus;
    soulInventorySecondGroup = NULL;
    switch (soulType)
    {
        case 0:
            soulInventorySecondGroup = status->redSoulInventory[1];
            break;

        case 1:
            soulInventorySecondGroup = status->blueSoulInventory[1];
            break;

        case 2:
            soulInventorySecondGroup = status->yellowSoulInventory[1];
            break;

        case 3:
            soulInventorySecondGroup = status->abilitySoulInventory;
            break;
    }

    if (soulInventorySecondGroup == NULL)
    {
        soulSecondGroupAmount = 0;
    }
    else
    {
        soulInventorySecondGroupAmount = soulInventorySecondGroup[soulIndex >> 1];
        if (soulIndex & 1)
        {
            soulInventorySecondGroupAmount = (soulInventorySecondGroupAmount >> 4) & 0xF;
        }
        else
        {
            soulInventorySecondGroupAmount = soulInventorySecondGroupAmount & 0xF;
        }
        soulSecondGroupAmount = soulInventorySecondGroupAmount;
    }
    return soulFirstGroupAmount + soulSecondGroupAmount;
}

/**
 * @brief 3278C | Add an additional amount to the total for the specified soul
 * 
 * @param soulType Soul type (red, blue, yellow, or ability)
 * @param soulIndex Soul index (into soul inventory)
 * @param additionalSoulAmount Additional amount to add to soul total
 */
void SoulInventory_AddAmountToSoulTotal(s32 soulType, s32 soulIndex, s32 additionalSoulAmount)
{
    struct PlayerStatus *status;
    u32 soulInventorySecondGroupAmount;
    u32 soulSecondGroupAmount;
    u8 soulAmount;
    u8 *soulInventorySecondGroup;

    status = &gEwramData->playerStatus;
    soulInventorySecondGroup = NULL;
    switch (soulType)
    {
        case 0:
            soulInventorySecondGroup = status->redSoulInventory[1];
            break;

        case 1:
            soulInventorySecondGroup = status->blueSoulInventory[1];
            break;

        case 2:
            soulInventorySecondGroup = status->yellowSoulInventory[1];
            break;

        case 3:
            soulInventorySecondGroup = status->abilitySoulInventory;
            break;
    }

    if (soulInventorySecondGroup == NULL)
    {
        soulSecondGroupAmount = 0;
    }
    else
    {
        soulInventorySecondGroupAmount = soulInventorySecondGroup[soulIndex >> 1];
        if (soulIndex & 1)
        {
            soulInventorySecondGroupAmount = (soulInventorySecondGroupAmount >> 4) & 0xF;
        }
        else
        {
            soulInventorySecondGroupAmount = soulInventorySecondGroupAmount & 0xF;
        }
        soulSecondGroupAmount = soulInventorySecondGroupAmount;
    }
    soulAmount = soulSecondGroupAmount;

    if (soulAmount != 0 && additionalSoulAmount > 0)
    {
        if ((s16) (SoulInventory_GetSoulTotal(soulType, soulIndex) + additionalSoulAmount) > 9)
        {
            SoulInventory_AddAmountToGroupTotal(1, soulType, soulIndex, -additionalSoulAmount);
        }
        SoulInventory_AddAmountToGroupTotal(0, soulType, soulIndex, additionalSoulAmount);
    }
    else
    {
        SoulInventory_AddAmountToGroupTotal(0, soulType, soulIndex, additionalSoulAmount);
    }
}

/**
 * @brief 32844 | Set the second group total to the first group total, and clear the first group total
 * 
 * @param soulType Soul type (red, blue, yellow, or ability)
 * @param soulIndex Soul index (into soul inventory)
 */
void SoulInventory_TransferFirstGroupTotalToSecondGroupTotal(s32 soulType, s32 soulIndex)
{
    struct PlayerStatus *status;
    u32 soulInventoryFirstGroupAmount;
    u32 soulFirstGroupAmount;
    u8 *soulInventoryFirstGroup;
    u8 additionalSoulAmount;
    u8 newSoulAmount;

    status = &gEwramData->playerStatus;
    soulInventoryFirstGroup = NULL;
    switch (soulType)
    {
        case 0:
            soulInventoryFirstGroup = status->redSoulInventory[0];
            break;

        case 1:
            soulInventoryFirstGroup = status->blueSoulInventory[0];
            break;

        case 2:
            soulInventoryFirstGroup = status->yellowSoulInventory[0];
            break;

        case 3:
            soulInventoryFirstGroup = status->abilitySoulInventory;
            break;
    }

    if (soulInventoryFirstGroup == NULL)
    {
        soulFirstGroupAmount = 0;
    }
    else
    {
        soulInventoryFirstGroupAmount = soulInventoryFirstGroup[soulIndex >> 1];
        if (soulIndex & 1)
        {
            soulInventoryFirstGroupAmount = (soulInventoryFirstGroupAmount >> 4) & 0xF;
        }
        else
        {
            soulInventoryFirstGroupAmount = soulInventoryFirstGroupAmount & 0xF;
        }
        soulFirstGroupAmount = soulInventoryFirstGroupAmount;
    }
    additionalSoulAmount = soulFirstGroupAmount;

    if (additionalSoulAmount == 0)
        return;

    SoulInventory_AddAmountToGroupTotal(1, soulType, soulIndex, additionalSoulAmount);

    newSoulAmount = 0;
    status = &gEwramData->playerStatus;
    soulInventoryFirstGroup = NULL;
    switch (soulType)
    {
        case 0:
            soulInventoryFirstGroup = status->redSoulInventory[0];
            break;

        case 1:
            soulInventoryFirstGroup = status->blueSoulInventory[0];
            break;

        case 2:
            soulInventoryFirstGroup = status->yellowSoulInventory[0];
            break;

        case 3:
            soulInventoryFirstGroup = status->abilitySoulInventory;
            break;
    }

    if (soulInventoryFirstGroup != NULL)
    {
        if (soulIndex & 1)
        {
            // Set upper nibble
            soulInventoryFirstGroup[soulIndex >> 1] = (soulInventoryFirstGroup[soulIndex >> 1] & 0xF) | newSoulAmount;
        }
        else
        {
            // Set lower nibble
            soulInventoryFirstGroup[soulIndex >> 1] = (soulInventoryFirstGroup[soulIndex >> 1] & 0xF0) | newSoulAmount;
        }
    }
}

/**
 * @brief 32938 | Set the first group total to the second group total, and clear the second group total
 * 
 * @param soulType Soul type (red, blue, yellow, or ability)
 * @param soulIndex Soul index (into soul inventory)
 */
void SoulInventory_TransferSecondGroupTotalToFirstGroupTotal(s32 soulType, s32 soulIndex)
{
    struct PlayerStatus *status;
    u32 soulInventorySecondGroupAmount;
    u32 soulSecondGroupAmount;
    u8 *soulInventorySecondGroup;
    u8 additionalSoulAmount;
    u8 newSoulAmount;

    status = &gEwramData->playerStatus;
    soulInventorySecondGroup = NULL;
    switch (soulType)
    {
        case 0:
            soulInventorySecondGroup = status->redSoulInventory[1];
            break;

        case 1:
            soulInventorySecondGroup = status->blueSoulInventory[1];
            break;

        case 2:
            soulInventorySecondGroup = status->yellowSoulInventory[1];
            break;

        case 3:
            soulInventorySecondGroup = status->abilitySoulInventory;
            break;
    }

    if (soulInventorySecondGroup == NULL)
    {
        soulSecondGroupAmount = 0;
    }
    else
    {
        soulInventorySecondGroupAmount = soulInventorySecondGroup[soulIndex >> 1];
        if (soulIndex & 1)
        {
            soulInventorySecondGroupAmount = (soulInventorySecondGroupAmount >> 4) & 0xF;
        }
        else
        {
            soulInventorySecondGroupAmount = soulInventorySecondGroupAmount & 0xF;
        }
        soulSecondGroupAmount = soulInventorySecondGroupAmount;
    }
    additionalSoulAmount = soulSecondGroupAmount;

    if (additionalSoulAmount != 0)
    {
        SoulInventory_AddAmountToGroupTotal(0, soulType, soulIndex, additionalSoulAmount);

        newSoulAmount = 0;
        status = &gEwramData->playerStatus;
        soulInventorySecondGroup = NULL;
        switch (soulType)
        {
            case 0:
                soulInventorySecondGroup = status->redSoulInventory[1];
                break;

            case 1:
                soulInventorySecondGroup = status->blueSoulInventory[1];
                break;

            case 2:
                soulInventorySecondGroup = status->yellowSoulInventory[1];
                break;

            case 3:
                soulInventorySecondGroup = status->abilitySoulInventory;
                break;
        }

        if (soulInventorySecondGroup != NULL)
        {
            if (soulIndex & 1)
            {
                soulInventorySecondGroup[soulIndex >> 1] = (soulInventorySecondGroup[soulIndex >> 1] & 0xF) | newSoulAmount;
            }
            else
            {
                soulInventorySecondGroup[soulIndex >> 1] = (soulInventorySecondGroup[soulIndex >> 1] & 0xF0) | newSoulAmount;
            }
        }
    }
}

/**
 * @brief 32A2C | Get the total for the specified soul in group 1
 * 
 * @param soulType Soul type (red, blue, yellow, or ability)
 * @param soulIndex Soul index (into soul inventory)
 * @return u32 Soul total
 */
u32 SoulInventory_GetFirstGroupSoulTotal(s32 soulType, s32 soulIndex)
{
    // TODO: likely an inline for SoulInventory_GetFirstGroupSoulTotalDifferent and SoulInventory_AddAmountToFirstGroupTotal
    u32 soulInventoryFirstGroupAmount;
    u8 *soulInventoryFirstGroup;

    soulInventoryFirstGroup = SoulInventory_GetInventoryFirstGroupPointer(soulType);
    if (soulInventoryFirstGroup == NULL)
    {
        return 0;
    }

    soulInventoryFirstGroupAmount = soulInventoryFirstGroup[soulIndex >> 1];
    if (soulIndex & 1)
    {
        soulInventoryFirstGroupAmount = (soulInventoryFirstGroupAmount >> 4) & 0xF;
    }
    else
    {
        soulInventoryFirstGroupAmount = soulInventoryFirstGroupAmount & 0xF;
    }
    return soulInventoryFirstGroupAmount;
}

/**
 * @brief 32A5C | Get the pointer to the first group soul type inventory
 * 
 * @param soulType Soul type (red, blue, yellow, or ability)
 * @return u8* Pointer to first group soul type inventory
 */
u8* SoulInventory_GetInventoryFirstGroupPointer(s32 soulType)
{
    return SoulInventory_GetInventoryPointer(0, soulType);
}

/**
 * @brief 32A6C | Set the new amount for the soul in the first group
 * 
 * @param soulType Soul type (red, blue, yellow, or ability)
 * @param soulIndex Soul index (into soul inventory)
 * @param newSoulAmount New soul amount
 */
void SoulInventory_SetFirstGroupSoulTotal(s32 soulType, s32 soulIndex, s8 newSoulAmount)
{
    // TODO: likely an inline for SoulInventory_AddAmountToFirstGroupTotal
    u8 *soulInventoryFirstGroupLower;
    u8 *soulInventoryFirstGroupUpper;

    soulInventoryFirstGroupLower = SoulInventory_GetInventoryPointer(0, soulType);
    soulInventoryFirstGroupUpper = soulInventoryFirstGroupLower;
    if (soulInventoryFirstGroupLower != NULL)
    {
        if (soulIndex & 1)
        {
            // Set upper nibble
            soulInventoryFirstGroupLower[soulIndex >> 1] = (soulInventoryFirstGroupLower[soulIndex >> 1] & 0xF) | (newSoulAmount << 4);
        }
        else
        {
            // Set lower nibble
            soulInventoryFirstGroupUpper[soulIndex >> 1] = (soulInventoryFirstGroupUpper[soulIndex >> 1] & 0xF0) | newSoulAmount;
        }
    }
}

/**
 * @brief Return the bit for ability index n in playerStatus.abilityFlags. Index 3
 * gates Soma_TryStartDiveAttack; indices 0, 1, 2 and 4 gate other
 * Soma movement inputs.
 */
u8 Player_HasAbility(s32 ability)
{
    struct PlayerStatus *status;
    s32 bit;

    status = &gEwramData->playerStatus;
    bit = ability & 7;
    return status->abilityFlags[ability >> 3] & (1 << bit);
}

/**
 * @brief Set (nonzero value) or clear ability bit n in abilityFlags. New-game
 * reset clears bits 0..7, Julius_InitEntity sets bit 5 and a debug
 * menu sets bits 0..5.
 */
void Player_SetAbility(s32 ability, s32 enabled)
{
    struct PlayerStatus *status;
    s32 bit;
    u8 *flags;

    status = &gEwramData->playerStatus;
    flags = &status->abilityFlags[ability >> 3];
    bit = ability & 7;
    if (enabled != 0)
    {
        *flags |= (1 << bit);
    }
    else
    {
        *flags &= ~(1 << bit);
    }
}

const u8 sSoulPickupPalettes[] = {
    7, 6, 8, 9
};

/**
 * @brief Return the shared pickup/effect palette for the supplied soul kind from the existing four-entry table; preserve unchecked indexing.
 */
u8 Soul_GetPickupPalette(s32 soulType)
{
    return sSoulPickupPalettes[soulType];
}

/**
 * @brief 32B20 | Get the pointer to the group soul type inventory
 * 
 * @param soulGroupOffset Soul group offset (0 for first, 1 for second)
 * @param soulType Soul type (red, blue, yellow, or ability)
 * @return u8* Pointer to group soul type inventory
 */
u8* SoulInventory_GetInventoryPointer(u8 soulGroupOffset, s32 soulType)
{
    // TODO: inline? looks similar to several functions above
    struct PlayerStatus *status;
    u8 *soulInventory;

    status = &gEwramData->playerStatus;
    soulInventory = NULL;
    switch (soulType)
    {
        case 0:
            soulInventory = status->redSoulInventory[soulGroupOffset];
            break;

        case 1:
            soulInventory = status->blueSoulInventory[soulGroupOffset];
            break;

        case 2:
            soulInventory = status->yellowSoulInventory[soulGroupOffset];
            break;

        case 3:
            soulInventory = status->abilitySoulInventory;
            break;
    }

    return soulInventory;
}

/**
 * @brief 32B88 | Get the total for the specified soul and group
 * 
 * @param soulGroupOffset Soul group offset (0 for first, 1 for second)
 * @param soulType Soul type (red, blue, yellow, or ability)
 * @param soulIndex Soul index (into soul inventory)
 * @return u8 To document
 */
u8 SoulInventory_GetGroupSoulTotal(u8 soulGroupOffset, s32 soulType, s32 soulIndex)
{
    // TODO: inline? looks similar to several functions above
    struct PlayerStatus *status;
    u8 soulInventoryAmount;
    u8 *soulInventory;

    status = &gEwramData->playerStatus;
    soulInventory = NULL;
    switch (soulType)
    {
        case 0:
            soulInventory = status->redSoulInventory[soulGroupOffset];
            break;

        case 1:
            soulInventory = status->blueSoulInventory[soulGroupOffset];
            break;

        case 2:
            soulInventory = status->yellowSoulInventory[soulGroupOffset];
            break;

        case 3:
            soulInventory = status->abilitySoulInventory;
            break;
    }

    if (soulInventory == NULL)
    {
        return 0;
    }

    soulInventoryAmount = soulInventory[soulIndex >> 1];
    if (soulIndex & 1)
    {
        soulInventoryAmount = (soulInventoryAmount >> 4) & 0xF;
    }
    else
    {
        soulInventoryAmount = soulInventoryAmount & 0xF;
    }
    return soulInventoryAmount;
}

/**
 * @brief 32C10 | To document
 * 
 * @param soulGroupOffset Soul group offset (0 for first, 1 for second)
 * @param soulType Soul type (red, blue, yellow, or ability)
 * @param soulIndex Soul index (into soul inventory)
 * @param newSoulAmount To document
 */
void SoulInventory_SetGroupTotalAmount(u8 soulGroupOffset, s32 soulType, s32 soulIndex, s8 newSoulAmount)
{
    // TODO: inline? looks similar to several functions above
    struct PlayerStatus *status;
    u8 *soulInventory;

    status = &gEwramData->playerStatus;
    soulInventory = NULL;

    switch (soulType)
    {
        case 0:
            soulInventory = status->redSoulInventory[soulGroupOffset];
            break;

        case 1:
            soulInventory = status->blueSoulInventory[soulGroupOffset];
            break;

        case 2:
            soulInventory = status->yellowSoulInventory[soulGroupOffset];
            break;

        case 3:
            soulInventory = status->abilitySoulInventory;
            break;
    }

    if (soulInventory != NULL)
    {
        if (soulIndex & 1)
        {
            soulInventory[soulIndex >> 1] = (soulInventory[soulIndex >> 1] & 0xF) | (newSoulAmount << 4);
        }
        else
        {
            soulInventory[soulIndex >> 1] = (soulInventory[soulIndex >> 1] & 0xF0) | newSoulAmount;
        }
    }
}

/**
 * @brief Set both slot links of the loaded progress block: slotLinks[0] (source
 * slot + 1 of a copy) and slotLinks[1] (copy slot + 1 of a source).
 */
void SaveLink_SetSlots(u8 sourceLink, u8 copyLink)
{
    struct GameState *gameState;

    gameState = &gEwramData->gameState;
    gameState->slotLinks[0] = sourceLink;
    gameState->slotLinks[1] = copyLink;
}

/**
 * @brief Set slot link slotLinks[index] (0 source, 1 copy) to slot + 1, or 0 to
 * clear it.
 */
void SaveLink_SetSlot(u8 index, u8 link)
{
    struct GameState *gameState;

    gameState = &gEwramData->gameState;
    gameState->slotLinks[index] = link;
}

/**
 * @brief Clear the copy link slotLinks[1] (the copy of this slot was deleted).
 */
void SaveLink_ClearCopySlot(void)
{
    gEwramData->gameState.slotLinks[1] = 0;
}

// Last index for each soul type
const u8 sSoulTypeBaseIndices[] = {
    0x37, 0x18, 0x23, 0x8
};

/**
 * @brief Clear the source link slotLinks[0] and move every soul count of the
 * second group back into the first group (the source slot was deleted).
 */
void SaveLink_DetachFromSource(void)
{
    struct GameState *gameState;
    s32 soulType;
    s32 soulIndex;

    gameState = &gEwramData->gameState;
    gameState->slotLinks[0] = 0;

    for (soulType = 0; soulType < 3; soulType++)
    {
        soulIndex = sSoulTypeBaseIndices[soulType];
        do
        {
            soulIndex -= 1;
            SoulInventory_TransferSecondGroupTotalToFirstGroupTotal(soulType, soulIndex);
        }
        while (soulIndex != 0);
    }
}

/**
 * @brief Mark the loaded slot as a copy of source (slotLinks[0] = source, no copy
 * link) and move every soul count of the first group into the second
 * group.
 */
void SaveLink_AttachToSource(u8 sourceLink)
{
    struct GameState *gameState;
    s32 soulType;
    s32 soulIndex;

    gameState = &gEwramData->gameState;
    gameState->slotLinks[0] = sourceLink;
    gameState->slotLinks[1] = 0;

    for (soulType = 0; soulType < 3; soulType++)
    {
        soulIndex = sSoulTypeBaseIndices[soulType];
        do
        {
            soulIndex -= 1;
            SoulInventory_TransferFirstGroupTotalToSecondGroupTotal(soulType, soulIndex);
        }
        while (soulIndex != 0);
    }
}

/**
 * @brief If the soul total is zero, clear a matching red/blue/yellow equipped soulIndex+1; retain the existing type dispatch.
 */
void SoulInventory_UnequipIfUnowned(s32 soulType, s32 soulIndex)
{
    struct PlayerStatus *status;

    status = &gEwramData->playerStatus;

    if (SoulInventory_GetSoulTotal(soulType, soulIndex) != 0)
        return;

    switch (soulType)
    {
        case 0:
            if (status->equippedRedSoul == (soulIndex + 1))
            {
                status->equippedRedSoul = 0;
            }
            break;

        case 1:
            if (status->equippedBlueSoul == (soulIndex + 1))
            {
                status->equippedBlueSoul = 0;
            }
            break;

        case 2:
            if (status->equippedYellowSoul == (soulIndex + 1))
            {
                status->equippedYellowSoul = 0;
            }
            break;
    }
}

/**
 * @brief Add only positive amounts to the cumulative collected-soul total and cap it at 999 using the existing field arithmetic.
 */
void SoulInventory_AddCollectedTotal(s32 amount)
{
    if (amount > 0)
    {
        gEwramData->playerStatus.totalNbrSoulsCollected += amount;
        if (gEwramData->playerStatus.totalNbrSoulsCollected > 999)
        {
            gEwramData->playerStatus.totalNbrSoulsCollected = 999;
        }
    }
}

/**
 * @brief Count distinct owned souls in types zero through two and return count*1000 divided by their total entry count; exclude ability souls.
 */
s32 SoulInventory_GetCompletionTenths(void)
{
    s32 soulIndex;
    s32 soulType;
    s32 total;
    s32 owned;

    owned = 0;
    for (soulType = 0; soulType < 3; soulType++)
    {
        total = sSoulTypeBaseIndices[soulType];
        for (soulIndex = 0; soulIndex < total; soulIndex++)
        {
            if (SoulInventory_GetSoulTotal(soulType, soulIndex) != 0)
            {
                owned += 1;
            }
        }
    }

    total = 0;
    for (soulType = 0; soulType < 3; soulType++)
    {
        total += sSoulTypeBaseIndices[soulType];
    }

    owned *= 1000;
    return Div(owned, total);
}
