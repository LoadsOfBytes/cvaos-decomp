/* Allocate an affine effect attached to an owner's entity slot. */
#include "code_08009A0.h"
#include "event_scene30.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void EventScene30CenterEffect_Update(struct EwramData_EntityData *);
extern u8 sEventScene30Gfx[],sEventScene30SpriteSheet[];
/**
 * @brief Create the central effect sprite (EventScene30CenterEffect_Update)
 * attached to the owner.
 */
void EventScene30_CreateCenterEffect(struct EwramData_EntityData *owner)
{
    u32 *callback=(u32 *)EventScene30CenterEffect_Update;
    struct EwramData_EntityData *effect=EntityCreateInRange(0x1B,0x48,callback);
    u8 zero;
    if(effect)
    {
        s32 slot=ObjGfx_LoadResource(sEventScene30Gfx,0);
        if(slot<0)goto fail;
        effect->basePalette=14;
        zero=0;
        Sprite_Attach(effect,sEventScene30SpriteSheet,slot,14);
        effect->drawFunc=(u32 *)Sprite_DrawEntity;
        ENTITY_PARENT_INDEX(effect)=ENTITY_INDEX(owner);
        effect->ext.event30Effect.scale=0x1A000;
        slot=ObjAffine_Alloc(1);
        if(slot<0)
        {
fail:
            EntityDelete(effect);
            return;
        }
        effect->affineIndex=slot;
        effect->affine=1;
        ObjAffine_Set(0x10000,effect->ext.event30Effect.scale,effect->ext.event30Effect.scale,0,zero,effect->affineIndex);
        effect->updateFunc=callback;
        effect->timer=32;
        effect->ext.raw.unk_4FC.part16.unk_4FC=zero;
        effect->posX.part16.integer=130;
        effect->posY.part16.integer=28;
        effect->velX.whole=-0x800;
        effect->velY.whole=0x800;
        effect->accelX.whole=-0x100;
        effect->accelY.whole=0x100;
        effect->ext.event30Effect.spin=(s16)0xFE00;
        effect->spriteLayer=3;
        effect->animFrame=zero;
    }
}
