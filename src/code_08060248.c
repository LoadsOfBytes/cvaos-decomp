#include "code_08009A0.h"
#include "event_scene09.h"
#include "code_080D73B8.h"
#include "code_08013960.h"
#include "code_08011DD0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void TextWindow_Begin(s32);
extern s32 TextWindow_Update(void);
extern void EventScene09_PlaceHelpers(struct EwramData_EntityData *);
extern void Interaction_End(struct EwramData_EntityData *, s32);

/* Event sequencing. Several original early-return paths leave r0 unspecified. */
/**
 * @brief Update of event scene variant 9 (sEventSceneUpdateFuncs[9]); returns the player
 * input mask.
 */
s32 EventScene09_Update(struct EwramData_EntityData *entity)
{
    s32 input = 0;
    struct GameState *context;
    /* The pinned flag pointer below preserves agbcc's scratch-address lifetime;
     * it reconstructs the binary, not the original source spelling. */
    struct EwramData *ew = gEwramData;
    context = &ew->gameState;
    switch (entity->state) {
    case 0:
        PlaySong(0x1B8);
        Music_StartOverride(40);
        TextWindow_Begin(20);
        { u8 *flag = &ew->gameState.textWindowPause; *flag = 0; }
        entity->state++;
    case 1:
        if (TextWindow_Update()) {
            EventScene09_PlaceHelpers(entity);
            entity->state = 0x80;
            Interaction_End(entity, 0);
            GameFlag_Set(0x26);
            return;
        }
        switch (*(vu8 *)&entity->subState) { /* volatile read (replaces a pinned register) */
        case 0:
        case 7:
            break;
        case 1: {
            switch (entity->stage) {
            case 0:
                context->textWindowPause = 1;
                entity->stage++;
            case 1: {
                struct EwramData_EntityData *other = EVENT09_ACTOR5(entity);
                if (other->updateFunc) {
                    Anim_StartIndexed(other, 6, 0, 0);
                    { struct EwramData_EntityData *updated = EVENT09_ACTOR5(entity);
                      updated->flipX = 1 - updated->flipX; }
                }
                Anim_StartIndexed(entity, 5, 0, 1);
                entity->stage++;
            }
            case 2: {
                struct EwramData_EntityData *other2 = EVENT09_ACTOR5(entity);
                if (other2->animDone) {
                    if (other2->animScriptIndex != 7)
                        Anim_StartIndexed(other2, 7, 0, 1);
                    entity->stage++;
                    entity->timer = 8;
                }
                break;
            }
            case 3:
                if ((u8)--entity->timer == 0xFF)
                    entity->stage++;
                break;
            case 4:
                context->textWindowPause = 0;
                break;
            }
            break;
        }
        case 2:
            switch (entity->stage) {
            case 0:
                context->textWindowPause = 1;
                entity->stage++;
                entity->timer = 16;
                if (entity->animScriptIndex != 4)
                    Anim_StartIndexed(entity, 4, 0, 0);
            case 1:
                if ((u8)--entity->timer == 0xFF) {
                    entity->stage++;
                    entity->timer = 0;
                }
                break;
            case 2: {
                struct EwramData_EntityData *player;
                s32 x;
                if (entity->timer == 36)
                    PlaySong(0x156);
                else if (entity->timer == 0)
                    PlaySong(0x155);
                entity->timer++;
                player = gEwramData->playerState.player;
                x = entity->posX.whole + 0x200000;
                if (player->posX.whole <= x) {
                    player->posX.whole = x;
                    entity->stage++;
                } else
                    input |= 0x20;
                break;
            }
            case 3:
                input |= 0x80;
                context->textWindowPause = 0;
                break;
            }
            if (EVENT09_SCALE_X(entity) != 0) {
                EVENT09_SCALE_X(entity) -= 0x800;
                EVENT09_SCALE_Y(entity) += 0x400;
                ObjAffine_Set(0x10000, EVENT09_SCALE_X(entity), EVENT09_SCALE_Y(entity),
                            0, 0, EVENT09_ACTOR5(entity)->affineIndex);
            } else
                EVENT09_ACTOR5(entity)->hidden = 1;
            break;
        case 3:
            switch (entity->stage) {
            case 0:
                context->textWindowPause = 1;
                entity->stage++;
                entity->timer = 48;
            case 1:
                if ((u8)--entity->timer == 0xFF) {
                    entity->stage++;
                    entity->timer = 96;
                    if (entity->animScriptIndex != 7)
                        Anim_StartIndexed(entity, 7, 0, 0);
                }
                break;
            case 2:
                if (entity->animDone)
                    context->textWindowPause = 0;
                break;
            }
            input |= 0x80;
            break;
        case 4:
            switch (entity->stage) {
            case 0:
                context->textWindowPause = 1;
                entity->timer = 48;
                entity->stage++;
                if (entity->animScriptIndex != 8)
                    Anim_StartIndexed(entity, 8, 0, 0);
            case 1:
                if ((u8)--entity->timer == 0xFF)
                    entity->stage++;
                break;
            case 2:
                context->textWindowPause = 0;
                break;
            }
            input |= 0x80;
            break;
        case 5:
            switch (entity->stage) {
            case 0:
                context->textWindowPause = 1;
                entity->stage++;
                entity->timer = 48;
            case 1:
                if (entity->timer == 0) {
                    context->textWindowPause = 0;
                    EVENT09_ACTOR1(entity)->state = 1;
                } else
                    entity->timer--;
                break;
            }
            input |= 0x80;
            break;
        case 6:
            gEwramData->playerState.player->flipX = 0;
            entity->spriteLayer = gEwramData->playerState.player->spriteLayer + 1;
            break;
        }
        { struct EwramData_EntityData *other = EVENT09_ACTOR1(entity);
        if (other->state) {
            s32 x = entity->posX.whole + 0x600000;
            if (other->posX.whole < x) {
                other->posX.whole = x;
                other = EVENT09_ACTOR1(entity);
                if (other->animScriptIndex != 1)
                    Anim_StartIndexed(other, 1, 0, 1);
                EVENT09_ACTOR1(entity)->state = 0;
            } else
                other->posX.whole += other->velX.whole;
        }
        }
        break;
    case 3:
        switch (entity->stage) {
        case 0:
            { struct EwramData_EntityData **player = &ew->playerState.player;
              (*player)->flipX = 1; }
            entity->stage++;
        case 1:
            if ((s16)gEwramData->playerState.player->posX.part16.integer <= 2)
                entity->stage++;
            else
                input = 0x20;
            break;
        case 2:
            break;
        }
        break;
    case 0x80:
        return;
    }
    Anim_Update(entity);
    return input;
}
