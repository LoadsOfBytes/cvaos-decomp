/* Spawn Belmont's scene object from a table and mark its event bit. */
#include "code_08009A0.h"
#include "belmont_entity.h"
#include "code/code_08039340.h"
#include "code/code_08050A3C.h"
#include "structs/ewram.h"
struct BelmontSceneObject { s16 x,y; u8 event,pad5,kind,pad7; u16 param1,param2; };
extern struct BelmontSceneObject sBelmontSceneObjects[];
/**
 * @brief When scene flag 0x10 is set, create the indexed table-configured
 * object 0x20, link it to Belmont and set its event bit. Retain room-
 * relative positioning for entry 0 and owner positioning for later entries.
 */
void Belmont_CreateSceneObject(struct EwramData_EntityData *entity,s32 index)
{
    struct BelmontSceneObject *info=&sBelmontSceneObjects[index];
    if(BELMONT_FLAGS(entity) & 0x10)
    {
        struct EwramData_EntityData *object=EntityCreateInRange(0x1B,0x48,(u32 *)EntityUpdateNothing);
        if(object)
        {
            s32 facing;
            BELMONT_SCENE_OBJECT_INDEX(entity)=ENTITY_INDEX(object);
            ENTITY_PARENT_INDEX(object)=ENTITY_INDEX(entity);
            if(index==0)
            {
                s32 x=info->x<<16;
                struct BgLayerState *bg=&gEwramData->bgInfo[1];
                object->posX.whole=x-bg->xPos.whole;
                object->posY.whole=(info->y<<16)-bg->yPos.whole;
            }
            else
            {
                object->posX.whole=entity->posX.whole;
                object->posY.whole=entity->posY.whole;
            }
            facing=0;
            if(object->posX.whole<gEwramData->playerState.player->posX.whole)
                facing=1;
            object->flipX=facing;
            object->scrollWithBg=1;
            object->unk_514.part16.spawnParam0=info->param1;
            object->unk_514.part16.spawnParam1=info->param2;
            object->unk_518.part8.spawnIndex=info->event;
            object->unk_518.part8.enemyId=info->kind;
            object->drawFunc=(u32 *)Sprite_DrawEntity;
            object->updateFunc=(u32 *)Object20Update;
            Object20Create(object);
            gEwramData->gameState.roomEntitySpawn.spawnedFlags[info->event>>5] |= 1 << (info->event & 31);
        }
    }
}
