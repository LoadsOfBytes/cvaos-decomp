/* Belmont hit reaction: airborne knockback, landing, slide and recovery. */
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_MoveWithCollisionProfile(struct EwramData_EntityData *,s16 *,u16,s32);
extern s32 Entity_ClampXToRoom(struct EwramData_EntityData *,s32);
extern s32 Entity_IsOffsetOutsideRoom(struct EwramData_EntityData *,s32,s32);
extern struct EwramData_EntityData *Belmont_CreateMovementEffect(struct EwramData_EntityData *);
extern s16 sBelmontMoveProfile[];
/**
 * @brief Hit-reaction state (11): handle airborne knockback, landing,
 * sliding and recovery. Select defeat state 0xF0 when HP is exhausted
 * or return to combat/dodge according to the existing room tests.
 */
void Belmont_UpdateHitReaction(struct EwramData_EntityData *entity)
{
    s32 state=entity->subState;
    s32 collision;
    switch(state)
    {
        case 0:
        {
            s32 ground;
            entity->accelX.whole=state;
            entity->velX.whole=state;
            entity->accelY.whole=state;
            entity->velY.whole=state;
            ground=Entity_MoveWithCollisionProfile(entity,sBelmontMoveProfile,4,0) & 8;
            if(ground)
            {
                entity->timer=24;
                entity->subState=255;
                entity->stage=state;
            }
            else
            {
                s32 speed;
                s32 acceleration;
                speed=-0x18000;
                entity->velX.whole=speed;
                acceleration=-0x800;
                entity->accelX.whole=acceleration;
                if(gEwramData->playerState.player->posX.whole<entity->posX.whole)
                {
                    entity->velX.whole=-speed;
                    entity->accelX.whole=-acceleration;
                }
                entity->velY.whole=-0x10000;
                entity->accelY.whole=0x2000;
                entity->subState++;
                entity->stage=ground;
            }
            if(entity->animScriptIndex!=12)
                Anim_StartIndexed(entity,12,1,0);
            break;
        }
        case 1:
            collision=Entity_MoveWithCollisionProfile(entity,sBelmontMoveProfile,4,0);
            if(collision & 3)
            {
                if(entity->flipX)
                    entity->accelX.whole=entity->velX.whole=0;
                else
                    entity->accelX.whole=entity->velX.whole=0;
            }
            if(Entity_ClampXToRoom(entity,1))
            {
                if(entity->flipX)
                    entity->accelX.whole=entity->velX.whole=0;
                else
                    entity->accelX.whole=entity->velX.whole=0;
            }
            if(collision & 8)
            {
                s32 speed;
                s32 acceleration;
                if(entity->animScriptIndex!=18)
                    Anim_StartIndexed(entity,18,1,0);
                speed=-0x18000;
                entity->velX.whole=speed;
                acceleration=0x800;
                entity->accelX.whole=acceleration;
                if(gEwramData->playerState.player->posX.whole<entity->posX.whole)
                {
                    entity->velX.whole=-speed;
                    entity->accelX.whole=-acceleration;
                }
                entity->velY.whole=0;
                entity->accelY.whole=0;
                entity->subState++;
                entity->stage=0;
            }
            break;
        case 2:
            if(entity->velX.whole!=0)
            {
                collision=Entity_MoveWithCollisionProfile(entity,sBelmontMoveProfile,4,0);
                if(collision & 3)
                {
                    if(entity->flipX)
                        entity->accelX.whole=entity->velX.whole=0;
                    else
                        entity->accelX.whole=entity->velX.whole=0;
                }
                if(Entity_ClampXToRoom(entity,1))
                {
                    if(entity->flipX)
                        entity->accelX.whole=entity->velX.whole=0;
                    else
                        entity->accelX.whole=entity->velX.whole=0;
                }
                if((u8)(entity->timer++ % 7)==0)
                    Belmont_CreateMovementEffect(entity);
            }
            {
                s32 inner=entity->stage;
                switch(inner)
                {
                    case 0:
                        if(entity->animDone)
                        {
                            if((s16)entity->hp<=0)
                            {
                                entity->state=240;
                                entity->stage=inner;
                                entity->subState=inner;
                            }
                            else
                            {
                                if(entity->animScriptIndex!=14)
                                    Anim_StartIndexed(entity,14,1,0);
                                entity->stage++;
                            }
                        }
                        break;
                    case 1:
                        inner &= ((u8 *)entity)[0x59];
                        if(inner)
                        {
                            if(entity->animScriptIndex!=4)
                                Anim_StartIndexed(entity,4,1,0);
                            entity->timer=8;
                            entity->accelX.whole=0;
                            entity->velX.whole=0;
                            entity->accelY.whole=0;
                            entity->velY.whole=0;
                            entity->subState=255;
                            entity->stage=0;
                        }
                        break;
                }
            }
            break;
        case 255:
        {
            if(--entity->timer==255)
            {
                if((s16)entity->hp<=0)
                {
                    entity->state=240;
                    entity->stage=0;
                    entity->subState=0;
                    return;
                }
                if((Entity_IsOffsetOutsideRoom(entity,128,0) && !entity->flipX) ||
                        (Entity_IsOffsetOutsideRoom(entity,-128,0) && entity->flipX))
                {
                    entity->state=9;
                    entity->stage=0;
                    entity->subState=0;
                    return;
                }
                entity->state=2;
                entity->stage=0;
                entity->subState=0;
                return;
            }
            break;
        }
    }
}
