/* Devil flight state machine; DEVIL_WINGS is a macro (an inline function loses constant CSE and gets cross-jumped). */
#include "code_08009A0.h"
#include "devil_entity.h"
#include "joint_entity.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "syscalls.h"
#include "structs/ewram.h"

extern void DevilPart_Attach(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern void EntityMove(struct EwramData_EntityData *);
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern void Devil_UpdateAllParts(struct EwramData_EntityData *entity);
extern void Devil_SaveParts(struct EwramData_EntityData *);
extern void Devil_RestoreParts(struct EwramData_EntityData *);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern s32 Target_IsInRect(s16, s16, s16, s16);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern s32 Entity_GetVisibilityPattern(s32);
extern s32 Entity_MoveTowardPoint(struct EwramData_EntityData *entity, s32 targetX, s32 targetY, s32 speed);

#define DEVIL_WINGS(entity) \
{ \
    switch (entity->stage) \
    { \
    case 0: \
        Devil_UpdateAllParts(entity); \
        if ((s8)--entity->timer <= 2) \
        { \
            entity->timer = 8; \
            entity->stage++; \
        } \
        break; \
    case 1: \
        Devil_UpdateAllParts(entity); \
        if ((s8)--entity->timer > 0) \
            break; \
        entity->timer = 0x16; \
        entity->stage++; \
        JOINT_POSE(entity) = 2; \
        DevilPart_Attach(entity, 1, 0, 0xA00, 0xA0000); \
        DevilPart_Attach(entity, 2, 0, 0xA00, 0xD0000); \
        DevilPart_Attach(entity, 3, 2, 0x800, 0xB0000); \
        DevilPart_Attach(entity, 5, 4, -0xA00, 0xA0000); \
        DevilPart_Attach(entity, 6, 4, -0xA00, 0xD0000); \
        DevilPart_Attach(entity, 7, 6, -0x800, 0xB0000); \
        break; \
    case 2: \
        Devil_UpdateAllParts(entity); \
        if ((s8)--entity->timer <= 2) \
        { \
            entity->timer = 8; \
            entity->stage++; \
        } \
        break; \
    case 3: \
        Devil_UpdateAllParts(entity); \
        if ((s8)--entity->timer > 0) \
            break; \
        entity->timer = 0x16; \
        entity->stage = 0; \
        Devil_RestoreParts(entity); \
        JOINT_POSE(entity) = 1; \
        DevilPart_Attach(entity, 1, 0, -0xA00, 0xA0000); \
        DevilPart_Attach(entity, 2, 0, -0xA00, 0xD0000); \
        DevilPart_Attach(entity, 3, 2, -0x800, 0xB0000); \
        DevilPart_Attach(entity, 5, 4, 0xA00, 0xA0000); \
        DevilPart_Attach(entity, 6, 4, 0xA00, 0xD0000); \
        DevilPart_Attach(entity, 7, 6, 0x800, 0xB0000); \
        break; \
    } \
}

/**
 * @brief Flight state machine (state 0): face the target and wait until it is
 * within 0x50 px or the Devil is damaged, then fly with rising speed
 * (Entity_MoveTowardPoint) to a point 0x28 px beside the target and 0..7 px
 * above it; on arrival, or once the target is close (|dx| < 0x2C,
 * |dy| < 0x10 px), switch to the attack state. Further sub-states drift and
 * retreat. Hidden (unk_50B) while above the screen top; DEVIL_WINGS flaps.
 */
void Devil_UpdateApproach(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];

    if (entity->posY.part16.integer < -0x18)
        DEVIL_PARTS_HIDDEN(entity) = 1;
    else
        DEVIL_PARTS_HIDDEN(entity) = 0;

    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        if (!Target_IsInRect(entity->posX.part16.integer - 0x50, entity->posY.part16.integer - 0x50, 0xA0, 0xA0)
            && !Enemy_IsDamaged(entity))
            break;
        entity->subState++;
        entity->stage = 0;
        if (entity->flipX)
            DEVIL_TARGET_X(entity) = Target_GetX(entity) + ({ s32 t = bg->xPos.whole + -0x280000; t; });
        else
            DEVIL_TARGET_X(entity) = Target_GetX(entity) + ({ s32 t = bg->xPos.whole + 0x280000; t; });
        DEVIL_TARGET_Y(entity) = Target_GetY(entity) - (RandomNumberGenerator() & 0x7FFFF) + bg->yPos.whole;
        Anim_StartIndexed(entity, entity->animScriptIndex + 1, 0, 1);
        entity->timer = 0x16;
        JOINT_POSE(entity) = 1;
        DevilPart_Attach(entity, 1, 0, -0xA00, 0xA0000);
        DevilPart_Attach(entity, 2, 0, -0xA00, 0xD0000);
        DevilPart_Attach(entity, 3, 2, -0x800, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0xA00, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0xA00, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x800, 0xB0000);
        break;
    case 1:
    {
        s32 done;
        s32 d;

        entity->flipX = Entity_IsTargetRight(entity);
        DEVIL_SPEED(entity) += 0x1000;
        if ((s32)DEVIL_SPEED(entity) > 0x1FFFF)
            DEVIL_SPEED(entity) = 0x20000;
        done = Entity_MoveTowardPoint(entity, DEVIL_TARGET_X(entity) - bg->xPos.whole,
                            DEVIL_TARGET_Y(entity) - bg->yPos.whole, DEVIL_SPEED(entity));
        if (done)
        {
            entity->state = 1;
            entity->subState = 0;
        }
        else
        {
            d = Target_GetX(entity) - entity->posX.whole;
            if (d < 0)
                d = -d;
            if (d <= 0x2BFFFF)
            {
                d = Target_GetY(entity) - entity->posY.whole;
                if (d < 0)
                    d = -d;
                if (d <= 0xFFFFF)
                {
                    entity->state = 1;
                    entity->subState = done;
                }
            }
        }
        DEVIL_WINGS(entity)
        break;
    }
    case 2:
        entity->flipX = Entity_IsTargetRight(entity);
        if (entity->flipX)
            entity->accelX.whole = -0x800;
        else
            entity->accelX.whole = 0x800;
        entity->accelY.whole = -0x800;
        entity->subState++;
        DEVIL_TIMER(entity) = 0;
    case 3:
        entity->flipX = Entity_IsTargetRight(entity);
        EntityAccelerateAndMove(entity, 0x20000);
        if (DEVIL_TIMER(entity)++ > 0x1D)
        {
            entity->subState++;
            DEVIL_TIMER(entity) = 0;
        }
        DEVIL_WINGS(entity)
        break;
    case 4:
        EntityDampVelocityX(entity, 0x18);
        EntityDampVelocityY(entity, 0x18);
        EntityMove(entity);
        if (DEVIL_TIMER(entity)++ > 0x1D)
        {
            entity->subState = 1;
            if (entity->flipX)
                DEVIL_TARGET_X(entity) = Target_GetX(entity) + ({ s32 t = bg->xPos.whole + -0x280000; t; });
            else
                DEVIL_TARGET_X(entity) = Target_GetX(entity) + ({ s32 t = bg->xPos.whole + 0x280000; t; });
            DEVIL_TARGET_Y(entity) = Target_GetY(entity) - (RandomNumberGenerator() & 0x7FFFF) + bg->yPos.whole;
            DEVIL_SPEED(entity) = 0;
        }
        DEVIL_WINGS(entity)
        break;
    case 10:
        entity->flipX = Entity_IsTargetRight(entity);
        DEVIL_INTANGIBLE(entity) = 1;
        if (Target_IsInRect(entity->posX.part16.integer - 0x50, entity->posY.part16.integer - 0x50, 0xA0, 0xA0))
        {
            entity->subState++;
            entity->timer = 0x28;
            PlaySong(0x6E);
        }
        break;
    case 11:
    {
        s32 r = Entity_GetVisibilityPattern(entity->timer);
        register u8 one asm("r3") = 1; /* FAKEMATCH: pinned register; constant shared with the unk_50A store */

        entity->hidden = r ^ one;
        DEVIL_INTANGIBLE(entity) = one;
    }
        if ((s8)--entity->timer > 0)
            break;
        entity->subState++;
        entity->timer = 8;
        entity->hidden = 0;
        DEVIL_INTANGIBLE(entity) = 0;
        JOINT_POSE(entity) = 3;
        Anim_StartIndexed(entity, entity->animScriptIndex + 1, 0, 1);
        DevilPart_Attach(entity, 1, 0, 0x10000, 0xA0000);
        DevilPart_Attach(entity, 2, 0, 0x10000, 0xD0000);
        DevilPart_Attach(entity, 3, 2, 0x10000, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0x18000, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0x18000, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x18000, 0xB0000);
        break;
    case 12:
        entity->flipX = Entity_IsTargetRight(entity);
        Devil_UpdateAllParts(entity);
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = 0x14;
        entity->subState++;
        DevilPart_Attach(entity, 1, 0, 0, 0xA0000);
        DevilPart_Attach(entity, 2, 0, 0, 0xD0000);
        DevilPart_Attach(entity, 3, 2, 0, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0, 0xB0000);
        break;
    case 13:
        entity->flipX = Entity_IsTargetRight(entity);
        Devil_UpdateAllParts(entity);
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = 0x16;
        entity->subState++;
        JOINT_POSE(entity) = 2;
        DevilPart_Attach(entity, 1, 0, 0x12A00, 0xA0000);
        DevilPart_Attach(entity, 2, 0, 0x12A00, 0xD0000);
        DevilPart_Attach(entity, 3, 2, 0x14800, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0x15600, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0x15600, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x13800, 0xB0000);
        break;
    case 14:
        entity->flipX = Entity_IsTargetRight(entity);
        Devil_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 2)
        {
            entity->timer = 8;
            entity->subState++;
        }
        break;
    case 15:
        entity->flipX = Entity_IsTargetRight(entity);
        Devil_UpdateAllParts(entity);
        if ((s8)--entity->timer > 0)
            break;
        Devil_SaveParts(entity);
        entity->timer = 0x16;
        entity->subState = 1;
        if (entity->flipX)
            DEVIL_TARGET_X(entity) = Target_GetX(entity) + ({ s32 t = bg->xPos.whole + -0x280000; t; });
        else
            DEVIL_TARGET_X(entity) = Target_GetX(entity) + ({ s32 t = bg->xPos.whole + 0x280000; t; });
        DEVIL_TARGET_Y(entity) = Target_GetY(entity) - (RandomNumberGenerator() & 0x7FFFF) + bg->yPos.whole;
        JOINT_POSE(entity) = 1;
        DevilPart_Attach(entity, 1, 0, -0xA00, 0xA0000);
        DevilPart_Attach(entity, 2, 0, -0xA00, 0xD0000);
        DevilPart_Attach(entity, 3, 2, -0x800, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0xA00, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0xA00, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x800, 0xB0000);
        break;
    }
}
