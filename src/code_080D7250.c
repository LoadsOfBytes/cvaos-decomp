/* Chaos_SpawnDefeatEffect. */
#include "gba.h"
#include "code_08009A0.h"
#include "code_08001194.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void ChaosDefeatEffect_Update(struct EwramData_EntityData *entity);
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
 * @brief Create the Chaos defeat controller at the supplied fixed-point
 * position and install temporary tiles through the existing inline
 * copy. Called by the core defeat scene.
 */
void Chaos_SpawnDefeatEffect(s32 x, s32 y)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)ChaosDefeatEffect_Update);

    if (child == NULL)
        return;
    BossDefeat_LoadEffectTilesOnce();
    child->posX.whole = x;
    child->posY.whole = y;
    child->scrollWithBg = 1;
    child->roomActive = 1;
}
