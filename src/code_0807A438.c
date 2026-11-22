/* Arachne's death flashes the palette, sheds particles and slowly sinks. */
#include "code_08009A0.h"
#include "structs/ewram.h"
extern s32 Entity_GetVisibilityPattern(s32);
extern void Effect_SpawnRandom(s32,s32,s32,s32,s32);
/**
 * @brief Death state (state 2): random effects above it every 4 frames, flash,
 * sink and request deletion when the timer ends.
 */
void Arachne_UpdateDeath(struct EwramData_EntityData *entity)
{
    if(!(entity->timer&3)) Effect_SpawnRandom(entity->posX.whole,entity->posY.whole-0x180000,0x100000,0x100000,entity->spriteLayer-1);
    if(Entity_GetVisibilityPattern(entity->timer)) entity->palette=5;
    else entity->palette=entity->unk_510.part8.basePalette;
    if((u8)(entity->timer-9)<=80) entity->posY.whole+=0x2000;
    if((s8)--entity->timer<=0) entity->deleteRequested=1;
}
