/* Recoil reverses gait, accelerates away from the target, and recovers at an obstacle. */
#include "code_08009A0.h"
#include "arachne_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_IsInRect(s16,s16,s16,s16);
extern s32 Entity_ClampXToRoom(struct EwramData_EntityData *,s32);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *,s32,s32,s32,s32);
/**
 * @brief Recoil state machine (state 3, after a hit of type 3 / 4): back off from
 * the target, then return to state 0.
 */
void Arachne_UpdateRecoil(struct EwramData_EntityData *entity)
{
    switch(entity->subState)
    {
    case 0:
        ARACHNE_GAIT(entity)=2;
        entity->subState++;
        ARACHNE_WALK_SPEED(entity)=-0x20000;
    case 1:
    {
        s32 phase=ARACHNE_WALK_PHASE(entity)+ARACHNE_WALK_SPEED(entity);
        u32 flipped;
        ARACHNE_WALK_PHASE(entity)=phase;
        if(phase>=0x240000) ARACHNE_WALK_PHASE(entity)=phase-0x240000;
        else if(phase<0) ARACHNE_WALK_PHASE(entity)=phase+0x240000;
        entity->flipX=Entity_IsTargetRight(entity);
        if(entity->flipX) entity->accelX.whole=-0x4000;
        else entity->accelX.whole=0x4000;
        flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(entity))&0x40;
        if((u8)flipped)
        {
            if(entity->velX.whole<=-0x20000) entity->accelX.whole=0;
        }
        else if(entity->velX.whole>=0x20000) entity->accelX.whole=flipped;
        if(entity->animDone && entity->animScriptIndex==3) Anim_StartIndexed(entity,0,0,1);
        if((Entity_MoveWithCollision(entity,0x10000,-0x200000,0x80000,12)&3) ||
           !Target_IsInRect(entity->posX.part16.integer-100,entity->posY.part16.integer-80,200,160) ||
           Entity_ClampXToRoom(entity,1))
        {
            ARACHNE_GAIT(entity)=1;
            entity->state=0;
            entity->subState=1;
            entity->velY.whole=0x10000;
            entity->accelY.whole=0x2800;
            Anim_StartIndexed(entity,0,0,1);
        }
        break;
    }
    }
}
