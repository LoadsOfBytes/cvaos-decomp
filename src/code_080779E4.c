/* A short spark converges on a saved world position while the camera moves. */
#include "code_08009A0.h"
#include "white_dragon_entity.h"
#include "structs/ewram.h"
void WhiteDragonChargeParticle_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
    s32 dx,dy,remaining;
    entity->flipX=RandomNumberGenerator()%2;
    entity->flipY=RandomNumberGenerator()%2;
    dx=(((s16)WHITE_DRAGON_CHARGE_TARGET_X(entity)<<16)-(entity->posX.whole+camera->xPos.whole));
    remaining=8-entity->timer;
    dx/=remaining;
    dy=((WHITE_DRAGON_CHARGE_TARGET_Y(entity)<<16)-(entity->posY.whole+camera->yPos.whole))/remaining;
    entity->posX.whole+=dx;
    entity->posY.whole+=dy;
    if(entity->timer++>8)
        EntityDelete(entity);
}
