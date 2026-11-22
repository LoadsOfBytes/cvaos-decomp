/* Dispatch Graham's scene, combat, defeat and special presentation states. */
#include "structs/ewram.h"
extern void Graham_UpdateIntro(struct EwramData_EntityData *);
extern void Graham_UpdateState2(struct EwramData_EntityData *);
extern void Graham_UpdateDefeatScene(struct EwramData_EntityData *);
extern void Graham_UpdateBestiaryPreview(struct EwramData_EntityData *);
extern void Graham_UpdatePhaseMonitor(struct EwramData_EntityData *);
void EnemyGrahamUpdate(struct EwramData_EntityData *entity)
{
    s32 state=entity->state;
    if(state==1)
        Graham_UpdateIntro(entity);
    else if(state==2)
        Graham_UpdateState2(entity);
    else if(state==0xF0)
        Graham_UpdateDefeatScene(entity);
    else if(state==0xD0)
        Graham_UpdateBestiaryPreview(entity);
    /* Preserve the separate state read in the original dispatch. */
    else if(*(volatile u8 *)&entity->state==3)
        Graham_UpdatePhaseMonitor(entity);
}
