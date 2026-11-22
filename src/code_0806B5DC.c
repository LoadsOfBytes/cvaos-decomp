/* Initialize and play a short shared effect animation. */
#include "code/code_08039340.h"
struct SharedEffectAnimation {
    u8 animation;
    u8 palette;
    u8 unused[2];
};
extern const struct SharedEffectAnimation sHitEffects[];
extern u8 sCommonObjectGfx[], sCommonObjectSpriteSheet[];
/**
 * @brief State 0: attach the shared object graphics with the palette and
 * animation of entry subState of sHitEffects (layer 3, scrolls with BG);
 * then animate and delete at the end of the animation.
 */
void HitEffect_Update(struct EwramData_EntityData *entity)
{
    if (entity->state == 0)
    {
        const struct SharedEffectAnimation *entry = &sHitEffects[entity->subState];
        s32 graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, entry->palette);
        Anim_StartIndexed(entity, entry->animation, 0, 0);
        entity->spriteLayer = 3;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        entity->state++;
    }
    if (Anim_Update(entity) == 3)
        entity->deleteRequested = 1;
}
