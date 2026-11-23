/* Kicker Skeleton breaks into seven sprite fragments (frames 26..32). */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"

extern void Effect_UpdateSpriteDebris(struct EwramData_EntityData *);
extern u8 sKickerSkeletonGfx[], sKickerSkeletonSpriteSheet[];

/**
 * @brief Spawn seven sprite fragments (frames 26 through 32) with random
 * velocities and stored attacker-relative horizontal direction. Return
 * nonzero immediately so the dispatcher removes the enemy.
 */
s32 KickerSkeleton_UpdateDeath(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *fragment;
    s32 frame;
    s32 graphics;
    PlaySong(0x6B);
    for (frame = 0x1A; frame <= 0x20; frame++)
    {
        fragment = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateSpriteDebris);
        if (fragment == NULL) continue;
        fragment->posX.whole = entity->posX.whole;
        fragment->posY.whole = entity->posY.whole;
        fragment->velX.whole = entity->ext.kickerSkeleton.hitSide * (((RandomNumberGenerator() & 7) << 14) + 0x8000);
        fragment->velY.whole = (((RandomNumberGenerator() & 0xF) - 8) << 12) - 0x28000;
        fragment->accelY.whole = 0x2000;
        fragment->spriteLayer = entity->spriteLayer;
        graphics = ObjGfx_Find(sKickerSkeletonGfx);
        Sprite_Attach(fragment, sKickerSkeletonSpriteSheet, graphics, entity->basePalette);
        fragment->flipX = entity->flipX;
        fragment->drawFunc = (u32 *)Sprite_DrawEntity;
        fragment->scrollWithBg = 1;
        fragment->timer = (RandomNumberGenerator() & 0xF) + 0x28;
        fragment->animFrame = frame;
    }
    return 1;
}
