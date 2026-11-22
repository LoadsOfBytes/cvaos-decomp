#include "code_08009A0.h"
#include "code/code_08039340.h"

struct MenuPage
{
    u8 pad_0[0xB];
    u8 subState;
    u8 stage;
    u8 pad_D[0x13 - 0xD];
    u8 descriptionMode;
    u8 unk_14;
    u8 unk_15;
};


/* Backup of the four palette slots inside the save buffer (0x1340C). */
#define BACKUP_SLOT(i) (*(s32 *)((u8 *)gEwramData + 0x1340C + (i) * 4))
#define BACKUP_VARIANT(i) ((u8 *)gEwramData + 0x13420 + (i) * 2)

extern void *memcpy(void *dst, const void *src, unsigned long size);
extern void Heap_SaveAndReset(void);
extern void MenuPage_Reset(struct MenuPage *page);
extern void BestiaryMenu_Setup(struct MenuPage *page);

/**
 * @brief Open the bestiary page: back up the palette slots, load the page
 * graphics (per language) and reset the page state.
 */
void BestiaryMenu_Open(struct MenuPage *page)
{
    u8 *gfx;

    BACKUP_SLOT(0) = gEwramData->gameState.sharedObjPaletteSources[0];
    BACKUP_SLOT(1) = gEwramData->gameState.sharedObjPaletteSources[1];
    BACKUP_SLOT(2) = gEwramData->gameState.sharedObjPaletteSources[2];
    BACKUP_SLOT(3) = gEwramData->gameState.sharedObjPaletteSources[3];
    *(u16 *)BACKUP_VARIANT(0) = *(u16 *)&gEwramData->gameState.sharedObjPaletteVariants[0];
    memcpy(BACKUP_VARIANT(1), &gEwramData->gameState.sharedObjPaletteVariants[1], 2);
    *(u16 *)BACKUP_VARIANT(2) = *(u16 *)&gEwramData->gameState.sharedObjPaletteVariants[2];
    memcpy(BACKUP_VARIANT(3), &gEwramData->gameState.sharedObjPaletteVariants[3], 2);
    BgPalette_Load((u8 *)0x0815E0A4);
    BgPalette_LoadRange((u8 *)0x080E5C20, 0xD, 1, 0xD);
    BgPalette_LoadRange((u8 *)0x081183F4, 1, 1, 0xF);
    switch (gEwramData->gameState.language)
    {
    case 2:
        gfx = (u8 *)0x0827799C;
        break;
    case 3:
        gfx = (u8 *)0x0826C858;
        break;
    default:
        gfx = (u8 *)0x0815E034;
        break;
    }
    Gfx_CopyResource(gfx, (void *)0x06000000, 0);
    page->unk_15 = 0;
    Heap_SaveAndReset();
    gEwramData->gameState.bestiaryPreview = 1;
    page->subState = 0;
    page->stage = 0;
    MenuPage_Reset(page);
    page->descriptionMode = 0;
    page->unk_14 = 0x70;
    BestiaryMenu_Setup(page);
}
