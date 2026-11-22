#include "code_08009A0.h"
#include "code_0800B700.h"
#include "code_08012744.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"

struct MenuPage
{
    u8 pad_0[0xA];
    u8 state;
    u8 subState;
    u8 stage;
    u8 timer;
    u8 pad_E[0x15 - 0xE];
    u8 unk_15;
};

extern void Cursor_Draw(void);
extern void Cursor_SetTilePosition(s32 a, s32 b);

/**
 * @brief 4BE0C | Menu save page update: A saves to the current slot
 * (messages 0x366 / 0x36B / 0x386), left/right toggles the yes/no cursor,
 * after a successful save fades out and returns to the title.
 *
 * @return -1 when the page is left, 0 otherwise
 */
s32 SaveMenu_Update(struct MenuPage *page)
{
    s32 ret;
    struct EwramData *ew;
    u16 repeated;
    u16 new;
    struct GameState *g;
    struct EwramData_EntityData *ent;
    s32 msg;

    ret = 0;
    ew = gEwramData;
    repeated = ew->inputData.repeatedInput;
    new = ew->inputData.newInput;
    g = &ew->gameState;
    switch (page->state)
    {
    case 0:
        if (new & 3)
        {
            msg = 0x366;
            if ((new & 1) && !(page->unk_15 & 1))
            {
                SaveData_UpdateSlotSummary(g);
                if (SaveData_SaveSlotToSram(gEwramData->gameState.currentSaveSlot + 1) == 0)
                {
                    msg += 5;
                    PlaySong(0xF4);
                    page->state++;
                }
                else
                {
                    PlaySong(0xEF);
                    msg = 0x386;
                    page->state = 0xFF;
                }
            }
            else
            {
                PlaySong(0xF0);
                page->state = 0xFF;
            }
            MenuText_ClearAndPrint(msg);
        }
        else if (repeated & 0x30)
        {
            ent = &ew->entityData[18];
            page->unk_15++;
            ent->unk_518.part16.hp = ((page->unk_15 & 1) * 4 + 8) * 12 + 5;
            ent->unk_518.part16.unk_51A = 0x89;
            PlaySong(0xF1);
        }
        break;
    case 1:
        if (page->subState == 0)
        {
            PlaySong(0x1010);
            gDisplayRegisters.bldY = ret;
            page->timer = 0;
            page->subState++;
        }
        else if (page->subState == 1)
        {
            page->timer++;
            gDisplayRegisters.bldY = page->timer >> 2;
            if ((page->timer >> 2) == 0x10)
            {
                page->timer = 0x20;
                page->subState++;
            }
        }
        else
        {
            page->timer--;
            if (page->timer == 0xFF)
            {
                Game_ResetSystems(g);
                gDisplayRegisters.bldY = 0x10;
                gDisplayRegisters.bldCnt = 0xFF;
                gEwramData->gameMode = 0;
            }
        }
        break;
    case 0xFF:
        Cursor_SetTilePosition(1, 0x10);
        page->state = ret;
        ret = -1;
        break;
    }
    if (page->state != 1)
        Cursor_Draw();
    return ret;
}
