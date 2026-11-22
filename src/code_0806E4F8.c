/* Effect helpers 0x0806E4F8..0x0806E5FB. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];

/**
 * @brief 6E4F8 | Hide (0x53C bit 5) while above the y minY, show
 * otherwise.
 */
void Entity_HideAboveY(struct EwramData_EntityData *entity, s32 minY)
{
    if ((s16)entity->posY.part16.integer < minY)
        entity->hidden = 1;
    else
        entity->hidden = 0;
}

/**
 * @brief 6E524 | Rising effect: state 0 takes the loaded graphics
 * sCommonObjectGfx (palette 7, animation 0); then rise 0.5 px per frame;
 * deleted when the animation ends.
 */
void RisingEffect_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
        {
            s32 gfx = ObjGfx_Find(sCommonObjectGfx);

            Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, 7);
            Anim_StartIndexed(entity, 0, 0, 0);
            entity->scrollWithBg = 1;
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
            entity->state++;
            break;
        }
        case 1:
            entity->posY.whole -= 0x8000;
            break;
    }
    if (Anim_Update(entity) == 3)
        EntityDelete(entity);
}

/**
 * @brief Increment the timer and accelerate/move with limit 0x80000,
 * then delete after timer 120. The creator supplies gravity;
 * this callback does not advance animation.
 */
void Effect_UpdateFallingFrameParticle(struct EwramData_EntityData *entity)
{
    entity->timer++;
    EntityAccelerateAndMove(entity, 0x80000);
    if (entity->timer > 0x78)
        EntityDelete(entity);
}

/**
 * @brief Advance animation and delete when Anim_Update returns 3.
 * Installed by the stationary animation factory.
 */
void Effect_UpdateStationaryAnimation(struct EwramData_EntityData *entity)
{
    if (Anim_Update(entity) == 3)
        EntityDelete(entity);
}

/**
 * @brief 6E5DC | Deleted when the timer timer runs out.
 */
void TimedEntity_Update(struct EwramData_EntityData *entity)
{
    u8 var_0[0x10];

    if ((s8)--entity->timer <= 0)
        EntityDelete(entity);
}

