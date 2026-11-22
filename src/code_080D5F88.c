/* Graham_SpawnBreakupEffect. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void GrahamBreakup_Update(struct EwramData_EntityData *entity);
extern void BossDefeat_LoadEffectTiles(void);

/**
 * @brief Create the breakup controller at the supplied fixed-point
 * position and begin palette fades plus the existing display effect
 * setup. Do nothing further if controller allocation fails.
 */
void Graham_SpawnBreakupEffect(s32 x, s32 y)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)GrahamBreakup_Update);

    if (child != NULL)
    {
        BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x20);
        ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x4000, 0x10000, 0x10);
        BossDefeat_LoadEffectTiles();
        child->posX.whole = x;
        child->posY.whole = y;
        child->scrollWithBg = 1;
        child->roomActive = 1;
    }
}

