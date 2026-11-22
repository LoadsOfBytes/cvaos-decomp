/* Animated fragment: play the selected animation, then expire after 8 frames. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern u8 sObject16Gfx[], sObject16Palette[], sObject16SpriteSheet[];
extern struct AnimScript sObject16EffectAnimScript[], sObject16EffectAnimScriptVariant0[];
/**
 * @brief One-shot effect of object 0x16: play its animation, then delete after
 * 8 frames.
 */
void Object16Effect_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
    {
        struct AnimScript *anim;
        s32 variant;
        if (!Entity_LoadGraphics(entity, sObject16Gfx, sObject16Palette, 0, sObject16SpriteSheet))
        { EntityDelete(entity); return; }
        variant = entity->subState;
        anim = sObject16EffectAnimScript;
        if (variant == 0) anim = sObject16EffectAnimScriptVariant0;
        Anim_Start(entity, anim, 0, 0);
        entity->spriteLayer = 0x11 - entity->subState;
        entity->scrollWithBg = 1;
        entity->state++;
        break;
    }
    case 1:
        if (Anim_Update(entity) == 3)
        {
            entity->timer = 8;
            entity->drawSkip = 1;
            entity->state++;
        }
        break;
    case 2:
        if (entity->timer != 0) entity->timer--;
        else entity->deleteRequested = 1;
        break;
    }
    EntityAccelerateAndMove(entity, 0x80000);
}
