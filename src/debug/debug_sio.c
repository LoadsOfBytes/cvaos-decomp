#include "debug_sio.h"
#include "core.h"
#include "bg_and_collision.h"
#include "in_game.h"
#include "room_entities.h"
#include "map_screen.h"
#include "debug_menu.h"
#include "sound.h"
#include "gba.h"
#include "agb_multi_sio_sync.h"

#include "constants/main.h"

#include "data/interrupt_table.h"
#include "data/debug_font.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "data/data_4F1444.h"
#include "constants/songs.h"

extern int sprintf(char *, const char *, ...);

static struct DebugSioTestState sDebugSioTestState;

struct DebugSioTestCallbacks {
    s32 (*updateFunc)(struct DebugSioTestState *test);
    void (*initFunc)(struct DebugSioTestState *test);
    void (*exitFunc)(struct DebugSioTestState *test);
}; /* size = 0xC */
struct DebugSioTestCallbacks sDebugSioTestCallbacks[] = {
    [0] = {
        .updateFunc = DebugSioTest0_Update,
        .initFunc = DebugSioTest0_Init,
        .exitFunc = DebugSioTest0_Exit
    },
    [1] = {
        .updateFunc = DebugSioTest1_Update,
        .initFunc = DebugSioTest1_Init,
        .exitFunc = DebugSioTest1_Exit
    }
};

/**
 * @brief Run SIO test 0 or 1 through its init/update/exit callbacks; Select+B
 * or a finished update exits. Return 1 when done.
 */
s32 DebugSioTest_Run(s32 testIndex)
{
    s32 done;
    struct DebugSioTestCallbacks *def;

    done = 0;
    def = &sDebugSioTestCallbacks[testIndex];

    // TODO: why the cast?
    switch ((u8)DebugTest_GetStage())
    {
        case 0:
            DMA_FILL_32(3, 0, &sDebugSioTestState, 0x24);
            BgCmdBuffer_WriteStringWide(2, 0x13, 1, "PUSH SELECT + B TO EXIT.");
            def->initFunc(&sDebugSioTestState);
            DebugTest_SetStage(1);
            break;

        case 1:
            if ((gEwramData->inputData.heldInput & (4 | 2)) == (4 | 2))
            {
                DebugTest_SetStage(2);
            }
            else if (def->updateFunc(&sDebugSioTestState) == 0)
            {
                DebugTest_SetStage(2);
            }
            Entity_UpdateAll();
            break;

        case 2:
            def->exitFunc(&sDebugSioTestState);
            Debug_ClearBgMaps();
            done = 1;
            break;
    }

    return done;
}

struct Unk_08037F60 {
    u16 playerHeldInput;
    u16 playerNewInput;
};

/**
 * @brief SIO TEST 0: start the link, load a room and play it with the input
 * exchanged over the serial link.
 */
s32 DebugSioTest0_Update(struct DebugSioTestState *test)
{
    struct Unk_08037F60 sent;
    struct Unk_08037F60 received;
    s32 running;
    u32 connected;

    running = 1;

    switch (test->stageOrTimer.stage)
    {
        case 0:
            Link_Init();
            test->stageOrTimer.stage = 1;
            break;

        case 1:
            gEwramData->startPauseEnabled = 1;
            PlaySong(MUS_CLOCK_TOWER);
            InGame_InitScene();
            Room_Load((struct RoomDefinition *)sDebugEventRoom12, 0, 0);
            Link_Start();
            gEwramData->bgPalette.base[0][0] = 0xFFFF;
            gEwramData->bgPalette.uploaded = 0;
            test->stageOrTimer.stage = 2;
            break;

        case 2:
            connected = (Link_IsConnected() != 0) && (Link_IsPeerActive() != 0);
            if (connected != 0)
            {
                BgCmdBuffer_WriteStringWide(2, 2, 1, "              ");
                test->stageOrTimer.stage = 3;
            }
            else
            {
                BgCmdBuffer_WriteStringWide(2, 2, 1, "Now Connect...");
            }
            break;

        case 3:
            sent.playerHeldInput = gEwramData->inputData.playerHeldInput;
            sent.playerNewInput = gEwramData->inputData.playerNewInput;
            Link_QueueSendBytes((u8 *) &sent, 4);
            Link_CopyReceivedBytes((u8 *) &received, 4);
            gEwramData->inputData.playerHeldInput = received.playerHeldInput;
            gEwramData->inputData.playerNewInput = received.playerNewInput;

            if (Room_IsPlayerInside(gEwramData->playerState.player) == 0)
            {
                RoomTransition_Begin(&gEwramData->gameState);
                test->stageOrTimer.stage = 4;
            }
            break;

        case 4:
            sent.playerHeldInput = gEwramData->inputData.playerHeldInput;
            sent.playerNewInput = gEwramData->inputData.playerNewInput;
            Link_QueueSendBytes((u8 *) &sent, 4);
            Link_CopyReceivedBytes((u8 *) &received, 4);
            gEwramData->inputData.playerHeldInput = received.playerHeldInput;
            gEwramData->inputData.playerNewInput = received.playerNewInput;

            if (RoomTransition_Update(&gEwramData->gameState) != 0)
            {
                Map_MarkCurrentCell(&gEwramData->gameState);
                gEwramData->bgPalette.base[0][0] = 0xFF03;
                gEwramData->bgPalette.uploaded = 0;
                test->stageOrTimer.stage = 3;
            }
            break;

        case 5:
            Link_Stop();
            running = 0;
            break;
    }

    return running;
}

/**
 * @brief SIO TEST 1: title, connect check and main data-exchange screens.
 */
s32 DebugSioTest1_Update(struct DebugSioTestState *test)
{
    s32 titleResult;
    s32 connectResult;
    s32 running;
    u8 *string;

    running = 1;
    
    if (Link_IsConnected() != 0)
    {
        string = "***";
    }
    else
    {
        string = "   ";
    }
    BgCmdBuffer_WriteStringWide(0, 0, 1, string);

    switch (test->state)
    {
        case 0:
            test->state = 1;
            test->subState = 0;
            break;

        case 1:
            titleResult = DebugSioTest1_UpdateTitle(test);
            if (titleResult != 0)
            {
                Link_Init();
                DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
                test->stageOrTimer.timer = 0;
                test->subState = 0;
                test->state = (titleResult > 0) ? 2 : 6;
            }
            break;

        case 2:
            Link_Start();
            test->state = 3;
            test->subState = 0;
            break;

        case 3:
            connectResult = DebugSioTest1_UpdateConnectCheck(test);
            if (connectResult != 0)
            {
                DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
                test->subState = 0;
                test->state = (connectResult > 0) ? 4 : 5;
            }
            break;

        case 4:
            if (DebugSioTest1_UpdateMain(test) != 0)
            {
                test->state = 5;
                test->subState = 0;
            }
            break;

        case 5:
            Link_Stop();
            test->state = 0;
            break;

        case 6:
            running = 0;
            break;
    }

    return running;
}

/**
 * @brief Draw the title banner with START/EXIT; return 1, -1 or 0.
 */
s32 DebugSioTest1_UpdateTitle(struct DebugSioTestState *test)
{
    s32 result;
    s32 cursorRow;
    s32 clearRow;

    result = 0;

    switch (test->subState)
    {
        case 0:
            DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
            DMA_FILL_32(3, 0, VRAM_BASE + 0xE800, 0x800);
            DMA_FILL_32(3, 0, VRAM_BASE + 0xF000, 0x800);
            DMA_FILL_32(3, 0, VRAM_BASE + 0xF800, 0x800);

            BgCmdBuffer_WriteStringWide(4,  1,  0, "*  ****     *  ***  ");
            BgCmdBuffer_WriteStringWide(4,  2,  0, " *   *      *       ");
            BgCmdBuffer_WriteStringWide(4,  3,  0, "   *****   * *******");
            BgCmdBuffer_WriteStringWide(4,  4,  0, "** * * *  **        ");
            BgCmdBuffer_WriteStringWide(4,  5,  0, " * *****  **  ***** ");
            BgCmdBuffer_WriteStringWide(4,  6,  0, "*  * * * * *        ");
            BgCmdBuffer_WriteStringWide(4,  7,  0, " * *****   *  ***** ");
            BgCmdBuffer_WriteStringWide(4,  8,  0, " * * * *   *        ");
            BgCmdBuffer_WriteStringWide(4,  9,  0, " * * * *   *  ***** ");
            BgCmdBuffer_WriteStringWide(4,  10, 0, "*          *  *   * ");
            BgCmdBuffer_WriteStringWide(4,  11, 0, " * * * *   *  ***** ");
            BgCmdBuffer_WriteStringWide(12, 14, 1, "START");
            BgCmdBuffer_WriteStringWide(12, 15, 1, "EXIT");
            BgCmdBuffer_WriteStringWide(11, 14, 2, ">");

            test->titleChoice = 0;
            test->subState = 1;
            break;

        case 1:
            if (gEwramData->inputData.newInput & (0x80 | 0x40))
            {
                test->titleChoice ^= 1;

                if (test->titleChoice == 0)
                {
                    cursorRow = 0xE;
                }
                else
                {
                    cursorRow = 0xF;
                }
                BgCmdBuffer_WriteStringWide(0xB, cursorRow, 2, ">");
                
                if (test->titleChoice == 1)
                {
                    clearRow = 0xE;
                }
                else
                {
                    clearRow = 0xF;
                }
                BgCmdBuffer_WriteStringWide(0xB, clearRow, 2, " ");
            }

            if (gEwramData->inputData.newInput & 1)
            {
                if (test->titleChoice == 0)
                {
                    result = 1;
                }
                else
                {
                    result = -1;
                }
            }
            break;
    }

    return result;
}

/**
 * @brief Wait for a stable link connection (B cancels); return 1, -1 or 0.
 */
s32 DebugSioTest1_UpdateConnectCheck(struct DebugSioTestState *test)
{
    s32 result;
    s32 connected;
    s32 connected2;

    result = 0;

    switch (test->subState)
    {
        case 0:
            BgCmdBuffer_WriteStringWide(4, 4, 1, "Connect Check");
            BgCmdBuffer_WriteStringWide(4, 7, 1, "Push B Button To Cancel");
            test->subState = 1;
            break;

        case 1:
            connected = (Link_IsConnected() != 0) && (Link_IsPeerActive() != 0);
            if (connected != 0)
            {
                BgCmdBuffer_WriteStringWide(4, 4, 1, "CONNECT OK   ");
                test->stageOrTimer.timer = 0;
                test->subState = 2;
            }
            break;

        case 2:
            connected2 = (Link_IsConnected() != 0) && (Link_IsPeerActive() != 0);
            if (connected2 == 0)
            {
                BgCmdBuffer_WriteStringWide(4, 4, 1, "CONNECT ERROR");
                test->subState = 1;
                break;
            }

            if (test->stageOrTimer.timer++ > 0x3C)
            {
                result = 1;
            }
            break;

        case 3:
            Link_Stop();
            result = -1;
            break;
    }

    if (gEwramData->inputData.newInput & 2)
    {
        test->subState = 3;
    }

    return result;
}

// extern u16* BgCmdBuffer_WriteStringWide(u16 tileIndex, u16 rowOffset, u8 tileInfo, u8 *string); // implicit due to no u16

/**
 * @brief Edit six bytes, exchange them over the link and show both sides;
 * return 1 to leave.
 */
s32 DebugSioTest1_UpdateMain(struct DebugSioTestState *test)
{
    char subroutine_arg0[5];
    s32 i;
    s32 result;
    u32 connected;

    result = 0;

    switch (test->subState)
    {
        case 0:
            BgCmdBuffer_WriteStringWide(1, 1, 0, "SIO TEST MAIN");
            BgCmdBuffer_WriteStringWide(2, 19, 1, "PUSH SELECT BUTTON TO EXIT.");

            test->stageOrTimer.timer = 0;
            test->unk_5 = 0;
            test->editIndex = 0;
            test->subState = 1;
            break;

        case 1:
            if (gEwramData->inputData.heldInput & 0x20)
            {
                test->sendBytes[test->editIndex] -= 1;
            }
            else if (gEwramData->inputData.heldInput & 0x10)
            {
                test->sendBytes[test->editIndex] += 1;
            }

            if (gEwramData->inputData.newInput & 1)
            {
                test->editIndex += 1;
            }
            else if (gEwramData->inputData.newInput & 2)
            {
                test->editIndex -= 1;
            }

            if (test->editIndex < 0)
            {
                BgCmdBuffer_WriteStringWide(2, 4, 1, "A To Exit. B To Main");
                test->subState = 2;
                test->editIndex = 0;
            }
            else if (test->editIndex > 5)
            {
                BgCmdBuffer_WriteStringWide(2, 4, 1, "OK ? (A:OK, B:NG)");
                test->subState = 3;
                test->editIndex = 5;
            }

            test->sendMarker = 0;
            Link_QueueSendBytes(test->sendBytes, 0xC);
            Link_CopyReceivedBytes(test->receivedBytes, 0xC);

            for (i = 0; i <= test->editIndex; i++)
            {
                sprintf(subroutine_arg0, "0x%02x", test->sendBytes[i]);
                BgCmdBuffer_WriteStringWide(2, i + 8, 2, subroutine_arg0);
            }

            for (i = test->editIndex + 1; i < 6; i++)
            {
                BgCmdBuffer_WriteStringWide(2, i + 8, 2, "        ");
            }

            for (i = 0; i < 6; i++)
            {
                sprintf(subroutine_arg0, "0x%02x", test->receivedBytes[i]);
                BgCmdBuffer_WriteStringWide(0xA, i + 8, 3, subroutine_arg0);
            }
            break;

        case 2:
            if (gEwramData->inputData.newInput & 1)
            {
                BgCmdBuffer_WriteStringWide(2, 4, 1, "                              ");
                test->subState = 8;
            }
            else if (gEwramData->inputData.newInput & 2)
            {
                BgCmdBuffer_WriteStringWide(2, 4, 1, "                              ");
                test->subState = 1;
            }
            break;

        case 3:
            if (gEwramData->inputData.newInput & 1)
            {
                BgCmdBuffer_WriteStringWide(2, 4, 1, "Please wait ...  ");
                test->subState = 4;
            }
            else if (gEwramData->inputData.newInput & 2)
            {
                BgCmdBuffer_WriteStringWide(2, 4, 1, "                 ");
                test->subState = 1;
            }
            break;

        case 4:
            test->sendMarker = 0x01234567;
            Link_QueueSendBytes(test->sendBytes, 0xC);
            Link_CopyReceivedBytes(test->receivedBytes, 0xC);
            if (test->receivedMarker == 0x01234567)
            {
                test->sendMarker = 0;
                test->subState = 5;
            }
            break;

        case 5:
            BgCmdBuffer_WriteStringWide(2, 4, 1, "Now Sending      ");
            switch ((test->stageOrTimer.timer++ >> 4) & 3)
            {
                case 0:
                    BgCmdBuffer_WriteStringWide(0xF, 4, 1, ".  ");
                    break;
                case 1:
                    BgCmdBuffer_WriteStringWide(0xF, 4, 1, ".. ");
                    break;
                case 2:
                    BgCmdBuffer_WriteStringWide(0xF, 4, 1, "...");
                    break;
            }

            if (test->stageOrTimer.timer > 0xB4)
            {
                BgCmdBuffer_WriteStringWide(2, 4, 1, "Send End         ");
                test->stageOrTimer.timer = 0;
                test->subState = 6;
            }
            break;

        case 6:
            if (test->stageOrTimer.timer++ > 0xB4)
            {
                BgCmdBuffer_WriteStringWide(2, 4, 1, "                 ");
                test->subState = 0;
            }
            break;

        case 7:
            if (test->stageOrTimer.timer++ > 0x3C)
            {
                test->subState = 8;
            }
            break;

        case 8:
            result = 1;
            break;
    }

    if (test->subState <= 6 || test->subState >= 9)
    {
        connected = (Link_IsConnected() != 0) && (Link_IsPeerActive() != 0);
        if (connected == 0)
        {
            BgCmdBuffer_WriteStringWide(4, 9, 1, "SIO ERROR : NOT CONNECT !!");
            test->unk_5 = 0;
            test->stageOrTimer.timer = 0;
            test->subState = 7;
        }
    }

    return result;
}

const u8 sUnk_080e2594[] = "../graph/dr_objhit.h";
const u8 sUnk_080e25ac[] = "obj_";

// TODO: what is this data?
const u8 sUnk_080e25b1[] = {
    0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x08, 0x00, 0x00, 0x01, 0x08, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0xff, 0x00, 0xf8, 0xf8, 0x10, 0x10
};

/**
 * @brief Clear the four BG maps at VRAM 0x0600E000..0x0600FFFF.
 */
void Debug_ClearBgMaps(void)
{
    DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
    DMA_FILL_32(3, 0, VRAM_BASE + 0xE800, 0x800);
    DMA_FILL_32(3, 0, VRAM_BASE + 0xF000, 0x800);
    DMA_FILL_32(3, 0, VRAM_BASE + 0xF800, 0x800);
}

/**
 * @brief Empty init callback of SIO test 0.
 */
void DebugSioTest0_Init(struct DebugSioTestState *test)
{
    return;
}

/**
 * @brief Stop the serial link.
 */
void DebugSioTest0_Exit(struct DebugSioTestState *test)
{
    Link_Stop();
}

/**
 * @brief Empty init callback of SIO test 1.
 */
void DebugSioTest1_Init(struct DebugSioTestState *test)
{
    return;
}

/**
 * @brief Stop the serial link.
 */
void DebugSioTest1_Exit(struct DebugSioTestState *test)
{
    Link_Stop();
}
