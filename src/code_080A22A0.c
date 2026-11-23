/* Chaos core defeat (22A0). */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/* allocation of 10 words in the Chaos core's unk_504 */
struct ChaosData {
    s32 graphics;
    s32 palette;
    s32 palette2;
    s32 bgRotation;
    s32 bgAngle;
    s32 bgScale;
    s32 bgRadius;
};

extern u8 sChaosDestinationRoom[];
extern void RoomJump_SetDestination(void *, s32, s32, s32, s32);
extern void RoomJump_Begin(struct GameState *);
extern s32 ObjPalette_GetPendingFades(void);
extern void Chaos_SpawnDefeatEffect(s32, s32);

#define ROT_SPEED(e) ((e)->velX.whole)
#define ANGLE_SPEED(e) ((e)->velY.whole)
#define SCALE_SPEED(e) ((e)->accelX.whole)

/* Defeat: the background spins faster, closes in and grows, the screen
 * fades to black and the ending sequence sChaosDestinationRoom starts. */
void Chaos_UpdateDefeat(struct EwramData_EntityData *core)
{
    struct ChaosData *data = (struct ChaosData *)core->ext.chaos.data.value;

    SetPlayerInput(2, 0);
    switch (core->subState)
    {
    case 0:
        Chaos_SpawnDefeatEffect(core->posX.whole, core->posY.whole);
        core->subState++;
        core->timer = 120;
        ROT_SPEED(core) = 0x80;
        ANGLE_SPEED(core) = 0x200;
        SCALE_SPEED(core) = 0;
        gEwramData->gameState.stateFlags |= 1;
        gEwramData->startPauseEnabled = 0;
        break;
    case 1:
        if ((s8)--core->timer <= 0)
        {
            core->subState++;
            core->stage = 0;
            SCALE_SPEED(core) = 1;
            core->timer = 0;
            PlaySong(0x149);
        }
        break;
    case 2:
        if (ROT_SPEED(core) <= 0x7FF)
            ROT_SPEED(core) += 4;
        if (ANGLE_SPEED(core) <= 0x3FF)
            ANGLE_SPEED(core) = 0x100;
        ANGLE_SPEED(core) += 0x20;
        if (data->bgRadius > 0)
        {
            data->bgRadius -= 0x800;
            if (data->bgRadius < 0)
                data->bgRadius = 0;
        }
        if (core->timer <= 59)
            core->timer++;
        else if (core->timer <= 89)
        {
            core->timer++;
            SCALE_SPEED(core) = 2;
        }
        else if (core->timer <= 119)
        {
            core->timer++;
            SCALE_SPEED(core) = 3;
        }
        if (data->bgScale > 0x200 && core->stage == 0)
        {
            BgPalette_StartColorFade(0, 0, 0, 0x10000, 0, 0xC0);
            ObjPalette_StartColorFade(0, 0, 0, 0x10000, 0, 0xD0);
            PlaySong(0x1010);
            core->stage++;
        }
        if (core->stage == 1 && !ObjPalette_GetPendingFades())
        {
            BgPalette_StopColorFade();
            ObjPalette_StopColorFade();
            RoomJump_SetDestination(sChaosDestinationRoom, 0, 0, 0x50, 0xA0);
            RoomJump_Begin(&gEwramData->gameState);
            gEwramData->gameState.stateFlags &= ~1;
        }
        break;
    }
    data->bgRotation += ROT_SPEED(core);
    data->bgAngle += ANGLE_SPEED(core);
    data->bgScale += SCALE_SPEED(core);
}
