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
            entity->unk_4F8.whole+=0x1000;
            x=Sin(entity->unk_4F8.whole+0x4000)*16;
            y=Sin(entity->unk_4F8.whole)*2;
            entity->unk_504.part16.unk_504-=2;
            entity->posX.whole=(entity->unk_500.part16.unk_500<<16)+x-(((s16)camera->xPos.part16.integer-entity->unk_500.part16.unk_502)<<16);
            entity->posY.whole=(entity->unk_504.part16.unk_504<<16)+y-(((s16)camera->yPos.part16.integer-entity->unk_504.part16.unk_506[0])<<16);
            if(y>0) entity->spriteLayer=entity->unk_4FC.whole+1;
            else entity->spriteLayer=entity->unk_4FC.whole-1;
            break;
        }
    }
    if(Anim_Update(entity)==3) EntityDelete(entity);
}
