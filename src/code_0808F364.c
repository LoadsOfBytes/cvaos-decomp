#include "code_08009A0.h"
#include "dead_crusader_entity.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_IsInRect(s16, s16, s16, s16);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern s32 Entity_ClampXToRoom(struct EwramData_EntityData *, s32);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s32 Player_GetFocusedActionFlags(void);

/**
 * @brief 8F364 | Enemy walk AI like Persephone_UpdateWalk (subState): wakes on the
 * player box 0xB4 x 0x60 (or Enemy_IsDamaged), walks towards (state 1) / away
 * from (state 2) the player at 0.75 px, turning every 60 frames; landing
 * with Player_GetFocusedActionFlags & 0x360 makes it stop and crouch (animation 7, state 5,
 * unk_505 = 1) for 60 frames. Attacks (state = 1) at random when the
 * player is in the 0x50 box.
 */
void DeadCrusader_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 speed;
    s32 res;
    s32 found;
    s32 t;
    s32 m;

    switch (entity->subState)
    {
    case 0:
        if (Target_IsInRect(entity->posX.part16.integer - 0x5A, entity->posY.part16.integer - 0x30, 0xB4, 0x60) || Enemy_IsDamaged(entity))
        {
            entity->subState++;
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
        }
        break;
    case 1:
        if (entity->timer++ > 0x3B)
        {
            entity->timer = 0;
            entity->flipX = Entity_IsTargetRight(entity);
        }
        if (entity->flipX)
            entity->velX.whole = 0xC000;
        else
            entity->velX.whole = -0xC000;
        speed = 0x10000;
        res = Entity_MoveWithCollision(entity, speed, -0x200000, 0x80000, 8);
        if (!(res & 4))
            break;
        res = Player_GetFocusedActionFlags() & 0x360;
        if (res)
        {
            Anim_StartIndexed(entity, 7, 0, 0);
            entity->subState = 5;
            entity->timer = 0;
            DEAD_CRUSADER_GUARDING(entity) = 1;
            entity->velX.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x2800;
            break;
        }
        found = Target_IsInRect(entity->posX.part16.integer - 0x28, entity->posY.part16.integer - 0x28, 0x50, 0x50);
        if (found)
        {
            if ((RandomNumberGenerator() & 0x7F) <= 0x40)
            {
                entity->state = 1;
                entity->subState = res;
                entity->timer = res;
                break;
            }
            entity->subState = 2;
            entity->timer = res;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x2800;
            break;
        }
        res = Entity_ClampXToRoom(entity, 1);
        if (res)
        {
            entity->subState = 2;
            entity->timer = found;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x2800;
            break;
        }
        if (EntityIsOutsideScreen(entity, 4))
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->subState = res;
        }
        break;
    case 2:
        if (entity->timer++ > 0x3B)
        {
            entity->timer = 0;
            entity->flipX = Entity_IsTargetRight(entity);
        }
        if (entity->flipX)
            entity->velX.whole = -0xC000;
        else
            entity->velX.whole = 0xC000;
        res = Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x80000, 8);
        if (!(res & 4))
            break;
        t = Player_GetFocusedActionFlags() & 0x360;
        if (t)
        {
            Anim_StartIndexed(entity, 7, 0, 0);
            entity->subState = 5;
            entity->timer = 0;
            DEAD_CRUSADER_GUARDING(entity) = 1;
            entity->velX.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
            break;
        }
        if (!Target_IsInRect(entity->posX.part16.integer - 0x3C, entity->posY.part16.integer - 0x28, 0x78, 0x50) || entity->timer > 0x3B)
        {
            entity->flipX = Entity_IsTargetRight(entity);
            entity->subState = 1;
            entity->timer = t;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
            break;
        }
        if (Entity_ClampXToRoom(entity, 1) || (m = res & 3) != 0)
        {
            entity->flipX = Entity_IsTargetRight(entity);
            entity->subState = 1;
            entity->timer = t;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
            break;
        }
        if (Target_IsInRect(entity->posX.part16.integer - 0x28, entity->posY.part16.integer - 0x28, 0x50, 0x50) && entity->timer > 0x3B
            && (RandomNumberGenerator() & 0x7F) <= 0x40)
        {
            entity->state = 1;
            entity->subState = m;
            entity->timer = m;
        }
        break;
    case 5:
        entity->flipX = Entity_IsTargetRight(entity);
        DEAD_CRUSADER_GUARDING(entity) = 1;
        speed = 0x10000;
        res = Entity_MoveWithCollision(entity, speed, -0x200000, 0x80000, 8);
        if (!(res & 4))
            break;
        if (entity->timer++ > 0x3B)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->subState = 1;
            entity->timer = 0;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x2800;
        }
        break;
    }
}
