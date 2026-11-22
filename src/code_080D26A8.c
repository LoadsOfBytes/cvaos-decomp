/* Graham scene setup, player-input pause and return after the scene finishes. */
#include "code_08009A0.h"
#include "graham_entity.h"
#include "structs/ewram.h"
extern s32 Game_SetMenusEnabled(s32);
extern struct EwramData_EntityData *Entity_CreateFromObjectTable(s32);
extern s32 Graham_CreateSceneObject(struct EwramData_EntityData *,s32);
extern void EnemyGrahamCreate(struct EwramData_EntityData *);
/**
 * @brief Root intro state (1): gate menus, create the temporary scene
 * helper, pause player input and start scene entry 0, then reinitialize
 * the encounter on scene completion. Preserve allocation/pointer assumptions.
 */
void Graham_UpdateIntro(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *object=&gEwramData->entityData[(u8)GRAHAM_SCENE_OBJECT_INDEX(entity)];
    struct EwramData_EntityData *helper=&gEwramData->entityData[GRAHAM_SCENE_HELPER_INDEX(entity)];
    Game_SetMenusEnabled(0);
    switch(entity->stage)
    {
        case 0:
        {
            struct BgLayerState *camera;
            helper=Entity_CreateFromObjectTable(4);
            GRAHAM_SCENE_HELPER_INDEX(entity)=ENTITY_INDEX(helper);
            helper->flipX=1;
            camera=&gEwramData->bgInfo[1];
            helper->posX.part16.integer=120-camera->xPos.part16.integer;
            helper->posY.part16.integer=191-camera->yPos.part16.integer;
            entity->timer=80;
            entity->stage++;
            break;
        }
        case 1:
            SetPlayerInput(2,0);
            if(--entity->timer==255)
                entity->stage++;
            break;
        case 2:
            if(!(gEwramData->gameState.stateFlags & 0x100))
            {
                entity->hidden=1;
                helper->deleteRequested=1;
                Graham_CreateSceneObject(entity,0);
                entity->stage++;
            }
            break;
        case 3:
            if(!object)
            {
                entity->unk_514.part16.spawnParam0=0;
                EnemyGrahamCreate(entity);
            }
            else if(object->state==128)
            {
                entity->unk_514.part16.spawnParam0=0;
                gEwramData->gameState.stateFlags&=~0x40000;
                EnemyGrahamCreate(entity);
            }
            break;
    }
}
