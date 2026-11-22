#include "code_08009A0.h"
#include "layer_object.h"
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
    HitBox_SetBody(entity, &boxes.box[entity->unk_514.part16.spawnParam1]);
    HitBox_SetAttack(entity, &boxes.box[entity->unk_514.part16.spawnParam1]);
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    Anim_StartIndexed(entity, sObject2AVariants[entity->unk_514.part16.spawnParam1].animation, 0, 1);
    Anim_Update(entity);
    entity->spriteLayer = 18;
    entity->state = 0;
    SHUTTLE_START(entity) = entity->posX.whole + camera->xPos.whole;
    SHUTTLE_END(entity) = entity->unk_514.part16.spawnParam0 << 16;
    BG1_OBJECT_X(entity) = entity->posX.whole + camera->xPos.whole;
    BG1_OBJECT_Y(entity) = entity->posY.whole + camera->yPos.whole;
    BG1_OBJECT_DRAW_X(entity) = BG1_OBJECT_X(entity) - camera->xPos.whole;
    BG1_OBJECT_DRAW_Y(entity) = BG1_OBJECT_Y(entity) - camera->yPos.whole;
    entity->scrollWithBg = 0;
    if ((s32)SHUTTLE_START(entity) > SHUTTLE_END(entity)) SHUTTLE_DIR(entity) = 1;
    else SHUTTLE_DIR(entity) = 0;
}
