/* Belmont defeat animation and linked scene completion. */
#include "code/code_08039340.h"
#include "code_08011DD0.h"
#include "code_08033CAC.h"
#include "structs/ewram.h"
extern void Effect_SpawnParticleAnim13(s32,s32,s32,s32,s32,s32);
extern void Belmont_CreateSceneObject(struct EwramData_EntityData *,s32);
/**
 * @brief Defeat state (0xF0): play defeat animation, create scene entry 1
 * and update completion flags/target tracking when the scene object
 * finishes. Retain the existing idle final substate.
 */
void Belmont_UpdateDefeat(struct EwramData_EntityData *entity)
{
    switch(entity->subState)
    {
        case 0:
            if(entity->stage==0)
            {
                if(entity->animScriptIndex!=13)
                    Anim_StartIndexed(entity,13,1,0);
                entity->palette=entity->unk_510.part8.basePalette;
                entity->stage++;
            }
            if(entity->animDone)
            {
                entity->timer=24;
                entity->subState+=2;
                entity->stage=0;
            }
            Anim_Update(entity);
            break;
        case 1:
            if(entity->timer & 1)
            {
                s32 offset=-0x40000;
                if(entity->flipX)
                    offset=-offset;
                Effect_SpawnParticleAnim13(entity->posX.whole+offset,entity->posY.whole-0x160000,
                             0x8000,0x8000,entity->spriteLayer-1,2);
            }
            if(--entity->timer==255)
            {
                if(entity->animScriptIndex!=12)
                    Anim_StartIndexed(entity,12,1,0);
                entity->timer=24;
                entity->subState++;
                entity->stage=0;
            }
            break;
        case 2:
            if(--entity->timer==255)
            {
                entity->subState++;
                entity->stage=0;
            }
            break;
        case 3:
            if(entity->stage==0)
            {
                Belmont_CreateSceneObject(entity,1);
                entity->stage++;
            }
            else
            {
                struct EwramData_EntityData *object=&gEwramData->entityData[entity->unk_4F8.part8.unk_4FB];
                if(object->updateFunc==0)
                {
                    GameFlag_Set(0x29);
                    gEwramData->gameState.bossEventFlags|=4;
                    entity->subState++;
                    entity->stage=0;
                    gEwramData->gameState.stateFlags&=~2;
                    Player_SetSelectedTarget(0);
                    gEwramData->gameState.stateFlags|=0x1000;
                    gEwramData->gameState.gameFlagsTail[1]|=0x2000000;
                }
                else if(object->state==127)
                {
                    gEwramData->gameState.stateFlags|=0x400000;
                    gEwramData->gameState.bossEventFlags|=4;
                }
            }
            break;
        case 4:
            if(entity->stage==0)
                entity->stage++;
            break;
    }
}
