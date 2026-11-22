/* Burst of four sparks (frames 0x40/0x41, palette 6) within +-4 px of the
 * owner, random horizontal speed and an upward kick, gravity 0x2000;
 * updated by Effect_UpdateBallisticDebris. Stops at the first failed spawn. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void Effect_UpdateBallisticDebris(struct EwramData_EntityData *);
extern u8 sCommonObjectGfx[], sCommonObjectSpriteSheet[];

/**
 * @brief Spawn four ballistic debris pieces with random offsets and velocity
 * at the bone projectile.
 */
void BoneSoul_SpawnShatterDebris(struct EwramData_EntityData *owner)
{
    s32 i;
    for (i = 0; i <= 3; i++)
    {
        struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateBallisticDebris);
        s32 graphics, offset;
        if (entity == NULL)
            return;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, 6);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->animFrame = (RandomNumberGenerator() & 1) + 0x40;
        offset = (RandomNumberGenerator() & 7) - 4;
        entity->posX.part16.integer = offset + owner->posX.part16.integer;
        offset = (RandomNumberGenerator() & 7) - 4;
        entity->posY.part16.integer = offset + owner->posY.part16.integer;
        entity->velX.whole = (RandomNumberGenerator() & 0xFF) << 8;
        if (RandomNumberGenerator() & 1)
            entity->velX.whole = -entity->velX.whole;
        entity->velY.whole = -0x10000 - ((RandomNumberGenerator() & 0xFF) << 8);
        entity->accelY.whole = 0x2000;
        entity->scrollWithBg = 1;
    }
}
