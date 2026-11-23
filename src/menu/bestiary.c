/*
 * Bestiary menu: detail screen, enemy preview, close, entry / drop / description drawing and the list.
 */
#include "gba.h"
#include "core.h"
#include "macros.h"
#include "structs/ewram.h"
#include "structs/entity_template.h"
#include "soul_inventory.h"
#include "types.h"
#include "gameplay.h"
#include "syscalls.h"
#include "bestiary.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "menu_common.h"
#include "oam.h"
#include "obj_gfx.h"
#include "palette.h"
#include "pause_menu.h"
#include "text12.h"
#include "sound.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"

/* BestiaryMenu_UpdateDetailScreen, BestiaryMenu_SpawnEnemyPreview. */
/* tile as s32: the header u16 would add a zero extension of the argument */
extern void HudMap_WriteTiles(s32 x, s32 y, s32 tile, s32 count);
struct MenuPage3
{
    u8 pad_0[0x13];
    u8 descriptionMode;
    u8 count;
    u8 cursor;
    u8 top;
};

void BestiaryMenu_DrawEntry(struct MenuPage3 *page);
s32 BestiaryMenu_SpawnEnemyPreview(struct EwramData_EntityData *menu);

/**
 * @brief 4C968 | Update of the bestiary detail screen for entity: state 0
 * loads the language dependent graphics, clears the map, draws the header
 * rows and sets mode 3; then every frame BestiaryMenu_ReleaseResources, the object preview
 * (BestiaryMenu_SpawnEnemyPreview) and the list (BestiaryMenu_DrawEntry); leaving mode 3 switches to
 * mode 7 with timer = 1.
 */
void BestiaryMenu_UpdateDetailScreen(struct EwramData_EntityData *entity)
{
    u8 *set;

    switch (entity->stage)
    {
        case 0:
            switch (gEwramData->gameState.language)
            {
                case 2:
                    set = sLocalizedWindowGfxLanguage2;
                    break;
                case 3:
                    set = sLocalizedWindowGfxLanguage3;
                    break;
                default:
                    set = sLocalizedWindowGfx;
                    break;
            }
            Gfx_CopyResource(set, VRAM_BASE + 0xA000, 0);
            BgMap_Setup(1, &sBestiaryDetailBgMap, 0, 0);
            HudMap_ClearAll();
            HudMap_WriteTiles(0x12, 8, 0x1320, 10);
            HudMap_WriteTiles(0x12, 9, 0x1330, 10);
            Text_SetForegroundColor(1);
            entity->subState = 3;
            entity->stage++;
        case 1:
            BestiaryMenu_ReleaseResources();
            BestiaryMenu_SpawnEnemyPreview(entity);
            BestiaryMenu_DrawEntry((struct MenuPage3 *)entity);
            if (entity->subState != 3)
            {
                entity->subState = 7;
                entity->timer = 1;
            }
            break;
    }
}

/**
 * @brief 4CA18 | Spawn the object of template unk_4F9 (of menu) at
 * (0x40, 0x58) plus the template offset: reset the cursor entityData[18],
 * allocate an entity with EntityUpdateNothing, copy hp, updateFunc and
 * the template index, then call the template's createFunc. Non-void in
 * the original (returns through r1) but no value is returned.
 */
s32 BestiaryMenu_SpawnEnemyPreview(struct EwramData_EntityData *menu)
{
    u8 index = menu->ext.menuPage.cursor;
    struct EntityTemplate *template = &sEnemyData[index];
    struct EwramData_EntityData *entity;

    Heap_Reset();
    /* The original adds the slot offset to the loaded gEwramData in place. */
    entity = (struct EwramData_EntityData *)gEwramData;
    entity = (struct EwramData_EntityData *)((u32)entity + OFFSET_OF(struct EwramData, entityData[18]));
    Sprite_Free(entity, 1);
    Hit_Disable(entity);
    entity = Entity_CreateObject(EntityUpdateNothing);
    EntityGetSlot(entity);
    if (entity != NULL)
    {
        entity->hp = template->hp;
        entity->updateFunc = template->updateFunc;
        entity->enemyId = index;
        entity->posX.part16.integer = 0x40;
        entity->posY.part16.integer = 0x58;
        entity->posX.part16.integer += template->previewOffsetX;
        entity->posY.part16.integer += template->previewOffsetY;
        template->createFunc(entity);
    }
}

struct MenuPage
{
    u8 pad_0[0xA];
    u8 state;
    u8 subState;
    u8 stage;
    u8 timer;
    u8 pad_E[0x13 - 0xE];
    u8 descriptionMode;
    s8 count;
    s8 cursor;
    s8 top;
};

void BestiaryMenu_DrawDetailTitle(struct MenuPage3 *page);
extern s32 BestiaryMenu_DrawDescription(s32 id, s32 mode);

/**
 * @brief Detail view input: left/right toggles between the enemy description and
 * its soul name (only when the soul is owned), up/down (or L/R) moves to the
 * previous / next defeated enemy, B closes the view.
 */
void BestiaryMenu_UpdateDetail(struct MenuPage *page)
{
    const struct EntityTemplate *entry;
    s32 new;
    s32 repeated;
    s32 old;
    s32 oldCursor;

    entry = &sEnemyData[page->cursor];
    new = gEwramData->inputData.newInput;
    repeated = gEwramData->inputData.repeatedInput;
    if (page->timer == 0)
        gUnk_03002CB0.dispCnt |= 0x1000;
    else
        page->timer--;
    if (entry->soulIndex != 0 && (new & 0xC0))
    {
        old = page->descriptionMode;
        if (new & 0x80)
        {
            if (SoulInventory_GetSoulTotal(entry->soulType, entry->soulIndex - 1))
                page->descriptionMode = 1;
        }
        else
        {
            if (SoulInventory_GetSoulTotal(entry->soulType, entry->soulIndex - 1))
                page->descriptionMode = 0;
            else
                page->descriptionMode = old = 1;
        }
        if (page->descriptionMode != old)
        {
            BestiaryMenu_DrawDetailTitle((struct MenuPage3 *)page);
            BestiaryMenu_DrawDescription(page->cursor, page->descriptionMode);
            PlaySong(0xF1);
        }
    }
    if (repeated & 0x110)
    {
        oldCursor = page->cursor;
        PlaySong(0xF1);
        do
        {
            if (page->cursor + 1 < page->count)
                page->cursor++;
            else
                page->cursor = 0;
        } while (!Bestiary_IsEnemyDefeated(page->cursor));
        goto moved;
    }
    else if (repeated & 0x220)
    {
        oldCursor = page->cursor;
        PlaySong(0xF1);
        do
        {
            if (page->cursor - 1 >= 0)
                page->cursor = page->cursor - 1;
            else
                page->cursor = page->count - 1;
        } while (!Bestiary_IsEnemyDefeated(page->cursor));
    moved:
        if (page->cursor != oldCursor)
        {
            page->subState = 2;
            gUnk_03002CB0.dispCnt &= 0xEFFF;
        }
    }
    else if ((gEwramData->inputData.newInput & 2) && page->timer == 0)
    {
        gUnk_03002CB0.dispCnt |= 0x1000;
        PlaySong(0xF0);
        page->subState = 6;
        page->stage = 0;
    }
}

/* Restore the four palette slots from the backup. */
struct MenuPage2
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
void BestiaryMenu_Close(struct MenuPage2 *page)
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

/* BestiaryMenu_ReleaseResources. */
/**
 * @brief 4CCC8 | Release the four palette slots sharedObjPaletteSources/sharedObjPaletteVariants, the
 * graphics slots 9..23 (ObjAffine_Free) and ObjAffine_ResetAll, delete and clear
 * the 16 entities entityData[208..223] (spriteSlot = 0xFF), then
 * PaletteAnim_Stop(10..13). Separate counters per loop and a count-up loop
 * (reversed by agbcc) are needed for the original allocation.
 */
void BestiaryMenu_ReleaseResources(void)
{
    s32 i;
    s32 bg;
    s32 n;
    struct EwramData_EntityData *entity;

    for (i = 0; i <= 3; i++)
    {
        if (gEwramData->gameState.sharedObjPaletteSources[i] != 0)
        {
            gEwramData->gameState.sharedObjPaletteSources[i] = 0;
            gEwramData->gameState.sharedObjPaletteVariants[i] = 0;
        }
    }
    for (bg = 9; bg <= 0x17; bg++)
        ObjAffine_Free(bg);
    ObjAffine_ResetAll();
    entity = &gEwramData->entityData[208];
    for (n = 0; n < 16; n++, entity++)
    {
        if (entity->updateFunc != NULL)
        {
            EntityGetSlot(entity);
            Sprite_Free(entity, 1);
            Hit_Disable(entity);
            DMA_FILL_32(3, 0, entity, sizeof(struct EwramData_EntityData));
            entity->spriteSlot = 0xFF;
        }
    }
    PaletteAnim_Stop(10);
    PaletteAnim_Stop(11);
    PaletteAnim_Stop(12);
    PaletteAnim_Stop(13);
}

/* MenuText_Measure: measure a menu text string (Text_GetLocalizedEntry(text) + 2).
 * mode 0: number of characters before the inline character decoder stops
 * (constants 0xFF00/0xF000 kept in locals);
 * otherwise: sum of glyph widths MenuText_GetRangeLength(c - 0x20) over printable
 * characters (c & 0xE0), used by BestiaryMenu_DrawDetailTitle to centre the soul name. */

/**
 * @brief Measure localized text entry text: mode 0 counts characters until the
 * inline control test stops (original constants kept); otherwise sum the
 * glyph widths (MenuText_GetRangeLength) of the leading characters >= 0x20.
 */
s32 MenuText_Measure(s32 text, s32 mode)
{
    if (mode == 0)
    {
        u8 *p = Text_GetLocalizedEntry(text) + 2;
        s32 i = 0;
        u16 a = 0xFF00, b = 0xF000;
        s32 c, v;
        do
        {
            c = p[i++];
            v = (u8)c & a;
            if (v == b) v = c & 0xFF; else v = -1;
        } while (v == -1);
        return i - 1;
    }
    else
    {
        u8 *p = Text_GetLocalizedEntry(text) + 2;
        s32 i = 0;
        u32 sum = 0;
        s32 c;
        do
        {
            c = p[i++];
            if (c & 0xE0)
            {
                sum += (u16)MenuText_GetRangeLength(c - 0x20);
                c = -1;
            }
        } while (c == -1);
        return sum;
    }
}

/* Bestiary page: enemy number, HP/EXP, weak/resist icons and soul icon. */

extern void HudMap_PrintNumber(s32 x, s32 y, s32 value, s32 digits, s32 color, s32 flag);
void BestiaryMenu_PrintDrops(struct EwramData_EntityData *menu);
extern void BestiaryMenu_DrawAttributeIcon(s32 x, s32 y, s32 type);

/**
 * @brief Draw the enemy entry under the cursor: number, HP, EXP, drop names, detail
 * title, description, weak/resist icons and soul icon.
 */
void BestiaryMenu_DrawEntry(struct MenuPage3 *page)
{
    const struct EntityTemplate *entry;
    s32 num;
    s32 base;
    s32 x;
    s32 y;
    s32 n;
    s32 i1, i2, i3, i4;
    s32 tile;
    s32 k;
    s32 pal;
    s32 sx;
    s32 sy;
    s32 y2;

    entry = &sEnemyData[page->cursor];
    num = page->cursor + 1;
    if (num <= 0x63)
    {
        HudMap_ClearTiles(6, 1, 1);
        HudMap_PrintNumber(8, 1, num, 2, 1, 1);
    }
    else
    {
        HudMap_PrintNumber(8, 1, num, 3, 1, 1);
    }
    HudMap_PrintNumber(0x14, 2, entry->hp, 4, 1, 0);
    HudMap_PrintNumber(0x1B, 0xB, entry->exp, 8, 1, 0);
    BestiaryMenu_PrintDrops((struct EwramData_EntityData *)page);
    BestiaryMenu_DrawDetailTitle(page);
    BestiaryMenu_DrawDescription((s8)page->cursor, page->descriptionMode);

    base = 0x15;
    x = base;
    y = 4;
    n = 0;
    for (i1 = 0; i1 <= 8; i1++)
    {
        if (!((entry->flags_1C >> i1) & 1))
            n++;
        else
        {
            BestiaryMenu_DrawAttributeIcon(x++, y, i1);
            if (x - base == 5)
            {
                y++;
                x = base;
            }
        }
    }
    do /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        y2 = 6;
    while (0);
    for (i2 = 0; i2 < n; i2++)
    {
        HudMap_ClearTiles(x++, y, 1);
        if (x - base == 5)
        {
            y++;
            x = base;
        }
    }
    x = base;
    y = y2;
    n = 0;
    for (i3 = 0; i3 <= 8; i3++)
    {
        if (!((entry->resist >> i3) & 1))
            n++;
        else
        {
            BestiaryMenu_DrawAttributeIcon(x++, y, i3);
            if (x - base == 5)
            {
                y++;
                x = base;
            }
        }
    }
    for (i4 = 0; i4 < n; i4++)
    {
        HudMap_ClearTiles(x++, y, 1);
        if (x - base == 5)
        {
            y++;
            x = base;
        }
    }

    sx = 0x18;
    sy = 1;
    k = 0xB;
    pal = 0x300;

    if (entry->soulType == 0 && entry->soulIndex == 0)
    {
        HudMap_ClearTiles(0x18, 2, 3);
        tile = 0x309;
        HudMap_WriteTiles(0x19, 1, tile + 0x90, 2);
        tile += 0xA0;
        HudMap_WriteTiles(0x19, 2, tile, 2);
    }
    else if (SoulInventory_GetSoulTotal(entry->soulType, entry->soulIndex - 1))
    {
        {
            /* otherwise local-alloc folds pal + 8 via REG_EQUIV */
            register s32 p_ asm("r2") = pal; /* FAKEMATCH: pinned register */
            tile = p_ + 8;
        }
        HudMap_WriteTiles(sx, sy, tile + (k << 4), 3);
        HudMap_WriteTiles(sx, sy + 1, tile += (k + 1) << 4, 3);
    }
    else
    {
        HudMap_ClearTiles(sx, sy, 3);
        HudMap_ClearTiles(sx, sy + 1, 3);
    }
}

/* Bit N of a u32 set; the word offset is computed before the set address,
 * as in the original. */
#define TestBit(SET, N) ({ u32 o_ = ((N) >> 5) * 4; u32 *b_ = (SET); *(u32 *)((u8 *)b_ + o_) & (1 << ((N) & 0x1F)); })

/**
 * @brief Print the two drop item names of the selected enemy; a drop not yet
 * obtained (bit sets enemyFlags[4..7] / [8..11]) prints text 10, no drop
 * prints the none text 0x34F.
 */
void BestiaryMenu_PrintDrops(struct EwramData_EntityData *menu)
{
    struct EwramData *ewram = gEwramData;
    struct PlayerStatus *player = &ewram->playerStatus;
    u8 index = menu->ext.menuPage.cursor;
    struct EntityTemplate *template = &sEnemyData[index];
    s32 text;

    Text8_SetRegion(0, 9, 14, 1);
    Text8_ClearRegion();
    if (template->dropItem0 == 0)
        text = 0x34F;
    else if (TestBit(&ewram->playerStatus.enemyFlags[4], index))
        text = sItemNameTextIds[template->dropItem0 - 1];
    else
        text = 10;
    Text8_Print(Text_GetLocalizedEntry(text), 0);
    Text8_SetRegion(0x15, 9, 14, 1);
    Text8_ClearRegion();
    Text8_SetPixelCursor(0x80, 9);
    if (template->dropItem1 == 0)
        text = 0x34F;
    else if (TestBit(&player->enemyFlags[8], index))
        text = sItemNameTextIds[template->dropItem1 - 1];
    else
        text = 10;
    Text8_Print(Text_GetLocalizedEntry(text), 0);
}

extern const u16 sEnemyNameTextIds[];

/**
 * @brief 4D0D0 | Draw the title row of the bestiary detail view: soul name
 * centred at x = 0xB0 and, in name mode, the soul kind icon.
 * Note: total is used uninitialised when the entry has no soul, as in
 * the original.
 */
void BestiaryMenu_DrawDetailTitle(struct MenuPage3 *page)
{
    s32 id;
    const struct EntityTemplate *entry;
    s32 mode;
    u8 total;
    s32 text;
    s32 width;
    s32 col;
    s32 row;

    id = page->cursor;
    entry = &sEnemyData[id];
    mode = page->descriptionMode;
    if (entry->soulIndex == 0)
        mode = 0;
    else
        total = SoulInventory_GetSoulTotal(entry->soulType, entry->soulIndex - 1);
    if (total == 0 && mode == 1)
        mode = 0;
    HudMap_ClearTiles(2, 0xC, 0x14);
    text = sEnemyNameTextIds[id];
    width = MenuText_Measure(text, 1);
    HudMap_WriteTiles(2, 0xC, 0x1310, 0xC);
    Text8_SetRegion(0x15, 8, 0x12, 1);
    Text8_ClearRegion();
    Text8_SetPixelCursor(0xB0 - width / 2, 8);
    Text8_Print(Text_GetLocalizedEntry(text), 0);
    if (mode != 0)
    {
        switch (entry->soulType)
        {
        case 0:
            col = 0xD;
            row = 0xF;
            break;
        case 1:
            col = 0xC;
            row = 0xF;
            break;
        case 2:
            col = 0xB;
            row = 0xF;
            break;
        case 3:
            col = 8;
            row = 0xA;
            break;
        }
        HudMap_WriteTiles(2, 0xC, (u16)((row * 16 - 0x3000 + col) | 0x1300), 1);
    }
}

/**
 * @brief Draw the description box of enemy id; mode 1 (only when its soul is
 * owned) shows the soul name instead of the enemy description text.
 *
 * @return the mode actually used
 */
s32 BestiaryMenu_DrawDescription(s32 id, s32 mode)
{
    const struct EntityTemplate *entry;
    u8 total;

    entry = &sEnemyData[id];
    if (entry->soulIndex != 0)
        total = SoulInventory_GetSoulTotal(entry->soulType, entry->soulIndex - 1);
    else
        total = 0;
    HudMap_WriteTiles(3, 0xE, 0x1200, 0x19);
    HudMap_WriteTiles(3, 0xF, 0x1220, 0x19);
    HudMap_WriteTiles(3, 0x10, 0x1240, 0x19);
    Text12_SetRegion(0, 0, 0x22, 2);
    if (total == 0 && mode == 1)
        mode = 0;
    if (mode == 0)
    {
        MenuText_ClearAndPrint(sEnemyNameTextIds[id] + 0x71);
    }
    else
    {
        Text12_ClearRegion();
        Menu_PrintSoulName(entry->soulType, entry->soulIndex);
    }
    if (total != 0)
    {
        if (mode == 0)
        {
            HudMap_WriteTiles(0x1C, 0x10, 0x134E, 1);
            HudMap_ClearTiles(0x1C, 0xE, 1);
        }
        else
        {
            HudMap_WriteTiles(0x1C, 0xE, 0x134D, 1);
            HudMap_ClearTiles(0x1C, 0x10, 1);
        }
    }
    else
    {
        HudMap_ClearTiles(0x1C, 0xE, 1);
        HudMap_ClearTiles(0x1C, 0x10, 1);
    }
    return mode;
}

/* Enemy list of the bestiary page.
 *
 * Compiler-sensitive constructs:
 * - case 0: the row-clearing loop is a guarded do-while counting k down from
 *   n.  The guard compares `z` (zero, set before the switch and again after
 *   it), which global-alloc leaves without a register, so reload
 *   rematerialises the zero (movs r0, #0; cmp r0, rN).  The constant 1 and
 *   the tile `t` are set after the guard in that order.
 * - `z = i = 0;` after the switch keeps `i` referenced last, so CSE keeps it
 *   as the head of the zero class: the case 1 guard compares i instead of j,
 *   j loses one reference and its priority drops below the constant 1
 *   (j in sl, 1 in sb).
 * - tail: `u` is separate from the tile variable (lives in r0); the m == 6
 *   address is written with `u + 0x100`, so CSE folds it into one
 *   u + 0x0600A118 whose constant reload puts into r4 (r4 joins the spill
 *   registers, which fixes the reload rotation of the whole function).
 * - the dead `odd = 14;` in the second tail loop adds one insn, so loop.c
 *   leaves the 0x10000 constant for its second pass (orrs before it). */

struct MenuPage4
{
    u8 pad_0[0xA];
    u8 state;
    u8 subState;
    u8 stage;
    u8 timer;
    u8 pad_E[0x14 - 0xE];
    s8 count;
    s8 cursor;
    s8 top;
};

struct Unk_08118510
{
    s32 unk[4];
};

extern const struct Unk_08118510 sUnk_08118510;

static inline s32 IsSeen(s32 id)
{
    s32 v = gEwramData->playerStatus.enemyFlags[id >> 5] & (1 << (id & 0x1F));
    return (u32)(-v | v) >> 31;
}

/**
 * @brief Draw the enemy list: up to ten entries starting at the page of the
 * cursor (name, number), blank rows, the page counter and the header;
 * state 0 first clears the rows.
 */
void BestiaryMenu_DrawList(struct MenuPage4 *page)
{
    s32 n;
    s32 id;
    s32 i;
    s32 y;
    s32 j;
    s32 row;
    s32 k;
    s32 total;
    s32 odd;
    s32 t;
    s32 z;
    s32 one;

    page->top = (s8)(page->cursor / 10) * 10;
    id = page->top;
    n = page->count - id;
    if (n > 10)
        n = 10;
    z = 0;
    switch (page->stage)
    {
    case 0:
        i = 0;
        y = 5;

        if (z < n)
        {
            one = 1;
            t = -0xCE0;
            k = n;
            do
            {
                HudMap_WriteTiles((i & one) ? 0x14 : 6, y, (u16)t, 10);
                Text_SetForegroundColor(15);
                i++;
                if (!(i & one))
                    y += 2;
                t += 16;
            } while (--k != 0);
        }
        page->stage++;
    case 1:
        i = 0;
        y = 5;
        j = 0;
        row = 2;
        for (; j < n; j++, row++, id++)
        {
            const struct EntityTemplate *entry = &sEnemyData[id];
            struct Unk_08118510 unused = sUnk_08118510;
            u16 yy;
            s32 x;
            s32 text;
            s32 num;

            if ((SoulInventory_GetSoulTotal(entry->soulType, entry->soulIndex - 1) != 0 && entry->soulIndex != 0)
                || entry->soulIndex == 0)
                Text_SetForegroundColor(15);
            else
                Text_SetForegroundColor(14);
            odd = row & 1;
            x = odd ? 0x15 : 0;
            yy = (row >> 1) + 8;
            Text8_SetRegion(x, yy, 14, 1);
            Text8_ClearRegion();
            if (odd)
                Text8_SetPixelCursor(0x80, yy);
            if (IsSeen(id))
                text = sEnemyNameTextIds[id];
            else
            {
                Text_SetForegroundColor(14);
                text = 0x34E;
            }
            Text8_Print(Text_GetLocalizedEntry(text), 0);
            num = page->top + j + 1;
            if (num <= 99)
            {
                HudMap_ClearTiles((i & 1) ? 0x10 : 2, y, 1);
                HudMap_PrintNumber((i & 1) ? 0x12 : 4, y, page->top + j + 1, 2, 1, 1);
            }
            else
            {
                HudMap_PrintNumber((i & 1) ? 0x12 : 4, y, num, 3, 1, 0);
            }
            i++;
            if (!(i & 1))
                y += 2;
        }
        Text_SetForegroundColor(15);
        for (; j <= 9; j++, row++)
        {
            u16 yy;
            s32 x;

            odd = row & 1;
            x = odd ? 0x15 : 0;
            yy = (row >> 1) + 8;
            Text8_SetRegion(x, yy, 13, 1);
            Text8_ClearRegion();
            if (odd)
                Text8_SetPixelCursor(0x80, yy);
            HudMap_ClearTilesLeft((i & 1) ? 0x12 : 4, y, 3);
            i++;
            if (!(i & 1))
                y += 2;
        }
        HudMap_PrintPaddedNumberInPalette(0xD, 0x11, (s8)(page->top / 10) + 1, 2, 1);
        HudMap_WriteTiles(0xE, 0x11, 0x139B, 1);
        HudMap_PrintPaddedNumberInPalette(0x10, 0x11, (page->count - 1) / 10 + 1, 2, 1);
        HudMap_WriteTiles(2, 0x11, 0x174C, 1);
        HudMap_WriteTiles(0x1C, 0x11, 0x134C, 1);
        Text8_SetRegion(0, 0xF, 0x29, 1);
        Text8_SetPixelCursor(0x60, 0xF);
        Text8_PutChar(0x84);
        Text8_PutChar(0x85);
        gEwramData->text8ColumnPixelOffset += 9;
        gEwramData->text8PixelX += 9;
        Text8_PutChar(0x86);
        Text8_PutChar(0x87);
        HudMap_WriteTiles(3, 0x11, 0xF3EC, 1);
        HudMap_WriteTiles(4, 0x11, 0xF3ED, 1);
        HudMap_WriteTiles(0x1A, 0x11, 0xF3EE, 1);
        HudMap_WriteTiles(0x1B, 0x11, 0xF3EF, 1);
    }
    z = i = 0;
    Text8_SetRegion(0x15, 0xE, 0x12, 1);
    Text8_ClearRegion();
    Text8_SetPixelCursor(0x80, 0xE);
    Text8_Print(Text_GetLocalizedEntry(0x3B1), 0);
    total = SoulInventory_GetCompletionTenths();
    Text8_SetRegion(0, 0xF, 0x10, 1);
    Text8_ClearRegion();
    Text8_Print(Text_GetLocalizedEntry(0x3B2), z);
    {
        s32 m;
        u32 *p;
        s32 u;

        for (m = 0, odd = 14, p = (u32 *)0x0600BC80; m <= 7; p++, m++)
        {
            if (m == 2 || m == 5)
                *p = 0x10000;
            else
                *p = 0;
            u = odd << 9;
        }
        for (m = 0; m <= 7; m++)
        {
            u32 a;
            odd = 14;
            a = (u | 0x100) + m * 4;
            if (m == 6)
                *(u32 *)(0x0600A000 + (u + 0x100) + m * 4) = 0x10000;
            else
                *(u32 *)(0x0600A000 + a) = 0;
        }
    }
    HudMap_PrintNumber(0x18, 0xF, Div(total, 10), 3, 1, 0);
    HudMap_PrintNumber(0x1A, 0xF, Mod(total, 10), 1, 1, 1);
}

/* Menu helpers 0x0804D73C..0x0804D793. */
struct BestiaryAttributeIcon {
    s32 tileColumn; // low four bits of the tile coordinate
    s32 tileRow; // row in the 16-column tile layout
};

extern const struct BestiaryAttributeIcon sBestiaryAttributeIcons[];

/**
 * @brief Return 1 when the defeated flag of enemy id is set (bit set
 * enemyFlags[0..3], set by Enemy_DropRewards), else 0.
 */
s32 Bestiary_IsEnemyDefeated(s32 id)
{
    s32 bit = gEwramData->playerStatus.enemyFlags[id >> 5] & (1 << (id & 0x1F));
    return (u32)(-bit | bit) >> 31; /* bit != 0 */
}

/**
 * @brief Put the single tile of attribute icon index (sBestiaryAttributeIcons) at (x, y);
 * used for the weak/resist rows of the bestiary entry.
 */
void BestiaryMenu_DrawAttributeIcon(s32 x, s32 y, s32 attribute)
{
    const struct BestiaryAttributeIcon *icon = &sBestiaryAttributeIcons[attribute];

    HudMap_WriteTiles(x, y, (u16)(icon->tileColumn - 0x2D00 + (icon->tileRow << 4)), 1);
}
