/* Dying Werejaguar emits shrinking particles and flashes its palette. */
#include "code_08009A0.h"
#include "structs/ewram.h"
extern s32 Entity_GetVisibilityPattern(s32);
extern void Werejaguar_SpawnDeathEffect(s32,s32,s32,s32);
/**
 * @brief Death state (state 2, also used by the Werewolf): spiral effects every 4
 * frames (Werejaguar_SpawnDeathEffect), flash and request deletion when the
 * timer ends.
 */
void Werejaguar_UpdateDeath(struct EwramData_EntityData *entity)
{
    if(!(entity->timer&3)) Werejaguar_SpawnDeathEffect(entity->posX.whole,entity->posY.whole-0x40000,entity->spriteLayer,entity->timer*0x666);
    if(Entity_GetVisibilityPattern(entity->timer)) entity->palette=5;
    else entity->palette=entity->basePalette;
    if((s8)--entity->timer<=0) entity->deleteRequested=1;
}
