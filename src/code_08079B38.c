/* Idle gait and transitions between approaching, retreating and attacking. */
#include "code_08009A0.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_IsInRect(s16,s16,s16,s16);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern s32 Entity_ClampXToRoom(struct EwramData_EntityData *,s32);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *,s32,s32,s32,s32);
#define GAIT(e) ((e)->unk_504.part8.unk_505)
#define WAIT(e) ((e)->unk_504.part8.unk_506)
/**
 * @brief Walk state machine (state 0): wake when the target is in the 160 x 160
 * box or the Arachne is damaged, then crawl with the gait tables toward or
 * away from it and start the beam attack (state 1).
 */
void Arachne_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 state=entity->subState;
    s32 phase;
    u32 flipped;
    switch(state)
    {
    case 0:
        entity->flipX=Entity_IsTargetRight(entity);
        if(Target_IsInRect(entity->posX.part16.integer-80,entity->posY.part16.integer-80,160,160) || Enemy_IsDamaged(entity))
        {
            entity->subState=1; GAIT(entity)=1;
            entity->unk_4FC.whole=state;
            entity->unk_500.whole=state;
            entity->velX.whole=state;
            entity->timer=state;
            if(Target_IsInRect(entity->posX.part16.integer-60,entity->posY.part16.integer-80,120,160))
            { GAIT(entity)=2; entity->subState=2; }
            else if(Target_IsInRect(entity->posX.part16.integer-100,entity->posY.part16.integer-80,200,160)) goto vertical;
            else { GAIT(entity)=1; entity->subState=1; }
        }
        break;
    case 1:
    {
        s32 target;
        u32 random;
        if(entity->unk_500.whole<0x10000) entity->unk_500.whole+=0x2000;
        phase=entity->unk_4FC.whole+entity->unk_500.whole;
        entity->unk_4FC.whole=phase;
        if(phase>=0x240000) entity->unk_4FC.whole=phase-0x240000;
        else if(phase<0) entity->unk_4FC.whole=phase+0x240000;
        if(entity->timer++>29) { entity->flipX=Entity_IsTargetRight(entity); entity->timer=0; }
        if(entity->flipX) entity->accelX.whole=0x1000;
        else entity->accelX.whole=-0x1000;
        flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(entity))&0x40;
        if((u8)flipped) { if(entity->velX.whole>=0x8000)entity->accelX.whole=0; }
        else if(entity->velX.whole<=-0x8000)entity->accelX.whole=flipped;
        Entity_MoveWithCollision(entity,0x10000,-0x200000,0x80000,12);
        target=Target_IsInRect(entity->posX.part16.integer-60,entity->posY.part16.integer-80,120,160);
        if(target)
        { GAIT(entity)=2; entity->subState=2; entity->timer=0; entity->velY.whole=0x10000; goto acceleration; }
        if(Entity_ClampXToRoom(entity,1))
        { GAIT(entity)=2; entity->subState=2; entity->timer=target; entity->velY.whole=0x10000; goto acceleration; }
        if(WAIT(entity)) WAIT(entity)--;
        else
        {
            random=RandomNumberGenerator()&31;
            if(!random && Target_IsInRect(entity->posX.part16.integer-80,entity->posY.part16.integer-80,160,160))
            { GAIT(entity)=3; entity->state=1; entity->subState=random; entity->timer=random; }
        }
        break;
    }
    case 2:
    {
        u32 random;
        if(entity->unk_500.whole>-0x10000) entity->unk_500.whole-=0x2000;
        phase=entity->unk_4FC.whole+entity->unk_500.whole;
        entity->unk_4FC.whole=phase;
        if(phase>=0x240000) entity->unk_4FC.whole=phase-0x240000;
        else if(phase<0) entity->unk_4FC.whole=phase+0x240000;
        entity->flipX=Entity_IsTargetRight(entity);
        if(entity->flipX) entity->accelX.whole=-0x1000;
        else entity->accelX.whole=0x1000;
        flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(entity))&0x40;
        if((u8)flipped) { if(entity->velX.whole<=-0x8000)entity->accelX.whole=0; }
        else if(entity->velX.whole>=0x8000)entity->accelX.whole=flipped;
        if((Entity_MoveWithCollision(entity,0x10000,-0x200000,0x80000,12)&3) ||
            !Target_IsInRect(entity->posX.part16.integer-100,entity->posY.part16.integer-80,200,160) || Entity_ClampXToRoom(entity,1))
        { GAIT(entity)=1; entity->subState=1; entity->timer=0;
vertical:
            entity->velY.whole=0x10000;
acceleration:
            entity->accelY.whole=0x2800;
            goto done;
        }
        if(!WAIT(entity))
        {
            random=RandomNumberGenerator()&31;
            if(!random && Target_IsInRect(entity->posX.part16.integer-80,entity->posY.part16.integer-80,160,160))
            { GAIT(entity)=3; entity->state=1; entity->subState=10; entity->timer=random; }
        }
        else WAIT(entity)--;
        break;
    }
    }
done:
    return;
}
