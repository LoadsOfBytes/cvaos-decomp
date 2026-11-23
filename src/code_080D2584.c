/* Create a Graham scene object and set its event bit. */
#include "code_08009A0.h"
#include "graham_entity.h"
#include "code/code_08039340.h"
#include "code/code_08050A3C.h"
#include "structs/ewram.h"
struct GrahamSceneObject { s16 x,y; u8 unused[4]; u16 param1,param2; };
extern struct GrahamSceneObject sGrahamSceneObjects[];
/**
 * @brief Create the indexed scene object 0x20 when owner/global flags
 * allow it, store slot links and set its event bit. Return 1 on success
 * or 0 when gated or allocation fails.
 */
s32 Graham_CreateSceneObject(struct EwramData_EntityData *entity,s32 index)
{
    struct GrahamSceneObject *info=&sGrahamSceneObjects[index];
    struct EwramData_EntityData *object;
    s32 facing;
    if(!(entity->ext.graham.flags & 0x80))
        return 0;
    if(gEwramData->gameState.stateFlags & 0x10)
        return 0;
    object=EntityCreateInRange(0x1B,0x48,(u32 *)EntityUpdateNothing);
    if(!object)
        return 0;
    entity->ext.grahamScene.objectIndex=ENTITY_INDEX(object);
    ENTITY_PARENT_INDEX(object)=ENTITY_INDEX(entity);
    if(index==0 || index==5)
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
    object->unk_518.part8.spawnIndex=index;
    object->unk_518.part8.enemyId=32;
    object->drawFunc=(u32 *)Sprite_DrawEntity;
    object->updateFunc=(u32 *)Object20Update;
    Object20Create(object);
    gEwramData->gameState.roomEntitySpawn.spawnedFlags[index>>5] |= 1 << (index & 31);
    return 1;
}
