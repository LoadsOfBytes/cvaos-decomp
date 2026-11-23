/* Bael collision motion, rotation, and update dispatcher. */
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_Walk(struct EwramData_EntityData *entity,s32 speed,void *profile,s32 flags);
extern s32 Entity_IsOffsetOutsideRoom(struct EwramData_EntityData *entity,s32 x,s32 y);
extern void Bael_UpdateState4Rotation(struct EwramData_EntityData *entity);
extern void Bael_UpdateDeath(struct EwramData_EntityData *entity);
extern void Bael_UpdateEntry(struct EwramData_EntityData *entity);
extern void Bael_UpdateActive(struct EwramData_EntityData *entity);
extern s32 Enemy_IsStunned(struct EwramData_EntityData *entity);
extern void Hit_CheckWithPlayer(void *buffer,struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern s16 sBaelWalkProfile[];
extern s32 Collision_GetPushUp(s32 x,s32 y);
static inline s32 RoomX(s32 pos) { struct BgLayerState *bg = &gEwramData->bgInfo[1]; return pos + (s16)bg->xPos.part16.integer; }
static inline s32 RoomY(s32 pos) { struct BgLayerState *bg = &gEwramData->bgInfo[1]; return pos + (s16)bg->yPos.part16.integer; }

/**
 * @brief Walk, adjust signed speed from contacts, handle blocking probes, rotate and update the linked head facing.
 */
void Bael_MoveAndRotate(struct EwramData_EntityData *entity,s32 mode)
{
    struct EwramData_EntityData *part = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 oldX = entity->posX.whole;
    s32 options = 2;
    s32 collision;
    s32 sample;
    if (mode == 0)
        options = 6;
    collision = Entity_Walk(entity,entity->ext.wheel.speed,sBaelWalkProfile,options);
    sample = 24;
    if ((s32)entity->ext.wheel.speed < 0)
        sample = -24;
    if (Entity_IsOffsetOutsideRoom(entity,sample,0))
        entity->posX.whole = oldX;
    {
        s32 speed;
        s32 adjusted;
        if (collision & 0x40)
        {
            speed = entity->ext.wheel.speed;
            if (speed > 0)
                adjusted = speed - 0x666;
            else
                adjusted = speed + 0x666;
            goto set_speed;
        }
        else if (collision & 0x20)
        {
            speed = entity->ext.wheel.speed;
            if (speed > 0)
                adjusted = speed + 0x666;
            else
                adjusted = speed - 0x666;
            goto set_speed;
        }
        else if ((speed = entity->ext.wheel.speed) >= 0)
        {
            entity->ext.wheel.speed = speed - 0x20;
            if ((s32)entity->ext.wheel.speed <= 0x6FFF)
            {
                adjusted = 0x7000;
set_speed:
                entity->ext.wheel.speed = adjusted;
            }
        }
        else
        {
            entity->ext.wheel.speed = speed + 0x20;
            if ((s32)entity->ext.wheel.speed > -0x7000)
                entity->ext.wheel.speed = -0x7000;
        }
    }
    entity->ext.wheel.angle += (s32)entity->ext.wheel.speed / 64;
    if (collision & 3)
    {
        if (Collision_GetPushUp(RoomX((s16)entity->posX.part16.integer),RoomY((s16)entity->posY.part16.integer)+24))
            entity->ext.wheel.speed = -(s32)entity->ext.wheel.speed / 2;
        else
            entity->ext.wheel.speed = 0;
    }
    else if (mode == 0 && !(collision & 8))
        entity->ext.wheel.speed = -(s32)entity->ext.wheel.speed / 2;
    ObjAffine_Set(entity->ext.wheel.angle,0x10000,0x10000,0,0,entity->affineIndex);
    if ((s32)entity->ext.wheel.speed >= 0)
        part->flipX = 1;
    else
        part->flipX = 0;
}


