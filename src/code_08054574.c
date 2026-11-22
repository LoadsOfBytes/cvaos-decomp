/* Object 0x1C palette transition, returning one when the transition is done. */
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 sSaveWarpRoomPalette[], sSavePointPalette[];
/**
 * @brief Unreferenced fade step next to the save point code: fade the sprite in
 * (state 0, loading save point palette row 5) or out (state 1, restoring
 * the shared row) in 0x2000 steps of unk_4F8; return 1 when finished.
 */
s32 SavePoint_StepPromptFade(struct EwramData_EntityData *entity)
{
    s32 done = 0;
    switch (entity->state)
    {
    case 0:
        switch (entity->subState)
        {
        case 0:
            entity->hidden = 0;
            entity->unk_4F8.whole = 0;
            ObjPalette_LoadRange(sSaveWarpRoomPalette, 5, 1, 14);
            goto advance;
        case 1:
            entity->unk_4F8.whole += 0x2000;
            if ((s32)entity->unk_4F8.whole <= 0xFFFF) goto end;
            goto full;
        case 2:
            done = 1;
            break;
        }
        break;
    case 1:
        switch (entity->subState)
        {
        case 0:
            entity->hidden = 0;
        full:
            entity->unk_4F8.whole = 0x10000;
        advance:
            entity->subState++;
            break;
        case 1:
            entity->unk_4F8.whole -= 0x2000;
            if ((s32)entity->unk_4F8.whole <= 0)
            {
                entity->unk_4F8.whole = 0;
                entity->subState++;
                ObjPalette_LoadRange(sSavePointPalette, 14, 1, 14);
            }
            break;
        case 2:
            entity->hidden = 1;
            done = 1;
            break;
        }
        break;
    }
end:
    return done;
}
