#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_080211F0.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

struct WarpLightPaletteStep
{
    u16 sourcePalette;
    u16 unk_2;
};

extern const struct WarpLightPaletteStep sWarpLightPaletteSteps[12];
extern u8 sWarpRoomPalette[];

static inline s32 Rem(s32 a, s32 b) { return a % b; }
#define FRAME(e) (*(s8 *)&(e)->unk_4F4.part8.unk_4F6)

/**
 * @brief While the idle player stands on the warp spot, cycle the light
 * palette through sWarpLightPaletteSteps; restore it when the player leaves.
 */
void WarpRoom_UpdateSideLight(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *other = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    struct WarpLightPaletteStep frames[12];
    s32 found;
    s32 m;

    memcpy(frames, sWarpLightPaletteSteps, sizeof(frames));
    switch (entity->state)
    {
    case 0:
        if (Entity_IsInRect(gEwramData->playerState.player, 0x55, 0x7F, 0x49, 2) == 0)
            break;
        if (Player_IsBusy() != 0)
            break;
        FRAME(entity) = 0;
        entity->state++;
        break;
    case 1:
        found = Entity_IsInRect(gEwramData->playerState.player, 0x55, 0x7F, 0x49, 2);
        if (found == 0 && other->state <= 2)
        {
            ObjPalette_LoadRange(sWarpRoomPalette, 4, 1, 0xB);
            entity->state = found;
            break;
        }
        ObjPalette_LoadRange(sWarpRoomPalette, frames[FRAME(entity)].sourcePalette, 1, 0xB);
        m = entity->timer++;
        if ((m = Rem(m, 8)) == 0)
        {
            if (++FRAME(entity) > 11)
                FRAME(entity) = m;
        }
        break;
    }
}
