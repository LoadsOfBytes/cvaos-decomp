/* Alternate attack repeats a projectile pattern with an alternating selector. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern void Graham_SpawnSpiralProjectiles(struct EwramData_EntityData *,s32);
/**
 * @brief Alternate attack state (0xE0): face the target and periodically
 * spawn the spiral pattern with alternating direction.
 */
void GrahamFirstForm_UpdateAlternateAttack(struct EwramData_EntityData *entity)
{
    s32 state=entity->subState;
    switch(state)
    {
        case 0:
            if(entity->animScriptIndex!=1)
                Anim_StartIndexed(entity,1,0,1);
            entity->subState++;
            entity->stage=state;
            entity->timer=state;
            break;
        case 1:
            Entity_FaceTarget(entity);
            if(--entity->timer==255)
            {
                if((state &= entity->stage) && !(RandomNumberGenerator() & 3))
                    PlaySong(0x1D9);
                else
                    PlaySong(0x10C);
                Graham_SpawnSpiralProjectiles(entity,entity->stage & 1);
                entity->timer=160;
                entity->stage++;
            }
            break;
    }
}
