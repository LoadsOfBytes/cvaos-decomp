/* BlueSoul_StopAll. */
#include "code_08009A0.h"
#include "code_08013960.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief 2DFF0 | For Soma (currentCharacter 0): delete all active blue
 * soul entities (releasing their affine slots as BlueSoul_DeleteEntity), then
 * Sound_StopTracked(0x9C), clear soulStateFlags bits 8-13 and the player's unk_4F4
 * bit 9, and free the blue soul graphics blueSoulGfx.
 */
void BlueSoul_StopAll(void)
{
    s8 i;

    if (gEwramData->playerStatus.currentCharacter != 0)
        return;
    for (i = 0; i < 8; i++)
    {
        struct EwramData_EntityData *soul = gEwramData->playerState.activeBlueSoulEntities[i];
        if (soul != NULL)
        {
            if (soul->affineIndex != 0 || soul->affine)
                ObjAffine_Free(soul->affineIndex);
            EntityDelete(soul);
            gEwramData->playerState.activeBlueSoulEntities[i] = NULL;
        }
    }
    Sound_StopTracked(0x9C);
    gEwramData->playerStatus.soulStateFlags &= ~0x3F00;
    gEwramData->playerState.player->unk_4F4.whole &= ~0x200;
    if (gEwramData->playerState.blueSoulGfx != -1)
    {
        ObjGfx_Unload(gEwramData->playerState.blueSoulGfx);
        gEwramData->playerState.blueSoulGfx = -1;
    }
}
