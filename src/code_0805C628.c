#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code_08013960.h"
#include "code/code_08039340.h"
#include "gba/dma.h"
#include "structs/ewram.h"

extern void TextWindow_Begin(s32);
extern s32 TextWindow_Update(void);
extern void TextWindow_Close(void);
extern void GameFlag_Set(s32);
extern s32 Entity_TryWalk(struct EwramData_EntityData *, s32, s32, s32, s32);
struct SignedCamera { void *metadata; u16 xFraction; s16 x; u16 yFraction; s16 y; };

/* Scene sequence; animation and transition semantics are not fully identified. */
/**
 * @brief Update of event scene variant 31 (sEventSceneUpdateFuncs[31], the last table
 * entry); returns the player input mask. Scene
 * sequence; animation and transition semantics are not fully identified.
 */
s32 EventScene31_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state) {
    case 0:
        if (entity->stage == 0) {
            entity->timer = 128;
            entity->stage++;
        }
        if ((u8)--entity->timer == 0xFF) {
            BgPalette_StartColorFade(0, 0, 0, 0, 0x10000, 128);
            ObjPalette_StartColorFade(0, 0, 0, 0, 0x10000, 128);
            entity->timer = 0;
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 1:
        if (entity->stage == 0) {
            Music_SetCurrent(33);
            entity->stage++;
        }
        if (BgPalette_GetPendingFades() == 0) {
            BgPalette_StopColorFade();
            ObjPalette_StopColorFade();
            DMA_FILL_32(3, 0, 0x06004000, 0x2000);
            DMA_FILL_16(3, 0, 0x0600E000, 0x800);
            gUnk_03002CB0.dispCnt |= 0x100;
            entity->timer = 255;
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
        } else
            gUnk_03002CB0.dispCnt &= 0xFEFF;
        break;
    case 2:
        switch (entity->subState) {
        case 0:
            if (entity->stage == 0) {
                entity->timer = 128;
                entity->stage++;
            }
            if ((u8)--entity->timer == 0xFF) {
                struct EwramData *ew = gEwramData;
                struct EwramData_EntityData **player = &ew->playerState.player;
                struct EwramData_EntityData *initial = *player;
                struct BgLayerState *bg = &ew->bgInfo[1];
                initial->posX.part16.integer = 208 - bg->xPos.part16.integer;
                (*player)->posY.part16.integer = 191 - bg->yPos.part16.integer;
                entity->subState++;
                entity->stage = 0;
            }
            break;
        case 1:
            if (entity->stage == 0) {
                entity->timer = 64;
                entity->stage++;
            }
            if ((u8)--entity->timer == 0xFF) {
                entity->subState++;
                entity->stage = 0;
            }
            break;
        case 2: {
            s32 x;
            struct EwramData *ew;
            struct SignedCamera *bg;
            if (entity->stage == 0) {
                if (entity->animScriptIndex != 1)
                    Anim_StartIndexed(entity, 1, 1, 1);
                entity->stage++;
            }
            x = (s16)entity->posX.part16.integer;
            ew = gEwramData;
            bg = (struct SignedCamera *)&ew->bgInfo[1];
            if (x + bg->x > 184) {
                if (!(Entity_TryWalk(entity, 0x10000, 0x200000, 0x20000, 1) & 4))
                    entity->posY.whole += entity->velY.whole;
            } else {
                struct EwramData_EntityData **player = &ew->playerState.player;
                (*player)->posX.part16.integer = 160 - bg->x;
                (*player)->posY.part16.integer = 191 - bg->y;
                entity->subState++;
                entity->stage = 0;
            }
            break;
        }
        case 3:
            if (entity->stage == 0) {
                if (entity->animScriptIndex != 2)
                    Anim_StartIndexed(entity, 2, 1, 0);
                entity->stage++;
            }
            if ((s16)entity->posX.part16.integer + ((struct SignedCamera *)&gEwramData->bgInfo[1])->x > 176) {
                if (!(Entity_TryWalk(entity, 0x8000, 0x200000, 0x20000, 1) & 4))
                    entity->posY.whole += entity->velY.whole;
            }
            if (entity->animDone) {
                if (entity->animScriptIndex != 0)
                    Anim_StartIndexed(entity, 0, 1, 1);
                entity->subState += 2;
                entity->stage = 0;
            }
            break;
        case 4:
            if (entity->stage == 0) {
                if (entity->animScriptIndex != 5)
                    Anim_StartIndexed(entity, 5, 1, 0);
                entity->stage++;
            }
            if ((s16)entity->posX.part16.integer + ((struct SignedCamera *)&gEwramData->bgInfo[1])->x > 176) {
                if (!(Entity_TryWalk(entity, 0x8000, 0x200000, 0x20000, 1) & 4))
                    entity->posY.whole += entity->velY.whole;
            }
            if (entity->animStep == 4)
                PlaySong(0x1DF);
            if (entity->animStep == 8)
                PlaySong(0xFC);
            if (entity->animDone) {
                if (entity->animScriptIndex != 0)
                    Anim_StartIndexed(entity, 0, 1, 1);
                entity->subState++;
                entity->stage = 0;
            }
            break;
        case 5:
            if (entity->stage == 0) {
                TextWindow_Begin(90);
                entity->timer = 68;
                entity->stage++;
            }
            if ((u8)--entity->timer == 0xFF) {
                entity->state++;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
        }
        break;
    case 3:
        if (TextWindow_Update()) {
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 4:
        if (entity->stage == 0) {
            entity->timer = 104;
            entity->stage++;
        }
        if ((u8)--entity->timer == 0xFF) {
            entity->unk_4F8.unk_4F8_entity->state++;
            { struct EwramData_EntityData *other = entity->unk_4F8.unk_4F8_entity;
              other->stage = 0; other->subState = 0; }
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 5:
        if (entity->unk_4F8.unk_4F8_entity->state == 0x80) {
            Music_SubmitCommandOnce(0x1040);
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 6:
        if (entity->stage == 0) {
            BgPalette_StartColorFade(0, 0, 0, 0x10000, 0, 128);
            ObjPalette_StartColorFade(0, 0, 0, 0x10000, 0, 128);
            gUnk_03002CB0.dispCnt &= 0xFEFF;
            GameFlag_Set(45);
            entity->stage++;
            entity->timer = 128;
        }
        if (BgPalette_GetPendingFades() == 0) {
            entity->state = 0x80;
            TextWindow_Close();
            gEwramData->gameState.subMode = 10;
        }
        break;
    }
    Anim_Update(entity);
    gEwramData->gameState.stateFlags |= 0x20;
    return 0;
}
