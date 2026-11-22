/* Stolas movement and transitions to attack states. */
#include <string.h>
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void Stolas_FaceTarget(struct EwramData_EntityData *);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern void Stolas_AttachJoint(struct EwramData_EntityData *, s32, s32, s32, s32);
extern void Stolas_InterpolateJointsInOrder(struct EwramData_EntityData *, s8 *);
extern s32 Entity_MoveWithCollisionDefault(struct EwramData_EntityData *, s32, s32);
extern s32 Entity_GetHorizontalRoomEdge(struct EwramData_EntityData *, s32);
extern void Stolas_TurnAround(struct EwramData_EntityData *);
extern s32 Tile_Classify(s16, s16);
extern s32 Target_GetPixelX(struct EwramData_EntityData *);
extern const s8 sUnk_081190AC[6];
#define POSE(e) ((e)->unk_504.part8.unk_504)
/**
 * @brief Movement state machine (state 0): face the target, wake when it is within
 * 160 px or Stolas is damaged, then fly with the joint poses and start the
 * attack (state 1).
 */
void Stolas_UpdateMove(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    s8 order[6];
    memcpy(order, sUnk_081190AC, sizeof(order));
    switch (entity->subState)
    {
    case 0:
        Stolas_FaceTarget(entity);
        if (!(Target_IsNearEntity(entity, 160, 160) || Enemy_IsDamaged(entity))) break;
        entity->subState = 1;
        entity->stage = 0;
        entity->timer = 36;
        Anim_StartIndexed(entity, 1, 0, 0);
        POSE(entity) = 1;
        Stolas_AttachJoint(entity, 1, 0, 0x14800, 0x80000);
        Stolas_AttachJoint(entity, 2, 1, 0x16800, 0xd0000);
        Stolas_AttachJoint(entity, 4, 3, 0x13000, 0x80000);
        Stolas_AttachJoint(entity, 5, 4, 0x13800, 0xd0000);
        if (entity->flipX) entity->velX.whole = 0xD0000 / entity->timer;
        else entity->velX.whole = -0xD0000 / entity->timer;
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0x2800;
        break;
    case 1:
        Stolas_InterpolateJointsInOrder(entity, order);
        Entity_MoveWithCollisionDefault(entity, 0x10000, -0x300000);
        if ((s8)--entity->timer > 0) break;
        entity->timer = 16;
        entity->subState++;
        POSE(entity) = 2;
        Stolas_AttachJoint(entity, 1, 0, 0x14000, 0x80000);
        Stolas_AttachJoint(entity, 2, 1, 0x15000, 0xd0000);
        Stolas_AttachJoint(entity, 4, 3, 0x12800, 0x80000);
        Stolas_AttachJoint(entity, 5, 4, 0x13000, 0xd0000);
        if (entity->flipX) entity->velX.whole = 0x40000 / entity->timer;
        else entity->velX.whole = -0x40000 / entity->timer;
        break;
    case 2:
        Stolas_InterpolateJointsInOrder(entity, order);
        Entity_MoveWithCollisionDefault(entity, 0x10000, -0x300000);
        if ((s8)--entity->timer <= 4)
        {
            entity->timer = 10;
            entity->subState++;
            POSE(entity) = 3;
        }
        break;
    case 3:
        Stolas_InterpolateJointsInOrder(entity, order);
        if ((s8)--entity->timer > 0) break;
        entity->timer = 36;
        entity->subState++;
        Anim_StartIndexed(entity, 1, 0, 0);
        POSE(entity) = 4;
        Stolas_AttachJoint(entity, 1, 0, 0x13000, 0x80000);
        Stolas_AttachJoint(entity, 2, 1, 0x13800, 0xd0000);
        Stolas_AttachJoint(entity, 4, 3, 0x14800, 0x80000);
        Stolas_AttachJoint(entity, 5, 4, 0x16800, 0xd0000);
        if (entity->flipX) entity->velX.whole = 0xD0000 / entity->timer;
        else entity->velX.whole = -0xD0000 / entity->timer;
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0x2800;
        break;
    case 4:
        Stolas_InterpolateJointsInOrder(entity, order);
        Entity_MoveWithCollisionDefault(entity, 0x10000, -0x300000);
        if ((s8)--entity->timer > 0) break;
        entity->timer = 16;
        entity->subState++;
        POSE(entity) = 5;
        Stolas_AttachJoint(entity, 1, 0, 0x12800, 0x80000);
        Stolas_AttachJoint(entity, 2, 1, 0x13000, 0xd0000);
        Stolas_AttachJoint(entity, 4, 3, 0x14000, 0x80000);
        Stolas_AttachJoint(entity, 5, 4, 0x15000, 0xd0000);
        if (entity->flipX) entity->velX.whole = 0x40000 / entity->timer;
        else entity->velX.whole = -0x40000 / entity->timer;
        break;
    case 5:
        Stolas_InterpolateJointsInOrder(entity, order);
        Entity_MoveWithCollisionDefault(entity, 0x10000, -0x300000);
        if ((s8)--entity->timer <= 4)
        {
            entity->timer = 10;
            entity->subState++;
            POSE(entity) = 6;
        }
        break;
    case 6:
        Stolas_InterpolateJointsInOrder(entity, order);
        if ((s8)--entity->timer <= 0)
        {
            s32 changed;
            Stolas_FaceTarget(entity);
            changed = 0;
            if (entity->flipX)
            {
                if (Entity_GetHorizontalRoomEdge(entity, 20) == 2)
                {
                    Stolas_TurnAround(entity);
                    goto reset;
                }
            }
            else if (Entity_GetHorizontalRoomEdge(entity, 20) == 1)
            {
                Stolas_TurnAround(entity);
                changed = 1;
            }
            if (!changed)
            {
                if ((RandomNumberGenerator() & 127) <= 63)
                {
                    s32 distance = 48;
                    /* Keep the X pointer in the original callee-saved register;
                     * automatic allocation swaps camera/X/Y storage. */
                    register u16 *entityX asm("r8") = &entity->posX.part16.integer; /* FAKEMATCH: pinned register */
                    s16 *entityY = &entity->posY.part16.integer;
                    for (; distance > 0; distance -= 8)
                    {
                        s32 x;
                        if (entity->flipX)
                            x = camera->xPos.part16.integer + *entityX + distance;
                        else
                            x = camera->xPos.part16.integer + *entityX - distance;
                        /* The statement expression delays Y evaluation until after
                         * X narrowing; unsigned component loads narrow only at the end. */
                        if (!Tile_Classify((s16)x, ({
                            s32 y = (u16)camera->yPos.part16.integer;
                            y += (u16)*entityY;
                            (s16)y;
                        }))) break;
                    }
                    entity->unk_508.part16.unk_508 = distance;
                    if (distance >= 0 && (s16)entity->unk_508.part16.unk_50A > 0)
                    {
                        entity->state = 1;
                        entity->subState = 0;
                        break;
                    }
                }
                else
                {
                    s32 distance = (s16)Target_GetPixelX(entity) - (s16)entity->posX.part16.integer;
                    if (distance < 0) distance = -distance;
                    if (distance <= 31)
                    {
                        entity->state = 1;
                        entity->subState = 10;
                        break;
                    }
                }
            }
reset:
            entity->timer = 36;
            entity->subState = 1;
            Anim_StartIndexed(entity, 1, 0, 0);
            POSE(entity) = 1;
            Stolas_AttachJoint(entity, 1, 0, 0x14800, 0x80000);
            Stolas_AttachJoint(entity, 2, 1, 0x16800, 0xd0000);
            Stolas_AttachJoint(entity, 4, 3, 0x13000, 0x80000);
            Stolas_AttachJoint(entity, 5, 4, 0x13800, 0xd0000);
            if (entity->flipX) entity->velX.whole = 0xD0000 / entity->timer;
            else entity->velX.whole = -0xD0000 / entity->timer;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
        }
        break;
    }
}
