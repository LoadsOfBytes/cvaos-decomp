/* Belmont charged attack waits on animation and emits a projectile wave. */
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void Belmont_SpawnProjectileWave(struct EwramData_EntityData *);
/**
 * @brief Wave state (5): sequence charge/release animations and countdowns,
 * spawn the projectile wave, then recover to combat.
 */
void Belmont_UpdateWaveAttack(struct EwramData_EntityData *entity)
{
    s32 state=entity->subState;
    switch(state)
    {
        case 0:
            if(entity->stage==0)
            {
                Anim_StartIndexed(entity,7,1,0);
                entity->stage++;
            }
            if(entity->animFrame==30)
            {
                entity->subState++;
                entity->stage=state;
                entity->animFlags|=2;
            }
            break;
        case 1:
            if(entity->stage==0)
            {
                entity->timer=24;
                entity->stage++;
            }
            if(--entity->timer==0xFF)
            {
                entity->subState++;
                entity->stage=0;
            }
            break;
        case 2:
            if(entity->stage==0)
            {
                Anim_StartIndexed(entity,4,1,0);
                entity->stage++;
            }
            if(entity->animDone)
            {
                entity->subState++;
                entity->stage=0;
                Belmont_SpawnProjectileWave(entity);
            }
            break;
        case 3:
            if(entity->stage==0)
            {
                entity->timer=128;
                entity->stage++;
            }
            if(--entity->timer==0xFF)
            {
                entity->state=2;
                entity->stage=0;
                entity->subState=0;
            }
            break;
    }
}
