#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_GetFlipXOrOpposite(struct EwramData_EntityData *entity);
extern s32 Target_IsInRect(s16, s16, s16, s16);
extern s32 Entity_ClampXToRoom(struct EwramData_EntityData *, s32);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s32 Player_GetFocusedActionFlags(void);
extern void Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);

/**
 * @brief 948DC | Enemy AI like Succubus_UpdateWalk driven by acceleration: wakes
 * (animation 4, visible, song 0x1C8) on the player box 0x64 x 0x50, then
 * walks towards (state 2) / away from (state 3) the player with accelX
 * +-0x800, lunging after a landing like Succubus_UpdateWalk; every 60 frames of
 * walking (unk_4FC) a distant player (0xC8 px) triggers attack 10
 * (state = 1), a close one attack 0.
 */
void Lilith_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 speed;
    s32 speed2;
    s32 speed3;
    s32 found;
    s32 found2;
    s32 land;
    s32 wall;

    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        if (Target_IsInRect(entity->posX.part16.integer - 0x32, entity->posY.part16.integer - 0x28, 0x64, 0x50))
        {
            entity->subState++;
            Anim_StartIndexed(entity, 4, 0, 0);
            entity->hidden = 0;
            PlaySong(0x1C8);
        }
        break;
    case 1:
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 5, 0, 1);
            entity->subState++;
            entity->timer = 0;
            entity->ext.lilith.hidden = 0;
        }
        break;
    case 2:
        if (entity->timer++ > 0x3B)
        {
            entity->flipX = Entity_IsTargetRight(entity);
            entity->timer = 0;
        }
        if (entity->flipX == (s8)Target_GetFlipXOrOpposite(entity)
            && Target_IsInRect(entity->posX.part16.integer - 0x32, entity->posY.part16.integer - 0x28, 0x64, 0x50))
        {
            land = Player_GetFocusedActionFlags() & 0x60;
            if (!land)
            {
                entity->ext.lilith.dodgeChecked = land;
            }
            else if (!*(s8 *)&entity->ext.lilith.dodgeChecked)
            {
                if (!(RandomNumberGenerator() & 1))
                {
                    if (entity->flipX)
                        entity->velX.whole = -0x28000;
                    else
                        entity->velX.whole = 0x28000;
                    entity->subState = 4;
                    entity->timer = 0;
                    entity->accelX.whole = 0;
                    Afterimage_Create(entity, 0x38, 0xC, 0x10);
                    Afterimage_Create(entity, 0x38, 6, 0x10);
                    if ((RandomNumberGenerator() & 0x7F) <= 0x2F)
                        PlaySong(0x1C8);
                    break;
                }
                entity->ext.lilith.dodgeChecked = 1;
            }
        }
        if (entity->flipX)
            entity->accelX.whole = 0x800;
        else
            entity->accelX.whole = -0x800;
        speed = 0x10000;
        Entity_MoveWithCollision(entity, speed, -0x200000, 0x6000, 8);
        found = Target_IsInRect(entity->posX.part16.integer - 0x28, entity->posY.part16.integer - 0x28, 0x50, 0x50);
        if (found)
        {
            if (!(RandomNumberGenerator() & 1))
            {
                entity->state = 1;
                entity->subState = 0;
                entity->timer = 0;
                break;
            }
            entity->subState = 3;
            entity->timer = 0;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x1800;
            break;
        }
        wall = Entity_ClampXToRoom(entity, 4);
        if (wall)
        {
            entity->subState = 3;
            entity->timer = found;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x1800;
            break;
        }
        if (entity->ext.lilith.walkFrame++ > 0x3B)
        {
            entity->ext.lilith.walkFrame = wall;
            goto far;
        }
        break;
    case 3:
        entity->flipX = Entity_IsTargetRight(entity);
        if (entity->flipX == (s8)Target_GetFlipXOrOpposite(entity)
            && Target_IsInRect(entity->posX.part16.integer - 0x32, entity->posY.part16.integer - 0x28, 0x64, 0x50))
        {
            land = Player_GetFocusedActionFlags() & 0x60;
            if (!land)
            {
                entity->ext.lilith.dodgeChecked = land;
            }
            else if (!*(s8 *)&entity->ext.lilith.dodgeChecked)
            {
                if (!(RandomNumberGenerator() & 1))
                {
                    if (entity->flipX)
                        entity->velX.whole = -0x28000;
                    else
                        entity->velX.whole = 0x28000;
                    entity->subState = 4;
                    entity->timer = 0;
                    entity->accelX.whole = 0;
                    Afterimage_Create(entity, 0x38, 0xC, 0x10);
                    Afterimage_Create(entity, 0x38, 6, 0x10);
                    break;
                }
                entity->ext.lilith.dodgeChecked = 1;
            }
        }
        if (entity->flipX)
            entity->accelX.whole = -0x800;
        else
            entity->accelX.whole = 0x800;
        speed2 = 0x10000;
        Entity_MoveWithCollision(entity, speed2, -0x200000, 0x6000, 8);
        found2 = Target_IsInRect(entity->posX.part16.integer - 0x46, entity->posY.part16.integer - 0x28, 0x8C, 0x50);
        if (!found2)
        {
            if (!(RandomNumberGenerator() & 1))
            {
                entity->subState = 2;
                entity->timer = found2;
                entity->velY.whole = speed2;
                entity->accelY.whole = 0x1800;
                break;
            }
            entity->state = 1;
            entity->subState = 10;
            entity->timer = found2;
            break;
        }
        if (Entity_ClampXToRoom(entity, 4))
        {
            entity->subState = 2;
            entity->timer = 0;
            entity->velY.whole = speed2;
            entity->accelY.whole = 0x1800;
            break;
        }
        if (Target_IsInRect(entity->posX.part16.integer - 0x28, entity->posY.part16.integer - 0x28, 0x50, 0x50) && !(RandomNumberGenerator() & 1))
        {
            entity->state = 1;
            entity->subState = 0;
            entity->timer = 0;
            break;
        }
        if (entity->ext.lilith.walkFrame++ > 0x3B)
        {
            entity->ext.lilith.walkFrame = 0;
        far:
            if (Target_IsInRect(entity->posX.part16.integer - 0xC8, entity->posY.part16.integer - 0x28, 0x190, 0x50))
            {
                s32 r = RandomNumberGenerator() & 1;
                if (!r)
                {
                    entity->state = 1;
                    entity->subState = 10;
                    entity->timer = r;
                }
            }
        }
        break;
    case 4:
        EntityDampVelocityX(entity, 0x18);
        speed3 = 0x10000;
        Entity_MoveWithCollision(entity, speed3, -0x200000, 0x40000, 8);
        Entity_ClampXToRoom(entity, 4);
        if (entity->timer++ > 0x27)
        {
            entity->subState = 2;
            entity->timer = 0;
            entity->velX.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = speed3;
            entity->accelY.whole = 0x1800;
            entity->ext.lilith.dodgeChecked = 0;
        }
        break;
    }
}
