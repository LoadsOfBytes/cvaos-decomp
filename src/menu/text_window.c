/*
 * Text window: background, opening / closing, update and text printing with commands.
 */
#include "text_window.h"
#include "gba.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "bg_effects.h"
#include "syscalls.h"
#include "gameplay.h"
#include "gba/dma.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "palette.h"
#include "scenes_c.h"
#include "text12.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "data/data_0E5364.h"
#include "sound.h"
#include "shop_update.h"

extern void *memcpy(void *dst, const void *src, unsigned long size);
extern void HudMap_WriteTiles(s32 x, s32 y, u16 tile, s32 count);

s32 Text_PrintWithCommands(u8 *text, s32 initialOffset, s32 mode);

/* Set up a language-dependent background: copy its map, load the given
 * tiles/palette and layer, and select palette 13 for every map entry. */

static inline u8 *GfxData(void *data)
{
    register u8 *src asm("r2") = NULL; /* FAKEMATCH: src in r2 */

    switch (*(u8 *)data)
    {
        case 1:
            ObjGfx_Decompress(data);
            { register u8 *g_ asm("r0") = (u8 *)gEwramData; /* FAKEMATCH: base in r0 */ src = g_ + 0x153F8; }
            break;
        case 0:
            src = (u8 *)data + 4;
            break;
    }
    return src;
}

static inline u8 *GfxDataK(void *data)
{
    u8 *src = NULL;

    switch (*(u8 *)data)
    {
        case 1:
            ObjGfx_Decompress(data);
            { register u8 *g_ asm("r0") = (u8 *)gEwramData; /* FAKEMATCH: base in r0 */ u32 k_ = 0x153F8; asm("" : "+r"(k_)); /* FAKEMATCH: opaque offset */ src = g_ + k_; }
            break;
        case 0:
            src = (u8 *)data + 4;
            break;
    }
    return src;
}

/**
 * @brief Load language-dependent window graphics, supplied tiles/palette and map layer, restore BG2 setup and OR 0xD100 into the window map entries.
 */
void TextWindow_LoadBackground(void *tiles, u8 *palette, struct BgMapMetadata *layer)
{
    struct BgLayerState bg[4];
    u8 *src;
    u16 *map;
    s32 i;
    s32 n = 0x400;

    memcpy(bg, gEwramData->bgInfo, sizeof(bg));
    gDisplayRegisters.bgCnt[0] = 0x1C04;
    switch (gEwramData->gameState.language)
    {
        case 2:
            src = GfxData(sLocalizedWindowGfxLanguage2);
            break;
        case 3:
            src = GfxDataK(sLocalizedWindowGfxLanguage3);
            break;
        default:
            src = GfxDataK(sLocalizedWindowGfx);
            break;
    }
    DMA_COPY_16(3, src + n, (void *)(VRAM_BASE + 0x5800), 0x400);
    BgPalette_LoadRange((u8 *)sSaveSelectBgPalette, 4, 1, 0xF);
    Gfx_CopyResource(tiles, (void *)(VRAM_BASE + 0x6000), 1);
    BgPalette_LoadRange(palette, 0, 1, 0xD);
    BgMap_SetupPrimary(0, layer, 0, 0);
    BgMap_Setup(2, bg[2].pBgMetadata, bg[2].xPos.part16.integer, bg[2].yPos.part16.integer);
    gDisplayRegisters.bgOfs[0].hOfs = 4;
    map = (u16 *)(VRAM_BASE + 0xE000);
    for (i = 0; i < 0x400; i++)
        *map++ |= 0xD100;
}

/* Advance and clamp the event window opening around its vertical center. */
/**
 * @brief Add delta to stored opening amount, clamp a local copy to 0..36 and derive window bounds around Y 124; return one at either local limit.
 */
s32 TextWindow_AdjustOpening(s32 delta)
{
    s32 finished = 0;
    s32 amount;
    gDisplayRegisters.bldY = 8;
    gDisplayRegisters.bldCnt = (gDisplayRegisters.bldCnt & 0xFF3F) | 0xC0;
    gDisplayRegisters.bldCnt |= 0x3F;
    gDisplayRegisters.bldCnt &= 0xFFFE;
    gUnk_03002CB0.dispCnt |= 0x2100;
    gEwramData->gameState.textWindowOpening += delta;
    amount = gEwramData->gameState.textWindowOpening;
    if (amount > 35) { amount = 36; finished = 1; }
    else if (amount <= 0) { amount = 0; finished = 1; }
    {s32 result = Div(amount * 32, 36); gDisplayRegisters.win0V = ((124-result)<<8) | (result+124);}
    return finished;
}

/* Close the event display and release its background/input state. */
/**
 * @brief Disable the event window display state, restore or clear its background under the existing flags and set execution field textWindowPause to 255.
 */
void TextWindow_Close(void)
{
    gUnk_03002CB0.dispCnt&=0x9FFF;
    gUnk_03002CB0.dispCnt&=0x1FFF;
    gDisplayRegisters.bldY=0;
    if(gEwramData->gameState.stateFlags&0x10000)
    {DMA_FILL_32(3,0,(void *)(VRAM_BASE + 0xE000),0x800);}
    else
    {Hud_Init();gEwramData->gameState.stateFlags&=~32;}
    gEwramData->gameState.textWindowPause=255;
}

/* Prepare a text event window and reset its execution state. */
/**
 * @brief Prepare the text window graphics state, select its localized command stream and reset the window execution state.
 */
void TextWindow_Begin(s32 textId)
{
    struct EwramData *ram = gEwramData;
    gUnk_03002CB0.dispCnt &= 0xFEFF;
    ram->gameState.stateFlags |= 0x21;
    Text_SetTileBuffer((void *)(VRAM_BASE + 0x4000));
    Text_SetBackgroundColor(0);
    Text12_SetRegion(0, 0, 31, 4);
    Text12_ClearRegion();
    gDisplayRegisters.win0H = 0x08E8;
    gDisplayRegisters.win0V = 0xC0C1;
    gDisplayRegisters.winIn_L = 0x3F;
    gDisplayRegisters.winOut_L = 0x1F;
    ram->gameState.conversationText = Text_GetLocalizedEntry(textId);
    ram->gameState.insertedTextAddress = 0;
    ram->gameState.textWindowState = 0;
    ram->gameState.insertedTextOffset = 0;
    ram->gameState.conversationTextOffset = 0;
    ram->gameState.textWindowPause = 0;
    ram->gameState.textCharacterDelay = 1;
}

/* Run the text commands until a character or a stop marker. */
static inline s32 ScanText(u8 *commands, s32 offset)
{
    s32 running = 1;
    void (*reset)(void) = Text12_ClearRegion;
    void (*advance)(void) = Text12_Newline;
    s32 (*scan)(u16) = Text12_PutChar;
    goto test;
    do
    {
        switch (scan(commands[offset]) & 0xFF)
        {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 7:
        case 10:
        case 0xFF:
            return offset;
        case 6:
            offset++;
            advance();
            break;
        case 8:
            {
                s32 parameter = commands[offset + 1];
                offset += 2;
                Text_SetForegroundColor(parameter);
            }
            break;
        case 9:
            offset++;
            reset();
            break;
        case 0:
        default:
            offset++;
            break;
        }
    test:;
    } while (running);
    return offset;
}

/* Inline wrapper: parameter copies give the original argument order. */
static inline void DrawRow(s32 x, s32 y, u16 tile, s32 count)
{
    HudMap_WriteTiles(x, y, tile, count);
}

static inline void DrawCursor(s32 blink)
{
    u16 *p = (u16 *)((gDisplayRegisters.bgOfs[0].hOfs >> 3) * 2
                     + ({ u32 t_ = (gDisplayRegisters.bgOfs[0].vOfs >> 3) * 64 + 0x0600E4B6; t_; }));
    if (blink == -1)
    {
        *p = 0x100;
        p += 0x20;
        *p = 0x100;
    }
    else
    {
        *p = blink ? 0xF0CE : 0xF0CF;
        p += 0x20;
        *p = blink ? 0xF0DE : 0xF0DF;
    }
}

/* Plain if/else (no do-while wrapper: loop notes would let loop.c move the block). */
#define SAVE_OFFSET(event, offset) \
    if ((event)->insertedTextAddress) (event)->insertedTextOffset = offset; \
    else (event)->conversationTextOffset = offset

/* Update the conversation text window; returns 1 when it has been closed. */
s32 TextWindow_Update(void)
{
    struct GameState *event = &gEwramData->gameState;
    s32 ret = 0;

    if (gEwramData->gameState.textWindowPause && gEwramData->gameState.textWindowState <= 4)
        return 0;
    if (event->textWindowState == 4)
    {
        if (event->textCharacterTimer == 0 || --event->textCharacterTimer == 0)
            event->textWindowState = 0;
    }
    switch (event->textWindowState)
    {
    case 0:
        {
            s32 cont = 1;
            struct EwramData_EntityData *obj = event->sceneController;
            gDisplayRegisters.bgCnt[0] = 0x1C04;
            do
            {
                u8 *text;
                u8 *commands;
                u8 *p;
                u8 c;
                s32 printable;
                s32 offset;
                if (event->insertedTextAddress)
                {
                    text = (u8 *)event->insertedTextAddress;
                    offset = event->insertedTextOffset;
                }
                else
                {
                    text = event->conversationText;
                    offset = event->conversationTextOffset;
                }
                commands = text + 2;
                offset = ScanText(text + 2, offset);
                SAVE_OFFSET(event, offset);
                c = commands[offset];
                printable = c & 0xE0;
                p = commands + offset;
                if (printable)
                {
                    u8 delay;
                    offset++;
                    if (!(gEwramData->inputData.heldInput & 2))
                        delay = event->textCharacterDelay;
                    else
                        delay = 0;
                    event->textCharacterTimer = delay;
                    if (event->textCharacterDelay)
                    {
                        event->textWindowState = 4;
                        cont = 0;
                    }
                }
                else
                {
                    switch (c)
                    {
                    case 3:
                        {
                            u8 i = p[1];
                            DMA_FILL_32(3, 0, VRAM_BASE + 0x6000, 0x1554);
                            DMA_FILL_16(3, 0, VRAM_BASE + 0xE000, 0x800);
                            DMA_FILL_32(3, 0, VRAM_BASE + 0x4000, 0x1000);
                            event->pad_4CE[0] = p[1];
                            gDisplayRegisters.bgOfs[0].hOfs = sTextWindowScrollOffsets[i * 2] << 3;
                            gDisplayRegisters.bgOfs[0].vOfs = ((sTextWindowScrollOffsets + 1)[i * 2] << 3) - 4;
                            offset += 2;
                            event->textWindowOpening = 0;
                            event->textWindowState = 2;
                            cont = 0;
                        }
                        break;
                    case 4:
                        offset++;
                        Text12_ClearRegion();
                        Text8_ClearRegion();
                        event->textWindowOpening = 0x24;
                        event->textWindowState = 3;
                        cont = 0;
                        DMA_FILL_32(3, 0, VRAM_BASE + 0x6000, 0x1554);
                        DMA_FILL_32(3, 0, VRAM_BASE + 0x6000, 0x1000);
                        DMA_FILL_16(3, 0, VRAM_BASE + 0xE000, 0x800);
                        break;
                    case 1:
                        event->insertedTextAddress = (u32)Text_GetLocalizedEntry(p[1]);
                    restart:
                        offset = 0;
                        break;
                    case 2:
                        {
                            u8 arg = p[1];
                            offset += 2;
                            switch (arg)
                            {
                            case 0:
                                event->textWindowPause = 1;
                                break;
                            case 1:
                                obj->subState++;
                                obj->stage = 0;
                                break;
                            }
                        }
                        break;
                    case 5:
                        offset++;
                        cont = 0;
                        event->textWindowState++;
                        break;
                    case 7:
                        {
                            u8 id = p[1];
                            offset += 2;
                            event->textWindowLabelId = id;
                            Text8_ClearRegion();
                            Text_PrintWithCommands(Text_GetLocalizedEntry(id), 0, 0);
                            event->textCharacterTimer = event->textCharacterDelay;
                            event->textWindowState = 4;
                            cont = 0;
                        }
                        break;
                    case 10:
                        if (event->insertedTextAddress)
                        {
                            event->insertedTextAddress = 0;
                            offset = event->conversationTextOffset + 2;
                        }
                        else
                        {
                            s32 r = Conversation_LoadNextText();
                            if (r != 0)
                                goto restart;
                            gDisplayRegisters.bldY = r;
                            event->textWindowState = 5;
                            cont = 0;
                        }
                        break;
                    default:
                        offset++;
                        break;
                    }
                }
                SAVE_OFFSET(event, offset);
            } while (cont);
        }
        break;
    case 1:
        DrawCursor(gEwramData->frameCounter & 8);
        break;
    case 2:
        if (TextWindow_AdjustOpening(3))
        {
            u8 i = event->pad_4CE[0];
            s32 x, y, y2;
            if (GameFlag_Is3ASet() && !(gEwramData->gameState.stateFlags & 0x10000) && i == 0)
                i = 7;
            TextWindow_LoadBackground(sTextWindowGfx[i].tiles, sTextWindowGfx[i].palette, sTextWindowGfx[i].layer);
            gDisplayRegisters.bgOfs[0].hOfs = sTextWindowScrollOffsets[i * 2] << 3;
            gDisplayRegisters.bgOfs[0].vOfs = ((sTextWindowScrollOffsets + 1)[i * 2] << 3) - 4;
            x = sTextWindowScrollOffsets[i * 2] + 5;
            y = (sTextWindowScrollOffsets + 1)[i * 2];
            y2 = y + 12;
            Text12_SetRegion(0, 0, 0x1F, 3);
            Text12_SetCursor(0, 0);
            DrawRow(x, y + 14, 0xE000, 0x16);
            DrawRow(x, y + 15, 0xE020, 0x16);
            DrawRow(x, y + 16, 0xE040, 0x16);
            DrawRow(x, y + 17, 0xE060, 0x16);
            DrawRow(x, y + 18, 0xE080, 0x16);
            Text8_SetRegion(0, 6, 0xF, 1);
            Text8_ClearRegion();
            DrawRow(x, y2, 0xE0C0, 0xB);
            event->textWindowState = 0;
        }
        break;
    case 3:
        if (TextWindow_AdjustOpening(-3))
            event->textWindowState = 0;
        break;
    case 4:
        break;
    case 5:
        gUnk_03002CB0.dispCnt &= 0xFEFF;
        event->textWindowState++;
        break;
    case 6:
        TextWindow_Close();
        gUnk_03002CB0.dispCnt |= 0x100;
        ret = 1;
        break;
    }
    if (gEwramData->inputData.newInput & 1)
    {
        if (event->textWindowState == 1)
        {
            PlaySong(0x132);
            DrawCursor(-1);
            event->textCharacterDelay = 1;
            event->textWindowState = 0;
        }
        else
            event->textCharacterDelay = 0;
    }
    return ret;
}

/* Scan text commands up to a stop marker, dispatching layout changes. */
/**
 * @brief Print text from offset up to a stop marker, handling clear and
 * newline commands, with the 12px (mode != 0) or 8px text routines.
 * Used by TextWindow_Update.
 */
s32 Text_PrintWithCommands(u8 *text, s32 initialOffset, s32 mode)
{
    s32 running = 1;
    u8 *commands = text + 2;
    s32 offset = initialOffset;
    void (*reset)(void);
    void (*advance)(void);
    s32 (*scan)(void *, s32);
    if (mode)
    {
        reset = Text12_ClearRegion;
        advance = Text12_Newline;
        scan = Text12_Print;
    }
    else
    {
        reset = Text8_ClearRegion;
        advance = Text8_Newline;
        scan = Text8_Print;
    }
    while (running)
    {
        offset = scan(text, offset);
        switch (commands[offset])
        {
        case 1:
        case 2:
        case 5:
        case 7:
        case 10:
            return offset;
        case 3:
            offset += 2;
            break;
        case 6:
            offset++;
            advance();
            break;
        case 8:
            {
                s32 parameter = commands[offset + 1];
                offset += 2;
                Text_SetForegroundColor(parameter);
            }
            break;
        case 9:
            offset++;
            reset();
            break;
        case -1:
        case 0:
        case 4:
        default:
            offset++;
            break;
        }
    }
    return offset;
}
