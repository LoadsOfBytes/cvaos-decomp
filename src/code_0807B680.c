#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_IsInRect(s16, s16, s16, s16);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern s32 Entity_ClampXToRoom(struct EwramData_EntityData *, s32);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s32);

/**
 * @brief 7B680 | Enemy walk/hop AI (subState): wake when the player is in a
 * 0xF0 x 0x60 box around it (or Enemy_IsDamaged), animation 1 with song 0x1C9
 * on frame 5, hop towards the player (velX +-1 px, velY 1 px, gravity
 * 0x2800), land and attack (state = 1) when the player is close; states
 * 5 / 6 walk towards / away from the player at half speed and switch on the
 * player box (Target_IsInRect), a wall (Entity_ClampXToRoom) or randomly.
 */
void Persephone_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 found;
    s32 r;
    s32 speed;
    s32 found2;

    switch (entity->subState)
    {
    case 0:
        if (Target_IsInRect(entity->posX.part16.integer - 0x78, entity->posY.part16.integer - 0x30, 0xF0, 0x60) || Enemy_IsDamaged(entity))
        {
            entity->subState++;
            entity->flipX = Entity_IsTargetRight(entity);
            Anim_StartIndexed(entity, 1, 0, 0);
            entity->timer = 0;
        }
        break;
    case 1:
        if (++entity->timer == 5)
        {
            if (entity->unk_514.part16.spawnParam0)
                PlaySong(0x1C9);
            else if ((RandomNumberGenerator() & 0x7F) < 0x40)
                PlaySong(0x1C9);
        }
        if (entity->animDone)
        {
            entity->subState++;
            entity->flipX = Entity_IsTargetRight(entity);
            Anim_StartIndexed(entity, 2, 0, 1);
            if (entity->flipX)
                entity->velX.whole = 0x10000;
            else
                entity->velX.whole = -0x10000;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
            entity->timer = 0;
        }
        break;
    case 2:
        if (entity->timer++ > 0x1D)
        {
            entity->flipX = Entity_IsTargetRight(entity);
            if (entity->flipX)
                entity->velX.whole = 0x10000;
            else
                entity->velX.whole = -0x10000;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
            entity->timer = 0;
        }
        if (Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x80000, 8) & 4)
        {
            if (Target_IsInRect(entity->posX.part16.integer - 0x28, entity->posY.part16.integer - 0x30, 0x50, 0x60))
            {
                entity->subState++;
                Anim_StartIndexed(entity, 3, 0, 0);
            }
        }
        break;
    case 3:
        if (entity->animDone)
        {
            entity->state = 1;
            entity->subState = 0;
            entity->timer = 0;
        }
        break;
    case 5:
        if (entity->timer++ > 0x1D)
        {
            entity->flipX = Entity_IsTargetRight(entity);
            entity->timer = 0;
        }
        if (entity->flipX)
            entity->velX.whole = 0x8000;
        else
            entity->velX.whole = -0x8000;
        speed = 0x10000;
        Entity_MoveWithCollision(entity, speed, -0x200000, 0x80000, 8);
        found = Target_IsInRect(entity->posX.part16.integer - 0x28, entity->posY.part16.integer - 0x30, 0x50, 0x60);
        if (found)
        {
            if (!(RandomNumberGenerator() & 1))
            {
                entity->state = 1;
                entity->subState = 0;
                entity->timer = 0;
                break;
            }
            entity->subState = 6;
            entity->timer = 0;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x2800;
            break;
        }
        r = Entity_ClampXToRoom(entity, 1);
        if (r)
        {
            entity->subState = 6;
            entity->timer = found;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x2800;
            break;
        }
        if (EntityIsOutsideScreen(entity, 4))
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->subState = r;
        }
        break;
    case 6:
        entity->flipX = Entity_IsTargetRight(entity);
        if (entity->flipX)
            entity->velX.whole = -0x8000;
        else
            entity->velX.whole = 0x8000;
        speed = 0x10000;
        Entity_MoveWithCollision(entity, speed, -0x200000, 0x80000, 8);
        found2 = Target_IsInRect(entity->posX.part16.integer - 0x3C, entity->posY.part16.integer - 0x30, 0x78, 0x60);
        if (!found2)
        {
            entity->subState = 5;
            entity->timer = found2;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x2800;
            break;
        }
        if (Entity_ClampXToRoom(entity, 1))
        {
            entity->subState = 5;
            entity->timer = 0;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x2800;
            break;
        }
        if (Target_IsInRect(entity->posX.part16.integer - 0x28, entity->posY.part16.integer - 0x30, 0x50, 0x60) && !(RandomNumberGenerator() & 1))
        {
            entity->state = 1;
            entity->subState = 0;
            entity->timer = 0;
        }
        break;
    }
}
