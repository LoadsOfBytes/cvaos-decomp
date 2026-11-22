/* Animated effect follows an owner with fixed X/Y offsets. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 sCommonObjectGfx[],sCommonObjectSpriteSheet[];
/**
 * @brief Animate an effect following its owner pointer at fixed offsets.
 * Request deletion when the owner pointer is NULL or animation completes.
 */
void GrahamPlayerEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=entity->unk_4FC.unk_4FC_entity;
    if(!owner) { entity->deleteRequested=1; return; }
    {
        switch(entity->state)
        {
            case 0:
            {
                s32 graphics;
                if((RandomNumberGenerator()&3)==0)
                    PlaySong(0x16D);
                graphics=ObjGfx_Find(sCommonObjectGfx);
                Sprite_Attach(entity,sCommonObjectSpriteSheet,graphics,entity->palette);
                Anim_StartIndexed(entity,(u8)entity->unk_4F8.part8.unk_4F8,0,0);
                entity->drawFunc=(u32 *)Sprite_DrawEntity;
                entity->scrollWithBg=1;
                entity->state++;
            }
            case 1:
                entity->posX.whole=owner->posX.whole+entity->accelX.whole;
                entity->posY.whole=owner->posY.whole+entity->accelY.whole;
                break;
        }
        if(Anim_Update(entity)==3)
            entity->deleteRequested=1;
    }

}
