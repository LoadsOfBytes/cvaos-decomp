#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
struct FourBoxes { u32 box[4]; };
struct ObjectVariant { u8 *graphics; void *palette; void *sprites; u8 animation; u8 paletteIndex; u16 pad; };
extern struct FourBoxes sObject29Boxes;
extern struct ObjectVariant sObject29Variants[];
extern u8 Object29_LoadGraphics(struct EwramData_EntityData *);
void Object29Create(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    struct FourBoxes boxes = sObject29Boxes;
    if (!Object29_LoadGraphics(entity)) { EntityDelete(entity); return; }
    Hit_Enable(entity, 8, NULL, 0);
    HitBox_SetBody(entity, &boxes.box[entity->unk_514.part16.spawnParam1]);
    HitBox_SetAttack(entity, &boxes.box[entity->unk_514.part16.spawnParam1]);
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    Anim_StartIndexed(entity, sObject29Variants[entity->unk_514.part16.spawnParam1].animation, 0, 1);
    Anim_Update(entity);
    entity->spriteLayer = 18;
    entity->state = 0;
    entity->unk_4FC.whole = entity->posY.whole + camera->yPos.whole;
    entity->unk_500.whole = entity->unk_514.part16.spawnParam0 << 16;
    entity->unk_4F4.whole = entity->posX.whole + camera->xPos.whole;
    entity->unk_4F8.whole = entity->posY.whole + camera->yPos.whole;
    entity->unk_508.whole = entity->unk_4F4.whole - camera->xPos.whole;
    entity->unk_50C.whole = entity->unk_4F8.whole - camera->yPos.whole;
    entity->scrollWithBg = 0;
    if ((s32)entity->unk_4FC.whole > entity->unk_500.whole) entity->unk_504.part8.unk_504 = 1;
    else entity->unk_504.part8.unk_504 = 0;
}
