/* EventScene04_Init, EventScene04_RunMusicScript. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 GameFlag_GetMask(s32 flag);
extern s32 Entity_SetupFromObjectTable(struct EwramData_EntityData *entity, s32 kind);
extern s32 Music_UpdateRoom(struct GameState *gameState);
extern s32 Music_SetCurrent(s32 song);

/**
 * @brief Init of event scene variant 4 (Object 0x20 unk_514 = 4,
 * sEventSceneCreateFuncs[4]). Create step of a scene object: deleted when flag
 * 0x21 is set or the player is right of x 0x80; else set up animation 0
 * at room (0xA0, 0xC0) moving right (0xC000), facing right, and set
 * gameState.stateFlags bit 0.
 */
void EventScene04_Init(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram;
    struct BgLayerState *bg;

    if (GameFlag_GetMask(0x21) != 0 || (s16)gEwramData->playerState.player->posX.part16.integer > 0x80)
    {
        entity->deleteRequested = 1;
        return;
    }
    if (Entity_SetupFromObjectTable(entity, 5) == 0)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    ewram = gEwramData;
    bg = &ewram->bgInfo[1];
    entity->posX.part16.integer = 0xA0 - bg->xPos.part16.integer;
    entity->posY.part16.integer = 0xC0 - bg->yPos.part16.integer;
    entity->velX.whole = 0xC000;
    entity->flipX = 0;
    ewram->gameState.stateFlags |= 1;
}

/* Script step and timer bytes 0x4FC/0x4FD of EventScene04_RunMusicScript. */
#define STEP(e) ((e)->unk_4FC.bytes[0])
#define TIMER(e) ((e)->unk_4FC.bytes[1])

/**
 * @brief 5F1CC | Timed script: song 0x1010 and wait 0xA0 frames, then
 * Music_SetCurrent(0x16); again song 0x1010 and wait, then Music_UpdateRoom;
 * returns 1 once finished.
 */
s32 EventScene04_RunMusicScript(struct EwramData_EntityData *entity)
{
    s32 done = 0;

    switch (STEP(entity))
    {
        case 0:
            PlaySong(0x1010);
            TIMER(entity) = 0xA0;
            STEP(entity)++;
            break;
        case 1:
            if (--TIMER(entity) == 0xFF)
            {
                Music_SetCurrent(0x16);
                STEP(entity)++;
            }
            break;
        case 2:
            break;
        case 3:
            PlaySong(0x1010);
            TIMER(entity) = 0xA0;
            STEP(entity)++;
            break;
        case 4:
            if (--TIMER(entity) == 0xFF)
            {
                Music_UpdateRoom(&gEwramData->gameState);
                STEP(entity)++;
            }
            break;
        case 5:
            done = 1;
            break;
    }
    return done;
}
