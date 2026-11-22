/* Spinning projectile (CreakingSkullProjectile_Update): moves, leaves a trail every other
 * frame (Effect_SpawnParticleAnim11) in front of/behind its layer by angle; state 1
 * bursts into six random sparks and deletes itself. */
#include "code_08009A0.h"
#include "creaking_skull_entity.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Effect_SpawnFadingParticle(s32, s32, s32, s32, s32, s32);
extern void Effect_SpawnParticleAnim0(s32, s32, s32, s32, s32);
extern void Effect_SpawnParticleAnim11(s32 x, s32 y, s32 vx, s32 vy, s32 layer);
extern s32 FixedMultiply(s32, s32);
extern void EntityMove(struct EwramData_EntityData *);

/**
 * @brief Move the projectile with its animated trail and contact checks;
 * on hit state 1 emit six effects and remove. Delete when outside
 * the screen during normal motion.
 */
void CreakingSkullProjectile_Update(struct EwramData_EntityData *entity)
{
    u32 buffer[4];
    s32 vy, k;
    s32 r;
    s32 a, b;
    s32 i;

    switch (entity->state)
    {
    case 0:
        if (entity->animScriptIndex == 0 && entity->animDone)
            Anim_StartIndexed(entity, 1, 0, 1);
        EntityMove(entity);
        if (EntityIsOutsideScreen(entity, 1))
        {
            EntityDelete(entity);
            break;
        }
        Anim_Update(entity);
        Hit_CheckWithPlayer(buffer, entity);
        entity->timer++;
        if (entity->timer & 1)
            break;
        vy = Sin(CREAKING_SKULL_PROJECTILE_ORBIT(entity)) << 1;
        b = Sin(CREAKING_SKULL_PROJECTILE_ORBIT(entity)) << 1;
        b += 0x20000;
        if (entity->flipX)
        {
            a = -Sin(CREAKING_SKULL_PROJECTILE_ORBIT(entity) + 0x4000) * 4;
            a += 0xC0000;
        }
        else
        {
            a = Sin(CREAKING_SKULL_PROJECTILE_ORBIT(entity) + 0x4000) * 4;
            a -= 0xC0000;
        }
        r = (u16)entity->unk_4FC.part16.unk_4FC;
        if (r > 0x4000 && r < 0xC000)
            Effect_SpawnParticleAnim11(entity->posX.whole + a, entity->posY.whole + b, 0, vy, entity->spriteLayer + 1);
        else
            Effect_SpawnParticleAnim11(entity->posX.whole + a, entity->posY.whole + b, 0, vy, entity->spriteLayer - 1);
        CREAKING_SKULL_PROJECTILE_ORBIT(entity) += 0x2400;
        break;
    case 1:
        for (i = 0; i < 3; i++)
        {
            r = RandomNumberGenerator() & 0xFFFF;
            a = Sin(r + 0x4000);
            b = Sin(r);
            a = FixedMultiply(a, 0x10000);
            b = FixedMultiply(b, 0x10000);
            Effect_SpawnFadingParticle(entity->posX.whole, entity->posY.whole, a, b, entity->spriteLayer, 1);
            r = RandomNumberGenerator() & 0xFFFF;
            a = Sin(r + 0x4000);
            b = Sin(r);
            a = FixedMultiply(a, 0x10000);
            b = FixedMultiply(b, 0x10000);
            Effect_SpawnParticleAnim0(entity->posX.whole, entity->posY.whole, a, b, entity->spriteLayer);
        }
        EntityDelete(entity);
        break;
    }
}
