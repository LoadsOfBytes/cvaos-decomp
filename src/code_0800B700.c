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
#include "code_080109F4.h"
#include "code_08011DD0.h"
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
#include "code/code_08060B98.h"
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
 * Sound test menu (GameModeSoundTestMenu) keeps its state in the slots of
 * entityData[2]: the selected row (category), the selected track of each
 * category and the song currently playing per category; the text entity
 * remembers which entry it shows. The arguments are entity lvalues
 * (gEwramData->entityData[2]) or pointers. Token-identical expansion.
 */
#define SOUND_TEST_ROW(s) s.unk_4FC.part16.unk_4FC
#define SOUND_TEST_TRACK(s, i) s.unk_4FC.part16.unk_4FE[i]
#define SOUND_TEST_PLAYING(s, i) s.unk_504.part16.unk_506[i]
#define SOUND_TEST_TEXT_ENTRY(e) e->unk_4FC.whole

// NOTE: HudMap_WriteTiles is implicitly called

/**
 * @brief Set up BG0 text for the sound test: load fonts, print the title text
 * id and the two selected numbers, and draw the 0x390..0x392 labels.
 */
void SoundTestMenu_DrawScreen(s32 index)
{
    s32 i;

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

    DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
    DMA_FILL_32(3, 0, VRAM_BASE + 0x8000, 0x800);
    Text12_SetRegion(0, 1, 0x1E, 4);
    Text12_ClearRegion();
    Text12_SetRegion(0, 1, 0x1C, 4);
    HudMap_ZeroTiles(0, 0x11, 0x40);

    for (i = 0; i < 4; i++)
    {
        HudMap_WriteTiles(3, i + 0xF, i * 0x20 + 0xF020, 0x15);
    }

    Text12_Print(Text_GetLocalizedEntry(index), 0);

    for (i = 0; i < 2; i++)
    {
        HudMap_PrintNumberAttr(0x12, i * 2 + 6, SOUND_TEST_TRACK(gEwramData->entityData[2], i), 2, 0xF200, 1);
    }

    Text8_SetRegion(0, 8, 8, 1);
    Text8_ClearRegion();
    Text8_Print((struct unk_08506B38 *) Text_GetLocalizedEntry(0x390), 0);

    Text8_SetRegion(0, 9, 8, 1);
    Text8_ClearRegion();
    Text8_Print((struct unk_08506B38 *) Text_GetLocalizedEntry(0x391), 0);

    Text8_SetRegion(0, 10, 8, 1);
    Text8_ClearRegion();
    Text8_Print((struct unk_08506B38 *) Text_GetLocalizedEntry(0x392), 0);

    HudMap_WriteTiles(0xB, 6, 0xF100, 6);
    HudMap_WriteTiles(0xB, 8, 0xF120, 6);
    HudMap_WriteTiles(0xB, 0xA, 0xF140, 6);
}

struct SoundTestCategoryRange {
    s16 baseSongId;
    s16 maxSelectionIndex;
};

const struct SoundTestCategoryRange sSoundTestCategoryRanges[3] = {
    [0] = {
        .baseSongId = 0,
        .maxSelectionIndex = 0x1C
    },
    [1] = {
        .baseSongId = 0x190,
        .maxSelectionIndex = 0x56
    },
    [2] = {
        .baseSongId = 0,
        .maxSelectionIndex = 0x1
    }
};

struct SoundTestTrackRecord {
    u16 songId;
    u16 titleTextId;
};

// TODO: determine if all of this data is sSoundTestTracks
const struct SoundTestTrackRecord sSoundTestTracks[95] = {
    [0] = {
        .songId = 0x1B,
        .titleTextId = 0x03AC
    },
    [1] = {
        .songId = 0x0E,
        .titleTextId = 0x03A0
    },
    [2] = {
        .songId = 0x15,
        .titleTextId = 0x03A8
    },
    [3] = {
        .songId = 0x02,
        .titleTextId = 0x0394
    },
    [4] = {
        .songId = 0x08,
        .titleTextId = 0x039A
    },
    [5] = {
        .songId = 0x07,
        .titleTextId = 0x0399
    },
    [6] = {
        .songId = 0x05,
        .titleTextId = 0x0397
    },
    [7] = {
        .songId = 0x06,
        .titleTextId = 0x0398
    },
    [8] = {
        .songId = 0x09,
        .titleTextId = 0x039B
    },
    [9] = {
        .songId = 0x01,
        .titleTextId = 0x0393
    },
    [10] = {
        .songId = 0x03,
        .titleTextId = 0x0395
    },
    [11] = {
        .songId = 0x0A,
        .titleTextId = 0x039C
    },
    [12] = {
        .songId = 0x04,
        .titleTextId = 0x0396
    },
    [13] = {
        .songId = 0x0B,
        .titleTextId = 0x039D
    },
    [14] = {
        .songId = 0x0C,
        .titleTextId = 0x039E
    },
    [15] = {
        .songId = 0x0D,
        .titleTextId = 0x039F
    },
    [16] = {
        .songId = 0x0F,
        .titleTextId = 0x03A1
    },
    [17] = {
        .songId = 0x10,
        .titleTextId = 0x03A2
    },
    [18] = {
        .songId = 0x11,
        .titleTextId = 0x03A3
    },
    [19] = {
        .songId = 0x13,
        .titleTextId = 0x03A5
    },
    [20] = {
        .songId = 0x1F,
        .titleTextId = 0x03A6
    },
    [21] = {
        .songId = 0x12,
        .titleTextId = 0x03A4
    },
    [22] = {
        .songId = 0x14,
        .titleTextId = 0x03A7
    },
    [23] = {
        .songId = 0x16,
        .titleTextId = 0x03A9
    },
    [24] = {
        .songId = 0x21,
        .titleTextId = 0x03AE
    },
    [25] = {
        .songId = 0x23,
        .titleTextId = 0x03AF
    },
    [26] = {
        .songId = 0x20,
        .titleTextId = 0x03AA
    },
    [27] = {
        .songId = 0x18,
        .titleTextId = 0x03AD
    },
    [28] = {
        .songId = 0x1A,
        .titleTextId = 0x03AB
    },
    [29] = {
        .songId = 0x400,
        .titleTextId = 0x0002
    },
    [30] = {
        .songId = 0x1F,
        .titleTextId = 0x001F
    },
    [31] = {
        .songId = 0x1F,
        .titleTextId = 0x001F
    },
    [32] = {
        .songId = 0x1F,
        .titleTextId = 0x001F
    },
    [33] = {
        .songId = 0x1F,
        .titleTextId = 0x001F
    },
    [34] = {
        .songId = 0x1F,
        .titleTextId = 0x001F
    },
    [35] = {
        .songId = 0x1F,
        .titleTextId = 0x001F
    },
    [36] = {
        .songId = 0x1F,
        .titleTextId = 0x001F
    },
    [37] = {
        .songId = 0x1F,
        .titleTextId = 0x001F
    },
    [38] = {
        .songId = 0x00,
        .titleTextId = 0x0000
    },
    [39] = {
        .songId = 0x00,
        .titleTextId = 0x0000
    },
    [40] = {
        .songId = 0x00,
        .titleTextId = 0x0000
    },
    [41] = {
        .songId = 0x00,
        .titleTextId = 0x0000
    },
    [42] = {
        .songId = 0x00,
        .titleTextId = 0x0000
    },
    [43] = {
        .songId = 0x00,
        .titleTextId = 0x0000
    },
    [44] = {
        .songId = 0x00,
        .titleTextId = 0x0000
    },
    [45] = {
        .songId = 0x00,
        .titleTextId = 0x0000
    },
    [46] = {
        .songId = 0x400,
        .titleTextId = 0x000D
    },
    [47] = {
        .songId = 0x39CE,
        .titleTextId = 0x6B5A
    },
    [48] = {
        .songId = 0x04,
        .titleTextId = 0x6400
    },
    [49] = {
        .songId = 0x3BFF,
        .titleTextId = 0x2EBF
    },
    [50] = {
        .songId = 0x23F,
        .titleTextId = 0x0154
    },
    [51] = {
        .songId = 0x7FFF,
        .titleTextId = 0x6F7B
    },
    [52] = {
        .songId = 0x4E73,
        .titleTextId = 0x7C00
    },
    [53] = {
        .songId = 0x7E45,
        .titleTextId = 0x001A
    },
    [54] = {
        .songId = 0x4A5F,
        .titleTextId = 0x355F
    },
    [55] = {
        .songId = 0x39CE,
        .titleTextId = 0x001F
    },
    [56] = {
        .songId = 0x04,
        .titleTextId = 0x0200
    },
    [57] = {
        .songId = 0x3BFF,
        .titleTextId = 0x2EBF
    },
    [58] = {
        .songId = 0x23F,
        .titleTextId = 0x0154
    },
    [59] = {
        .songId = 0x7FFF,
        .titleTextId = 0x6318
    },
    [60] = {
        .songId = 0x4E73,
        .titleTextId = 0x7C00
    },
    [61] = {
        .songId = 0x7E45,
        .titleTextId = 0x001A
    },
    [62] = {
        .songId = 0x4A5F,
        .titleTextId = 0x355F
    },
    [63] = {
        .songId = 0x39CE,
        .titleTextId = 0x7E02
    },
    [64] = {
        .songId = 0x04,
        .titleTextId = 0x01E0
    },
    [65] = {
        .songId = 0x3BFF,
        .titleTextId = 0x2EBF
    },
    [66] = {
        .songId = 0x23F,
        .titleTextId = 0x0154
    },
    [67] = {
        .songId = 0x7FFF,
        .titleTextId = 0x6318
    },
    [68] = {
        .songId = 0x4E73,
        .titleTextId = 0x7C00
    },
    [69] = {
        .songId = 0x7E45,
        .titleTextId = 0x001A
    },
    [70] = {
        .songId = 0x4A5F,
        .titleTextId = 0x355F
    },
    [71] = {
        .songId = 0x39CE,
        .titleTextId = 0x03FF
    },
    [72] = {
        .songId = 0x04,
        .titleTextId = 0x01E0
    },
    [73] = {
        .songId = 0x3BFF,
        .titleTextId = 0x2EBF
    },
    [74] = {
        .songId = 0x23F,
        .titleTextId = 0x0154
    },
    [75] = {
        .songId = 0x7FFF,
        .titleTextId = 0x6318
    },
    [76] = {
        .songId = 0x4E73,
        .titleTextId = 0x7C00
    },
    [77] = {
        .songId = 0x7E45,
        .titleTextId = 0x001A
    },
    [78] = {
        .songId = 0x4A5F,
        .titleTextId = 0x355F
    },
    [79] = {
        .songId = 0x39CE,
        .titleTextId = 0x02A0
    },
    [80] = {
        .songId = 0x04,
        .titleTextId = 0x01E0
    },
    [81] = {
        .songId = 0x3BFF,
        .titleTextId = 0x2EBF
    },
    [82] = {
        .songId = 0x23F,
        .titleTextId = 0x0154
    },
    [83] = {
        .songId = 0x7FFF,
        .titleTextId = 0x6318
    },
    [84] = {
        .songId = 0x4E73,
        .titleTextId = 0x7C00
    },
    [85] = {
        .songId = 0x7E45,
        .titleTextId = 0x001A
    },
    [86] = {
        .songId = 0x4A5F,
        .titleTextId = 0x355F
    },
    [87] = {
        .songId = 0x39CE,
        .titleTextId = 0x294A
    },
    [88] = {
        .songId = 0x04,
        .titleTextId = 0x01E0
    },
    [89] = {
        .songId = 0x3BFF,
        .titleTextId = 0x2EBF
    },
    [90] = {
        .songId = 0x23F,
        .titleTextId = 0x0154
    },
    [91] = {
        .songId = 0x7FFF,
        .titleTextId = 0x6318
    },
    [92] = {
        .songId = 0x4E73,
        .titleTextId = 0x7C00
    },
    [93] = {
        .songId = 0x7E45,
        .titleTextId = 0x001A
    },
    [94] = {
        .songId = 0x4A5F,
        .titleTextId = 0x355F
    },
};

// similar to GameModeSoulTradeMenu_inline_1, but uses unk_4FC instead of unk_508
static inline void GameModeSoundTestMenu_inline_0(struct EwramData_EntityData *textEntity, s32 entry)
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
    if (SOUND_TEST_TEXT_ENTRY(textEntity) != entry)
    {
        SOUND_TEST_TEXT_ENTRY(textEntity) = entry;
        Text12_SetRegion(0, 1, 0x1E, 4);
        Text12_ClearRegion();
        Text12_SetRegion(0, 1, 0x1C, 4);
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

static inline void GameModeSoundTestMenu_inline_2(s32 value, s32 max, s16 *dst, s32 wrapTo)
{
    if (value > max)
    {
        *dst = wrapTo;
    }
    else if (value < 0)
    {
        *dst = max;
    }
}

static inline void GameModeSoundTestMenu_inline_1(struct SoundTestCategoryRange range, s32 rightKey, s16 *value)
{
    s32 zero;
    s32 max;

    do {
        zero = 0;
    } while(0);
    max = range.maxSelectionIndex;

    if (gEwramData->inputData.repeatedInput & rightKey) // KEY_RIGHT
    {
        *value += 1;
    }
    else if (gEwramData->inputData.repeatedInput & (rightKey << 1)) // KEY_LEFT
    {
        *value -= 1;
    }

    if (gEwramData->inputData.repeatedInput & (rightKey << 4)) // KEY_R
    {
        *value += 10;
    }
    if (gEwramData->inputData.repeatedInput & (rightKey << 5)) // KEY_L
    {
        *value -= 10;
    }

    GameModeSoundTestMenu_inline_2(*value, max, value, zero);
}

static inline void GameModeSoundTestMenu_inline_3(s32 song, s32 channel)
{
    SOUND_TEST_PLAYING(gEwramData->entityData[2], channel) = song;
}

/**
 * @brief B8D0 | Handle sound test menu game mode
 * 
 * @return s32 Game mode, -1 is no change, -2 is next game mode, else the specified game mode
 */
s32 GameModeSoundTestMenu(void)
{
    struct EwramData_EntityData *cursor;
    struct EwramData_EntityData *textEntity;
    s32 gameMode;
    s32 i;
    u16 alpha;
    u16 song;
    s16 *selection;
    s32 downKey;
    s32 zero;
    s32 max;

    cursor = &gEwramData->entityData[3];
    textEntity = &gEwramData->entityData[4];
    gameMode = GAME_MODE_SAME_MODE;

    switch (gEwramData->gameModeUpdateStage)
    {
        case 0:
            gUnk_03002CB0.dispCnt = DCNT_OBJ | DCNT_BG3 | DCNT_BG1 | DCNT_BG0;
            gDisplayRegisters.bldCnt = BLDCNT_SCREEN_SECOND_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT | BLDCNT_SCREEN_FIRST_TARGET;
            gDisplayRegisters.bldY = BLDY_MAX_VALUE;

            gEwramData->gameModeUpdateStage = 1;
            gEwramData->gameModeSubStage = 0;
            PlaySong(AUDIO_STOP);
            EntityDelete(&gEwramData->entityData[3]);
            EntityDelete(&gEwramData->entityData[2]);
            EntityDelete(&gEwramData->entityData[4]);
            break;
    
        case 1:
            DMA_FILL_32(3, 0, VRAM_BASE + 0xE800, 0x800);
            DMA_FILL_32(3, 0, VRAM_BASE + 0xF000, 0x800);
            DMA_FILL_32(3, 0, VRAM_BASE + 0xF800, 0x800);

            gUnk_03002CB0.dispCnt &= ~DCNT_BG2;
            gDisplayRegisters.bldCnt = BLDCNT_BG3_SECOND_TARGET_PIXEL | BLDCNT_ALPHA_BLENDING_EFFECT | BLDCNT_OBJ_FIRST_TARGET_PIXEL | BLDCNT_BG1_FIRST_TARGET_PIXEL | BLDCNT_BG0_FIRST_TARGET_PIXEL;
            gDisplayRegisters.bldAlpha = C_16_2_8(10, 6);
            gDisplayRegisters.bldY = 0;
            gDisplayRegisters.bgCnt[1] = CREATE_BGCNT(0, 29, BGCNT_HIGH_MID_PRIORITY, BGCNT_SIZE_256x256);
            gDisplayRegisters.bgCnt[2] = CREATE_BGCNT(0, 30, BGCNT_LOW_MID_PRIORITY, BGCNT_SIZE_256x256);
            gDisplayRegisters.bgCnt[3] = CREATE_BGCNT(0, 31, BGCNT_LOW_PRIORITY, BGCNT_SIZE_256x256);

            Gfx_CopyResource((u8 *)0x080E5BB0, VRAM_BASE, 0);
            Gfx_CopyResource((u8 *)0x080E5BB8, VRAM_BASE + 0x2000, 0);
    
            switch (gEwramData->gameState.language)
            {
                case 2:
                    Gfx_CopyResource((u8 *)0x08277994, VRAM_BASE + 0x4000, 0);
                    break;
                case 3:
                    Gfx_CopyResource((u8 *)0x0826C850, VRAM_BASE + 0x4000, 0);
                    break;
                default:
                    Gfx_CopyResource((u8 *)0x080E5BC0, VRAM_BASE + 0x4000, 0);
                    break;
            }
            
            BgPalette_Load((u8 *)0x080E5E24);
            BgMap_Setup(3, (struct BgMapMetadata *)0x080E74C4, 0, 0);
            BgMap_Setup(2, (struct BgMapMetadata *)0x080E6CE4, 0, 0);
            BgMap_Setup(1, (struct BgMapMetadata *)0x080E6D74, 0, 0);
            SoundTestMenu_DrawScreen(0x3AC);
            gEwramData->gameModeTimer = BLDALPHA_MAX_VALUE;
            gEwramData->gameModeUpdateStage = 2;
            gEwramData->gameModeSubStage = 0;
            CursorSprite_Create(cursor, 0xE);
            cursor->posX.part16.integer = 0x52;
            cursor->posY.part16.integer = 0x34;
            break;
    
        case 2:
            if (gEwramData->gameModeTimer != 0)
            {
                gEwramData->gameModeTimer -= 2;
                alpha = gEwramData->gameModeTimer;
                gDisplayRegisters.bldAlpha = (alpha << 8) | (BLDALPHA_MAX_VALUE - alpha);
            }
            else
            {
                gUnk_03002CB0.dispCnt |= DCNT_BG3 | DCNT_BG2 | DCNT_BG1 | DCNT_BG0;
                gDisplayRegisters.bldCnt = BLDCNT_BG3_SECOND_TARGET_PIXEL | BLDCNT_ALPHA_BLENDING_EFFECT | BLDCNT_BG2_FIRST_TARGET_PIXEL;
                gDisplayRegisters.bldAlpha = C_16_2_8(10, 6);
                gEwramData->gameModeUpdateStage = 3;
                gEwramData->gameModeSubStage = 0;
            }
            break;

        case 3:
            if (gEwramData->inputData.newInput & 8)
            {
                gEwramData->gameModeUpdateStage = 4;
                gEwramData->gameModeSubStage = 0;
                break;
            }

            downKey = 0x80;
            selection = &SOUND_TEST_ROW(gEwramData->entityData[2]);
            do{zero = 0;}while(0);
            max = 2;
            if (gEwramData->inputData.repeatedInput & downKey) // KEY_DOWN
            {
                *selection += 1;
            }
            else if (gEwramData->inputData.repeatedInput & 0x40) // KEY_UP
            {
                *selection -= 1;
            }

            GameModeSoundTestMenu_inline_2(*selection, max, selection, zero);

            if (SOUND_TEST_ROW(gEwramData->entityData[2]) < 2)
            {
                GameModeSoundTestMenu_inline_1(sSoundTestCategoryRanges[SOUND_TEST_ROW(gEwramData->entityData[2])], 0x10, &SOUND_TEST_TRACK(gEwramData->entityData[2], SOUND_TEST_ROW(gEwramData->entityData[2])));
            }
            cursor->posY.part16.integer = (SOUND_TEST_ROW(gEwramData->entityData[2]) * 0x10) + 0x34;

            if (gEwramData->inputData.newInput & 1)
            {
                switch (SOUND_TEST_ROW(gEwramData->entityData[2]))
                {
                    case 0:
                    case 1:
                        if (SOUND_TEST_ROW(gEwramData->entityData[2]) == 0)
                        {
                            song = sSoundTestTracks[SOUND_TEST_TRACK(gEwramData->entityData[2], SOUND_TEST_ROW(gEwramData->entityData[2]))].songId;
                        }
                        else
                        {
                            song = sSoundTestCategoryRanges[SOUND_TEST_ROW(gEwramData->entityData[2])].baseSongId + SOUND_TEST_TRACK(gEwramData->entityData[2], SOUND_TEST_ROW(gEwramData->entityData[2]));
                        }

                        if (SOUND_TEST_PLAYING(gEwramData->entityData[2], SOUND_TEST_ROW(gEwramData->entityData[2])) != 0)
                        {
                            PlaySong(SOUND_TEST_PLAYING(gEwramData->entityData[2], SOUND_TEST_ROW(gEwramData->entityData[2])) | STOP_SONG);
                        }

                        GameModeSoundTestMenu_inline_3(song, SOUND_TEST_ROW(gEwramData->entityData[2]));
                        PlaySong(song);
                        break;
                    
                    case 2:
                        PlaySong(AUDIO_STOP);
                        gEwramData->gameModeUpdateStage = 4;
                        gEwramData->gameModeSubStage = 0;
                        break;
                }
            }

            if (SOUND_TEST_ROW(gEwramData->entityData[2]) == 0)
            {
                GameModeSoundTestMenu_inline_0(textEntity, sSoundTestTracks[SOUND_TEST_TRACK(gEwramData->entityData[2], SOUND_TEST_ROW(gEwramData->entityData[2]))].titleTextId);
            }

            if (gEwramData->inputData.newInput & 2)
            {
                for (i = 0; i < 3; i++)
                {
                    GameModeSoundTestMenu_inline_3(0, i);
                }
                PlaySong(AUDIO_STOP);
            }

            for (i = 0; i < 2; i++)
            {
                HudMap_PrintNumberAttr(0x12, i * 2 + 6, SOUND_TEST_TRACK(gEwramData->entityData[2], i), 2, 0xF200, 1);
            }
            break;

        case 4:
            gEwramData->gameModeUpdateStage = 5;
            gEwramData->gameModeSubStage = 0;
            gUnk_03002CB0.dispCnt &= ~DCNT_BG2;
            gDisplayRegisters.bldCnt = BLDCNT_SCREEN_SECOND_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT | BLDCNT_SCREEN_FIRST_TARGET;
            gDisplayRegisters.bldY = BLDY_MAX_VALUE;
            gEwramData->gameModeTimer = 0;
            gEwramData->gameModeUpdateStage = 5;
            gEwramData->gameModeSubStage = 0;
            break;

        case 5:
            if (gEwramData->gameModeTimer != BLDALPHA_MAX_VALUE)
            {
                gEwramData->gameModeTimer += 2;
                alpha = gEwramData->gameModeTimer;
                gDisplayRegisters.bldAlpha = (alpha << 8) | (BLDALPHA_MAX_VALUE - alpha);
            }
            else
            {
                gUnk_03002CB0.dispCnt |= DCNT_BG3 | DCNT_BG2 | DCNT_BG1 | DCNT_BG0;
                gameMode = GAME_MODE_TITLE_SCREEN;
            }
            break;
    }

    Entity_UpdateAll();
    if (gameMode != GAME_MODE_SAME_MODE)
    {
        EntityDeleteAll();
        ObjTiles_Init();
        Obj_Init();
        Sprite_Init();
        Palette_Init();
        Text_Initialize();
        HitBox_ClearAll();
    }

    return gameMode;
}

static inline void GameModeInGameUpdate_inline_0(s32 enabled)
{
    gEwramData->startPauseEnabled = enabled;
}

/**
 * @brief BEC0 | Handle in game game mode
 * 
 * @return s32 Game mode, -1 is no change, -2 is next game mode, else the specified game mode
 */
s32 GameModeInGameUpdate(void)
{
    s32 gameMode;
    struct GameState *gameState;
    u32 connected;
    s32 to;
    s32 zero;

    gameState = &gEwramData->gameState;
    gameMode = GAME_MODE_SAME_MODE;

    switch (gameState->subMode)
    {
        case 0:
            GameModeInGameUpdate_inline_0(0);
            gEwramData->paused = 0;

            switch (gameState->subModeStep)
            {
                case 0:
                    to = 0;
                    gEwramData->startPauseEnabled = 0;
                    BgPalette_StartColorFade(0, 0, 0, 0x10000, to, 8);
                    ObjPalette_StartColorFade(0, 0, 0, 0x10000, to, 8);
                    gameState->subModeStep += 1;
                    /* fallthrough */
                case 1:
                    if (ObjPalette_GetPendingFades() == 0)
                    {
                        BgPalette_StopColorFade();
                        ObjPalette_StopColorFade();
                        SetPlayerInput(1, 0);
                        gameState->subModeStep += 1;
                    }
                    break;
    
                case 2:
                    InGame_InitScene();
                    zero = 0;
                    gEwramData->startPauseEnabled = 0;
                    gDisplayRegisters.bldY = zero;
                    /* fallthrough */
                case 3:
                    BgPalette_StartColorFade(0, 0, 0, 0, 0x10000, 8);
                    ObjPalette_StartColorFade(0, 0, 0, 0, 0x10000, 8);
                    SetPlayerInput(1, 0);
                    Entity_UpdateAll();
                    gameState->subModeStep += 1;
                    break;
    
                case 4:
                    if ((gEwramData->gameState.stateFlags & 1) && (gEwramData->playerStatus.currentCharacter == 0))
                    {
                        gEwramData->paused = 0;
                    }
                    else
                    {
                        gEwramData->paused = 1;
                    }

                    Entity_UpdateAll();
                    gEwramData->paused = 0;

                    if (ObjPalette_GetPendingFades() == 0)
                    {
                        BgPalette_StopColorFade();
                        ObjPalette_StopColorFade();

                        GameModeInGameUpdate_inline_0(1);
                        gEwramData->paused = 0;

                        gameState->subModeStep = 0;
                        gameState->bgEffectFrameCounter = 0;

                        if (!gEwramData->gameState.linkPlay)
                        {
                            if (gameState->subMode == 0)
                            {
                                gameState->subMode = 1;
                            }
                        }
                        else
                        {
                            gEwramData->playerState.linkFrameCounter = 0;
                            gameState->subMode = 0xB;
                            Link_Init();
                            Link_Start();
                            gameState->subModeCounter = 0x10;
                            GameModeInGameUpdate_inline_0(1);
                        }
                    }
                    break;
            }
            break;

        case 1:
            if (gEwramData->playerStatus.currentCharacter != 0)
            {
                if (gEwramData->paused && gEwramData->startPauseEnabled)
                {
                     gEwramData->paused = 0;
                }
            }
            Entity_UpdateAll();

            if (!gEwramData->paused)
            {
                InGame_UpdateTimerAndHud();
            }
            ScreenShake_Update();
            AreaName_Update(gameState);

            if ((Room_IsPlayerInside(gEwramData->playerState.player) == 0) && !gEwramData->paused)
            {
                RoomTransition_Begin(gameState);
            }
            else
            {
                Map_MarkCurrentCell(gameState);
                RoomEntity_SpawnOnScroll();

                if (gEwramData->startPauseEnabled)
                {
                    if ((gEwramData->inputData.newInput & KEY_SELECT) && (gameState->currentRoomDefinition != NULL))
                    {
                        AreaName_Dismiss(gameState);
                        MapScreen_Open(gameState);
                    }
                    else if (gEwramData->playerStatus.currentCharacter == 0 && (gEwramData->inputData.newInput & KEY_START))
                    {
                        AreaName_Dismiss(gameState);
                        PauseMenu_Enter(gameState);
                    }
                }
            }
            break;

        case 2:
            gameMode = InGame_UpdateDeath();
            break;

        case 3:
            if (gEwramData->gameState.linkPlay)
            {
                SetPlayerInput(1, 0);
                Link_ExchangePlayerState();
                Link_UpdatePlayerSync();
            }

            if (RoomTransition_Update(gameState) != 0)
            {
                if (!gEwramData->gameState.linkPlay)
                {
                    gameState->subMode = 1;
                    GameModeInGameUpdate_inline_0(1);
                }
                else
                {
                    gameState->subMode = 0xB;
                    gameState->subModeCounter = 0x10;
                    gEwramData->playerState.player->posX.whole = 0x10000;
                }
            }
            Entity_UpdateAll();
            break;

        case 5:
            Entity_UpdateAll();
            if (RoomFadeTransition_Update(gameState) != 0)
            {
                gameState->subMode = 1;
            }
            break;

        case 4:
            SetPlayerInput(1, 0);
            if (MapScreen_Update(gameState) != 0)
            {
                SetPlayerInput(1, 0);
                gameState->subMode = 1;
            }
            break;

        case 6:
            SetPlayerInput(1, 0);
            if (PauseMenu_Update(gameState) != 0)
            {
                gameState->subMode = 1;
            }
            break;

        case 7:
            if (WarpMenu_Update(gameState) == 0)
            {
                break;
            }

            if (gEwramData->gameState.selectedWarpIndex < 0)
            {
                gameState->subMode = 1;
                
            }
            else if (gEwramData->gameState.stateFlags & 0x20000)
            {
                gameState->subMode = 1;
                gEwramData->gameState.stateFlags &= ~0x20000;
            }
            else
            {
                RoomJump_Begin(gameState);
            }
            break;

        case 8:
            if (RoomJump_Update(gameState) != 0)
            {
                gameState->subMode = 1;
            }
            break;

        case 9:
            if (Shop_Update(gameState) != 0)
            {
                gameState->subMode = 1;
            }
            break;

        case 10:
            gameMode = InGame_EnterCredits();
            break;

        case 11:
            Link_ExchangePlayerState();
            Link_UpdatePlayerSync();
            if (gameState->subModeCounter != 0)
            {
                gameState->subModeCounter -= 1;
            }
            else
            {
                gameState->subMode = 0xC;
                gEwramData->playerState.linkFrameCounter = 0;
                gEwramData->playerState.linkDisconnectCounter = 0;
                gEwramData->randomNumber2 = 0;
            }
            break;

        case 12:
            Link_ExchangePlayerState();
            Link_UpdatePlayerSync();

            gEwramData->startPauseEnabled = 0;
            Entity_UpdateAll();

            connected = Link_IsConnected() && Link_IsPeerActive();
            if (connected == 0)
            {
                if (++gameState->subModeCounter > 4)
                {
                    Link_Stop();
                    gEwramData->gameState.subMode = 2;
                    gEwramData->gameState.subModeStep = 0;
                }
            }
            else
            {
                gameState->subModeCounter = 0;
                gEwramData->playerState.linkDisconnectCounter = 0;

                if (gEwramData->playerState.linkFrameCounter >= 0xB && (gEwramData->linkReceivedState.flags & 0x10))
                {
                    if (gEwramData->playerState.linkFrameCounter <= gEwramData->linkReceivedState.frameCounter)
                    {
                        Link_Stop();
                        gEwramData->gameState.subMode = 2;
                        gEwramData->gameState.subModeStep = 0;
                    }
                    else
                    {
                        gameState->subModeCounter = gEwramData->playerState.linkFrameCounter - gEwramData->linkReceivedState.frameCounter;
                        gEwramData->playerState.linkDisconnectCounter = 0;
                        gameState->subMode = 0xD;
                    }
                }
            }
            break;

        case 13:
            Link_ExchangePlayerState();
            Link_UpdatePlayerSync();
            Entity_UpdateAll();

            if (gameState->subModeCounter)
            {
                gameState->subModeCounter -= 1;
                break;
            }

            connected = 0;
            if (Link_IsConnected() != 0)
            {
                connected = connected != Link_IsPeerActive();
            }

            if (connected == 0)
            {
                Link_Stop();
                gEwramData->gameState.subMode = 2;
                gEwramData->gameState.subModeStep = connected;
            }
            else
            {
                gameState->subMode = 0xE;
            }
            break;

        case 14:
            gameState->subModeCounter += 1;
            Link_ExchangePlayerState();
            Link_ApplyPartnerHit();
            Entity_UpdateAll();

            connected = 0;
            if (Link_IsConnected() != 0)
            {
                connected = connected != Link_IsPeerActive();
            }

            if (connected == 0)
            {
                if (++gEwramData->playerState.linkDisconnectCounter > 0)
                {
                    Link_Stop();
                    gEwramData->gameState.subMode = 2;
                    gEwramData->gameState.subModeStep = 0;
                    break;
                }
            }
            else
            {
                gEwramData->playerState.linkDisconnectCounter = 0;
            }

            if (!gEwramData->paused)
            {
                InGame_UpdateTimerAndHud();
            }
            ScreenShake_Update();

            if (Room_IsPlayerInside(gEwramData->playerState.player) == 0)
            {
                RoomTransition_Begin(gameState);
                Player_SetSelectedTarget(NULL);
            }
            else
            {
                Map_MarkCurrentCell(gameState);
                RoomEntity_SpawnOnScroll();
            }
            break;
    }

    ItemIconCache_InvalidateUpload();
    return gameMode;
}

/**
 * @brief Store enabled in startPauseEnabled, which gates Start (pause menu / pause
 * toggle) and Select (map) during play, and return it.
 */
s32 Game_SetMenusEnabled(s32 enabled)
{
    // TODO: Game_SetMenusEnabled is the same as GameModeInGameUpdate_inline_0
    gEwramData->startPauseEnabled = enabled;
    return enabled;
}

/**
 * @brief C5D4 | Handle unk_3 (empty) game mode
 * 
 * @return s32 Game mode, -1 is no change, -2 is next game mode, else the specified game mode
 */
s32 GameModeUnk3Update(void)
{
    return;
}

s16 sCommonObjectPaletteIndices[] = {
    -1, -1, -1, -1, 0, 1, 2, 3, 4, 6, -1, -1, -1, -1, 5, -1
};

/**
 * @brief Apply the default blend, clear per-room effects, load the common
 * object palettes and language-specific object graphics, init the item
 * icon cache, create the player and position it for a new or loaded
 * game, then load the starting room.
 */
void InGame_InitScene(void)
{
    s32 i;

    gDisplayRegisters.bldAlpha = C_16_2_8(11, 10);
    if (gDisplayRegisters.bldCnt == 0)
    {
        gDisplayRegisters.bldCnt = (BLDCNT_SCREEN_SECOND_TARGET & ~BLDCNT_BACKDROP_SECOND_TARGET_PIXEL) | BLDCNT_ALPHA_BLENDING_EFFECT;
        gDisplayRegisters.bldAlpha = C_16_2_8(11, 10);
    }
    Room_ResetTransientState();

    for (i = 0; i < ARRAY_SIZE(sCommonObjectPaletteIndices); i++)
    {
        if (sCommonObjectPaletteIndices[i] >= 0)
        {
            ObjPalette_LoadRange((u8 *)0x082099FC, sCommonObjectPaletteIndices[i], 1, i);
        }
    }

    switch (gEwramData->gameState.language)
    {
        default:
            ObjGfx_LoadResource((u8 *)0x081C15F4, 0x10000);
            break;
        case 2:
            ObjGfx_LoadResource((u8 *)0x0826F8A4, 0x10000);
            break;
        case 3:
            ObjGfx_LoadResource((u8 *)0x082718A8, 0x10000);
            break;
    }
    ItemIconCache_Init();
    Player_Create(0);

    if (gEwramData->gameState.linkPlay)
    {
        LinkPartner_Create();
        gEwramData->playerState.player->posX.part16.integer = 0x78;
        gEwramData->playerState.player->posY.part16.integer = 0x50;
    }
    else
    {
        if (gEwramData->playerStatus.currentLevel == 0)
        {
            Player_InitNewGameIfNeeded();
        }
        if (gEwramData->gameState.roomData == NULL)
        {
            gEwramData->playerState.player->posX.part16.integer = 0x78;
            gEwramData->playerState.player->posY.part16.integer = 0x50;
        }
    }

    Player_RefreshEquipmentStats();
    Room_LoadInitial();
    gEwramData->startPauseEnabled = 1;
}

/**
 * @brief Load the common object palettes listed in sCommonObjectPaletteIndices.
 */
void InGame_LoadCommonObjectPalettes(void)
{
    // inline for InGame_InitScene?
    s32 i;

    for (i = 0; i < ARRAY_SIZE(sCommonObjectPaletteIndices); i++)
    {
        if (sCommonObjectPaletteIndices[i] >= 0)
        {
            ObjPalette_LoadRange((u8 *)0x082099FC, sCommonObjectPaletteIndices[i], 1, i);
        }
    }
}

/**
 * @brief Load the language-specific common object graphics and init the
 * item icon cache.
 */
void InGame_LoadCommonObjectGraphics(void)
{
    // inline for InGame_InitScene?
    switch (gEwramData->gameState.language)
    {
        default:
            ObjGfx_LoadResource((u8 *)0x081C15F4, 0x10000);
            break;
        case 2:
            ObjGfx_LoadResource((u8 *)0x0826F8A4, 0x10000);
            break;
        case 3:
            ObjGfx_LoadResource((u8 *)0x082718A8, 0x10000);
            break;
    }
    ItemIconCache_Init();
}

/**
 * @brief Set blend alpha 11/10 and, when no blend mode is active, enable
 * alpha blending onto all second targets except the backdrop.
 */
void Display_SetDefaultBlend(void)
{
    // inline for InGame_InitScene?
    gDisplayRegisters.bldAlpha = C_16_2_8(11, 10);
    if (gDisplayRegisters.bldCnt == 0)
    {
        gDisplayRegisters.bldCnt = (BLDCNT_SCREEN_SECOND_TARGET & ~BLDCNT_BACKDROP_SECOND_TARGET_PIXEL) | BLDCNT_ALPHA_BLENDING_EFFECT;
        gDisplayRegisters.bldAlpha = C_16_2_8(11, 10);
    }
}

/**
 * @brief Reset display registers and BG settings, delete all entities, reset
 * object, sprite, palette, text and hit-box systems, clear the progress,
 * player and stats blocks, restore default buttons, reload system flags
 * and language, stop audio and the H-blank effect.
 */
void Game_ResetSystems(struct GameState *gameState)
{
    // Is gameState unused or implicit call?

    DMA_FILL_32(3, 0, &gDisplayRegisters, sizeof(gDisplayRegisters))

    gDisplayRegisters.bg3PD = 0x100;
    gDisplayRegisters.bg3PA = 0x100;
    gDisplayRegisters.bg2PD = 0x100;
    gDisplayRegisters.bg2PA = 0x100;

    gDisplayRegisters.bgCnt[0] = CREATE_BGCNT(0, 28, BGCNT_LOW_MID_PRIORITY, BGCNT_SIZE_256x256);
    gDisplayRegisters.bgCnt[1] = CREATE_BGCNT(0, 29, BGCNT_LOW_MID_PRIORITY, BGCNT_SIZE_256x256);
    gDisplayRegisters.bgCnt[2] = CREATE_BGCNT(0, 30, BGCNT_LOW_MID_PRIORITY, BGCNT_SIZE_256x256);
    gDisplayRegisters.bgCnt[3] = CREATE_BGCNT(0, 31, BGCNT_LOW_MID_PRIORITY, BGCNT_SIZE_256x256);

    Bg_SyncPriorities();
    EntityDeleteAll();
    ObjTiles_Init();
    Obj_Init();
    Sprite_Init();
    Palette_Init();
    Text_Initialize();
    HitBox_ClearAll();

    gUnk_03002CB0.dispCnt = DCNT_OBJ | DCNT_BG3 | DCNT_BG2 | DCNT_BG1 | DCNT_BG0;

    DMA_FILL_32(3, 0, &gEwramData->gameState, sizeof(gEwramData->gameState));
    DMA_FILL_32(3, 0, &gEwramData->playerState.player, 0x14C);
    DMA_FILL_32(3, 0, &gEwramData->playerStatus, sizeof(gEwramData->playerStatus));

    ButtonConfig_ResetDefaults();
    SaveData_LoadSystemFlags();
    SaveData_LoadLanguage();
    PlaySong(AUDIO_STOP);

    gEwramData->hBlankEffect.requestStop = 1;
    gEwramData->bgControlMinWriteLevel = gEwramData->unk_A074_4 = 0;
    gEwramData->paused = 0;
}

/**
 * @brief In-game sub-mode 2 (entered by Soma_StartDeath/Julius_StartDeath):
 * close the message window, cancel souls, fade out; then either jump
 * to room 0x085236A4 (stateFlags bit 0x200000) or reset systems and return
 * GAME_MODE_GAME_OVER.
 */
s32 InGame_UpdateDeath(void)
{
    s32 gameMode;
    struct GameState *timerState;
    struct GameState *gameState;

    gameState = &gEwramData->gameState;
    gameMode = GAME_MODE_SAME_MODE;

    gEwramData->paused ^= 1;
    Entity_UpdateAll();

    timerState = &gEwramData->gameState;
    UpdateInGameTimer();
    timerState->bgEffectFrameCounter += 1;

    if (!(gEwramData->gameState.stateFlags & 0x20))
    {
        Hud_UpdateMessageWindow(timerState);
        Hud_UpdateHpMpGauges(timerState);
        gEwramData->gameState.stateFlags &= ~0x2000000;
    }

    switch (gameState->subModeStep)
    {
        case 0:
            Hud_CloseMessageWindow();
            RedSoul_CancelAll();
            BlueSoul_StopAll();
            gEwramData->playerState.enemyTarget = 0;
            PlaySong(SONG_MUTE);
            gameState->inGameTimer = 0;
            gameState->subModeStep += 1;
            BgPalette_StartColorFade(0, 0, 0, 0x10000, 0, 0x40);
            ObjPalette_StartColorFade(0, 0, 0, 0x10000, 0, 0x80);
            /* fallthrough */
        case 1:
            if (ObjPalette_GetPendingFades() == 0)
            {
                BgPalette_StopColorFade();
                ObjPalette_StopColorFade();
                if (gEwramData->gameState.stateFlags & 0x200000)
                {
                    gEwramData->startPauseEnabled = 0;
                    gEwramData->gameState.stateFlags |= 0x800000;
                    RoomJump_SetDestination((void *)0x085236A4, 0, 0, 0, 0);
                    RoomJump_Begin(&gEwramData->gameState);
                    gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT;
                    gameMode = GAME_MODE_IN_GAME;
                }
                else
                {
                    Game_ResetSystems(gameState);
                    gameMode = GAME_MODE_GAME_OVER;
                }
            }
            break;
    }
    return gameMode;
}

/**
 * @brief Advance the play timer and frame counter and, unless the HUD is
 * suppressed (stateFlags bit 0x20), update the message window and gauges.
 * Return 1.
 */
s32 InGame_UpdateTimerAndHud(void)
{
    // inline for InGame_UpdateDeath?
    struct GameState *gameState;

    gameState = &gEwramData->gameState;
    UpdateInGameTimer();
    gameState->bgEffectFrameCounter += 1;

    if (!(gEwramData->gameState.stateFlags & 0x20))
    {
        Hud_UpdateMessageWindow(gameState);
        Hud_UpdateHpMpGauges(gameState);
        gEwramData->gameState.stateFlags &= ~0x2000000;
    }

    return 1;
}

/**
 * @brief Reset systems while keeping stateFlags, stop colour fades, black out
 * and return GAME_MODE_CREDITS (in-game sub-mode 10).
 */
s32 InGame_EnterCredits(void)
{
    struct GameState *gameState;
    u32 savedFlags;

    gameState = &gEwramData->gameState;

    savedFlags = gameState->stateFlags;
    Game_ResetSystems(gameState);
    gameState->stateFlags = savedFlags;

    BgPalette_StopColorFade();
    ObjPalette_StopColorFade();

    gDisplayRegisters.bldY = BLDY_MAX_VALUE;
    gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT;

    return GAME_MODE_CREDITS;
}

/**
 * @brief CADC | To document
 * 
 */
void UpdateInGameTimer(void)
{
    if (gEwramData->gameState.inGameTimer < 0x014996C4)
    {
        gEwramData->gameState.inGameTimer += 1;
    }
}
