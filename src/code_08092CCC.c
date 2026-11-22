#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
extern s32 Entity_MoveWithPointCollision(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Effect_SpawnFadingParticle(s32, s32, s32, s32, s32, s32);
extern void Effect_SpawnConfiguredParticle(s32, s32, s32, s32, s32, s32, s32, s32);
extern void Effect_SpawnParticleAnim0(s32, s32, s32, s32, s32);

struct HitRect8 {
    s32 x:8;
    s32 y:8;
    u32 w:8;
    u32 h:8;
};

/**
 * @brief 92CCC | Falling spark bomb: state 0 bounces with Entity_MoveWithPointCollision
 * (velY / 3, ceiling -0x40000), reversing velY on bit 2 (song 0x11B on the
 * first bounce) and exploding (state 2, song 0x76) on the second bounce or on
 * landing; deleted off screen. States 1/2 emit random sparks (Effect_SpawnFadingParticle,
 * every odd frame Effect_SpawnParticleAnim0, a burst Effect_SpawnConfiguredParticle on frame 3); state 2
 * also grows a centred hit box (corner -2 - frame / 2) for 15 frames and
 * dies after 0x17 frames.
 */
void ZombieSoldierGrenade_Update(struct EwramData_EntityData *entity)
{
    u32 buffer[4];
    struct HitRect8 rect = { -12, -12, 24, 24 };
    s32 res;
    s32 r, a, b;
    s32 v;
    

    if (entity->state == 0)
    {
        res = Entity_MoveWithPointCollision(entity, entity->velY.whole / 3, -0x40000);
        if (res & 4)
        {
            entity->velY.whole = -entity->velY.whole;
            if (++entity->timer > 1)
                goto land;
            PlaySong(0x11B);
        }
        if (res & 8)
        {
        land:
            entity->state = 2;
            PlaySong(0x76);
            return;
        }
        if (EntityIsOutsideScreen(entity, 2))
            goto delete;
        goto hit;
    }
    else if (entity->state == 1)
    {
        entity->hidden = 1;
        entity->timer++;
        r = RandomNumberGenerator() & 0xFFFF;
        a = Sin(r + 0x4000);
        b = Sin(r);
        a = FixedMultiply(a, 0x10000);
        b = FixedMultiply(b, 0x10000);
        Effect_SpawnFadingParticle(entity->posX.whole, entity->posY.whole, a, b, entity->spriteLayer, 1);
        if (entity->timer == 3)
            Effect_SpawnConfiguredParticle(entity->posX.whole, entity->posY.whole, 0, 0, entity->spriteLayer, 1, 7, 5);
        if (entity->timer & 1)
        {
            r = RandomNumberGenerator() & 0xFFFF;
            a = Sin(r + 0x4000);
            b = Sin(r);
            a = FixedMultiply(a, 0x10000);
            b = FixedMultiply(b, 0x10000);
            Effect_SpawnParticleAnim0(entity->posX.whole, entity->posY.whole, a, b, entity->spriteLayer);
        }
        if (entity->timer <= 6)
            return;
        goto delete;
    }
    else if (entity->state == 2)
    {
        entity->hidden = 1;
        entity->timer++;
        if (entity->timer <= 8)
        {
            r = RandomNumberGenerator() & 0xFFFF;
            a = Sin(r + 0x4000);
            b = Sin(r);
            a = FixedMultiply(a, 0x10000);
            b = FixedMultiply(b, 0x10000);
            Effect_SpawnFadingParticle(entity->posX.whole, entity->posY.whole, a, b, entity->spriteLayer, 1);
            if (entity->timer == 3)
                Effect_SpawnConfiguredParticle(entity->posX.whole, entity->posY.whole, 0, 0, entity->spriteLayer, 1, 7, 5);
            if (entity->timer & 1)
            {
                r = RandomNumberGenerator() & 0xFFFF;
                a = Sin(r + 0x4000);
                b = Sin(r);
                a = FixedMultiply(a, 0x10000);
                b = FixedMultiply(b, 0x10000);
                Effect_SpawnParticleAnim0(entity->posX.whole, entity->posY.whole, a, b, entity->spriteLayer);
            }
        }
        if (entity->timer <= 14)
        {
            rect.x = 0xFE - entity->timer / 2;
            rect.y = 0xFE - entity->timer / 2;
            rect.w = (rect.x < 0 ? -rect.x : rect.x) * 2;
            rect.h = (rect.y < 0 ? -rect.y : rect.y) * 2;
            HitBox_SetAttack(entity, &rect);
            entity->hit.flags |= 4;
        }
        if (entity->timer > 0x17)
        {
        delete:
            EntityDelete(entity);
            return;
        }
        if (entity->timer <= 0x12)
        {
        hit:
            Hit_CheckWithPlayer(buffer, entity);
        }
        else
            entity->hit.flags |= 2;
    }
}
