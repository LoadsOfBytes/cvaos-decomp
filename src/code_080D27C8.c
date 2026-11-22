/* Wait for the attached entity to signal the transition to the next phase. */
#include "code_080D73B8.h"
#include "structs/ewram.h"
extern void Graham_CreateCombatBody(struct EwramData_EntityData *);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
/**
 * @brief Root state (3): wait for flag 4, create the later combat body,
 * then watch its defeat flag. Drop root rewards and enter defeat
 * scene state 0xF0 when that linked body is defeated.
 */
void Graham_UpdatePhaseMonitor(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child=&gEwramData->entityData[entity->unk_4F4.part8.unk_4F6];
    s32 state=entity->subState;
    switch(state)
    {
        case 0:
            if(entity->unk_4F8.part16.unk_4FA & 4)
            {
                PlaySong(0x1DD);
                Graham_CreateCombatBody(entity);
                entity->subState=1;
                entity->stage=state;
            }
            break;
        case 1:
            if(child->unk_518.part8.enemyId==entity->unk_518.part8.enemyId && child && (child->enemyFlags & 2))
            {
                Enemy_DropRewards(entity);
                entity->state=0xF0;
                entity->stage=0;
                entity->subState=0;
            }
            break;
    }
}
