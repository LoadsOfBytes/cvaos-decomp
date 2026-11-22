/* ZombieSoldier_UpdateAttack: enemy attack states (throws ZombieSoldier_ThrowGrenade); matched. */
/* Throw a projectile toward the player's horizontal position, then return
 * to the idle state when the animation ends. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern void ZombieSoldier_ThrowGrenade(struct EwramData_EntityData *owner, s32 x, s32 y, s32 velX, s32 velY);

/**
 * @brief Attack state machine (state 1): melee swing (sound 0x85) or, from
 * sub-state 10, throw a grenade (ZombieSoldier_ThrowGrenade); then back to
 * the walk state.
 */
void ZombieSoldier_UpdateAttack(struct EwramData_EntityData *entity)
{
    s32 ky = -0x250000;

    switch (entity->subState)
    {
        case 0:
            entity->subState = 1;
            break;
        case 1:
            if (*(u16 *)&entity->animStep == 2)
                PlaySong(0x85);
            if (entity->animDone)
            {
                Anim_StartIndexed(entity, 0, 0, 1);
                entity->state = 0;
                entity->subState = 0;
                entity->velX.whole = 0;
                entity->velY.whole = 0x10000;
                entity->accelY.whole = 0x2800;
            }
            break;
        case 10:
            entity->flipX = Entity_IsTargetRight(entity);
            if (*(u16 *)&entity->animStep == 10)
            {
                s32 offset;
                s32 vx;

                offset = entity->flipX ? 0x100000 : -0x100000;
                vx = (Target_GetX(entity) - (entity->posX.whole + offset)) / 44;
                if (entity->flipX)
                {
                    if (vx > 0x13FFF)
                        vx = 0x14000;
                }
                else if (vx <= -0x14000)
                {
                    vx = -0x14000;
                }
                ZombieSoldier_ThrowGrenade(entity, offset, ky, vx, -0x20000);
                entity->subState++;
            }
            break;
        case 11:
            if (entity->animDone)
            {
                Anim_StartIndexed(entity, 0, 0, 1);
                entity->state = 0;
                entity->subState = 0;
                entity->velX.whole = 0;
                entity->velY.whole = 0x10000;
                entity->accelY.whole = 0x2800;
            }
            break;
    }
}
