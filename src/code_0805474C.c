/* Object1CUpdate, decompiled from the original assembly (save point / save
 * room object 0x1C). Bg_SetControlIfLevelAllows is declared here with an s32 third
 * parameter: the original caller passes 0x10 - x without narrowing it to u16.
 * The state is read into a local before &gEwramData is copied into its own
 * register. */
#include "code_08009A0.h"
#include "save_point_entity.h"
#include "code_080D73B8.h"
#include "code_0800CB00.h"
#include "code_08011DD0.h"
#include "code_08012744.h"
#include "code/code_080211F0.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern u8 sSaveWarpRoomPalette[], sSavePointPalette[], sSavePromptBgPalette[], sUnk_08119898[];
extern void ObjPalette_LoadRange(u8 *palette, u16 first, u16 count, u16 slot);
extern void BgPalette_LoadRange(u8 *palette, u16 first, u16 count, u16 slot);
extern void ObjPalette_StopColorFade(void);
extern void BgPalette_StopColorFade(void);
extern void ObjAffine_Set(s32 angle, s32 scaleX, s32 scaleY, u16 flipX, u16 flipY, u16 index);
extern s32 Bg_SetControlIfLevelAllows(s32 level, s32 reg, s32 value);
extern void Text12_ClearRegion(void);
extern void Text_SetBackgroundColor(s32 color);
extern void SavePoint_ShowSavePrompt(void);
extern s32 SavePoint_SpawnOrbitLights(struct EwramData_EntityData *parent);

#define OWNER(e) (&(*ew)->entityData[(e)->unk_518.part8.unk_519])

void Object1CUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData **ew;
    u8 state;
    u16 held = gEwramData->inputData.heldInput;
    u16 pressed = gEwramData->inputData.newInput;
    state = entity->state;
    ew = &gEwramData;

    switch (state)
    {
    case 0:
    {
        u8 sub = entity->subState;
        if (sub == 0)
        {
            if ((pressed & 0x40) && Entity_IsInRect((*ew)->playerState.player, 0x68, 0x80, 0x20, 0x37))
            {
                entity->subState++;
                entity->timer = sub;
                break;
            }
            gEwramData->gameState.stateFlags &= ~1;
            break;
        }
        {
            s32 r = Player_IsBusy();
            if (r == 0)
            {
                PlaySong(0xF3);
                BgPalette_StopColorFade();
                ObjPalette_StopColorFade();
                RedSoul_CancelAll();
                gEwramData->gameState.stateFlags |= 1;
                SavePoint_ShowSavePrompt();
                Player_RestoreHPAndMP();
                gEwramData->startPauseEnabled = 0;
                entity->ext.savePoint.yesSelected = 1;
                entity->subState = r;
                goto next;
            }
            if (entity->timer == 3)
                entity->subState = 0;
        }
        break;
    }
    case 1:
    {
        struct EwramData_EntityData *owner = OWNER(entity);
        ObjPalette_LoadRange(sSaveWarpRoomPalette, 4, 1, 0xE);
        owner->hidden = 0;
        goto next;
    }
    case 2:
    {
        s32 ab = pressed & 3;
        if (ab)
        {
            if ((held & 1) && entity->ext.savePoint.yesSelected)
            {
                struct EwramData *base = *ew;
                s32 res;
                SaveData_StorePlayerPosition(&base->gameState);
                SaveData_UpdateSlotSummary(&base->gameState);
                res = SaveData_SaveSlotToSram(base->gameState.currentSaveSlot);
                if (res == 0)
                {
                    struct EwramData_EntityData *owner;
                    Text_SetDefaultColors();
                    entity->timer = res;
                    entity->state = 0x14;
                    entity->ext.savePoint.flags |= 2;
                    PlaySong(0xF3);
                    owner = OWNER(entity);
                    ObjPalette_LoadRange(sSavePointPalette, 5, 1, 0xE);
                    owner->hidden = 1;
                    Text12_ClearRegion();
                    break;
                }
                else
                {
                    struct EwramData_EntityData *owner;
                    PlaySong(0xEF);
                    owner = OWNER(entity);
                    ObjPalette_LoadRange(sSavePointPalette, 5, 1, 0xE);
                    owner->hidden = 1;
                    Text12_ClearRegion();
                    BgPalette_LoadRange(sSavePromptBgPalette, 0, 2, 0xD);
                    Text_SetBackgroundColor(2);
                    goto reset;
                }
            }
            PlaySong(0xF0);
            goto next;
        }
        if ((held & 0x10) && entity->ext.savePoint.yesSelected)
        {
            PlaySong(0xF1);
            entity->ext.savePoint.yesSelected = ab;
            break;
        }
        if (!(held & 0x20))
            break;
        if (entity->ext.savePoint.yesSelected)
            break;
        PlaySong(0xF1);
        entity->ext.savePoint.yesSelected = 1;
        break;
    }
    case 3:
    {
        struct EwramData_EntityData *owner = OWNER(entity);
        ObjPalette_LoadRange(sSavePointPalette, 5, 1, 0xE);
        owner->hidden = 1;
        Text12_ClearRegion();
    }
    reset:
        (*ew)->startPauseEnabled = 1;
        entity->timer = 0;
        entity->state = 0;
        break;
    case 11:
    {
        if (entity->unk_514.part8.unk_516 == 0)
        {
            if (entity->unk_514.part8.unk_517 == 0)
            {
                entity->ext.savePoint.scale += 0x800;
                if (entity->ext.savePoint.scale > 0xFFFF)
                    entity->ext.savePoint.scale = 0x10000;
                break;
            }
            entity->ext.savePoint.scale -= 0xCC;
            if (entity->ext.savePoint.scale <= 0)
                entity->ext.savePoint.scale = 0;
            break;
        }
        Bg_SetControlIfLevelAllows(3, 0x50, 0x3FBF);
        Bg_SetControlIfLevelAllows(3, 0x54, 0);
        goto clear;
    }
    case 12:
        Bg_SetControlIfLevelAllows(3, 0x54, entity->timer >> 2);
        if (entity->timer > 0x3F)
        {
            entity->hidden = 1;
            entity->timer = 0;
            entity->state++;
        }
        break;
    clear:
        entity->timer = 0;
    next:
        entity->state++;
        break;
    case 13:
        Bg_SetControlIfLevelAllows(3, 0x54, 0x10 - entity->timer);
        if (entity->timer <= 0xF)
            break;
        gEwramData->gameState.stateFlags &= ~1;
        goto done;
    case 10:
    case 20:
        if (entity->unk_514.part8.unk_515 != 1)
            break;
        SavePoint_SpawnOrbitLights(entity);
        BgPalette_LoadRange(sUnk_08119898 + 0x378, 0xC, 2, 2);
        entity->state++;
        PlaySong(0xF6);
        break;
    case 21:
    {
        if (entity->unk_514.part8.unk_516 == 0)
        {
            if (entity->unk_514.part8.unk_517 == 0)
            {
                entity->ext.savePoint.scale += 0x800;
                if (entity->ext.savePoint.scale > 0xFFFF)
                    entity->ext.savePoint.scale = 0x10000;
                if (entity->subState == 0)
                {
                    entity->subState = 1;
                    entity->timer = 0;
                }
            }
            else
            {
                entity->ext.savePoint.scale -= 0xCC;
                if (entity->ext.savePoint.scale <= 0)
                    entity->ext.savePoint.scale = 0;
            }
        }
        switch (entity->subState)
        {
        case 0:
            break;
        case 1:
            if (entity->timer == 0xE)
            {
                Bg_SetControlIfLevelAllows(3, 0x50, 0x3FBF);
                Bg_SetControlIfLevelAllows(3, 0x54, 0);
                entity->timer = 0;
                entity->subState++;
            }
        case 2:
            Bg_SetControlIfLevelAllows(3, 0x54, entity->timer >> 1);
            if (entity->timer > 0x1F)
            {
                Bg_SetControlIfLevelAllows(3, 0x54, 0x10);
                entity->subState++;
                entity->timer = 0;
            }
            break;
        case 3:
            Bg_SetControlIfLevelAllows(3, 0x54, 0x10 - (entity->timer >> 3));
            if (entity->timer > 0x3F)
                entity->subState++;
            break;
        case 4:
            if (entity->unk_514.part8.unk_516 == 1)
            {
                (*ew)->gameState.stateFlags &= ~1;
                Bg_SetControlIfLevelAllows(3, 0x54, 0);
                entity->timer = 0;
                entity->subState = 0;
            done:
                entity->state = 0x63;
            }
            break;
        }
        break;
    }
    case 99:
    {
        struct EwramData_EntityData *owner = OWNER(entity);
        ObjPalette_LoadRange(sSavePointPalette, 5, 1, 0xE);
        owner->hidden = 1;
        Text12_ClearRegion();
        entity->deleteRequested = 1;
        (*ew)->startPauseEnabled = 1;
        break;
    }
    }

    if (entity->state != 0)
        SetPlayerInput(1, 0x40);
    ObjAffine_Set(0, entity->ext.savePoint.scale, entity->ext.savePoint.scale, 0, 0, entity->ext.savePoint.affine);
    entity->timer++;
    if ((u8)(entity->timer % 3) == 0 && entity->unk_514.part8.unk_515 != 1)
    {
        u8 *blink = &SAVE_POINT_BLINK(entity);
        *blink = 1 - *blink;
        if (*blink)
            BgPalette_LoadRange(sUnk_08119898 + 0x378, 0xA, 2, 2);
        else
            BgPalette_LoadRange(sUnk_08119898 + 0x378, 2, 2, 2);
    }
}
