#include "code_080096AC.h"
#include "code_08009A0.h"
#include "code_08001004.h"
#include "code_08001194.h"
#include "code_08002454.h"
#include "code_08005894.h"
#include "code_08008750.h"
#include "code_080096AC.h"
#include "code_0800B700.h"
#include "code_0800CB00.h"
#include "code_0800F1FC.h"
#include "code_08012744.h"
#include "code/code_08014548.h"
#include "code/code_080211F0.h"
#include "code_08032444.h"
#include "code_08032E4C.h"
#include "code_08033CAC.h"
#include "code_08035930.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08050A3C.h"
#include "code_080D73B8.h"
#include "agb_multi_sio_sync.h"
#include "gba.h"
#include "macros.h"
#include "agb_sram.h"
#include "syscalls.h"

#include "constants/main.h"
#include "constants/songs.h"

#include "data/data_0E0334.h"
#include "data/data_0E3464.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"

/*
 * Work slots of the soul trade menu entity (this TU): own offer kind / soul
 * id and partner offer kind / soul id (sent with SoulTrade_SetOffer /
 * SoulTrade_SetAcknowledge, printed by SoulTrade_UpdateOfferNames); selected
 * soul kind (list 0-2, sSoulTradeListTextBases); partner-offer-shown flag;
 * selected soul list entry (halfword, and its low byte as read by
 * SoulTrade_MoveListCursor); cursor row inside the eight-row window and the
 * window's first row; link wait counter of GameModeSoulTradeMenu (counts to 6,
 * once stores a link id). Token-identical expansion.
 */
#define TRADE_OWN_KIND(e) e->unk_504.part8.unk_504
#define TRADE_OWN_SOUL(e) e->unk_504.part8.unk_505
#define TRADE_PARTNER_KIND(e) e->unk_504.part8.unk_506
#define TRADE_PARTNER_SOUL(e) e->unk_504.part8.unk_507
#define TRADE_LIST_KIND(e) e->unk_4F8.part8.unk_4F8
#define TRADE_PARTNER_SHOWN(e) e->unk_4F8.part8.unk_4F9
#define TRADE_SELECTED(e) e->unk_4FC.part16.unk_4FC
#define TRADE_SELECTED_BYTE(e) e->unk_4FC.part8.unk_4FC
#define TRADE_LIST_ROW(e) e->unk_50C.part16.unk_50C
#define TRADE_LIST_TOP(e) e->unk_50C.part16.unk_50E
#define TRADE_WAIT_COUNTER(e) e->unk_500.part16.unk_502

struct SoulKey {
    u8 soulType;
    u8 oneBasedSoulId;
    u8 pad_2[0x4 - 0x2];
};

extern struct SoulKey sKeySouls[];

u32 sSoulTypeCounts[4] = {
    0x37, 0x18, 0x23, 0x8
};

/**
 * @brief Fill the list at entityScratchBuffer with the owned souls of the kind, skipping
 * the five untradeable entries of sKeySouls, and pad it with 0xFF.
 * Return 1 when the list is not empty.
 */
s16 SoulTrade_BuildSoulList(s32 soulType)
{
    s16 any;
    s32 temp_r3;
    s32 j;
    s32 i;
    s32 count;
    struct SoulKey *excluded;
    s32 soulCount;
    s32 keep;

    soulCount = sSoulTypeCounts[soulType];
    count = 0;
    any = 1;

    for (i = 0; i < soulCount; )
    {
        if (SoulInventory_GetSoulTotal(soulType, i) == 0)
        {
            i++;
        }
        else
        {
            excluded = sKeySouls;
            j = 0;
            
            do
            {
                if (((soulType == excluded->soulType) && (i == (excluded->oneBasedSoulId - 1))))
                {
                    keep = 0;
                    break;
                }
                j += 1;
                excluded += 1;
                keep = 1;
            } while (j < 5);

            if (keep)
            {
                count[gEwramData->entityScratchBuffer] = i;
                i++;
                count++;
            }
            else
            {
                i++;
            }
        }
    }

    if (count == 0)
    {
        any = 0;
    }

    for (; count < 0x80; count++)
    {
        count[gEwramData->entityScratchBuffer] = 0xFF;
    }
    return any;
}

/* SoulTrade_MoveListCursor: was NONMATCH, matched.
 * One cursor helper for all three moves; it negates its own parameter in
 * the backward branch (the parameter copy is r4, -delta goes to the same
 * register).  The positive branch walks an explicit byte pointer.  The slot
 * read after the move goes through SlotAt() so the 0x143F4 offset is folded
 * into the add (reloaded from the pool) instead of being CSE'd into a
 * register shared with the next loop. */
static inline u8 SlotAt(s32 i)
{
    return i[gEwramData->entityScratchBuffer];
}

static inline s16 MoveCursor(s32 cursor, s32 delta)
{
    s32 count;

    count = 0;
    if (delta >= 0)
    {
        while (count < delta)
        {
            cursor++;
            count++;
            if (cursor[gEwramData->entityScratchBuffer] == 0xFF)
                return cursor - 1;
        }
    }
    else
    {
        delta = -delta;
        while (count < delta)
        {
            cursor--;
            count++;
            if (cursor < 0)
                return cursor + 1;
        }
    }
    return cursor;
}

static inline s32 FindSlot(s32 top, u8 id)
{
    s32 i;

    for (i = 0; i < 0x80; i++)
    {
        if (i[gEwramData->entityScratchBuffer] == id)
        {
            i -= top;
            if (i < 0)
                i = 0;
            return i;
        }
    }
    return -1;
}

/**
 * @brief Move the soul list cursor by delta entries, scrolling the eight-row
 * window when needed; select the soul under it. Return 1 when the
 * window scrolled.
 */
s32 SoulTrade_MoveListCursor(struct EwramData_EntityData *entity, s32 delta)
{
    s16 pos;

    if (delta > 0)
    {
        pos = MoveCursor(TRADE_LIST_ROW(entity) + TRADE_LIST_TOP(entity), delta);
        if (pos >= 0)
        {
            TRADE_SELECTED(entity) = SlotAt(pos);
            if (pos >= TRADE_LIST_TOP(entity) + 8)
            {
                pos = MoveCursor(TRADE_LIST_TOP(entity), delta);
                if (pos >= 0)
                {
                    TRADE_LIST_TOP(entity) = pos;
                    TRADE_LIST_ROW(entity) = FindSlot(TRADE_LIST_TOP(entity), TRADE_SELECTED_BYTE(entity));
                    return 1;
                }
            }
            else
            {
                TRADE_LIST_ROW(entity) = FindSlot(TRADE_LIST_TOP(entity), TRADE_SELECTED(entity));
            }
        }
    }
    else if (delta < 0)
    {
        pos = MoveCursor(TRADE_LIST_ROW(entity) + TRADE_LIST_TOP(entity), delta);
        if (pos >= 0)
        {
            TRADE_SELECTED(entity) = SlotAt(pos);
            if (pos < TRADE_LIST_TOP(entity))
            {
                pos = MoveCursor(TRADE_LIST_TOP(entity), delta);
                if (pos >= 0)
                {
                    TRADE_LIST_TOP(entity) = pos;
                    TRADE_LIST_ROW(entity) = FindSlot(TRADE_LIST_TOP(entity), TRADE_SELECTED_BYTE(entity));
                    return 1;
                }
            }
            else
            {
                TRADE_LIST_ROW(entity) = FindSlot(TRADE_LIST_TOP(entity), TRADE_SELECTED(entity));
            }
        }
    }
    return 0;
}

static inline void SoulTrade_SetOffer(s32 val_0, s32 val_1)
{
    (&gEwramData->soulTradePackets[0])->offeredSoulKind = val_0;
    (&gEwramData->soulTradePackets[0])->soulIds[0] = val_1;
}

static inline void SoulTrade_SetAcknowledge(s32 val_0, s32 val_1)
{
    u8* soulIds;
    (&gEwramData->soulTradePackets[0])->acknowledgedSoulKind = val_0;
    soulIds = gEwramData->soulTradePackets[0].soulIds;
    soulIds[1] = val_1;
}

static inline void GameModeSoulTradeMenu_inline_0(u8 status)
{
    (&gEwramData->soulTradePackets[0])->packetTag = 0xD;
    (&gEwramData->soulTradePackets[0])->handshakeFlags = status;
    Link_QueueSendBytes((u8*)&gEwramData->soulTradePackets[0], 4);
    Link_CopyReceivedBytes((u8*)&gEwramData->soulTradePackets[1], 4);
}

static inline void GameModeSoulTradeMenu_inline_1(struct EwramData_EntityData *menu, s32 entry)
{
    s32 baseRow;
    u8 *script;
    s32 col;
    s32 code;
    u8 *text;
    s32 more;
    s32 pos;

    text = Text_GetLocalizedEntry(entry);
    col = 0;
    baseRow = 1;
    more = 0;
    pos = -1;
    if (menu->unk_508.whole != entry)
    {
        menu->unk_508.whole = entry;
        Text12_SetRegion(0U, 1U, 0x20U, 4U);
        Text12_ClearRegion();
        Text12_SetRegion(0U, 1U, 0x1EU, 4U);
        Text12_SetCursor(0, 1);
        
        do
        {
            script = text;
            script += 2;
            Text12_SetCursor(col, baseRow + more);
            pos = Text12_Print(text, pos + 1);
            code = script[pos];
            more += 1;
            if (code != 6)
            {
                more = 0;
            }
        } while (more != 0);
    }
}

/**
 * @brief Play the cancel sound, return to trade stage 0xA, clear the own offer,
 * send status 2 and delete entity 6.
 */
void SoulTrade_ResetOffers(struct EwramData_EntityData *entity)
{
    struct SoulTradePacket* soulTradePackets;
    PlaySong(SE_240);
    gEwramData->gameModeUpdateStage = 0xA;
    gEwramData->gameModeSubStage = 0;

    SoulTrade_SetOffer(0, 0);

    SoulTrade_ExchangeStatus(2);
    TRADE_PARTNER_KIND(entity) = 0;
    TRADE_PARTNER_SOUL(entity) = 0;
    TRADE_OWN_KIND(entity) = 0;
    TRADE_OWN_SOUL(entity) = 0;

    EntityDelete(&gEwramData->entityData[6]);
}

u32 sSoulTradeListTextBases[3] = {
    0x1E4, 0x21C, 0x234
};

/**
 * @brief Reset the offers, reprint the description of the selected soul and
 * continue with the soul list (selection step 2).
 */
void SoulTrade_ReturnToSoulList(struct EwramData_EntityData *entity)
{
    u32 baseEntry;

    baseEntry = sSoulTradeListTextBases[TRADE_LIST_KIND(entity)];
    SoulTrade_ResetOffers(entity);
    SoulTrade_PrintDescription(entity, baseEntry + TRADE_SELECTED(entity));
    gEwramData->gameModeSubStage = 2;
    SoulTrade_SetOffer(0, 0);
    Text8_SetRegion(0, 0x12, 0xE, 1);
    Text8_ClearRegion();
}

/**
 * @brief Draw the kind icons of the own and the partner offer.
 */
void SoulTrade_DrawOfferIcons(struct EwramData_EntityData *entity)
{
    entity->posX.part16.integer = 0xF;

    if (TRADE_OWN_KIND(entity) != 0)
    {
        entity->animFrame = TRADE_OWN_KIND(entity) + 0xC;
        entity->posY.part16.integer = 0x38;
        Sprite_Draw(entity);
    }

    if (TRADE_PARTNER_KIND(entity) != 0)
    {
        entity->animFrame = TRADE_PARTNER_KIND(entity) + 0xC;
        entity->posY.part16.integer = 0x60;
        Sprite_Draw(entity);
    }
}

u32 sSoulNameTextBases[] = {
    0xE3, 0x11B, 0x133
};

// TODO: fake macros?
#define unk_25484_0_32(x) (((*(u32*)&gEwramData->soulTradePackets[x]) << 0x1E) >> 0x1E)
#define unk_25484_0_8(x) (((*(u8*)&gEwramData->soulTradePackets[x])) & 3)

/**
 * @brief Print the names of the own and the partner offered souls when they
 * change; clear the partner line when the partner withdraws.
 */
void SoulTrade_UpdateOfferNames(struct EwramData_EntityData *entity)
{
    s32 theirKind;
    s32 myKind;

    if (unk_25484_0_8(0))
    {
        myKind = unk_25484_0_32(0);
        if ((TRADE_OWN_KIND(entity) != myKind) || (TRADE_OWN_SOUL(entity) != (&gEwramData->soulTradePackets[0])->soulIds[0]))
        {
            TRADE_OWN_KIND(entity) = myKind;
            TRADE_OWN_SOUL(entity) = (&gEwramData->soulTradePackets[0])->soulIds[0];
            Text8_SetRegion(0, 0x12, 0xE, 1);
            Text8_ClearRegion();
            Text8_Print((struct unk_08506B38 *) Text_GetLocalizedEntry(sSoulNameTextBases[myKind - 1] + (&gEwramData->soulTradePackets[0])->soulIds[0]), 0);
        }
    }

    if (unk_25484_0_8(1))
    {
        theirKind = unk_25484_0_32(1);
        if ((TRADE_PARTNER_KIND(entity) != theirKind) || (TRADE_PARTNER_SOUL(entity) != (&gEwramData->soulTradePackets[1])->soulIds[0]))
        {
            TRADE_PARTNER_KIND(entity) = theirKind;
            TRADE_PARTNER_SOUL(entity) = (&gEwramData->soulTradePackets[1])->soulIds[0];
            Text8_SetRegion(0, 0x13, 0xE, 1);
            Text8_ClearRegion();
            Text8_Print((struct unk_08506B38 *) Text_GetLocalizedEntry(sSoulNameTextBases[theirKind - 1] + (&gEwramData->soulTradePackets[1])->soulIds[0]), 0);
            TRADE_PARTNER_SHOWN(entity) = 1;
        }
    }
    else if (TRADE_PARTNER_SHOWN(entity) != 0)
    {
        TRADE_PARTNER_SHOWN(entity) = TRADE_PARTNER_KIND(entity) = unk_25484_0_8(1);
        Text8_SetRegion(0, 0x13, 0xE, 1);
        Text8_ClearRegion();
    }
}

// TODO: is the parameter ignored or does A108 call this implicitly
/**
 * @brief Clear the windows and print the three soul kind entries (texts
 * 0x36C..0x36E) and the offer frames.
 */
s32 SoulTrade_DrawKindMenu(struct EwramData_EntityData *entity)
{
    s32 i;

    Text12_SetRegion(0, 1, 0x20, 4);
    Text12_ClearRegion();
    Text12_SetRegion(0, 1, 0x1E, 4);

    HudMap_ZeroTiles(0, 0x11, 0x40);
    Text8_SetRegion(0, 8, 0xE, 8);
    Text8_ClearRegion();

    for (i = 0; i < 8; i++)
    {
        Text8_SetRegion(0, i + 0x8, 0xE, 1);
        Text8_ClearRegion();
        HudMap_ZeroTiles(0x10, i + 4, 0xD);
    }

    for (i = 0; i < 3; i++)
    {
        Text8_SetRegion(0, i + 0x6, 0xE, 1);
        Text8_ClearRegion();
        Text8_Print((struct unk_08506B38 *) Text_GetLocalizedEntry(i + 0x36C), 0);
    }

    Text8_SetRegion(0, 0x12, 0xE, 1);
    Text8_ClearRegion();
    HudMap_WriteTiles(3, 7, 0xF240, 0xA);

    Text8_SetRegion(0, 0x13, 0xE, 1);
    Text8_ClearRegion();
    HudMap_WriteTiles(3, 0xC, 0xF260, 0xA);
}


static inline s32 SoulTrade_FindSoulIndex(s32 soul)
{
    s32 index;

    for (index = 0; index < 0x80; index++)
    {
        if (index[gEwramData->entityScratchBuffer] == soul)
        {
            if (index < 0)
            {
                index = 0;
            }
            return index;
        }
    }
    return -1;
}

// NOTE: HudMap_WriteTiles is implicitly called
/**
 * @brief Choose a soul kind with Up/Down; A opens its soul list when it has
 * tradeable souls (return 1), B cancels (return -1).
 */
s32 SoulTrade_HandleKindMenuInput(struct EwramData_EntityData *entity)
{
    s32 var_r0_2;
    s32 var_r2;
    s32 i;
    u8 firstSoul;
    s32 kind;
    struct EwramData *ewram;
    s32 cursorX;
    s32 cursorY;

    if (gEwramData->inputData.repeatedInput & KEY_DOWN)
    {
        PlaySong(SE_241);
        TRADE_LIST_KIND(entity) += 1;
        if (TRADE_LIST_KIND(entity) > 2)
        {
            TRADE_LIST_KIND(entity) = 0;
        }
    }
    else if (gEwramData->inputData.repeatedInput & KEY_UP)
    {
        PlaySong(SE_241);
        TRADE_LIST_KIND(entity) -= 1;
        if (TRADE_LIST_KIND(entity) < 0)
        {
            TRADE_LIST_KIND(entity) = 2;
        }
    }

    if (gEwramData->inputData.repeatedInput & KEY_A)
    {
        if (SoulTrade_BuildSoulList(TRADE_LIST_KIND(entity)) != 0)
        {
            TRADE_LIST_ROW(entity) = 0;
            TRADE_LIST_TOP(entity) = 0;
            firstSoul = TRADE_SELECTED(entity) = gEwramData->entityScratchBuffer[0];
            TRADE_LIST_ROW(entity) = SoulTrade_FindSoulIndex(firstSoul);
            
            SoulTrade_ClearMessageWindow(entity);
            SoulTrade_OpenSoulList(entity);
            PlaySong(SE_243);
            return 1;
        }
    }
    else if (gEwramData->inputData.repeatedInput & KEY_B)
    {
        PlaySong(SE_240);
        return -1;
    }

    // TODO: what?
    kind = TRADE_LIST_KIND(entity);
    ewram = gEwramData;
    cursorX = 0x7C;
    cursorY = (kind * 8) + 0x24;
    ewram->entityData[3].posX.part16.integer = cursorX;
    ewram->entityData[3].posY.part16.integer = cursorY;

    for (i = 0; i < 3; i++)
    {
        if (TRADE_LIST_KIND(entity) != i)
        {
            HudMap_WriteTiles(0x10, i + 4, i * 0x20 + 0xE0C0, 0xA);
        }
        else
        {
            HudMap_WriteTiles(0x10, i + 4, i * 0x20 + 0xF0C0, 0xA);
        }
    }
    return 0;
}

// NOTE: HudMap_WriteTiles is implicitly called
/**
 * @brief Print the description of the selected soul and draw the soul list.
 */
void SoulTrade_OpenSoulList(struct EwramData_EntityData *entity)
{
    s32 i;
    s32 baseEntry;

    baseEntry = sSoulTradeListTextBases[TRADE_LIST_KIND(entity)];

    GameModeSoulTradeMenu_inline_1(entity, baseEntry + TRADE_SELECTED(entity));

    for (i = 0; i < 8; i++)
    {
        Text8_SetRegion(0, i + 8, 0xE, 1);
        Text8_ClearRegion();
        HudMap_WriteTiles(0x10, i + 4, i * 0x20 + 0xF100, 0xA);
    }
    SoulTrade_DrawSoulList(TRADE_LIST_KIND(entity), TRADE_LIST_ROW(entity));
}


/**
 * @brief Draw up to eight soul names with their counts from list row top, with
 * the scroll arrows.
 */
void SoulTrade_DrawSoulList(s32 soulType, s32 first)
{
    s32 row;
    u32 baseEntry;

    row = 0;
    baseEntry = soulType[sSoulNameTextBases];
    if (first > 0)
    {
        HudMap_WriteTiles(0x1B, 4, 0xF24D, 1);
    }
    else
    {
        HudMap_ZeroTiles(0x1B, 4, 1);
    }

    for (row; row < 9; row++)
    {
        if ((first + row)[gEwramData->entityScratchBuffer] == 0xFF)
        {
            break;
        }
    }

    if (row == 9)
    {
        HudMap_WriteTiles(0x1B, 0xB, 0xF24E, 1);
    }
    else
    {
        HudMap_ZeroTiles(0x1B, 0xB, 1);
    }

    for (row = 0; row < 8; row++)
    {
        if (first[gEwramData->entityScratchBuffer] == 0xFF)
        {
            break;
        }
        Text8_SetRegion(0, row + 8, 0xE, 1);
        Text8_ClearRegion();
        Text8_Print((struct unk_08506B38 *) Text_GetLocalizedEntry(first[gEwramData->entityScratchBuffer] + baseEntry), 0);
        HudMap_PrintNumberAttr(0x1A, row + 4, SoulInventory_GetSoulTotal(soulType, first[gEwramData->entityScratchBuffer]), 1, 0xF200, 0);
        first++;
    }

    for (row; row < 8; row++)
    {
        Text8_SetRegion(0, row + 0x8, 0xE, 1);
        Text8_ClearRegion();
        HudMap_ZeroTiles(0x1A, row + 4, 1);
    }
}


// NOTE: HudMap_WriteTiles is implicitly called
/**
 * @brief Set up the BG0 text layer and window frame and print trade message
 * 0x354 + index.
 */
void SoulTrade_SetupMessageWindow(s32 choice)
{
    s32 i;

    DMA_FILL_32(3, 0, 0x0600E000, 0x800);
    gDisplayRegisters.bgCnt[0] = CREATE_BGCNT(2, 28, BGCNT_HIGH_PRIORITY, BGCNT_SIZE_256x256);
    BgPalette_LoadRange((u8 *)0x080E5C20, 4, 2, 0xD);
    BgPalette_LoadRange((u8 *)0x080E5C20, 3, 1, 0xF);
    Text_SetTileBuffer(VRAM_BASE + 0x8000);
    Text_SetForegroundColor(1);
    Text_SetBackgroundColor(0);

    switch (gEwramData->gameState.language)
    {
        case 2:
            Gfx_CopyResource((u8 *)0x08277984, VRAM_BASE + 0xC000, 0);
            break;

        case 3:
            Gfx_CopyResource((u8 *)0x0826C848, VRAM_BASE + 0xC000, 0);
            break;

        default:
            Gfx_CopyResource((u8 *)0x0827B200, VRAM_BASE + 0xC000, 0);
            break;
    }

    Text12_SetRegion(0, 1, 0x20, 4);
    Text12_ClearRegion();
    Text12_SetRegion(0, 1, 0x1E, 4);
    Text12_SetCursor(0, 1);

    for (i = 0; i < 4; i++)
    {
        HudMap_WriteTiles(3, i + 0xF, i * 0x20 + 0xF020, 0x17);
    }

    Text12_Print(Text_GetLocalizedEntry(choice + 0x354), 0);
}

/**
 * @brief Move through the soul list (Up/Down, Left/Right/L/R by eight); A
 * offers the soul (return 1), B goes back (return -1), Select accepts
 * when an offer exists.
 */
s32 SoulTrade_HandleSoulListInput(struct EwramData_EntityData *entity)
{
    s32 scrolled;
    s16 prevSoul;
    u32 baseEntry;
    s32 row;
    struct EwramData *ewram;
    s32 cursorX;
    s32 cursorY;

    baseEntry = sSoulTradeListTextBases[TRADE_LIST_KIND(entity)];
    prevSoul = TRADE_SELECTED(entity);
    scrolled = 0;

    if (gEwramData->inputData.repeatedInput & KEY_DOWN)
    {
        scrolled = SoulTrade_MoveListCursor(entity, 1);
    }
    else if (gEwramData->inputData.repeatedInput & KEY_UP)
    {
        scrolled = SoulTrade_MoveListCursor(entity, -1);
    }
    else if (gEwramData->inputData.repeatedInput & (KEY_RIGHT | KEY_R))
    {
        scrolled = SoulTrade_MoveListCursor(entity, 8);
    }
    else if (gEwramData->inputData.repeatedInput & (KEY_LEFT | KEY_L))
    {
        scrolled = SoulTrade_MoveListCursor(entity, -8);
    }

    if (prevSoul != TRADE_SELECTED(entity))
    {
        PlaySong(SE_241);
        GameModeSoulTradeMenu_inline_1(entity, baseEntry + TRADE_SELECTED(entity));
    }
    if (scrolled != 0)
    {
        SoulTrade_DrawSoulList(TRADE_LIST_KIND(entity), TRADE_LIST_TOP(entity));
    }

    // TODO: what?
    row = TRADE_LIST_ROW(entity);
    ewram = gEwramData;
    cursorX = 0x7C;
    cursorY = (row * 8) + 0x24;
    ewram->entityData[3].posX.part16.integer = cursorX;
    ewram->entityData[3].posY.part16.integer = cursorY;
    // gEwramData->unk_4E4[3].posX.part16.integer = 0x7C;
    // gEwramData->unk_4E4[3].posY.part16.integer = (entity->unk_50C * 8) + 0x24;

    if (gEwramData->inputData.repeatedInput & KEY_A)
    {
        SoulTrade_SetOffer(TRADE_LIST_KIND(entity) + 1, TRADE_SELECTED(entity));
        return 1;
    }
    else if (gEwramData->inputData.repeatedInput & KEY_B)
    {
        SoulTrade_DrawKindMenu(entity);
        PlaySong(SE_240);
        return -1;
    }
    else if ((TRADE_OWN_KIND(entity) != 0) && (gEwramData->inputData.repeatedInput & KEY_SELECT))
    {
        return 1;
    }
    return 0;
}

/**
 * @brief Selection steps: kind menu, soul list, then store both offers and
 * return 1; return -1 when cancelled.
 */
s32 SoulTrade_UpdateSelection(struct EwramData_EntityData *entity)
{
    s32 kindResult;
    s32 listResult;

    switch (gEwramData->gameModeSubStage)
    {
        case 0:
            SoulTrade_DrawKindMenu(entity);
            gEwramData->gameModeSubStage = 1;
            break;

        case 1:
            kindResult = SoulTrade_HandleKindMenuInput(entity);
            if (kindResult != 1)
            {
                if (kindResult == -1)
                {
                    return -1;
                }
            }
            else
            {
                gEwramData->gameModeSubStage = 2;
            }
            SoulTrade_UpdateOfferNames(entity);
            break;

        case 2:
            listResult = SoulTrade_HandleSoulListInput(entity);
            if (listResult == 1)
            {
                gEwramData->gameModeSubStage = 3;
            }
            else if (listResult == -1)
            {
                TRADE_OWN_KIND(entity) = 0;
                TRADE_PARTNER_KIND(entity) = 0;
                gEwramData->gameModeSubStage = 1;
            }
            SoulTrade_UpdateOfferNames(entity);
            break;

        case 3:
            SoulTrade_SetOffer(TRADE_OWN_KIND(entity), TRADE_OWN_SOUL(entity));
            SoulTrade_SetAcknowledge(TRADE_PARTNER_KIND(entity), TRADE_PARTNER_SOUL(entity));
            PlaySong(SE_243);
            SoulTrade_UpdateOfferNames(entity);
            return 1;
    }
    return 0;
}

#define unk_25484_4_32(x) (((*(u32*)&gEwramData->soulTradePackets[x]) << 0x18) >> 0x1C)
#define unk_25484_2_32(x) (((*(u32*)&gEwramData->soulTradePackets[x]) << 0x1C) >> 0x1E)

// HudMap_WriteTiles is implicitly called here
// Matching from the upstream scratch https://decomp.me/scratch/Luxz2 (field names mapped)


#define unk_25484_4_32(x) (((*(u32*)&gEwramData->soulTradePackets[x]) << 0x18) >> 0x1C)
#define unk_25484_2_32(x) (((*(u32*)&gEwramData->soulTradePackets[x]) << 0x1C) >> 0x1E)

static inline void SoulTrade_ExchangeStatusInline(u8 status)
{
    (&gEwramData->soulTradePackets[0])->packetTag = 0xD;
    (&gEwramData->soulTradePackets[0])->handshakeFlags = status;
    Link_QueueSendBytes((u8 *)&gEwramData->soulTradePackets[0], 4);
    Link_CopyReceivedBytes((u8 *)&gEwramData->soulTradePackets[1], 4);
}

static inline void SoulTrade_DrawTextEntry(struct EwramData_EntityData *menu, s32 entry)
{
    s32 baseRow;
    u8 *script;
    s32 col;
    s32 code;
    u8 *text;
    s32 more;
    s32 pos;

    text = Text_GetLocalizedEntry(entry);
    col = 0;
    baseRow = 1;
    more = 0;
    pos = -1;
    if (menu->unk_508.whole != entry)
    {
        menu->unk_508.whole = entry;
        Text12_SetRegion(0U, 1U, 0x20U, 4U);
        Text12_ClearRegion();
        Text12_SetRegion(0U, 1U, 0x1EU, 4U);
        Text12_SetCursor(0, 1);
        
        do
        {
            script = text;
            script += 2;
            Text12_SetCursor(col, baseRow + more);
            pos = Text12_Print(text, pos + 1);
            code = script[pos];
            more += 1;
            if (code != 6)
            {
                more = 0;
            }
        } while (more != 0);
    }
}

// HudMap_WriteTiles is implicitly called here
s32 GameModeSoulTradeMenu(void)
{
    s32 result;
    s32 linkId;
    s32 gfx;
    s32 ready8;
    s32 ready10;
    s32 ready12;
    s32 ready13;
    s32 ready14;
    s32 ready15;
    s32 status;
    s32 var_r4_16;
    s32 i;
    s32 j;
    s32 k;
    struct EwramData_EntityData *saveSelect;
    struct EwramData_EntityData *trade;
    u16 fadeIn;
    u16 fadeOut;
    u32 linked12;
    u32 linked13;
    u32 linked14;
    u32 linked15;
    s32 linked8;
    u32 linked10;
    u8 temp_r1_10;
    s32 choice;
    struct EwramData_EntityData *cursor;
    s32 cursorY;
    s32 cursorX;
    s32 peerFlags;
    s32 peerDone;
    s32 peerFlagsB;
    s32 peerConfirmed;
    s32 var_4;
    s32 var_5;
    s32 peerReady;

    saveSelect = &gEwramData->entityData[4];
    trade = &gEwramData->entityData[5];
    result = -1;
    gEwramData->softResetSkipOnce = 1;
    switch (gEwramData->gameModeUpdateStage)
    {
        case 0:
            DMA_FILL_32(3, 0, &gEwramData->soulTradePackets[0], 4);
            DMA_FILL_32(3, 0, &gEwramData->soulTradePackets[1], 4);
            gUnk_03002CB0.dispCnt = 0x1F00;
            gDisplayRegisters.bldCnt = 0x844;
            gDisplayRegisters.bldAlpha = 0xA06;
            gDisplayRegisters.bldY = 0x10;
            gEwramData->gameState.bossRushMode = 0;
            SaveSelect_Open(saveSelect);
            BgPalette_StartColorFade(0U, 0U, 0U, 0, 0x10000, 0x10);
            ObjPalette_StartColorFade(0U, 0U, 0U, 0, 0x10000, 0x10);
            gEwramData->gameModeUpdateStage = 1;
            gEwramData->gameModeSubStage = 0;
            break;

        case 1:
            Entity_UpdateAll();
            status = BgPalette_GetPendingFades();
            if (status == 0)
            {
                ObjPalette_StopColorFade();
                BgPalette_StopColorFade();
                gEwramData->gameModeUpdateStage = 3;
                gEwramData->gameModeSubStage = 0;
            }
            break;

        case 3:
            status = SaveSelect_Update(saveSelect);
            if (status == -1)
            {
                PlaySong(0xF0U);
                EntityDelete(&gEwramData->entityData[3]);
                EntityDelete(&gEwramData->entityData[6]);
                EntityDelete(&gEwramData->entityData[5]);
                SaveSelect_Close(&gEwramData->entityData[4]);
                gDisplayRegisters.bldCnt = 0xFF;
                gDisplayRegisters.bldY = 0x10;
                result = 1;
            }
            else if (status != 0)
            {
                PlaySong(0xF4U);
                gUnk_03002CB0.dispCnt &= 0xFBFF;
                gDisplayRegisters.bldCnt = 0x853;
                gDisplayRegisters.bldAlpha = 0x10;
                gEwramData->gameModeTimer = 0;
                gEwramData->gameModeUpdateStage = 4;
                gEwramData->gameModeSubStage = 0;
            }
            break;

        case 4:
            if (gDisplayRegisters.bldAlpha < 0x1000)
            {
                gEwramData->gameModeTimer += 2;
                fadeIn = gEwramData->gameModeTimer;
                gDisplayRegisters.bldAlpha = (fadeIn << 8) | (0x10 - fadeIn);
                if (gEwramData->gameModeTimer >= 0xD)
                {
                    saveSelect->state = 0;
                }
            }
            else
            {
                gEwramData->hBlankEffect.requestStop = 1;
                SaveSelect_Close(&gEwramData->entityData[4]);
                gDisplayRegisters.bldCnt = 0x853;
                gUnk_03002CB0.dispCnt &= 0xFBFF;
                SoulTrade_SetupMessageWindow(0);
                EntityDelete(&gEwramData->entityData[3]);
                EntityDelete(&gEwramData->entityData[6]);
                EntityDelete(trade);
                switch (gEwramData->gameState.language)
                {
                    default:
                        gfx = ObjGfx_LoadResource((u8 *)0x082052D8, 0);
                        break;
                    case 2:
                        gfx = ObjGfx_LoadResource((u8 *)0x0827798C, 0);
                        break;
                    case 3:
                        gfx = ObjGfx_LoadResource((u8 *)0x0826C840, 0);
                        break;
                }
                ObjPalette_LoadRange((u8 *)0x080E5E24, 6U, 1U, 0xDU);
                Sprite_Attach(trade, (u8 *)0x082571E0, gfx, 0xD);
                trade->updateFunc = (u32 *) SoulTrade_DrawOfferIcons;
                DMA_FILL_32(3, 0, 0x0600E000, 0x800);
                CursorSprite_Create(&gEwramData->entityData[3], 0xEU);
                BgMap_Setup(1U, (struct BgMapMetadata *)0x080E7164, 0U, 0U);
                BgMap_Setup(2U, (struct BgMapMetadata *)0x080E6F24, 0U, 0U);
                gDisplayRegisters.bgCnt[1] = 0x1D01;
                gDisplayRegisters.bgCnt[2] = 0x1E02;
                gDisplayRegisters.bgCnt[3] = 0x1F03;
                Text12_SetRegion(0U, 1U, 0x20U, 4U);
                Text12_ClearRegion();
                Text12_SetRegion(0U, 1U, 0x1EU, 4U);
                Text12_SetCursor(0, 1);
                for (i = 0; i < 4; i++)
                {
                    HudMap_WriteTiles(3, i + 0xF, i * 0x20 + 0xF020, 0x17);
                }
                SoulTrade_DrawTextEntry(trade, 0x373);
                gEwramData->gameModeTimer = 0x10;
                gEwramData->gameModeUpdateStage = 5;
                gEwramData->gameModeSubStage = 0;
            }
            Entity_UpdateAll();
            break;

        case 5:
            if (gEwramData->gameModeTimer != 0)
            {
                gEwramData->gameModeTimer -= 2;
                fadeOut = gEwramData->gameModeTimer;
                gDisplayRegisters.bldAlpha = (fadeOut << 8) | (0x10 - fadeOut);
            }
            else
            {
                gUnk_03002CB0.dispCnt |= 0xF00;
                gDisplayRegisters.bldCnt = 0x844;
                gDisplayRegisters.bldAlpha = 0xA06;
                gEwramData->gameModeUpdateStage = 6;
                gEwramData->gameModeSubStage = 0;
            }
            break;

        case 6:
            gEwramData->gameModeUpdateStage = 7;
            gEwramData->gameModeSubStage = 0;
            break;

        case 7:
            Link_Init();
            Link_Start();
            gEwramData->gameModeUpdateStage = 8;
            gEwramData->gameModeSubStage = 0;
            break;

        case 8:
            SoulTrade_DrawTextEntry(trade, 0x373);
            if (gEwramData->inputData.repeatedInput & 2)
            {
                gEwramData->gameModeUpdateStage = 9;
                gEwramData->gameModeSubStage = 0;
            }
            else
            {
                SoulTrade_ExchangeStatusInline(0);
                linked8 = 0;
                if (Link_IsConnected() != 0)
                {
                    linked8 = (Link_IsPeerActive() != linked8);
                }
                if (linked8 != 0)
                {
                    if ((&gEwramData->soulTradePackets[1])->packetTag != 0xD)
                    {
                        ready8 = 0;
                    }
                    else
                    {
                        ready8 = 1;
                    }
                }
                else
                {
                    ready8 = 0;
                }
                if (ready8 != 0)
                {
                    if ((&gEwramData->soulTradePackets[1])->handshakeFlags == 0)
                    {
                        if (++gEwramData->gameModeSubStage > 4)
                        {
                            if (MultiSioSyncGetId() < 2)
                            {
                                gEwramData->gameModeUpdateStage = 0xA;
                                gEwramData->gameModeSubStage = 0;
                            }
                            else
                            {
                                gEwramData->gameModeUpdateStage = 0x14;
                                gEwramData->gameModeSubStage = 0;
                            }
                        }
                    }
                }
            }
            break;

        case 20:
            EntityDelete(&gEwramData->entityData[3]);
            EntityDelete(&gEwramData->entityData[6]);
            Text8_SetRegion(0U, 8U, 0xEU, 8U);
            Text8_ClearRegion();
            for (j = 0; j < 8; j++)
            {
                Text8_SetRegion(0U, j + 8, 0xE, 1);
                Text8_ClearRegion();
                HudMap_ZeroTiles(0x10, j + 4, 0xD);
            }
            Link_Stop();
            Text12_SetRegion(0U, 1U, 0x20U, 4U);
            Text12_ClearRegion();
            Text12_SetRegion(0U, 1U, 0x1EU, 4U);
            Text12_SetCursor(0, 1);
            for (k = 0; k < 4; k++)
            {
                HudMap_WriteTiles(3, k + 0xF, k * 0x20 + 0xF020, 0x17);
            }
            SoulTrade_DrawTextEntry(trade, 0x372);
            gEwramData->gameModeUpdateStage = 0x15;
            gEwramData->gameModeSubStage = 0;
            gEwramData->gameModeTimer = 0;
            /* fallthrough */
        case 21:
            if (!(gEwramData->gameModeTimer & 0xF))
            {
                if (gEwramData->gameModeTimer & 0x10)
                {
                    HudMap_WriteTiles(0x1A, 0x11, 0xF22FU, 1);
                    HudMap_WriteTiles(0x1A, 0x12, 0xF23FU, 1);
                }
                else
                {
                    HudMap_WriteTiles(0x1A, 0x11, 0xF22EU, 1);
                    HudMap_WriteTiles(0x1A, 0x12, 0xF23EU, 1);
                }
            }
            gEwramData->gameModeTimer += 1;
            if (gEwramData->inputData.repeatedInput & 3)
            {
                gEwramData->gameModeTimer = 0;
                PlaySong(0xF0U);
                EntityDelete(&gEwramData->entityData[5]);
                SaveSelect_Close(&gEwramData->entityData[4]);
                gDisplayRegisters.bldCnt = 0xFF;
                gDisplayRegisters.bldY = 0x10;
                result = 1;
            }
            break;

        case 9:
            Link_Stop();
            PlaySong(0xF0U);
            EntityDelete(&gEwramData->entityData[3]);
            EntityDelete(&gEwramData->entityData[6]);
            EntityDelete(&gEwramData->entityData[5]);
            gDisplayRegisters.bldCnt = 0xFF;
            gDisplayRegisters.bldY = 0x10;
            result = 1;
            break;

        case 10:
            status = SoulTrade_UpdateSelection(trade);
            SoulTrade_ExchangeStatusInline(0);
            linked10 = 0;
            if (Link_IsConnected() != 0)
            {
                linked10 = (Link_IsPeerActive() != linked10);
            }
            if (linked10 != 0)
            {
                if ((&gEwramData->soulTradePackets[1])->packetTag != 0xD)
                {
                    ready10 = 0;
                }
                else
                {
                    ready10 = 1;
                }
            }
            else
            {
                ready10 = 0;
            }

            // TODO: get rid of gotos
            if (ready10 != 0)
                goto lbl_8ca;
            {
                if (TRADE_WAIT_COUNTER(trade) >= 6)
                    goto lbl_8d0;
                {
                    TRADE_WAIT_COUNTER(trade) += 1;
                    goto block_81;
                }
                lbl_8ca:
                TRADE_WAIT_COUNTER(trade) = 0;
                goto block_81;
                lbl_8d0:
                gEwramData->gameModeUpdateStage = 0x14;
                gEwramData->gameModeSubStage = 0;
                break;
            }
    block_81:
            if (status == -1)
            {
                gEwramData->gameModeUpdateStage = 9;
                gEwramData->gameModeSubStage = 0;
            }
            else if (status != 0)
            {
                CursorSprite_Create(&gEwramData->entityData[6], 0xEU);
                gEwramData->gameModeUpdateStage = 0xC;
                gEwramData->gameModeSubStage = 0;
            }
            Entity_UpdateAll();
            break;

        case 12:
            SoulTrade_DrawTextEntry(trade, 0x376);
            SoulTrade_UpdateOfferNames(trade);
            SoulTrade_ExchangeStatusInline(0);
            linked12 = 0;
            if (Link_IsConnected() != 0)
            {
                linked12 = (Link_IsPeerActive() != linked12);
            }
            if (linked12 != 0)
            {
                if ((&gEwramData->soulTradePackets[1])->packetTag != 0xD)
                {
                    ready12 = 0;
                }
                else
                {
                    ready12 = 1;
                }
            }
            else
            {
                ready12 = 0;
            }

            // TODO: get rid of gotos
            if (ready12 != 0)
                goto lbl_a22;
            {
                if (TRADE_WAIT_COUNTER(trade) >= 6)
                    goto lbl_a28;
                {
                    TRADE_WAIT_COUNTER(trade) += 1;
                    goto block_102;
                }
                lbl_a22:
                TRADE_WAIT_COUNTER(trade) = 0;
                goto block_102;
                lbl_a28:
                gEwramData->gameModeUpdateStage = 0x14;
                gEwramData->gameModeSubStage = 0;
                break;
            }
    block_102:
            Entity_UpdateAll();
            if (gEwramData->inputData.repeatedInput & 0x30)
            {
                PlaySong(0xF1U);
                gEwramData->gameModeSubStage ^= 1;
            }
    
            // TODO: what?
            choice = gEwramData->gameModeSubStage;
            cursor = &gEwramData->entityData[6];
            cursorX = (choice * 0x3C) + 0x50;
            cursorY = 0x8E;
            gEwramData->entityData[6].posX.part16.integer = cursorX;
            gEwramData->entityData[6].posY.part16.integer = cursorY;
            // gEwramData->entityData[6].posX.part16.integer = (gEwramData->gameModeSubStage * 0x3C) + 0x50;
            // gEwramData->entityData[6].posY.part16.integer = 0x8E;
    
            if (gEwramData->inputData.repeatedInput & 1)
            {
                if (gEwramData->gameModeSubStage == 0)
                {
                    EntityDelete(cursor);
                    gEwramData->gameModeUpdateStage = 0xD;
                    gEwramData->gameModeSubStage = 0;
                }
                else
                {
                    SoulTrade_ReturnToSoulList(trade);
                }
            }
            else if (gEwramData->inputData.repeatedInput & 2)
            {
                SoulTrade_ReturnToSoulList(trade);
            }
            break;

        case 13:
            SoulTrade_DrawTextEntry(trade, 0x373);
            Entity_UpdateAll();
            if (gEwramData->inputData.repeatedInput & 2)
            {
                SoulTrade_ResetOffers(trade);
            }
            else
            {
                SoulTrade_SetOffer(TRADE_OWN_KIND(trade), TRADE_OWN_SOUL(trade));
                SoulTrade_SetAcknowledge(TRADE_PARTNER_KIND(trade), TRADE_PARTNER_SOUL(trade));
                SoulTrade_ExchangeStatusInline(1);
                linked13 = 0;
                if (Link_IsConnected() != 0)
                {
                    linked13 = Link_IsPeerActive() != linked13;
                }
                if (linked13 != 0)
                {
                    if ((&gEwramData->soulTradePackets[1])->packetTag != 0xD)
                    {
                        ready13 = 0;
                    }
                    else
                    {
                        ready13 = 1;
                    }
                }
                else
                {
                    ready13 = 0;
                }

                if (ready13 == 0)
                {
                    if (TRADE_WAIT_COUNTER(trade) < 6)
                    {
                        TRADE_WAIT_COUNTER(trade) += 1;
                        linked13 = 2;
                    }
                    else
                    {
                        status = 0;
                        gEwramData->gameModeUpdateStage = 0x14;
                        gEwramData->gameModeSubStage = status;
                        break;
                    }
                }
                else
                {
                    TRADE_WAIT_COUNTER(trade) = 0;
                    linked13 = 1;
                }

                if (linked13 == 1)
                {
                    SoulTrade_UpdateOfferNames(trade);
                }
                // TODO: what?
                peerFlags = unk_25484_4_32(1);
                peerDone = 1;
                peerDone &= peerFlags;
                if (peerDone)
                {
                    gEwramData->gameModeUpdateStage = 0xE;
                    gEwramData->gameModeSubStage = 0;
                    TRADE_WAIT_COUNTER(trade) = 0;
                }
            }
            break;

        case 14:
            SoulTrade_DrawTextEntry(trade, 0x374);
            gEwramData->gameModeSubStage += 1;
            if (gEwramData->gameModeSubStage > 0x10)
            {
                gEwramData->gameModeUpdateStage = 0x14;
                gEwramData->gameModeSubStage = 0;
            }
            else
            {
                SoulTrade_SetOffer(TRADE_OWN_KIND(trade), TRADE_OWN_SOUL(trade));
                SoulTrade_SetAcknowledge(TRADE_PARTNER_KIND(trade), TRADE_PARTNER_SOUL(trade));
                SoulTrade_ExchangeStatusInline(1);
                linked14 = 0;
                if (Link_IsConnected() != 0)
                {
                    linked14 = Link_IsPeerActive() != linked14;
                }
                if (linked14 != 0)
                {
                    if ((&gEwramData->soulTradePackets[1])->packetTag != 0xD)
                    {
                        ready14 = 0;
                    }
                    else
                    {
                        ready14 = 1;
                    }
                }
                else
                {
                    ready14 = 0;
                }

                if (ready14 == 0)
                {
                    if (TRADE_WAIT_COUNTER(trade) <= 5)
                    {
                        TRADE_WAIT_COUNTER(trade) += 1;
                        status = 2;
                    }
                    else
                    {
                        status = 0;
                        gEwramData->gameModeUpdateStage = 0x14;
                        gEwramData->gameModeSubStage = status;
                        break;
                    }
                }
                else
                {
                    TRADE_WAIT_COUNTER(trade) = 0;
                    status = 1;
                }

                if (status == 1)
                {
                    { register u32 x_ asm("r0") = unk_25484_4_32(1); peerReady = x_ & status; } /* FAKEMATCH: pinned register (copy before the AND keeps this tail from cross-jumping with case 6) */
                    // var_4 = var_4 & 1;
                    // temp_r1_10 = unk_25484_4_32(1); // TODO
                    if (!(peerReady))
                    {
                        gEwramData->gameModeUpdateStage = 0x14;
                        gEwramData->gameModeSubStage = peerReady;
                    }
                    else if ((TRADE_LIST_KIND(trade) + 1) != unk_25484_2_32(1))
                    {
    
                    }
                    else if (TRADE_SELECTED(trade) != (&gEwramData->soulTradePackets[1])->soulIds[1])
                    {
    
                    }
                    else if (MultiSioSyncGetId() == 1)
                    {
                        // TODO: what?
                        peerFlagsB = unk_25484_4_32(1);
                        peerConfirmed = 4;
                        peerConfirmed &= peerFlagsB;
                        if (peerConfirmed)
                        {
                            gEwramData->gameModeUpdateStage = 0x10;
                            gEwramData->gameModeSubStage = 0;
                        }
                    }
                    else
                    {
                        linkId = MultiSioSyncGetId();
                        if (linkId == 0)
                        {
                            if (++trade->unk_500.part16.unk_500 > 4)
                            {
                                TRADE_WAIT_COUNTER(trade) = linkId;
                                gEwramData->gameModeUpdateStage = 0xF;
                                gEwramData->gameModeSubStage = 0;
                            }
                        }
                    }
                }
            }
            break;

        case 15:
            SoulTrade_DrawTextEntry(trade, 0x374);
            SoulTrade_SetOffer(TRADE_OWN_KIND(trade), TRADE_OWN_SOUL(trade));
            SoulTrade_SetAcknowledge(TRADE_PARTNER_KIND(trade), TRADE_PARTNER_SOUL(trade));
            SoulTrade_ExchangeStatusInline(5);
            linked15 = 0;
            if (Link_IsConnected() != 0)
            {
                linked15 = Link_IsPeerActive() != linked15;
            }
            if (linked15 != 0)
            {
                if ((&gEwramData->soulTradePackets[1])->packetTag != 0xD)
                {
                    ready15 = 0;
                }
                else
                {
                    ready15 = 1;
                }
            }
            else
            {
                ready15 = 0;
            }

            // TODO: get rid of gotos
            if (ready15 != 0)
                goto lbl_fde;
            {
                if (TRADE_WAIT_COUNTER(trade) >= 6)
                    goto lbl_4e4;
                {
                    TRADE_WAIT_COUNTER(trade) += 1;
                    goto block_186;
                }
                lbl_fde:
                TRADE_WAIT_COUNTER(trade) = 0;
                goto block_186;
                lbl_4e4:
                gEwramData->gameModeUpdateStage = 0x14;
                gEwramData->gameModeSubStage = 0;
                break;
            }
    block_186:
            if (++gEwramData->gameModeSubStage > 0x10)
            {
                gEwramData->gameModeUpdateStage = 0x10;
                gEwramData->gameModeSubStage = 0;
            }
            break;

        case 16:
            if (TRADE_PARTNER_KIND(trade) != 0)
            {
                SoulTrade_DrawTextEntry(trade, sSoulTradeListTextBases[TRADE_PARTNER_KIND(trade) - 1] + TRADE_PARTNER_SOUL(trade));
            }
            gEwramData->gameModeUpdateStage = 0x11;
            gEwramData->gameModeSubStage = 0;
            break;

        case 17:
            if (TRADE_PARTNER_KIND(trade) != 0)
            {
                SoulInventory_AddAmountToSoulTotal(TRADE_PARTNER_KIND(trade) - 1, TRADE_PARTNER_SOUL(trade), 1);
                SaveData_SaveSlotToSram(gEwramData->gameState.currentSaveSlot);
            }
            if (TRADE_OWN_KIND(trade) != 0)
            {
                SaveData_GiveAwaySoul(gEwramData->gameState.currentSaveSlot, TRADE_OWN_KIND(trade) - 1, TRADE_OWN_SOUL(trade));
            }
            else
            {
                SaveData_SaveSlotToSram(gEwramData->gameState.currentSaveSlot);
            }
            gEwramData->gameModeUpdateStage = 0x12;
            gEwramData->gameModeSubStage = 0;
            break;

        case 18:
            SoulTrade_DrawTextEntry(trade, 0x375);
            if (gEwramData->gameModeSubStage++ > 0x2F)
            {
                gEwramData->gameModeUpdateStage = 0x13;
                gEwramData->gameModeSubStage = 0;
            }
            break;

        case 19:
            TRADE_OWN_KIND(trade) = 0;
            TRADE_PARTNER_KIND(trade) = 0;
            SoulTrade_SetOffer(0, 0);
            gDisplayRegisters.bldCnt = 0x3FFF;
            SaveSelect_Open(saveSelect);
            EntityDelete(&gEwramData->entityData[3]);
            EntityDelete(&gEwramData->entityData[6]);
            gEwramData->gameModeTimer = 0;
            Link_Stop();
            gEwramData->gameModeUpdateStage = 3;
            gEwramData->gameModeSubStage = 0;
            break;
    }
    return result;
}


// NOTE: HudMap_WriteTiles is implicitly called
/**
 * @brief Clear the message window and redraw its frame.
 */
void SoulTrade_ClearMessageWindow(struct EwramData_EntityData *entity)
{
    s32 i;
    s32 var_r5;

    Text12_SetRegion(0U, 1U, 0x20U, 4U);
    Text12_ClearRegion();

    Text12_SetRegion(0U, 1U, 0x1EU, 4U);
    Text12_SetCursor(0U, 1U);

    for (i = 0; i < 4; i++)
    {
        HudMap_WriteTiles(3, i + 0xF, i * 0x20 + 0xF020, 0x17);
    }
}

/**
 * @brief Print the multi-line text id in the message window unless it is
 * already shown.
 */
void SoulTrade_PrintDescription(struct EwramData_EntityData *entity, s32 entry)
{
    // TODO: SoulTrade_PrintDescription is the same as GameModeSoulTradeMenu_inline_1
    s32 baseRow;
    u8 *script;
    s32 col;
    s32 code;
    u8 *text;
    s32 more;
    s32 pos;

    text = Text_GetLocalizedEntry(entry);
    col = 0;
    baseRow = 1;
    more = 0;
    pos = -1;
    if (entity->unk_508.whole != entry)
    {
        entity->unk_508.whole = entry;
        Text12_SetRegion(0U, 1U, 0x20U, 4U);
        Text12_ClearRegion();
        Text12_SetRegion(0U, 1U, 0x1EU, 4U);
        Text12_SetCursor(0, 1);
        
        do
        {
            script = text;
            script += 2;
            Text12_SetCursor(col, baseRow + more);
            pos = Text12_Print(text, pos + 1);
            code = script[pos];
            more += 1;
            if (code != 6)
            {
                more = 0;
            }
        } while (more != 0);
    }
}

/**
 * @brief Set the own trade status byte, queue the 4-byte trade record for
 * sending and copy the partner record.
 */
void SoulTrade_ExchangeStatus(u8 status)
{
    // TODO: SoulTrade_ExchangeStatus is the same as GameModeSoulTradeMenu_inline_0
    (&gEwramData->soulTradePackets[0])->packetTag = 0xD;
    (&gEwramData->soulTradePackets[0])->handshakeFlags = status;
    Link_QueueSendBytes((u8*)&gEwramData->soulTradePackets[0], 4);
    Link_CopyReceivedBytes((u8*)&gEwramData->soulTradePackets[1], 4);
}
