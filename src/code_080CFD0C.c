/* Belmont introduction waits, creates the scene object, then starts combat. */
#include "structs/ewram.h"
#include "belmont_entity.h"
#include "code_08009A0.h"
#include "code_0800B700.h"
extern void Belmont_CreateSceneObject(struct EwramData_EntityData *,s32);
extern void EnemyBelmontCreate(struct EwramData_EntityData *);
/**
 * @brief Intro state (1): gate menus, supply player input, wait and create
 * scene entry 0, then reinitialize Belmont for combat when the scene
 * object is gone. Preserve the existing slot/pointer tests.
 */
void Belmont_UpdateIntro(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *object=&gEwramData->entityData[entity->ext.belmontScene.objectIndex];
    Game_SetMenusEnabled(0);
    if(entity->stage<=3)
        SetPlayerInput(1,0x20);
    switch(entity->stage)
    {
        case 0:
            entity->timer=56;
            entity->stage++;
            break;
        case 2:
            entity->hidden=1;
            Belmont_CreateSceneObject(entity,0);
            entity->stage++;
            entity->timer=32;
            break;
        case 1:
        case 3:
            if(--entity->timer==0xFF)
                entity->stage++;
            break;
        case 4:
            if(object==NULL)
            {
                entity->unk_514.part16.spawnParam0=0;
                EnemyBelmontCreate(entity);
            }
            else if(object->updateFunc==NULL || object->unk_518.part8.enemyId!=0x20)
            {
                entity->unk_514.part16.spawnParam0=0;
                EnemyBelmontCreate(entity);
            }
            break;
    }
}
