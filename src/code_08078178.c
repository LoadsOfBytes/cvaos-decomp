/* Demo flight records a pair of byte displacements for each frame. */
#include "code_08009A0.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
/**
 * @brief Bestiary preview state (state 3): swing between x 24 and 119 px with the
 * same wave motion, recording the position history for the segments.
 */
void Quezlcoatl_UpdatePreview(struct EwramData_EntityData *entity)
{
    u8 (*history)[2]=(u8 (*)[2])entity->ext.quezlcoatl.history;
    s32 x,y,i;
    switch(entity->subState)
    {
        case 0: entity->subState=1;
        case 1:
            x=(s16)entity->posX.part16.integer;
            if(x<=24) entity->flipX=1;
            else if(x>119) entity->flipX=0;
            if(entity->flipX) entity->velX.whole=0x8000;
            else entity->velX.whole=-0x8000;
            entity->velY.whole=-0x20000;
            entity->accelY.whole=0x1800;
            entity->subState++;
            entity->timer=0;
            for(i=119;i>0;i--)
            {
                history[i][0]=history[i-1][0];
                history[i][1]=history[i-1][1];
            }
            history[0][0]=0;
            history[0][1]=0;
            break;
        case 2:
            x=(s16)entity->posX.part16.integer;
            y=entity->posY.part16.integer;
            EntityAccelerateAndMove(entity,0x40000);
            x=(s16)entity->posX.part16.integer-x;
            y=entity->posY.part16.integer-y;
            for(i=119;i>0;i--)
            {
                history[i][0]=history[i-1][0];
                history[i][1]=history[i-1][1];
            }
            history[0][0]=x;
            history[0][1]=y;
            if(entity->posY.part16.integer<=70)
            {
                if((u32)(entity->velY.whole+0xFFFF)<=0x1FFFE) entity->animFrame=13;
                else entity->animFrame=12;
            }
            else { entity->subState=1; entity->animFrame=12; }
            break;
    }
}
