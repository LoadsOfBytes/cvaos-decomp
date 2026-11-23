/* Six legs select delayed gait frames from their owner's phase. */
#include "code_08009A0.h"
#include "structs/ewram.h"
#include <string.h>
extern u8 sArachneLegsFrames[];
#define LEG(e) (*(s8 *)&e->ext.arachne.legIndex)
#define GAIT(e) (*(s8 *)&e->ext.arachne.gait)
#define FIXED_INTEGER(x) ((x)<0 ? -(-(x)>>16) : (x)>>16)
/**
 * @brief Leg sprite update: follow the owner (type 0x17) with its visibility,
 * palette and facing, picking the frame from the owner's gait (sArachneLegsFrames);
 * delete when the owner is gone.
 */
void ArachneLegs_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=ENTITY_OWNER(entity);
    u8 scratch[16]; /* Stack workspace reserved by the original routine. */
    u8 frames[36];
    s32 step;
    s32 gait;
    u8 current;
    s8 *ownerGait;
    s8 *index;
    memcpy(frames,sArachneLegsFrames,36);
    if(entity->state==0)
    {
        if(!owner || !owner->updateFunc || owner->enemyId!=23) goto fail;
        entity->hidden=owner->hidden;
        entity->palette=owner->palette;
        entity->flipX=owner->flipX;
        entity->posX.part16.integer=owner->posX.part16.integer;
        entity->posY.part16.integer=owner->posY.part16.integer;
        if(entity->flipX) entity->posX.part16.integer-=entity->velX.part16.integer;
        else entity->posX.part16.integer+=entity->velX.part16.integer;
        gait=GAIT(owner);
        ownerGait=(s8 *)&owner->ext.arachne.gait;
        switch(gait)
        {
        case 0:
            entity->animFrame=21;
            entity->subState=0;
            current=*ownerGait; GAIT(entity)=current;
            break;
        case 1:
            step=FIXED_INTEGER((s32)owner->ext.arachne.walkPhase);
            index=(s8 *)&entity->ext.arachne.legIndex;
            step-=(*index/2)*7;
            if(GAIT(entity)==2) entity->subState=1;
            if(entity->subState==0)
            {
                if(step>=0) { entity->subState=1; current=*ownerGait; GAIT(entity)=current; }
                else step=0;
            }
            else if(step<0) step+=36;
            entity->animFrame=frames[step];
            current=*ownerGait; GAIT(entity)=current;
            break;
        case 2:
            step=FIXED_INTEGER((s32)owner->ext.arachne.walkPhase);
            index=(s8 *)&entity->ext.arachne.legIndex;
            step-=(*index/2)*7;
            if(GAIT(entity)==1) entity->subState=1;
            if(entity->subState==0)
            {
                if((u32)(step-1)>21) step=0;
                else entity->subState=1;
            }
            if(step<0) step+=36;
            entity->animFrame=frames[step];
            current=*ownerGait; GAIT(entity)=current;
            break;
        case 3:
            step=FIXED_INTEGER((s32)owner->ext.arachne.walkPhase);
            index=(s8 *)&entity->ext.arachne.legIndex;
            step-=(*index/2)*7;
            if(step<0)step+=36;
            entity->animFrame=frames[step];
            current=*ownerGait; GAIT(entity)=current;
            break;
        }
        index=(s8 *)&entity->ext.arachne.legIndex;
        if(*index&1) entity->animFrame+=4;
        if(owner->state==2) { entity->state=1; entity->subState=0; entity->timer=90; }
        return;
fail:
        EntityDelete(entity);
    }
    else
    {
        if(entity->subState==0) { entity->subState++; entity->scrollWithBg=1; }
        entity->palette=owner->palette;
        if(LEG(entity)==2) { if(entity->timer&12) entity->animFrame=22; else entity->animFrame=23; }
        else if(LEG(entity)==4) { if(!(entity->timer&24)) entity->animFrame=22; else entity->animFrame=21; }
        else if(LEG(entity)==0) entity->animFrame=23;
        if((s8)--entity->timer<=0) entity->deleteRequested=1;
    }
}

