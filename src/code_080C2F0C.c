/* Visibility flashes alternate around movement, damage and death processing. */
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern void GhostDancer_UpdateBestiaryPreview(struct EwramData_EntityData *);
extern void GhostDancer_UpdateMovement(struct EwramData_EntityData *);
extern void GhostDancer_UpdateDeath(struct EwramData_EntityData *);
extern s32 Enemy_IsStunned(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *,struct EwramData_EntityData *);
#define MODE(e) ((e)->ext.raw.unk_4FC.part8.unk_4FC)
#define TIMER(e) ((e)->ext.raw.unk_4FC.part8.unk_4FD)
void EnemyGhostDancerUpdate(struct EwramData_EntityData *entity)
{
    s32 bounds[4];
    if(entity->state==5)
    {
        GhostDancer_UpdateBestiaryPreview(entity);
        return;
    }
    entity->hidden=EntityIsOutsideScreen(entity,4);
    if(entity->state!=3)
    {
        s32 mode=MODE(entity);
        switch(mode)
        {
            case 0:
                entity->objMode=0;
                break;
            case 1:
                if(mode &= TIMER(entity))
                    entity->objMode=0;
                else
                    entity->objMode=1;
                if((s8)TIMER(entity)++>32)
                {
                    MODE(entity)++;
                    TIMER(entity)=0;
                }
                break;
            case 2:
                entity->objMode=1;
                break;
            case 3:
                if(TIMER(entity) & 1)
                    entity->objMode=1;
                else
                    entity->objMode=0;
                if((s8)TIMER(entity)++>32)
                {
                    MODE(entity)=0;
                    TIMER(entity)=0;
                }
                break;
        }
    }
    if(Enemy_IsStunned(entity))
    {
        if(entity->state!=3)
            Hit_CheckWithPlayer(bounds,entity);
        return;
    }
    switch(entity->state)
    {
        case 1:
        case 2:
            GhostDancer_UpdateMovement(entity);
            break;
        case 3:
            GhostDancer_UpdateDeath(entity);
            break;
    }
    Entity_AnimateWithBoxes(entity);
    if(entity->state!=3)
        Hit_CheckWithPlayer(bounds,entity);
}
