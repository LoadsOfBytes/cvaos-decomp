#ifndef CODE_08032444_H
#define CODE_08032444_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

u8 SoulInventory_GetFirstGroupSoulTotalDifferent(s32 soulType, s32 soulIndex);
void SoulInventory_AddAmountToFirstGroupTotal(s32 soulType, s32 soulIndex, s32 additionalSoulAmount);
void SoulInventory_AddAmountToGroupTotal(u8 soulGroupOffset, s32 soulType, s32 soulIndex, s32 additionalSoulAmount);
u8 SoulInventory_GetSoulTotal(s32 soulType, s32 soulIndex);
void SoulInventory_AddAmountToSoulTotal(s32 soulType, s32 soulIndex, s32 additionalSoulAmount);
void SoulInventory_TransferFirstGroupTotalToSecondGroupTotal(s32 soulType, s32 soulIndex);
void SoulInventory_TransferSecondGroupTotalToFirstGroupTotal(s32 soulType, s32 soulIndex);
u32 SoulInventory_GetFirstGroupSoulTotal(s32 soulType, s32 soulIndex);
u8* SoulInventory_GetInventoryFirstGroupPointer(s32 soulType);
void SoulInventory_SetFirstGroupSoulTotal(s32 soulType, s32 soulIndex, s8 newSoulAmount);
u8 Player_HasAbility(s32 ability);
void Player_SetAbility(s32 ability, s32 enabled);
u8 Soul_GetPickupPalette(s32 soulType);
u8* SoulInventory_GetInventoryPointer(u8 soulGroupOffset, s32 soulType);
u8 SoulInventory_GetGroupSoulTotal(u8 soulGroupOffset, s32 soulType, s32 soulIndex);
void SoulInventory_SetGroupTotalAmount(u8 soulGroupOffset, s32 soulType, s32 soulIndex, s8 newSoulAmount);
void SaveLink_SetSlots(u8 sourceLink, u8 copyLink);
void SaveLink_SetSlot(u8 index, u8 link);
void SaveLink_ClearCopySlot(void);
void SaveLink_DetachFromSource(void);
void SaveLink_AttachToSource(u8 sourceLink);
void SoulInventory_UnequipIfUnowned(s32 soulType, s32 soulIndex);
void SoulInventory_AddCollectedTotal(s32 amount);
s32 SoulInventory_GetCompletionTenths(void);

#endif /* CODE_08032444_H */