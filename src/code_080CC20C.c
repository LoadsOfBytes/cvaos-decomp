/* Buer collision motion, rotation, and update dispatcher. */
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_Walk(struct EwramData_EntityData *entity,s32 speed,void *profile,s32 flags);
extern s32 Entity_IsOffsetOutsideRoom(struct EwramData_EntityData *entity,s32 x,s32 y);
extern void Buer_UpdateState4Rotation(struct EwramData_EntityData *entity);
extern void Buer_UpdateDeath(struct EwramData_EntityData *entity);
extern void Buer_UpdateEntry(struct EwramData_EntityData *entity);
extern void Buer_UpdateActive(struct EwramData_EntityData *entity);
extern s32 Enemy_IsStunned(struct EwramData_EntityData *entity);
extern void Hit_CheckWithPlayer(void *buffer,struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern s16 sBuerWalkProfile[];

/**
 * @brief Walk with the selected collision mode, adjust speed from contact flags, rotate and update the linked head facing.
 */
void Buer_MoveAndRotate(struct EwramData_EntityData *entity,s32 mode)
{
    struct EwramData_EntityData *part = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 oldX = entity->posX.whole;
    s32 options = 2;
    s32 collision;
    s32 sample;
    if (mode == 0)
        options = 6;
    collision = Entity_Walk(entity,entity->ext.wheel.speed,sBuerWalkProfile,options);
    sample = 24;
    if ((s32)entity->ext.wheel.speed < 0)
        sample = -24;
    if (Entity_IsOffsetOutsideRoom(entity,sample,0))
        entity->posX.whole = oldX;
    if (collision & 0x40)
    {
        if ((s32)entity->ext.wheel.speed > 0)
            entity->ext.wheel.speed -= 0x333;
        else
            entity->ext.wheel.speed += 0x333;
    }
    else if (collision & 0x20)
    {
        if ((s32)entity->ext.wheel.speed > 0)
            entity->ext.wheel.speed += 0x333;
        else
            entity->ext.wheel.speed -= 0x333;
    }
    else
    {
        if ((s32)entity->ext.wheel.speed >= 0)
        {
            entity->ext.wheel.speed -= 0x40;
            if ((s32)entity->ext.wheel.speed <= 0x3FFF)
                entity->ext.wheel.speed = 0x4000;
        }
        else
        {
            entity->ext.wheel.speed += 0x40;
            if ((s32)entity->ext.wheel.speed > -0x4000)
                entity->ext.wheel.speed = -0x4000;
        }
    }
    entity->ext.wheel.angle += (s32)entity->ext.wheel.speed / 64;
    if ((collision & 3) || (mode == 0 && !(collision & 8)))
        entity->ext.wheel.speed = -(s32)entity->ext.wheel.speed / 2;
    ObjAffine_Set(entity->ext.wheel.angle,0x10000,0x10000,0,0,entity->affineIndex);
    if ((s32)entity->ext.wheel.speed >= 0)
        part->flipX = 1;
    else
        part->flipX = 0;
}

void EnemyBuerUpdate(struct EwramData_EntityData *entity)
{
    u8 buffer[16];
    if (entity->state == 4)
    {
        Buer_UpdateState4Rotation(entity);
        return;
    }
    if (entity->state == 3)
    {
        Buer_UpdateDeath(entity);
        return;
    }
    Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity))
    {
        Hit_CheckWithPlayer(buffer,entity);
        return;
    }
    switch (entity->state)
    {
        case 1: Buer_UpdateEntry(entity); break;
        case 2: Buer_UpdateActive(entity); break;
        case 3: Buer_UpdateDeath(entity); break;
    }
    if (entity->state != 3)
        Hit_CheckWithPlayer(buffer,entity);
}
