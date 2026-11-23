/* Belmont's powered sequence with its attached weapon and rising particles. */
#include "code_08013960.h"
#include "belmont_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Belmont_CreateAttachedWeapon(struct EwramData_EntityData *);
/* This original call site consumes the sign-extended value in all of r0. */
extern s32 Collision_GetPushUp(s32,s32);
extern struct EwramData_EntityData *Belmont_CreatePowerParticle(struct EwramData_EntityData *);
extern u8 sBelmontWeaponSolidCallbacks[];
/**
 * @brief Powered state (8): create the attached weapon, sequence tracked
 * sounds and rising particles, then descend and clean up the weapon
 * on ground contact. Keep original parent tests and raw event calls.
 */
void Belmont_UpdatePoweredPhase(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *weapon=&gEwramData->entityData[entity->ext.belmontWeapon.index];
    s32 state=entity->subState;
    switch(state)
    {
        case 0:
            entity->ext.belmont.powerStep++;
            if(!Belmont_CreateAttachedWeapon(entity))
            {
                entity->state=2;
                entity->stage=state;
                entity->subState=state;
                return;
            }
            entity->accelX.whole=state;
            entity->velX.whole=state;
            entity->accelY.whole=state;
            entity->velY.whole=-0x2000;
            entity->subState++;
            entity->stage=state;
            weapon=&gEwramData->entityData[entity->ext.belmontWeapon.index];
            if(weapon->unk_518.part8.enemyId==entity->unk_518.part8.enemyId && weapon)
                Player_RegisterSolidObject(weapon,sBelmontWeaponSolidCallbacks);
            break;
        case 1:
            if(entity->stage==0)
            {
                Sound_PlayTracked(250);
                if(entity->animScriptIndex!=10)
                    Anim_StartIndexed(entity,10,1,1);
                entity->stage++;
                entity->timer=255;
            }
            else
            {
                state &= entity->timer;
                if(state)
                    Belmont_CreatePowerParticle(entity);
                if(--entity->timer==255)
                {
                    entity->velY.whole=0;
                    entity->subState++;
                    entity->stage=0;
                }
            }
            break;
        case 2:
        {
            s32 inner=entity->stage;
            if(inner==0)
            {
                if(weapon->animScriptIndex!=0)
                    Anim_StartIndexed(weapon,0,0,0);
                weapon->state++;
                weapon->stage=inner;
                weapon->subState=inner;
                Sound_StopTracked(250);
                Sound_PlayTracked(251);
                if(entity->animScriptIndex!=11)
                    Anim_StartIndexed(entity,11,1,1);
                entity->timer=255;
                entity->stage++;
            }
            else
            {
                if(entity->timer & 1)
                {
                    struct EwramData_EntityData *effect=Belmont_CreatePowerParticle(entity);
                    if(effect)
                        effect->ext.belmontParticle.step=state;
                }
                if(weapon->animDone && weapon->animScriptIndex!=1)
                    Anim_StartIndexed(weapon,1,0,1);
                if(--entity->timer==255)
                {
                    entity->subState++;
                    entity->stage=0;
                }
            }
            break;
        }
        case 3:
        {
            
            if(entity->stage==0)
            {
                entity->velY.whole=0x8000;
                entity->stage++;
                weapon->drawSkip=1;
            }
            else
            {
              s32 x=(s16)entity->posX.part16.integer;
              struct BgLayerState *camera=&gEwramData->bgInfo[1];
              x+=(s16)camera->xPos.part16.integer;
              if(Collision_GetPushUp(x,(s16)entity->posY.part16.integer+(s16)camera->yPos.part16.integer))
              {
                Sound_StopTracked(251);
                if(entity->animScriptIndex!=0)
                    Anim_StartIndexed(entity,0,1,1);
                entity->state=2;
                entity->stage=0;
                entity->subState=0;
                entity->ext.belmontWeapon.index=0;
                weapon->deleteRequested=1;
                Player_ClearSolidObject();
              }
            }
            break;
        }
    }
    entity->posX.whole+=entity->velX.whole;
    entity->posY.whole+=entity->velY.whole;
    entity->velX.whole+=entity->accelX.whole;
    entity->velY.whole+=entity->accelY.whole;
}
