/* Emit a fading fragment every fourth tick and finish the source effect. */
#include <string.h>
#include "code_08009A0.h"
#include "structs/ewram.h"
extern void RisingEffect_Update(struct EwramData_EntityData *);
extern const u8 sEntityVisibilityPattern[];
static inline s32 RandomOffset(s32 range)
{
    return RandomNumberGenerator() % range - range / 2;
}
static inline void SpawnFragment(s32 x, s32 y, s32 frame)
{
    struct EwramData_EntityData *fragment = EntityCreateInRange(0x49, 0xCF, (u32 *)RisingEffect_Update);
    if (fragment)
    {
        fragment->posX.whole = x;
        fragment->posY.whole = y;
        fragment->flipX = RandomNumberGenerator() % 2;
        fragment->spriteLayer = frame;
    }
}
static inline s32 FlashValue(s32 index)
{
    s8 values[40];
    s16 bound = index;
    /* Same inline as in code_0806BE74.c: the loop note of the do-while stops
       regmove from moving the test onto the copy. */
    do { memcpy(values, sEntityVisibilityPattern, sizeof(values)); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
    if (index > 39) return 0;
    return values[bound];
}
/**
 * @brief Emit a random fragment every fourth timer tick, apply the visibility table and request deletion when the signed byte timer expires.
 */
void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *entity)
{
    if ((entity->timer & 3) == 0)
    {
        s32 frame = entity->spriteLayer;
        s32 x = entity->posX.whole;
        s32 y = entity->posY.whole - 0x60000;
        s32 fragmentFrame = frame - 1;
        s32 dx = RandomOffset(0x100000);
        s32 dy = RandomOffset(0xC0000);
        SpawnFragment(x + dx, y + dy, fragmentFrame);
    }
    {
        entity->hidden = FlashValue(entity->timer);
    }
    if ((s8)--entity->timer <= 0)
        entity->deleteRequested = 1;
}
