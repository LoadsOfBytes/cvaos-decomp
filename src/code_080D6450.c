/* BossDefeatEffect_Spawn. */
#include "gba.h"
#include "code_08009A0.h"
#include "code_08001194.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void BossDefeatEffect_Update(struct EwramData_EntityData *entity);
extern u8 sCommonObjectGfx[];
/* Two 0x200-byte tile blocks copied into the slot of sCommonObjectGfx. */
extern u8 sDefeatEffectTiles[];

/* Same as BossDefeat_LoadEffectTiles. */
static inline void BossDefeat_LoadEffectTilesOnce(void)
{
    if (!(gEwramData->gameState.stateFlags & 8))
    {
        u8 *src = sDefeatEffectTiles;
        s32 tileOffset = ObjTiles_GetSlotTile((u8)ObjGfx_Find(sCommonObjectGfx)) * 32;

        DmaQueue_IndirectCopy(0x200, (u32 *)src, (u32 *)(VRAM_BASE + 0x10000 + tileOffset));
        src += 0x200;
        DmaQueue_IndirectCopy(0x200, (u32 *)src, (u32 *)(VRAM_BASE + 0x10400 + tileOffset));
        gEwramData->gameState.stateFlags |= 8;
    }
}

/**
 * @brief Create the shared boss-defeat sweep at the supplied fixed-point
 * position, start white palette fades unless flag 0x10 blocks them,
 * and install its temporary effect tiles through the inline copy.
 */
void BossDefeatEffect_Spawn(s32 x, s32 y)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)BossDefeatEffect_Update);

    if (child == NULL)
        return;
    if (!(gEwramData->gameState.stateFlags & 0x10))
    {
        BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x20);
        ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x4000, 0x10000, 0x10);
    }
    BossDefeat_LoadEffectTilesOnce();
    child->posX.whole = x;
    child->posY.whole = y;
    child->scrollWithBg = 1;
    child->roomActive = 1;
}
