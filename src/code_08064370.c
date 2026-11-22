/* Allocate an affine effect attached to an owner's entity slot. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void EventScene30Halo_Update(struct EwramData_EntityData *);
extern u8 sEventScene30Gfx[],sEventScene30SpriteSheet[];
/**
 * @brief Create the halo sprite (EventScene30Halo_Update) attached to the
 * owner.
 */
void EventScene30_CreateHalo(struct EwramData_EntityData *owner)
{
    struct EwramData_EntityData *effect=EntityCreateInRange(0x1B,0x48,(u32 *)EventScene30Halo_Update);
    u8 zero;
    if(effect)
    {
        s32 slot=ObjGfx_LoadResource(sEventScene30Gfx,0);
        if(slot<0)goto fail;
        effect->unk_510.part8.basePalette=14;
        zero=0;
        Sprite_Attach(effect,sEventScene30SpriteSheet,slot,14);
        effect->drawFunc=(u32 *)Sprite_DrawEntity;
        effect->unk_4F4.part8.unk_4F5=owner->unk_4F4.part8.unk_4F4;
        effect->unk_500.whole=0x1A000;
        slot=ObjAffine_Alloc(1);
        if(slot<0)
        {
fail:
            EntityDelete(effect);
            return;
        }
        effect->affineIndex=slot;
        effect->affine=1;
        ObjAffine_Set(0x10000,effect->unk_500.whole,effect->unk_500.whole,0,zero,effect->affineIndex);
        effect->unk_4FC.part16.unk_4FC=zero;
        effect->posX.part16.integer=(*(s8 *)&owner->unk_508.part16.unk_50A-48)+owner->unk_504.part16.unk_504;
        effect->posY.part16.integer=(s16)((s8)owner->unk_508.part8.unk_50B+8)+owner->unk_504.part16.unk_506[0];
        effect->velX.whole=-1000;
        effect->velY.whole=0x1000;
        effect->accelX.whole=-0x800;
        effect->accelY.whole=0x600;
        effect->unk_51C=zero;
        effect->unk_508.part16.unk_50A=(s16)0xFF56;
        effect->spriteLayer=3;
        effect->animFrame=24;
    }
}
