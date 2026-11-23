/* Ease two motion parameters back to rest and leave the reaction state. */
#include "structs/ewram.h"
/**
 * @brief Recoil state (state 3): pull the neck in for 30 frames, then back to
 * state 0.
 */
void WhiteDragon_UpdateRecoil(struct EwramData_EntityData *entity)
{
    u16 old=(u16)entity->velY.part16.integer;
    if(entity->velY.part16.integer>48)
        entity->velY.part16.integer=old-2;
    { u16 angle=(u16)entity->ext.whiteDragon.sway;
    if(entity->ext.whiteDragon.sway<=255)
        entity->ext.whiteDragon.sway=angle+2;
    }
    if(entity->timer++>29)
    {
        entity->state=0;
        entity->subState=0;
        entity->timer=0;
        entity->animFrame=0;
    }
}
