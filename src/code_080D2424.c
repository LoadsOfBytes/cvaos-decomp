/* Graham encounter setup, including character/event dependent scene paths. */
#include "code_08009A0.h"
#include "graham_entity.h"
#include "code_0800B700.h"
#include "code_08013960.h"
#include "code_08033CAC.h"
#include "structs/ewram.h"
extern s32 Graham_CreateSceneObject(struct EwramData_EntityData *,s32);
extern void Graham_CreateCombatBody(struct EwramData_EntityData *);
extern s32 GrahamFirstForm_Create(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
void EnemyGrahamCreate(struct EwramData_EntityData *entity)
{
    if(gEwramData->gameState.stateFlags & 0x10000)
    {
        entity->deleteRequested=1;
        return;
    }
    {
        struct BgLayerState *camera=&gEwramData->bgInfo[1];
        entity->posX.whole=0x780000-camera->xPos.whole;
        entity->posY.whole=0x8F0000-camera->yPos.whole;
    }
    if(entity->unk_514.part16.spawnParam0==1)
    {
        if(!gEwramData->gameState.bestiaryPreview && gEwramData->playerStatus.currentCharacter!=1)
        {
            s32 zero;
            GRAHAM_FLAGS(entity)|=0x80;
            zero=0;
            if(gEwramData->gameState.bossEventFlags & 1)
            {
                Graham_CreateSceneObject(entity,5);
                entity->state=2;
                entity->stage=zero;
                entity->subState=zero;
                return;
            }
            Game_SetMenusEnabled(0);
            entity->state=1;
            entity->stage=zero;
            entity->subState=zero;
            return;
        }
    }
    if(!gEwramData->gameState.bestiaryPreview)
    {
        Game_SetMenusEnabled(1);
        Player_SetSelectedTarget(entity);
        gEwramData->gameState.stateFlags|=2;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer=20;
    if(gEwramData->gameState.bestiaryPreview)
    {
        s32 zero;
        entity->posX.part16.integer-=56;
        zero=0;
        entity->posY.part16.integer-=64;
        Graham_CreateCombatBody(entity);
        entity->state=0xD0;
        entity->stage=zero;
        entity->subState=zero;
        return;
    }
    if((gEwramData->gameState.bossEventFlags & 1) && gEwramData->gameState.bossRushMode==0)
    {
        EntityDelete(entity);
        return;
    }
    Music_SetCurrent(19);
    GrahamFirstForm_Create(entity);
    entity->state=3;
    entity->stage=0;
    entity->subState=0;
}
