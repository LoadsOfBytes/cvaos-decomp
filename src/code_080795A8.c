/* A spiralling particle rises from a saved origin, independently of the camera. */
#include "code_08009A0.h"
#include "werejaguar_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 sCommonObjectGfx[],sCommonObjectSpriteSheet[];
void WerejaguarDeathEffect_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
    switch(entity->state)
    {
        case 0:
        {
            s32 graphics=ObjGfx_Find(sCommonObjectGfx);
            Sprite_Attach(entity,sCommonObjectSpriteSheet,graphics,7);
            Anim_StartIndexed(entity,0,0,0);
            entity->scrollWithBg=0;
            entity->drawFunc=(u32 *)Sprite_DrawEntity;
            entity->state++;
            break;
        }
        case 1:
        {
            s32 x,y;
            WEREJAGUAR_EFFECT_ANGLE(entity)+=0x1000;
            x=Sin(WEREJAGUAR_EFFECT_ANGLE(entity)+0x4000)*16;
            y=Sin(WEREJAGUAR_EFFECT_ANGLE(entity))*2;
            WEREJAGUAR_EFFECT_Y(entity)-=2;
            entity->posX.whole=(WEREJAGUAR_EFFECT_X(entity)<<16)+x-(((s16)camera->xPos.part16.integer-WEREJAGUAR_EFFECT_CAMERA_X(entity))<<16);
            entity->posY.whole=(WEREJAGUAR_EFFECT_Y(entity)<<16)+y-(((s16)camera->yPos.part16.integer-WEREJAGUAR_EFFECT_CAMERA_Y(entity))<<16);
            if(y>0) entity->spriteLayer=WEREJAGUAR_EFFECT_LAYER(entity)+1;
            else entity->spriteLayer=WEREJAGUAR_EFFECT_LAYER(entity)-1;
            break;
        }
    }
    if(Anim_Update(entity)==3) EntityDelete(entity);
}
