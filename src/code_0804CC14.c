/* Restore the four palette slots from the backup. */
#include "gba.h"
#include "structs/ewram.h"
#include "macros.h"

struct MenuPage
{
    u8 pad_0[0xA];
    u8 state;
    u8 subState;
    u8 stage;
};

struct U16Box
{
    u16 v;
};

extern void BestiaryMenu_ReleaseResources(void);
extern void Heap_Restore(void);
extern void *memcpy(void *dst, const void *src, unsigned long size);

/**
 * @brief 4CC14 | Release the menu slots (BestiaryMenu_ReleaseResources), clear bestiaryPreview and
 * the page cursor, then copy sharedObjPaletteSources[0..3] and sharedObjPaletteVariants[0..3] back from the
 * backup at sharedScratchBuffer[0x18] / [0x2C] (2-byte source stride).
 * The copies are memcpy calls on byte offsets: the builtin expands the
 * even (2-aligned) destinations to ldrh/strh and keeps the library call for
 * the odd ones. The first word and the first halfword are in-struct stores,
 * so the following copy reuses the loaded gEwramData.
 */
void BestiaryMenu_Close(struct MenuPage *page)
{
    BestiaryMenu_ReleaseResources();
    gEwramData->gameState.bestiaryPreview = 0;
    page->subState = 0;
    page->stage = 0;
    gEwramData->gameState.sharedObjPaletteSources[0] = *(s32 *)&gEwramData->sharedScratchBuffer[0x18];
    memcpy((u8 *)gEwramData + 0x3E4, (u8 *)gEwramData + 0x13410, 4);
    memcpy((u8 *)gEwramData + 0x3E8, (u8 *)gEwramData + 0x13414, 4);
    memcpy((u8 *)gEwramData + 0x3EC, (u8 *)gEwramData + 0x13418, 4);
    ((struct U16Box *)gEwramData->gameState.sharedObjPaletteVariants)->v = *(u16 *)&gEwramData->sharedScratchBuffer[0x2C];
    memcpy((u8 *)gEwramData + 0x3F1, (u8 *)gEwramData + 0x13422, 2);
    memcpy((u8 *)gEwramData + 0x3F2, (u8 *)gEwramData + 0x13424, 2);
    memcpy((u8 *)gEwramData + 0x3F3, (u8 *)gEwramData + 0x13426, 2);
    Heap_Restore();
}
