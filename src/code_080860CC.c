/* Headhunter lightning-burst flicker. The creator does not allocate an
 * affine slot; retain the existing affine-free call on owner cancel. */
#include "code_08009A0.h"
#include "headhunter_entity.h"
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
/* bit 0 of the owner's unk_4FE ends its effects (TODO: meaning) */
#define OWNER_DONE(o) (HEADHUNTER_FLAGS((o)) & 1)

/**
 * @brief Randomize among three frames through signed timer 49, then
 * remove. Preserve the existing affine-free call only on the owner
 * cancel branch; the creator does not allocate an affine slot.
 */
void HeadhunterLightningBurstFlicker_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    struct SingleFrameAnimation animation = sHeadhunterFrameAnim;
    Anim_Start(entity, (struct AnimScript *)&animation, 2, 1);
    animation.frame = HEADHUNTER_EFFECT_FRAME(entity) + RandomNumberGenerator() % 3;
    Anim_Update(entity);
    if ((s8)entity->timer++ > 49)
        EntityDelete(entity);
    else if (OWNER_DONE(owner))
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
}
