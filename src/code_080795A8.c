/* A spiralling particle rises from a saved origin, independently of the camera. */
#include "code_08009A0.h"
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
            entity->ext.werejaguarEffect.angle+=0x1000;
            x=Sin(entity->ext.werejaguarEffect.angle+0x4000)*16;
            y=Sin(entity->ext.werejaguarEffect.angle)*2;
            entity->ext.werejaguarEffect.y-=2;
            entity->posX.whole=(entity->ext.werejaguarEffect.x<<16)+x-(((s16)camera->xPos.part16.integer-entity->ext.werejaguarEffect.cameraX)<<16);
            entity->posY.whole=(entity->ext.werejaguarEffect.y<<16)+y-(((s16)camera->yPos.part16.integer-entity->ext.werejaguarEffect.cameraY)<<16);
            if(y>0) entity->spriteLayer=entity->ext.werejaguarEffect.layer+1;
            else entity->spriteLayer=entity->ext.werejaguarEffect.layer-1;
            break;
        }
    }
    if(Anim_Update(entity)==3) EntityDelete(entity);
}
