/* Allocate an affine effect attached to an owner's entity slot. */
#include "code_08009A0.h"
#include "event_scene30.h"
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
        effect->ext.event30Fragment.phase=zero;
        effect->posX.part16.integer=(*(s8 *)&owner->ext.event30.jitterX-48)+owner->ext.event30.centerX;
        effect->posY.part16.integer=(s16)((s8)EVENT30_JITTER_Y(owner)+8)+owner->ext.event30.centerY;
        effect->velX.whole=-1000;
        effect->velY.whole=0x1000;
        effect->accelX.whole=-0x800;
        effect->accelY.whole=0x600;
        EVENT30_EFFECT_ANGLE(effect)=zero;
        effect->ext.event30Effect.spin=(s16)0xFF56;
        effect->spriteLayer=3;
        effect->animFrame=24;
    }
}
