/* Chaos core: intro (20D0). (signed modulo through a separate variable). */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
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
    struct EwramData_EntityData *orbs[3];
};

extern void Player_StartScriptedAnimation(s32, s32, s32, s32);
extern void Effect_CreateHomingOrb(struct EwramData_EntityData *, s32, s32, s32, s32);
extern void Chaos_CreateMark(struct EwramData_EntityData *, s32);

/* Intro: hold the player, fade from white, shake the screen, show the
 * marks and let the three orbs fire at the player, then start the fight. */
void Chaos_UpdateIntro(struct EwramData_EntityData *core)
{
    struct ChaosData *data = (struct ChaosData *)core->unk_504.whole;

    SetPlayerInput(2, 0);
    switch (core->subState)
    {
    case 0:
        core->subState = 1;
        BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x78);
        ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x78);
        core->timer = 0x78;
        core->stage = 0;
        Player_StartScriptedAnimation(0x32, 1, 0, 0);
        gEwramData->startPauseEnabled = 0;
        break;
    case 1:
        Player_StartScriptedAnimation(0x32, 1, 0, 0);
        if (--core->timer == 0)
        {
            ObjPalette_StopColorFade();
            BgPalette_StopColorFade();
            core->subState++;
            core->timer = 0;
        }
        break;
    case 2:
        if (!(core->timer & 4))
            Player_StartScriptedAnimation(0x52, 0, 0, 0x8000);
        else
            Player_StartScriptedAnimation(0x52, 0, 0, -0x8000);
        if (core->timer == 0x51)
        {
            Chaos_CreateMark(core, 0);
            Chaos_CreateMark(core, 1);
            Chaos_CreateMark(core, 2);
        }
        if (core->timer >= 0x29 && core->timer <= 0x95 && (core->timer & 7) == 0)
        {
            s32 v = core->timer >> 3;
            s32 n = v % 3;

            switch (n)
            {
            case 0:
                Effect_CreateHomingOrb(data->orbs[0], (s16)gEwramData->playerState.player->posX.part16.integer,
                             (s16)gEwramData->playerState.player->posY.part16.integer - 0x10, 0, n);
                break;
            case 1:
                Effect_CreateHomingOrb(data->orbs[1], (s16)gEwramData->playerState.player->posX.part16.integer,
                             (s16)gEwramData->playerState.player->posY.part16.integer - 0x10, 0, n);
                break;
            case 2:
                Effect_CreateHomingOrb(data->orbs[2], (s16)gEwramData->playerState.player->posX.part16.integer,
                             (s16)gEwramData->playerState.player->posY.part16.integer - 0x10, 0, n);
                break;
            }
        }
        if (core->timer++ > 0xF9)
        {
            gEwramData->gameState.stateFlags &= ~1;
            gEwramData->startPauseEnabled = 1;
            core->state = 1;
            core->subState = 0;
            core->stage = 0;
            Player_LockAndCancelAllSouls();
        }
        break;
    }
}
