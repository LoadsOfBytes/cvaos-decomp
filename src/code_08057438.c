#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
struct FourBoxes { u32 box[4]; };
struct ObjectVariant { u8 *graphics; void *palette; void *sprites; u8 animation; u8 paletteIndex; u16 pad; };
extern struct FourBoxes sObject2ABoxes;
extern struct ObjectVariant sObject2AVariants[];
extern u8 Object2A_LoadGraphics(struct EwramData_EntityData *);
void Object2ACreate(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    struct FourBoxes boxes = sObject2ABoxes;
    if (!Object2A_LoadGraphics(entity)) { EntityDelete(entity); return; }
    Hit_Enable(entity, 8, NULL, 0);
    HitBox_SetBody(entity, &boxes.box[entity->spawnParam1]);
    HitBox_SetAttack(entity, &boxes.box[entity->spawnParam1]);
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    Anim_StartIndexed(entity, sObject2AVariants[entity->spawnParam1].animation, 0, 1);
    Anim_Update(entity);
    entity->spriteLayer = 18;
    entity->state = 0;
    entity->ext.layerObject.shuttleStart = entity->posX.whole + camera->xPos.whole;
    entity->ext.layerObject.shuttleEnd = entity->spawnParam0 << 16;
    entity->ext.layerObject.x = entity->posX.whole + camera->xPos.whole;
    entity->ext.layerObject.y = entity->posY.whole + camera->yPos.whole;
    entity->ext.layerObject.drawX = entity->ext.layerObject.x - camera->xPos.whole;
    entity->ext.layerObject.drawY = entity->ext.layerObject.y - camera->yPos.whole;
    entity->scrollWithBg = 0;
    if ((s32)entity->ext.layerObject.shuttleStart > entity->ext.layerObject.shuttleEnd) entity->ext.layerObject.shuttleDir = 1;
    else entity->ext.layerObject.shuttleDir = 0;
}
