/* Horizontal and vertical sine motion, alternating frames at turning points. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *);
#define PHASE(e) ((e)->unk_4F8.part16.unk_4F8)
/**
 * @brief Alive states (1/2): move with horizontal and vertical sine phases,
 * swap animations at horizontal turning points and advance visibility
 * modes at half-period boundaries.
 */
void GhostDancer_UpdateMovement(struct EwramData_EntityData *entity)
{
    s32 state=entity->subState;
    switch(state)
    {
        case 0:
            Anim_StartIndexed(entity,0,0,1);
            entity->velX.whole=state;
            entity->velY.whole=state;
            if((s16)Entity_GetTargetDistanceX(entity)>0)
                entity->flipX=0;
            else
                entity->flipX=1;
            if(!entity->flipX)
                entity->velX.whole=-entity->velX.whole;
            PHASE(entity)=0;
            entity->unk_4F8.part8.unk_4FA=0;
            entity->timer=0;
            entity->subState++;
            break;
        case 1:
            entity->velX.whole=(u32)Sin(PHASE(entity)+0x4000)<<1;
            entity->posX.whole+=entity->velX.whole;
            entity->posY.whole+=entity->velY.whole+Sin(PHASE(entity)*4);
            PHASE(entity)+=128;
            if(!PHASE(entity))
                entity->unk_4F8.part8.unk_4FA++;
            if(!entity->velX.whole)
                Anim_StartIndexed(entity,(u16)(1-entity->animScriptIndex),0,1);
            { s32 phase=PHASE(entity);
            s32 period=0x8000;
            if(phase%period==0)
            { u32 mode=(u8)entity->unk_4FC.part8.unk_4FC;
            if(!(state &= mode))
            {
                entity->unk_4F8.part8.unk_4FB=1-entity->unk_4F8.part8.unk_4FB;
                entity->unk_4FC.part8.unk_4FC=mode+1;
                entity->unk_4FC.part8.unk_4FD=0;
            }
            }
            }
            break;
    }
}
