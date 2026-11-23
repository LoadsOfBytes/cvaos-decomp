/* Allocate a shared effect, optionally reserving an affine sprite slot. */
#include "code_08009A0.h"
#include "event_scene30.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 sEventScene30Gfx[],sEventScene30SpriteSheet[];
/**
 * @brief Create an effect sprite entity of event scene 30 (graphics
 * sEventScene30Gfx, palette 14), optionally with an affine slot at scale
 * 0x12000; return NULL on failure.
 */
struct EwramData_EntityData *EventScene30_CreateEffectSprite(s32 affine)
{
    struct EwramData_EntityData *entity=EntityCreateInRange(0x1B,0x48,(u32 *)EntityUpdateNothing);
    if(entity)
    {
        s32 slot=ObjGfx_LoadResource(sEventScene30Gfx,0);
        if(slot<0)goto fail;
        entity->unk_510.part8.basePalette=14;
        Sprite_Attach(entity,sEventScene30SpriteSheet,slot,14);
        entity->drawFunc=(u32 *)Sprite_DrawEntity;
        if(affine)
        {
            entity->ext.event30Effect.scale=0x12000;
            slot=ObjAffine_Alloc(1);
            if(slot<0)
            {
fail:
                EntityDelete(entity);return 0;
            }
            entity->affineIndex=slot;entity->affine=1;
            ObjAffine_Set(0x10000,entity->ext.event30Effect.scale,entity->ext.event30Effect.scale,0,0,entity->affineIndex);
        }
        entity->spriteLayer=17;
    }
    return entity;
}
