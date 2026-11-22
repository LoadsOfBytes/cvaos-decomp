/* A short spark converges on a saved world position while the camera moves. */
#include "code_08009A0.h"
#include "structs/ewram.h"
void WhiteDragonChargeParticle_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
    s32 dx,dy,remaining;
    entity->flipX=RandomNumberGenerator()%2;
    entity->flipY=RandomNumberGenerator()%2;
    dx=(((s16)entity->unk_4F8.part16.unk_4F8<<16)-(entity->posX.whole+camera->xPos.whole));
    remaining=8-entity->timer;
    dx/=remaining;
    dy=((entity->unk_4F8.part16.unk_4FA<<16)-(entity->posY.whole+camera->yPos.whole))/remaining;
    entity->posX.whole+=dx;
    entity->posY.whole+=dy;
    if(entity->timer++>8)
        EntityDelete(entity);
}
