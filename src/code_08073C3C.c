/* Projectile: wait 16 frames, then (state 1) fall until off screen; in
 * state 2 it falls emitting two random sparks per frame (Effect_SpawnFadingParticle /
 * Effect_SpawnParticleAnim0) and vanishes after a few frames. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Effect_SpawnFadingParticle(s32, s32, s32, s32, s32, s32);
extern void Effect_SpawnParticleAnim0(s32, s32, s32, s32, s32);

/**
 * @brief Breath shot update: wait 16 frames hidden, then fly (animation 6) until
 * off screen; state 2 bursts into particles and vanishes.
 */
void ManticoreShot_Update(struct EwramData_EntityData *entity)
{
    u32 buffer[4];
    s32 vx, vy;

    switch (entity->state)
    {
    case 0:
        if (entity->timer++ > 15)
        {
            entity->state++;
            Anim_StartIndexed(entity, 6, 0, 1);
            entity->hit.flags &= ~4;
            entity->hit.flags &= ~2;
            entity->hidden = 0;
        }
        break;
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        if (!EntityIsOutsideScreen(entity, 1))
            break;
        goto delete;
    case 2:
        EntityAccelerateAndMove(entity, 0x80000);
        vx = entity->flipX ? 0x30000 : -0x30000;
        vx += RandomNumberGenerator() % 0x30000 - 0x18000;
        vy = RandomNumberGenerator() % 0x30000 - 0x18000;
        Effect_SpawnFadingParticle(entity->posX.whole, entity->posY.whole, vx, vy, 10, 1);
        vx = entity->flipX ? 0x30000 : -0x30000;
        vx += RandomNumberGenerator() % 0x30000 - 0x18000;
        vy = RandomNumberGenerator() % 0x30000 - 0x18000;
        Effect_SpawnParticleAnim0(entity->posX.whole, entity->posY.whole, vx, vy, 10);
        if (entity->timer++ > 1)
            entity->hidden = 1;
        if (entity->timer > 5)
        {
        delete:
            EntityDelete(entity);
            return;
        }
        break;
    }
    if (entity->state != 2)
        Hit_CheckWithPlayer(buffer, entity);
    Anim_Update(entity);
}
