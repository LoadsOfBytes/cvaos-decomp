/* The projectile's travel angle oscillates over a 32-frame cycle. */
#include "code_08009A0.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern void EntityMove(struct EwramData_EntityData *);
extern void EntitySetVelocityFromAngle(struct EwramData_EntityData *,s32,s32);
extern void Hit_CheckWithPlayer(void *,struct EwramData_EntityData *);
void HarpyWavyFeather_Update(struct EwramData_EntityData *entity)
{
    s32 bounds[4];
    s32 state=entity->state;
    switch(state)
    {
        case 0:
        {
            u8 timer=++entity->timer;
            if(timer<=15)
                entity->unk_4F8.whole+=0x400;
            else if(timer<=31)
            {
                entity->unk_4F8.whole-=0x400;
                if(timer>30) entity->timer=state;
            }
            EntitySetVelocityFromAngle(entity,entity->unk_4F8.whole,0x20000);
            EntityMove(entity);
            Hit_CheckWithPlayer(bounds,entity);
            if(!EntityIsOutsideScreen(entity,1)) break;
        }
        case 1:
            EntityDelete(entity);
            break;
        case 2:
            EntityDelete(entity);
            break;
    }
}
