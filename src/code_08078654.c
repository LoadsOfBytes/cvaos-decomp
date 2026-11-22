/* Falling fragments blink, emit periodic particles and expire. */
#include "code_08009A0.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern s32 Entity_GetVisibilityPattern(s32);
extern void Effect_SpawnAt(s32,s32,s32);
/**
 * @brief Death piece update: fall, blink, spawn an effect every 8 frames and
 * delete when the timer ends.
 */
void QuezlcoatlPiece_Update(struct EwramData_EntityData *entity)
{
    EntityAccelerateAndMove(entity,0x70000);
    entity->hidden=Entity_GetVisibilityPattern(entity->timer);
    if(!(entity->timer&7))
        Effect_SpawnAt(entity->posX.whole,entity->posY.whole,entity->spriteLayer-1);
    if((s8)entity->timer--<=0) EntityDelete(entity);
}
