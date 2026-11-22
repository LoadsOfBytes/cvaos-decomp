/* Flying enemy that circles the player: approach / coast / brake, and after
 * being hit (unk_505) the same three phases away from the player. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_IsInRect(s16, s16, s16, s16);
extern s32 Entity_AimVelocityAtPoint(struct EwramData_EntityData *entity, s32 x, s32 y, s32 speed);
extern void EntityMove(struct EwramData_EntityData *entity);

#define SPEED(e) ((e)->unk_4F8.whole)
#define HIT(e) ((e)->unk_504.part8.unk_505)

/* Aim the velocity at a point 0x30 px above the player.  `px` must be a
 * block-local variable: it lives inside one basic block, so local-alloc
 * gives it r4 before the entity pointer is allocated (entity ends up in r5). */
#define AIM_AT_PLAYER(e)                                                                         \
    do {                                                                                         \
        s32 px = Target_GetX(e);                                                                \
        px += bg->xPos.whole;                                                                    \
        Entity_AimVelocityAtPoint(e, px, Target_GetY(e) + bg->yPos.whole - 0x300000, SPEED(e));              \
    } while (0)

/**
 * @brief 7D33C | Movement state machine (subState): 0 choose, 1 accelerate
 * towards the player, 2 coast, 3 brake; 10-13 the same sequence flying away
 * after a hit.  The sprite is hidden while it is vertically off screen.
 */
void GiantGhost_UpdateMove(struct EwramData_EntityData *entity)
{
    s32 hit1;
    s32 hit3;
    s32 hit13;
    s32 x;
    struct BgLayerState *bg = gEwramData->bgInfo + 1;

    if ((u16)(entity->posY.part16.integer + 0x1C) > 0xDC)
        entity->hidden = 1;
    else
        entity->hidden = 0;
    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        x = RandomNumberGenerator() % 3;
        if (x == 0 && Target_IsInRect(entity->posX.part16.integer - 0x50, entity->posY.part16.integer - 0x50, 0xA0, 0xA0))
        {
            entity->state = 1;
            entity->subState = x;
            SPEED(entity) = x;
            Anim_StartIndexed(entity, 0, 0, 0);
        }
        else
        {
            entity->timer = 0;
            entity->subState++;
            HIT(entity) = 0;
            Anim_StartIndexed(entity, 3, 0, 0);
        }
        break;
    case 1:
        hit1 = (s8)HIT(entity);
        if (hit1)
        {
            entity->subState = 10;
            HIT(entity) = 0;
            Anim_StartIndexed(entity, 2, 0, 1);
            break;
        }
        if (SPEED(entity) < 0xC000)
            SPEED(entity) += 0x1000;
        AIM_AT_PLAYER(entity);
        EntityMove(entity);
        if (++entity->timer > 0x13)
        {
            entity->subState++;
            entity->timer = hit1;
        }
        break;
    case 2:
        x = (s8)HIT(entity);
        if (x)
        {
            entity->subState = 10;
            HIT(entity) = 0;
            Anim_StartIndexed(entity, 2, 0, 1);
            break;
        }
        EntityMove(entity);
        if (++entity->timer > 0x13)
        {
            entity->subState++;
            entity->timer = x;
            Anim_StartIndexed(entity, 0, 0, 1);
        }
        break;
    case 3:
        hit3 = (s8)HIT(entity);
        if (hit3)
        {
            entity->subState = 10;
            HIT(entity) = 0;
            Anim_StartIndexed(entity, 2, 0, 1);
            break;
        }
        if (SPEED(entity) > 0x1000)
            SPEED(entity) -= 0x800;
        AIM_AT_PLAYER(entity);
        EntityMove(entity);
        if (++entity->timer > 0x1D)
        {
            entity->subState = hit3;
            entity->timer = hit3;
        }
        break;
    case 10:
        SPEED(entity) = 0;
        entity->subState++;
        entity->timer = 0;
    case 11:
        if (SPEED(entity) < 0xC000)
            SPEED(entity) += 0x1000;
        AIM_AT_PLAYER(entity);
        entity->velX.whole = -entity->velX.whole;
        entity->velY.whole = -SPEED(entity);
        EntityMove(entity);
        if (++entity->timer > 0x13)
        {
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 12:
        x = (s8)HIT(entity);
        if (x)
        {
            entity->subState = 10;
            HIT(entity) = 0;
            break;
        }
        EntityMove(entity);
        if (++entity->timer > 0x13)
        {
            entity->subState++;
            entity->timer = x;
            Anim_StartIndexed(entity, 0, 0, 1);
        }
        break;
    case 13:
        hit13 = (s8)HIT(entity);
        if (hit13)
        {
            entity->subState = 10;
            HIT(entity) = 0;
            break;
        }
        if (SPEED(entity) > 0x1000)
            SPEED(entity) -= 0x800;
        AIM_AT_PLAYER(entity);
        entity->velX.whole = -entity->velX.whole;
        entity->velY.whole = -entity->velY.whole;
        EntityMove(entity);
        if (++entity->timer > 0x1D)
        {
            entity->subState = hit13;
            entity->timer = hit13;
        }
        break;
    }
}
