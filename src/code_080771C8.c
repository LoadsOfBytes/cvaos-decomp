/* Body segments sample older angles from the head's motion history. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#define FIXED_INTEGER(x) ((x)<0 ? -(-(x)>>16) : (x)>>16)
#define SEGMENT(e) (*(s8 *)&e->ext.whiteDragonSegment.index)
/**
 * @brief Draw function of a neck segment: place it along the owner's (type 11)
 * neck path, 8 px apart (6 while dying), then draw it.
 */
void WhiteDragonSegment_Draw(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=ENTITY_OWNER(entity);
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
    if(owner && owner->updateFunc && owner->enemyId==11)
    {
        s16 *history=(s16 *)owner->ext.whiteDragon.history;
        u8 state=owner->state;
        s32 spacing=8;
        s32 wave,sway,x,y;
        if(state==2) spacing=6;
        entity->posX.part16.integer=owner->ext.whiteDragon.anchorX-camera->xPos.part16.integer+owner->ext.whiteDragon.offsetX;
        entity->posY.part16.integer=owner->ext.whiteDragon.anchorY-camera->yPos.part16.integer+owner->ext.whiteDragon.offsetY;
        wave=(Sin(history[(SEGMENT(entity)+1)*spacing])*owner->ext.whiteDragon.sway)/4096;
        sway=Sin((u16)owner->ext.whiteDragon.swayPhase)/16;
        x=FIXED_INTEGER(Sin(owner->velX.part16.integer+wave+sway+0x4000)*owner->velY.part16.integer*(9-SEGMENT(entity))/10);
        y=FIXED_INTEGER(Sin(owner->velX.part16.integer+wave+sway)*owner->velY.part16.integer*(9-SEGMENT(entity))/10);
        entity->posX.part16.integer+=x;
        entity->posY.part16.integer+=y;
        Sprite_DrawEntity(entity);
    }
    else EntityDelete(entity);
}

