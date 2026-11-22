/* Object effect helpers decompiled from assembly. */
#include "structs/ewram.h"
#include "code_08009A0.h"
#include "code/code_08039340.h"

extern u8 sObject17Gfx[], sUnk_08209FD8[], sObject17SpriteSheet[];
extern s32 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);

/**
 * @brief Wall-left correction slot of the object 0x17 solid table (0x08118598):
 * none, return 0.
 */
s32 Object17_WallLeftCorrection(void)
{
    return 0;
}

/**
 * @brief Wall-right correction slot of the object 0x17 solid table
 * (0x08118598): none, return 0.
 */
s32 Object17_WallRightCorrection(void)
{
    return 0;
}

/* Load the effect once, then advance its animation on every update. */
/**
 * @brief Load the effect once (animation subState), then advance it every
 * update.
 */
void Object17Effect_Update(struct EwramData_EntityData *entity)
{
    if (entity->state == 0)
    {
        if (!(u8)Entity_LoadGraphics(entity, sObject17Gfx, sUnk_08209FD8, 4, sObject17SpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        entity->spriteLayer = 0x11;
        Anim_StartIndexed(entity, entity->subState, 0, 1);
        entity->state++;
    }
    Anim_Update(entity);
}

/* Follow the entity stored in unk_4F8 and advance the animation. */
/**
 * @brief Follow the entity in unk_4F8 and advance the animation.
 */
void Object17Follower_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->unk_4F8.unk_4F8_entity;
    entity->posX.whole = parent->posX.whole;
    entity->posY.whole = parent->posY.whole;
    Anim_Update(entity);
}
