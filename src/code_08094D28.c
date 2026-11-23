#include "code_08009A0.h"
#include "lilith_entity.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Entity_ClampXToRoom(struct EwramData_EntityData *, s32);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s32);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern void Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);
extern void Lilith_SpawnShot(struct EwramData_EntityData *, s32, s32);

/**
 * @brief 94D28 | Enemy attack AI (state != 0 branch), all states damp velX
 * and apply gravity (Entity_MoveWithCollision): states 0-2 lunge in animation 7 (dust
 * via Afterimage_Create, sound 0x85; LILITH_LUNGE_HIT chains a second lunge), state 3
 * recovers for 30 frames; states 10-12 fire a projectile (Lilith_SpawnShot,
 * sound 0x7B) in animation 6. Both paths return to the walk animation 5.
 */
void Lilith_UpdateAttack(struct EwramData_EntityData *entity)
{
    s32 speed;

    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 7, 0, 0);
        entity->subState++;
        entity->accelX.whole = 0;
        entity->timer = 0;
    case 1:
        EntityDampVelocityX(entity, 0x18);
        Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x40000, 8);
        Entity_ClampXToRoom(entity, 4);
        if (entity->animStep <= 3)
            break;
        if (entity->flipX)
            entity->velX.whole = 0x30000;
        else
            entity->velX.whole = -0x30000;
        entity->subState++;
        entity->timer = 0;
        Afterimage_Create(entity, 0x38, 8, 0x10);
        Afterimage_Create(entity, 0x38, 4, 0x10);
        LILITH_LUNGE_HIT(entity) = 0;
        PlaySong(0x85);
        break;
    case 2:
        EntityDampVelocityX(entity, 0x18);
        speed = 0x10000;
        Entity_MoveWithCollision(entity, speed, -0x200000, 0x40000, 8);
        Entity_ClampXToRoom(entity, 4);
        if (*(s8 *)&LILITH_LUNGE_HIT(entity))
            entity->animTimer = 0x64;
        if (entity->animStep <= 0xC)
            break;
        entity->flipX = Entity_IsTargetRight(entity);
        if (*(s8 *)&LILITH_LUNGE_HIT(entity))
        {
            if (entity->flipX)
                entity->velX.whole = -0x20000;
            else
                entity->velX.whole = 0x20000;
            entity->subState++;
            entity->timer = 0;
            entity->accelX.whole = 0;
            entity->animTimer = 0;
            break;
        }
        entity->state = 0;
        entity->subState = 3;
        entity->timer = 0;
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
        entity->velY.whole = speed;
        entity->accelY.whole = 0x1800;
        Anim_StartIndexed(entity, 5, 0, 1);
        break;
    case 3:
        EntityDampVelocityX(entity, 0x18);
        speed = 0x10000;
        Entity_MoveWithCollision(entity, speed, -0x200000, 0x40000, 8);
        Entity_ClampXToRoom(entity, 4);
        if (entity->timer++ > 0x1D)
        {
            entity->state = 0;
            entity->subState = 2;
            entity->timer = 0;
            entity->velX.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x1800;
            LILITH_DODGE_CHECKED(entity) = 0;
            Anim_StartIndexed(entity, 5, 0, 1);
        }
        break;
    case 10:
        Anim_StartIndexed(entity, 6, 0, 0);
        entity->subState++;
        entity->accelX.whole = 0;
        entity->timer = 0;
    case 11:
        EntityDampVelocityX(entity, 0x18);
        Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x40000, 8);
        Entity_ClampXToRoom(entity, 4);
        if (entity->animStep > 4)
        {
            Lilith_SpawnShot(entity, 0, -0x1C0000);
            entity->subState++;
            PlaySong(0x7B);
        }
        break;
    case 12:
        EntityDampVelocityX(entity, 0x18);
        speed = 0x10000;
        Entity_MoveWithCollision(entity, speed, -0x200000, 0x40000, 8);
        Entity_ClampXToRoom(entity, 4);
        if (entity->animDone)
        {
            entity->flipX = Entity_IsTargetRight(entity);
            entity->state = 0;
            entity->subState = 2;
            entity->timer = 0;
            entity->velX.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x1800;
            Anim_StartIndexed(entity, 5, 0, 1);
        }
        break;
    }
}
