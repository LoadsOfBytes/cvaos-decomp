#include "gba/dma.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
extern s32 Text_PrintWithCommands(u8 *text, s32 initialOffset, s32 mode);
extern s32 Conversation_LoadNextText(void);
extern s32 TextWindow_AdjustOpening(s32 delta);
extern u32 GameFlag_Is3ASet(void);
extern void TextWindow_Close(void);
extern void HudMap_WriteTiles(s32 x, s32 y, u16 tile, s32 count);
extern void TextWindow_LoadBackground(void *tiles, u8 *palette, void *layer);
extern void PlaySong(u16 song);

struct WindowGfx {
    void *tiles;
    u8 *palette;
    void *layer;
};
extern struct WindowGfx sTextWindowGfx[];
extern u8 sTextWindowScrollOffsets[];

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
                            DMA_FILL_32(3, 0, 0x06006000, 0x1554);
                            DMA_FILL_16(3, 0, 0x0600E000, 0x800);
                            DMA_FILL_32(3, 0, 0x06004000, 0x1000);
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
                        DMA_FILL_32(3, 0, 0x06006000, 0x1554);
                        DMA_FILL_32(3, 0, 0x06006000, 0x1000);
                        DMA_FILL_16(3, 0, 0x0600E000, 0x800);
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
