/* Shop_ShowConfirmWindow: wavy HBlank BG1 vertical offset table (rows 1..63 = 0x40) and windows 0/1 setup. */
#include "gba.h"
#include "code_08001194.h"
#include "structs/ewram.h"
#include "structs/main.h"

/**
 * @brief Open the yes/no confirmation: BG1 H-blank offsets and windows 0/1.
 */
void Shop_ShowConfirmWindow(void)
{
    s16 *buf = gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer];
    s16 *rows = &buf[0x60];
    s32 i;

    HBlankEffectSetup(0xA0, 0xA0, 2, (void *)REG_BG1VOFS);
    { s16 v = 0x40; s32 n = 0x40; s16 *p = &rows[1]; for (i = 1; i < n; i++) *p++ = v; }
    rows[i] = 0;
    gEwramData->hBlankEffect.requestStart = 1;
    gUnk_03002CB0.dispCnt |= DCNT_WIN0;
    {
        struct DisplayRegisters *dr = &gDisplayRegisters;
        u8 *out = &dr->winOut_L;
        u8 v = 0x3F;
        dr->winIn_L = 0x3E;
        dr->win0H = 0x28F0;
        gDisplayRegisters.win0V = 0x7898;
        gUnk_03002CB0.dispCnt |= DCNT_WIN1;
        *out = v;
        dr->winIn_H = 0x2F;
        dr->win1H = 0x28;
        dr->win1V = 0x7898;
    }
}
