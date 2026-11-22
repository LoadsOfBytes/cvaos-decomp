#ifndef CODE_080096AC_H
#define CODE_080096AC_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

s16 SoulTrade_BuildSoulList(s32 soulType);
s32 SoulTrade_MoveListCursor(struct EwramData_EntityData *entity, s32 delta);
void SoulTrade_ResetOffers(struct EwramData_EntityData *entity);
void SoulTrade_ReturnToSoulList(struct EwramData_EntityData *entity);
void SoulTrade_DrawOfferIcons(struct EwramData_EntityData *entity);
void SoulTrade_UpdateOfferNames(struct EwramData_EntityData *entity);
s32 SoulTrade_DrawKindMenu(struct EwramData_EntityData *entity);
s32 SoulTrade_HandleKindMenuInput(struct EwramData_EntityData *entity);
void SoulTrade_OpenSoulList(struct EwramData_EntityData *entity);
void SoulTrade_DrawSoulList(s32 soulType, s32 first);
void SoulTrade_SetupMessageWindow(s32 choice);
s32 SoulTrade_HandleSoulListInput(struct EwramData_EntityData *entity);
s32 SoulTrade_UpdateSelection(struct EwramData_EntityData *entity);
s32 GameModeSoulTradeMenu(void);
void SoulTrade_ClearMessageWindow(struct EwramData_EntityData *entity);
void SoulTrade_PrintDescription(struct EwramData_EntityData *entity, s32 entry);
void SoulTrade_ExchangeStatus(u8 status);

#endif /* CODE_080096AC_H */
