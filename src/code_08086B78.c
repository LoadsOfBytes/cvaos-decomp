/* Triton movement (Triton_UpdateMove) and attack: fire the homing shot, or raise the spear and emit wave
 * particles until the spear comes back. */
#include "code_08009A0.h"
#include "triton_entity.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s16);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern void EntityDampVelocityY(struct EwramData_EntityData *, s32);
extern void Triton_SpawnHomingShot(struct EwramData_EntityData *, s32, s32);
extern void Triton_SpawnWaveParticle(struct EwramData_EntityData *, s32, s32);
extern s32 Entity_MoveAndCollide(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s32 Collision_FindWaterSurfaceOffset(s32, s32);
extern s32 Target_IsWithinDistance(struct EwramData_EntityData *, s32);
extern s32 Tile_Classify(s16, s16);
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
#define NEXT_STATE(e) (TRITON_NEXT_STATE((e)))
#define ACCEL_X(e) ((e)->accelX.whole)
#define ACCEL_Y(e) ((e)->accelY.whole)
/* Sink slowly in water (tile type 3), faster elsewhere. */
/* Triton movement: wait for the target in range, walk/swim towards it,
 * turn around, and switch to the attack (state 1) when close enough. */
void Triton_UpdateMove(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    s32 flags;
    s32 range;
    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        NEXT_STATE(entity) = 0;
        flags = Entity_MoveAndCollide(entity, 0x1C0000, -0x1B0000, 0, 0x40000);
        if (!(flags & 0x80))
            entity->posY.part16.integer += Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer - 27);
        range = 0x5A0000;
        goto check_range;
    case 1:
        entity->flipX = Entity_IsTargetRight(entity);
        if (entity->flipX) ACCEL_X(entity) = 0x400;
        else ACCEL_X(entity) = -0x400;
        flags = Entity_MoveAndCollide(entity, 0x1C0000, -0x1B0000, 0, 0x10000);
        if (!(flags & 0x80))
            entity->posY.part16.integer += Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer - 27);
        break;
    case 10:
        NEXT_STATE(entity) = 10;
        if (entity->flipX != Entity_IsTargetRight(entity)) goto turn;
        range = 0x640000;
    check_range:
        if (Target_IsWithinDistance(entity, range) || Enemy_IsDamaged(entity)) entity->subState++;
        break;
    case 11:
        if (entity->flipX != Entity_IsTargetRight(entity))
        {
        turn:
            entity->subState = 12;
            break;
        }
        if (entity->flipX) ACCEL_X(entity) = 0x400;
        else ACCEL_X(entity) = -0x400;
        flags = Entity_MoveWithCollision(entity, 0x10000, 0, 0x8000, 32);
        if (entity->timer++ > 59)
        {
            entity->timer = 0;
            if (Target_IsWithinDistance(entity, 0x640000) && RandomNumberGenerator() % 128 < 96 && (flags & 4))
            {
                entity->state = 1;
                entity->subState = 0;
                break;
            }
        }
        /* sink slowly in water (tile type 3), faster elsewhere */
        if (Tile_Classify((s16)(camera->xPos.part16.integer + entity->posX.part16.integer),
                         (s16)(camera->yPos.part16.integer + entity->posY.part16.integer)) == 3)
            ACCEL_Y(entity) = 0x800;
        else
            ACCEL_Y(entity) = 0x2000;
        break;
    case 12:
        Anim_StartIndexed(entity, 5, 0, 0);
        entity->subState++;
        ACCEL_X(entity) = 0;
        ACCEL_Y(entity) = 0;
        EntityDampVelocityX(entity, 0x10);
        EntityDampVelocityY(entity, 0x10);
        Entity_MoveWithCollision(entity, 0x10000, 0, 0x8000, 32);
        /* sink slowly in water (tile type 3), faster elsewhere */
        if (Tile_Classify((s16)(camera->xPos.part16.integer + entity->posX.part16.integer),
                         (s16)(camera->yPos.part16.integer + entity->posY.part16.integer)) == 3)
            ACCEL_Y(entity) = 0x800;
        else
            ACCEL_Y(entity) = 0x2000;
        break;
    case 13:
        EntityDampVelocityX(entity, 0x10);
        EntityDampVelocityY(entity, 0x10);
        Entity_MoveWithCollision(entity, 0x10000, 0, 0x8000, 32);
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->flipX = ~entity->flipX;
            entity->subState = 11;
        }
        /* sink slowly in water (tile type 3), faster elsewhere */
        if (Tile_Classify((s16)(camera->xPos.part16.integer + entity->posX.part16.integer),
                         (s16)(camera->yPos.part16.integer + entity->posY.part16.integer)) == 3)
            ACCEL_Y(entity) = 0x800;
        else
            ACCEL_Y(entity) = 0x2000;
        break;
    }
}

#define FRAME_PAIR(e) (*(u16 *)&(e)->animStep)
#define SPEAR_FLAG(e) (*(u8 *)&TRITON_THRUST((e)))
/**
 * @brief Attack state machine (state 1): randomly thrust the spear, fire a homing
 * shot (Triton_SpawnHomingShot) or send wave particles
 * (Triton_SpawnWaveParticle); then back to state 0.
 */
void Triton_UpdateAttack(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *spear = TRITON_SPEAR(entity);
    switch (entity->subState)
    {
    case 0:
        Entity_MoveWithCollision(entity, 0x10000, 0, 0x8000, 32);
        if (RandomNumberGenerator() % 128 < 32)
        {
            entity->subState = 10;
            entity->accelX.whole = 0;
            entity->accelY.whole = 0;
        }
        else
            entity->subState++;
        break;
    case 1:
        Entity_MoveWithCollision(entity, 0x10000, 0, 0x8000, 32);
        if (FRAME_PAIR(entity) == 1)
        {
            if (entity->flipX) Triton_SpawnHomingShot(entity, -0xC0000, -0xA0000);
            else Triton_SpawnHomingShot(entity, 0xE0000, -0xA0000);
            entity->subState = 2;
        }
        break;
    case 2:
        Entity_MoveWithCollision(entity, 0x10000, 0, 0x8000, 32);
        goto finish;
    case 10:
        EntityDampVelocityX(entity, 0x20);
        EntityDampVelocityY(entity, 0x20);
        Entity_MoveWithCollision(entity, 0x10000, 0, 0x8000, 32);
        if (FRAME_PAIR(entity) == 0)
        {
            Anim_StartIndexed(entity, 3, 0, 0);
            entity->timer = 0;
            entity->subState++;
        }
        break;
    case 11:
        EntityDampVelocityX(entity, 0x20);
        EntityDampVelocityY(entity, 0x20);
        Entity_MoveWithCollision(entity, 0x10000, 0, 0x8000, 32);
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 2, 0, 1);
            entity->timer = 0;
            entity->subState++;
            PlaySong(0x8A);
        }
        break;
    case 12:
        EntityDampVelocityX(entity, 0x20);
        EntityDampVelocityY(entity, 0x20);
        Entity_MoveWithCollision(entity, 0x10000, 0, 0x8000, 32);
        if (entity->timer++ > 29)
        {
            TRITON_THRUST(entity) = 1;
            TRITON_THRUST(spear) = 0;
            entity->subState++;
        }
        break;
    case 13:
        EntityDampVelocityX(entity, 0x20);
        EntityDampVelocityY(entity, 0x20);
        Entity_MoveWithCollision(entity, 0x10000, 0, 0x8000, 32);
        entity->timer++;
        if ((entity->timer & 0x1F) == 0)
        {
            PlaySong(0x8A);
            if (entity->flipX) Triton_SpawnWaveParticle(entity, 0xE0000, -0x3A0000);
            else Triton_SpawnWaveParticle(entity, -0xC0000, -0x3A0000);
        }
        if (SPEAR_FLAG(spear))
        {
            SPEAR_FLAG(entity) = 0;
            Anim_StartIndexed(entity, 4, 0, 0);
            entity->subState++;
        }
        break;
    case 14:
        EntityDampVelocityX(entity, 0x20);
        EntityDampVelocityY(entity, 0x20);
        Entity_MoveWithCollision(entity, 0x10000, 0, 0x8000, 32);
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
        finish:
            entity->state = 0;
            entity->subState = NEXT_STATE(entity);
            entity->timer = 0;
        }
        break;
    }
}
