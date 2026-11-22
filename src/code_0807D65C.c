#include "code_08009A0.h"
#include "giant_ghost_entity.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Entity_AimVelocityAtPoint(struct EwramData_EntityData *entity, s32 x, s32 y, s32 speed);
extern void EntityMove(struct EwramData_EntityData *entity);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern void EntityDampVelocityY(struct EwramData_EntityData *, s32);

/**
 * @brief 7D65C | Swooping enemy attack (subState): backs away from the
 * player (Entity_AimVelocityAtPoint towards the player 0x10 px up, velocity negated)
 * accelerating unk_4F8 up to 0xC000, coasts, backs away decelerating, turns
 * to the player, then (song 0x87, animation 3) dives at the stored player
 * position (unk_4FC / unk_500, 0x20 px up, speed 2 px), brakes for 45
 * frames and returns to idle (state = 0) after 30 frames.
 */
void GiantGhost_UpdateAttack(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = gEwramData->bgInfo + 1;
    s32 x;

    switch (entity->subState)
    {
    case 0:
        if (GIANT_GHOST_SPEED(entity) < 0xC000)
            GIANT_GHOST_SPEED(entity) += 0x1000;
        x = Target_GetX(entity);
        x += bg->xPos.whole;
        Entity_AimVelocityAtPoint(entity, x, Target_GetY(entity) + bg->yPos.whole - 0x100000, GIANT_GHOST_SPEED(entity));
        entity->velX.whole = -entity->velX.whole;
        entity->velY.whole = -entity->velY.whole;
        EntityMove(entity);
        if (++entity->timer > 0x13)
        {
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 1:
        EntityMove(entity);
        if (++entity->timer > 0x13)
        {
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 2:
        if (GIANT_GHOST_SPEED(entity) > 0x1000)
            GIANT_GHOST_SPEED(entity) -= 0x800;
        x = Target_GetX(entity);
        x += bg->xPos.whole;
        Entity_AimVelocityAtPoint(entity, x, Target_GetY(entity) + bg->yPos.whole - 0x100000, GIANT_GHOST_SPEED(entity));
        entity->velX.whole = -entity->velX.whole;
        entity->velY.whole = -entity->velY.whole;
        EntityMove(entity);
        if (++entity->timer > 0x1D)
        {
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 3:
        entity->flipX = Entity_IsTargetRight(entity);
        if (++entity->timer > 0x13)
        {
            entity->subState++;
            entity->timer = 0;
            PlaySong(0x87);
            GIANT_GHOST_DIVE_X(entity) = Target_GetX(entity) + bg->xPos.whole;
            GIANT_GHOST_DIVE_Y(entity) = Target_GetY(entity) + bg->yPos.whole - 0x100000;
            Anim_StartIndexed(entity, 3, 0, 0);
        }
        break;
    case 4:
        if (Entity_AimVelocityAtPoint(entity, GIANT_GHOST_DIVE_X(entity), GIANT_GHOST_DIVE_Y(entity) - 0x200000, 0x20000))
        {
            entity->subState++;
            Anim_StartIndexed(entity, 4, 0, 1);
            entity->timer = 0;
        }
        EntityMove(entity);
        break;
    case 5:
        EntityDampVelocityX(entity, 8);
        EntityDampVelocityY(entity, 8);
        EntityMove(entity);
        if (entity->timer++ > 0x2C)
        {
            entity->subState++;
            Anim_StartIndexed(entity, 0, 0, 0);
            entity->timer = 0;
        }
        break;
    case 6:
        if (entity->timer++ > 0x1D)
        {
            entity->state = 0;
            entity->subState = 0;
            entity->timer = 0;
            GIANT_GHOST_HIT_PLAYER(entity) = 0;
        }
        break;
    }
}
