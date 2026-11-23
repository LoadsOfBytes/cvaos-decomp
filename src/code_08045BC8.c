/* Spawners 0x08045BC8..0x08045C9B. */
#include "code_08009A0.h"
#include "particle_effect.h"
#include "code_08032444.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"

extern void Effect_UpdateAttachedAnimation(void);
extern void SoulAcquisition_DrawAndWait(void);
extern void EntityUpdateNothing(void);
extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];

/**
 * @brief Spawn an animated child at the supplied parent-relative offsets and layer 12; do nothing if parent or entity allocation is NULL.
 */
void Effect_SpawnAttachedAnimation(struct EwramData_EntityData *parent, u8 anim, u8 palette, s32 offsetX, s32 offsetY)
{
    struct EwramData_EntityData *child;

    if (parent == NULL)
        return;
    child = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateAttachedAnimation);
    if (child == NULL)
        return;
    PARTICLE_ATTACH_ANIM(child) = anim;
    PARTICLE_ATTACH_PARENT(child) = parent;
    child->palette = palette;
    child->accelX.whole = offsetX;
    child->accelY.whole = offsetY;
    child->posX.whole = parent->posX.whole + offsetX;
    child->posY.whole = parent->posY.whole + offsetY;
    child->spriteLayer = 0xC;
}

/**
 * @brief 45C34 | Spawn an effect entity (update SoulAcquisition_DrawAndWait) using the
 * shared graphics 081C15F4/0820ED60 and play sound 0xBE.
 */
/* Declared non-void: the original epilogue returns through r1, but no value
 * is set (callers ignore it). */
/**
 * @brief Create the hidden acquisition effect with the soul-kind palette and 64-frame wait, then play sound 0xBE; preserve the unset return value and allocation assumption.
 */
s32 SoulAcquisition_CreateEffect(s32 kind)
{
    struct EwramData_EntityData *entity;
    s32 gfx;
    u8 *anim;

    entity = EntityCreateInRange(0x16, 0x19, (u32 *)EntityUpdateNothing);
    gfx = ObjGfx_Find(sCommonObjectGfx);
    anim = sCommonObjectSpriteSheet;
    Sprite_Attach(entity, anim, gfx, Soul_GetPickupPalette(kind));
    entity->hidden = 1;
    entity->drawFunc = (u32 *)SoulAcquisition_DrawAndWait;
    SOUL_ACQUIRE_DURATION(entity) = 0x40;
    PlaySong(0xBE);
}
