/* Delayed wave projectile: rises, alternates two frames, then expires. */
#include "code_08009A0.h"
#include "structs/ewram.h"
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
/**
 * @brief Wait for the initial delay, rise with gravity and alternate
 * frames 7/8, then remove at the existing downward-velocity threshold.
 * Keep player hit checks throughout the state machine.
 */
void BelmontWaveProjectile_Update(struct EwramData_EntityData *entity)
{
    s32 state=entity->state;
    switch(state)
    {
        case 0:
            if(--entity->timer==0xFF)
            {
                entity->state++;
                entity->stage=state;
                entity->subState=state;
            }
            break;
        case 1:
            entity->hidden=0;
            entity->state++;
            entity->stage=0;
            entity->subState=0;
            entity->velY.whole=-0x1F000;
            entity->accelY.whole=0x1000;
            entity->animFrame=7;
            break;
        case 2:
            if((entity->timer++ & 1)!=0)
            {
                if(entity->animFrame==7)
                    entity->animFrame=8;
                else
                    entity->animFrame=7;
            }
            entity->posY.whole+=entity->velY.whole;
            entity->velY.whole+=entity->accelY.whole;
            if(entity->velY.whole==0x20000)
            {
                EntityDelete(entity);
                return;
            }
            break;
    }
    entity->hit.flags|=4;
    Enemy_CheckHitPlayer(entity);
}
