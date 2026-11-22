/* Headhunter lightning-impact flicker and orbiting-particle creation.
 * The particle follows a stored anchor with an expanding radius. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
struct SingleFrameAnimation {
    u16 count;
    u16 format;
    u8 frame;
    u8 duration;
    u16 unk_6;
};
extern struct SingleFrameAnimation sHeadhunterFrameAnim;
extern void HeadhunterOrbitingParticle_Update(struct EwramData_EntityData *);
extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];
/* bit 0 of the owner's unk_4FE ends its effects (TODO: meaning) */
#define OWNER_DONE(o) ((o)->unk_4FC.part8.unk_4FE & 1)

/**
 * @brief Randomize between two frames and remove after signed timer 7
 * or when the retained boss sets effect-cancel bit 0.
 */
void HeadhunterLightningImpactFlicker_Update(struct EwramData_EntityData *entity)
{
    struct SingleFrameAnimation animation = sHeadhunterFrameAnim;
    struct EwramData_EntityData *owner = entity->unk_4F4.unk_4F4_entity;
    Anim_Start(entity, (struct AnimScript *)&animation, 2, 1);
    animation.frame = RandomNumberGenerator() % 2 + entity->unk_4FC.part8.unk_4FF;
    Anim_Update(entity);
    if ((s8)entity->timer++ > 7)
        EntityDelete(entity);
    else if (OWNER_DONE(owner))
        EntityDelete(entity);
}
/**
 * @brief Create an orbiting particle with the supplied initial angle,
 * an owner-relative anchor and stored camera offsets. Retain the
 * separate boss link used by its effect-cancel check.
 */
void Headhunter_SpawnOrbitingParticle(struct EwramData_EntityData *owner, struct EwramData_EntityData *source, s32 x, s32 y, s32 speed)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, (u32 *)HeadhunterOrbitingParticle_Update);
    if (effect != NULL)
    {
        s32 graphics;
        effect->velX.whole = owner->posX.whole + x;
        effect->velY.whole = owner->posY.whole + y;
        effect->accelX.whole = speed;
        effect->accelY.whole = 0x100;
        effect->unk_4FC.whole = camera->xPos.whole;
        effect->unk_500.whole = camera->yPos.whole;
        effect->flipX = RandomNumberGenerator() % 2;
        effect->flipY = 1;
        effect->spriteLayer = owner->spriteLayer - 1;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(effect, sCommonObjectSpriteSheet, graphics, 7);
        Anim_StartIndexed(effect, 0, 0, 0);
        Anim_Update(effect);
        effect->unk_4F8.unk_4F8_entity = owner;
        effect->unk_4F4.unk_4F4_entity = source;
        effect->scrollWithBg = 0;
        effect->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}
