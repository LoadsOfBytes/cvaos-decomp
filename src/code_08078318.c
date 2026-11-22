/* Follow delayed head displacements, then detach and fall when it dies. */
#include "code_08009A0.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern s32 Enemy_IsStunnedNoFlash(struct EwramData_EntityData *);
extern s32 Entity_GetVisibilityPattern(s32);
extern void Hit_CheckWithPlayer(void *,struct EwramData_EntityData *);
#define SEGMENT(e) (*(s8 *)&(e)->unk_500.part8.unk_500)
/**
 * @brief Body segment update: follow the owner (type 0x1A) through its position
 * history with its visibility and palette; delete when the owner is gone.
 */
void QuezlcoatlSegment_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=entity->unk_4F8.unk_4F8_entity;
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
    s32 bounds[4];
    if(entity->state==0)
    {
        s8 *history;
        if(!owner || !owner->updateFunc || owner->unk_518.part8.enemyId!=26) goto fail;
        history=(s8 *)owner->unk_504.whole;
        entity->hidden=owner->hidden;
        entity->palette=owner->palette;
        entity->timer++;
        if(!Enemy_IsStunnedNoFlash(owner))
        {
            entity->velX.part16.integer=history[SEGMENT(entity)*8+8]+(entity->unk_4FC.part16.unk_4FC-camera->xPos.part16.integer);
            entity->velY.part16.integer=history[SEGMENT(entity)*8+9]+(entity->unk_4FC.part16.unk_4FE[0]-camera->yPos.part16.integer);
            entity->posX.part16.integer+=entity->velX.part16.integer;
            entity->posY.part16.integer+=entity->velY.part16.integer;
            entity->unk_4FC.part16.unk_4FC=camera->xPos.part16.integer;
            entity->unk_4FC.part16.unk_4FE[0]=camera->yPos.part16.integer;
            if(history[SEGMENT(entity)*8]>0) entity->flipX=1;
            else if(history[SEGMENT(entity)*8]<0) entity->flipX=0;
            if(SEGMENT(entity)<=5)
            {
                if(entity->timer&8) entity->animFrame=4;
                else entity->animFrame=3;
            }
        }
        else
        {
            entity->posX.part16.integer+=entity->unk_4FC.part16.unk_4FC-camera->xPos.part16.integer;
            entity->posY.part16.integer+=entity->unk_4FC.part16.unk_4FE[0]-camera->yPos.part16.integer;
            entity->unk_4FC.part16.unk_4FC=camera->xPos.part16.integer;
            entity->unk_4FC.part16.unk_4FE[0]=camera->yPos.part16.integer;
        }
        if(!(entity->timer&7) && !gEwramData->gameState.bestiaryPreview) Hit_CheckWithPlayer(bounds,entity);
        if(owner->state==2)
        {
            entity->state=1;
            entity->subState=0;
            entity->accelX.whole=0;
            entity->accelY.whole=0x2000;
            entity->timer=RandomNumberGenerator()%4+60;
            entity->velX.part16.integer=history[SEGMENT(entity)*8+8];
            entity->velY.part16.integer=history[SEGMENT(entity)*8+9];
        }
        return;
fail:
        EntityDelete(entity);
    }
    else
    {
        if(entity->subState==0) { entity->subState++; entity->scrollWithBg=1; }
        EntityAccelerateAndMove(entity,0x70000);
        entity->hidden=Entity_GetVisibilityPattern(entity->timer);
        if((s8)--entity->timer<=0) entity->deleteRequested=1;
    }
}

