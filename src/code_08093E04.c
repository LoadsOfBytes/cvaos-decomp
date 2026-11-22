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
 * @brief 93E04 | Enemy AI (subState): growls (song 0x1C7) once the player is
 * near and wakes when unk_4FC is set or the player is in front; walks
 * towards (state 2) / away from (state 3) the player at 0.375 px, turning
 * every 30 frames. Facing the player (Target_GetFlipXOrOpposite) within 0x32 px, after a
 * landing (Player_GetFocusedActionFlags & 0x60) it may lunge (velX 2.5 px, two dust puffs
 * via Afterimage_Create, state 4 for 40 frames) once per landing (unk_4FD);
 * attacks (state = 1) at random when the player is in the 0x50 box.
 */
void Succubus_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 speed;
    s32 speed2;
    s32 speed3;
    s32 found;
    s32 found2;
    s32 land;
    s32 facing;

    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        if (!*(s8 *)&entity->unk_504.part8.unk_505
            && Target_IsInRect(entity->posX.part16.integer - 0x5A, entity->posY.part16.integer - 0x40, 0xB4, 0x46))
        {
            entity->unk_504.part8.unk_505 = 1;
            PlaySong(0x1C7);
        }
        if (entity->unk_4FC.part8.unk_4FC == 1
            || Target_IsInRect(entity->posX.part16.integer - 0x1E, entity->posY.part16.integer - 0x40, 0x3C, 0x46))
        {
            entity->subState++;
            Anim_StartIndexed(entity, 0, 0, 0);
        }
        break;
    case 1:
        if (*(u16 *)&entity->animStep == 5)
            PlaySong(0x1C8);
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->subState++;
            entity->timer = 0;
            entity->unk_4FC.part8.unk_4FE = 0;
            entity->unk_4FC.part8.unk_4FF = 0;
        }
        break;
    case 2:
        if (entity->timer++ > 0x1D)
        {
            entity->timer = 0;
            entity->flipX = Entity_IsTargetRight(entity);
            if (entity->flipX)
                entity->velX.whole = 0x6000;
            else
                entity->velX.whole = -0x6000;
        }
        if (entity->flipX == (s8)Target_GetFlipXOrOpposite(entity)
            && Target_IsInRect(entity->posX.part16.integer - 0x32, entity->posY.part16.integer - 0x50, 0x64, 0xA0))
        {
            land = Player_GetFocusedActionFlags() & 0x60;
            if (!land)
            {
                entity->unk_4FC.part8.unk_4FD = land;
            }
            else if (!*(s8 *)&entity->unk_4FC.part8.unk_4FD)
            {
                if (!(RandomNumberGenerator() & 1))
                {
                    if (entity->flipX)
                        entity->velX.whole = -0x28000;
                    else
                        entity->velX.whole = 0x28000;
                    entity->subState = 4;
                    entity->timer = 0;
                    Afterimage_Create(entity, 0x38, 0xC, 0x10);
                    Afterimage_Create(entity, 0x38, 6, 0x10);
                    if ((RandomNumberGenerator() & 0x7F) <= 0x2F)
                        PlaySong(0x1C8);
                    break;
                }
                entity->unk_4FC.part8.unk_4FD = 1;
            }
        }
        speed = 0x10000;
        Entity_MoveWithCollision(entity, speed, -0x200000, 0x40000, 8);
        found = Target_IsInRect(entity->posX.part16.integer - 0x28, entity->posY.part16.integer - 0x50, 0x50, 0xA0);
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
        if (Entity_ClampXToRoom(entity, 4))
        {
            entity->subState = 3;
            entity->timer = found;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x1800;
        }
        break;
    case 3:
        if (entity->timer++ > 0x1D)
        {
            entity->timer = 0;
            entity->flipX = Entity_IsTargetRight(entity);
            if (entity->flipX)
                entity->velX.whole = -0x6000;
            else
                entity->velX.whole = 0x6000;
        }
        if (entity->flipX == (s8)Target_GetFlipXOrOpposite(entity)
            && Target_IsInRect(entity->posX.part16.integer - 0x32, entity->posY.part16.integer - 0x50, 0x64, 0xA0))
        {
            land = Player_GetFocusedActionFlags() & 0x60;
            if (!land)
            {
                entity->unk_4FC.part8.unk_4FD = land;
            }
            else if (!*(s8 *)&entity->unk_4FC.part8.unk_4FD)
            {
                if (!(RandomNumberGenerator() & 1))
                {
                    if (entity->flipX)
                        entity->velX.whole = -0x28000;
                    else
                        entity->velX.whole = 0x28000;
                    entity->subState = 4;
                    entity->timer = 0;
                    Afterimage_Create(entity, 0x38, 0xC, 0x10);
                    Afterimage_Create(entity, 0x38, 6, 0x10);
                    break;
                }
                entity->unk_4FC.part8.unk_4FD = 1;
            }
        }
        speed2 = 0x10000;
        Entity_MoveWithCollision(entity, speed2, -0x200000, 0x40000, 8);
        found2 = Target_IsInRect(entity->posX.part16.integer - 0x46, entity->posY.part16.integer - 0x50, 0x8C, 0xA0);
        if (!found2)
        {
            entity->subState = 2;
            entity->timer = found2;
            entity->velY.whole = speed2;
            entity->accelY.whole = 0x1800;
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
        if (Target_IsInRect(entity->posX.part16.integer - 0x28, entity->posY.part16.integer - 0x50, 0x50, 0xA0) && !(RandomNumberGenerator() & 1))
        {
            entity->state = 1;
            entity->subState = 0;
            entity->timer = 0;
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
            entity->velY.whole = speed3;
            entity->accelY.whole = 0x1800;
            entity->unk_4FC.part8.unk_4FD = 0;
        }
        break;
    }
}
