/* Upstream functions 0x0803AEFC..0x0803B183 from the previously unlinked
 * src/code/code_08039340.c, now linked as matching C.
 * Sprite_BuildSingle and ObjGfx_RestoreState (no upstream C yet) stay in asm. Hard-coded
 * ROM addresses of the draft were replaced by data labels. */
#include "structs/render_slots.h"
#include "code/code_08039340.h"
#include "code_08009A0.h"
#include "code_08001004.h"
#include "code_08001194.h"
#include "code_08008750.h"
#include "code_0800CB00.h"
#include "code_0800F1FC.h"
#include "code/code_08014548.h"
#include "code/code_080211F0.h"
#include "code_08032444.h"
#include "code_08032E4C.h"
#include "code_08033CAC.h"
#include "code_08035930.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "code/code_080E032C.h"
#include "gba.h"
#include "macros.h"
#include "agb_sram.h"
#include "syscalls.h"

#include "constants/main.h"

#include "data/data_0E0334.h"
#include "data/data_0E3464.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectGfxLanguage2[];
extern u8 sCommonObjectGfxLanguage3[];

struct SingleSpriteOamView {
    u8 attribute0Low;
    u8 attribute0High;
    s16 attribute1;
};

/**
 * @brief 3AEFC | Draw the entity as one OAM sprite at its screen position
 *
 * @param entity Entity
 * @return s32 1 if drawn, 0 if off screen or no OAM entry is free
 */
s32 Sprite_DrawSingle(struct EwramData_EntityData *entity)
{
    s16 screenY;
    s32 y;
    s32 x;
    s32 row;
    u32 column;
    u64 *oam;
    struct SingleSpriteOamView *attr;

    if ((u32)((entity->posX.part16.integer + 0x40) << 0x10) > 0x01700000U)
        return 0;

    screenY = entity->posY.part16.integer;
    if (screenY < -0x20)
        return 0;
    if (screenY > 0xE0)
        return 0;

    oam = (void *)Obj_Alloc();
    if (oam == NULL)
        return 0;

    attr = (struct SingleSpriteOamView *)oam;
    {
        struct EwramData *ewram = gEwramData;

        *oam = *(u64 *)ewram->singleSpriteOam;
        y = ewram->singleSpriteY + entity->posY.whole;
    }
    if (y < 0)
        row = -(-y >> 0x10);
    else
        row = (u32)y >> 0x10;
    attr->attribute0Low = row;

    x = gEwramData->singleSpriteX + entity->posX.whole;
    if (x < 0)
        column = -(-x >> 0x10) & 0x1FF;
    else
        column = ((u32)x >> 0x10) & 0x1FF;
    attr->attribute1 = (attr->attribute1 & ~0x1FF) | column;

    Obj_AddToLayer((void *)oam, entity->spriteLayer);
    return 1;
}

/**
 * @brief 3AFB8 | Find the tile slot of an already loaded resource
 *
 * sCommonObjectGfx is replaced by its language 2/3 variant first.
 *
 * @param resource Graphics resource
 * @return s32 First tile slot, or -1 if not loaded
 */
s32 ObjGfx_Find(u8 *resource)
{
    s32 entry;

    entry = gEwramData->objGfxUsedHead;
    if (resource == sCommonObjectGfx)
    {
        switch (gEwramData->gameState.language)
        {
            case 2:
                resource = sCommonObjectGfxLanguage2;
                break;
            case 3:
                resource = sCommonObjectGfxLanguage3;
                break;
        }
    }

    while (entry != -1)
    {
        if (gEwramData->objGfx[entry].source == resource)
        {
            return gEwramData->objGfx[entry].tileSlot;
        }
        entry = gEwramData->objGfx[entry].next;
    }
    return -1;
}

/**
 * @brief 3B04C | Back up the graphics cache and drop entries with lifetime 0
 *
 * Used before menus/cutscenes; the dropped entries are reloaded by
 * ObjGfx_RestoreState.
 *
 * @return s32 1, or 0 if a backup already exists
 */
s32 ObjGfx_SaveState(void)
{
    s32 i;

    if (gEwramData->objGfxSaved == 0)
    {
        DMA_COPY_32(3, &gEwramData->objGfx, &gEwramData->objGfxBackup, 0x140);
        DMA_COPY_32(3, &gEwramData->objGfxFreeHead, &gEwramData->objGfxListBackup, 0x8);
        DMA_COPY_32(3, &gEwramData->objGfxSetList, &gEwramData->objGfxSetListBackup, 0x1C);
        gEwramData->objGfxSetBackup = gEwramData->objGfxSet;
        gEwramData->objGfxSet = 0;

        for (i = 0; i < 0x10; i++)
        {
            if (gEwramData->objGfx[i].tileSlot != -1)
            {
                if (gEwramData->objGfx[i].lifetime == 0)
                {
                    ObjTiles_Free(gEwramData->objGfx[i].tileSlot);
                    gEwramData->objGfx[i].tileSlot = -1;
                    gEwramData->objGfx[i].kind = 0;
                    gEwramData->objGfx[i].lifetime = 0;
                    gEwramData->objGfx[i].frames[0] = 0;
                    gEwramData->objGfx[i].frames[1] = 0;
                    gEwramData->objGfx[i].source = 0;
                }
            }
        }

        gEwramData->objGfxSaved = 1;
        return 1;
    }
    return 0;
}
