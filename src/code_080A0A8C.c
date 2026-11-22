#include <string.h>
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
extern s32 Entity_MoveWithCollisionAlt(struct EwramData_EntityData *, s32, s32, s32);
extern s32 Entity_GetHorizontalRoomEdge(struct EwramData_EntityData *, s32);
extern void Effect_SpawnParticleWithDamping(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 variant, s32 damping, s32 flags, s32 palette);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern const s16 sGladiatorOffsetsA[12];
extern const s16 sGladiatorOffsetsB[12];

/**
 * @brief A0A8C | Charging part: state 0 accelerates (accelX = +-0x1000)
 * along the floor until it hits a wall on the facing side (Entity_GetHorizontalRoomEdge),
 * with sound 0x7D every 32 frames, then crashes (animation 0xD, sound
 * 0x127); state 1 brakes and throws debris (Effect_SpawnParticleWithDamping) every 4th frame
 * from the s16 offsets sGladiatorOffsetsA / sGladiatorOffsetsB (indexed by the
 * animation frame) until the animation ends, then turns around.
 */
void Gladiator_UpdateCharge(struct EwramData_EntityData *entity)
{
    s16 offsA[12];
    s16 offsB[12];
    s32 state;
    s32 hit;
    s32 speed;
    s32 x;
    s32 vx;

    memcpy(offsA, sGladiatorOffsetsA, sizeof(offsA));
    memcpy(offsB, sGladiatorOffsetsB, sizeof(offsB));
    state = entity->subState;
    switch (state)
    {
    case 0:
        if (entity->flipX)
            entity->accelX.whole = 0x1000;
        else
            entity->accelX.whole = -0x1000;
        Entity_MoveWithCollisionAlt(entity, 0x20000, 0, 0x30000);
        hit = 0;
        if (entity->flipX)
        {
            if (Entity_GetHorizontalRoomEdge(entity, 0x18) == 2)
                hit = 1;
        }
        else if (Entity_GetHorizontalRoomEdge(entity, 0x18) == 1)
        {
            hit = 1;
        }
        if (!(entity->timer & 0x1F) && !EntityIsOutsideScreen(entity, 6))
            PlaySong(0x7D);
        entity->timer++;
        if (!hit)
            break;
        entity->subState++;
        Anim_StartIndexed(entity, 0xD, 0, 0);
        entity->timer = 0;
        if (!EntityIsOutsideScreen(entity, 6))
            PlaySong(0x127);
        break;
    case 1:
        EntityDampVelocityX(entity, 10);
        speed = 0x20000;
        Entity_MoveWithCollisionAlt(entity, speed, 0, 0x30000);
        if (entity->animDone)
        {
            entity->subState = 0;
            entity->timer = 0;
            Anim_StartIndexed(entity, 0xC, 0, 1);
            entity->flipX ^= 1;
            if (entity->flipX)
            {
                x = -0x300000;
                vx = -(RandomNumberGenerator() & 0x7FFF) - 0x20000;
            }
            else
            {
                x = 0x300000;
                vx = (RandomNumberGenerator() & 0x7FFF) + speed;
            }
            Effect_SpawnParticleWithDamping(entity->posX.whole + x, entity->posY.whole, vx, -0x8000 - (RandomNumberGenerator() & 0x7FFF), entity->spriteLayer - 2, 0, 0x10, 0xF, 6);
            break;
        }
        if (!(entity->timer++ & 3))
        {
            if (entity->flipX)
            {
                x = -offsA[entity->animStep] << 16;
                vx = (RandomNumberGenerator() & 0x7FFF) + speed;
            }
            else
            {
                x = offsA[entity->animStep] << 16;
                vx = -(RandomNumberGenerator() & 0x7FFF) - 0x20000;
            }
            Effect_SpawnParticleWithDamping(entity->posX.whole + x, entity->posY.whole, vx, -0x8000 - (RandomNumberGenerator() & 0x7FFF), entity->spriteLayer - 2, 0, 0x10, 0xF, 6);
        }
        if ((entity->timer & 3) == 2)
        {
            if (entity->flipX)
            {
                x = -offsB[entity->animStep] << 16;
                vx = (RandomNumberGenerator() & 0x7FFF) + 0x20000;
            }
            else
            {
                x = offsB[entity->animStep] << 16;
                vx = -0x20000 - (RandomNumberGenerator() & 0x7FFF);
            }
            Effect_SpawnParticleWithDamping(entity->posX.whole + x, entity->posY.whole, vx, -0x8000 - (RandomNumberGenerator() & 0x7FFF), entity->spriteLayer - 2, 0, 0x10, 0xF, 6);
        }
        break;
    }
}
