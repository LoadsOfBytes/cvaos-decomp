/* Devil (08220A38 skeleton enemy) main state machine. */
#include "code_08009A0.h"
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
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern void DevilChargeEffect_Spawn(struct EwramData_EntityData *owner, s32 dx, s32 dy, u8 timer);
extern void DevilProjectile_Spawn(struct EwramData_EntityData *owner, s32 x, s32 y, s32 vx, s32 vy, u8 id);

static inline void DevilStep(struct EwramData_EntityData *entity)
{
    entity->flipX = Entity_IsTargetRight(entity);
    EntityDampVelocityX(entity, 0x18);
    EntityDampVelocityY(entity, 0x10);
    EntityMove(entity);
    Devil_UpdateAllParts(entity);
}

/**
 * @brief Attack state machine (state 1): types other than 0x30 randomly raise the
 * wings and throw a projectile at the target (DevilProjectile_Spawn, sound
 * 0x78; types 0x48 / 0x66 first show DevilChargeEffect_Spawn above the
 * head); otherwise a wing-beat lunge. Both end in state 0 aimed at a new
 * point beside the target.
 */
void Devil_UpdateAttack(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];

    switch (entity->subState)
    {
    case 0:
        if (entity->unk_518.part8.enemyId != 0x30 && (RandomNumberGenerator() & 0x7F) > 0x40)
        {
            JOINT_POSE(entity) = 4;
            entity->timer = 0xE;
            Anim_StartIndexed(entity, entity->animScriptIndex + 1, 0, 0);
            DevilPart_Attach(entity, 1, 0, 0x10000, 0xA0000);
            DevilPart_Attach(entity, 2, 0, 0x10000, 0xD0000);
            DevilPart_Attach(entity, 3, 2, 0x10000, 0xB0000);
            DevilPart_Attach(entity, 5, 4, 0x18000, 0xA0000);
            DevilPart_Attach(entity, 6, 4, 0x18000, 0xD0000);
            DevilPart_Attach(entity, 7, 6, 0x18000, 0xB0000);
            entity->subState = 10;
            EntityDampVelocityX(entity, 0x18);
            EntityDampVelocityY(entity, 0x10);
            EntityMove(entity);
            break;
        }
        entity->animFlags |= 2;
        JOINT_POSE(entity) = 0;
        entity->timer = 0xE;
        DevilPart_Attach(entity, 1, 0, 0x12000, 0xA0000);
        DevilPart_Attach(entity, 2, 0, 0x12000, 0xD0000);
        DevilPart_Attach(entity, 3, 2, 0x19000, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0x16000, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0x16000, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x11000, 0xB0000);
        entity->subState++;
    case 1:
        DevilStep(entity);
        if ((s8)--entity->timer <= 2)
        {
            entity->timer = 0x12;
            entity->subState++;
        }
        break;
    case 2:
        DevilStep(entity);
        if ((s8)--entity->timer > 0)
            break;
        entity->subState++;
        entity->timer = 8;
        JOINT_POSE(entity) = 3;
        PlaySong(0x86);
        DevilPart_Attach(entity, 1, 0, 0x10000, 0xA0000);
        DevilPart_Attach(entity, 2, 0, 0x10000, 0xD0000);
        DevilPart_Attach(entity, 3, 2, 0x10000, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0x18000, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0x18000, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x18000, 0xB0000);
        break;
    case 3:
        DevilStep(entity);
        if ((s8)--entity->timer <= 0)
        {
            entity->timer = 0x10;
            entity->subState++;
        }
        break;
    case 4:
        DevilStep(entity);
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = 0x16;
        entity->subState++;
        JOINT_POSE(entity) = 2;
        Anim_StartIndexed(entity, entity->animScriptIndex, 0, 1);
        DevilPart_Attach(entity, 1, 0, 0x12A00, 0xA0000);
        DevilPart_Attach(entity, 2, 0, 0x12A00, 0xD0000);
        DevilPart_Attach(entity, 3, 2, 0x14800, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0x15600, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0x15600, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x13800, 0xB0000);
        break;
    case 6:
        DevilStep(entity);
        if ((s8)--entity->timer > 0)
            break;
        Devil_SaveParts(entity);
        entity->timer = 0x16;
        entity->state = 0;
        entity->subState = 2;
        entity->stage = 0;
        JOINT_POSE(entity) = 1;
        entity->unk_4FC.whole = 0;
        if (entity->flipX)
            entity->unk_4F4.whole = Target_GetX(entity) + ({ s32 t = bg->xPos.whole + -0x280000; t; });
        else
            entity->unk_4F4.whole = Target_GetX(entity) + ({ s32 t = bg->xPos.whole + 0x280000; t; });
        entity->unk_4F8.whole = Target_GetY(entity) - (RandomNumberGenerator() & 0x7FFFF) + bg->yPos.whole;
        DevilPart_Attach(entity, 1, 0, -0xA00, 0xA0000);
        DevilPart_Attach(entity, 2, 0, -0xA00, 0xD0000);
        DevilPart_Attach(entity, 3, 2, -0x800, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0xA00, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0xA00, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x800, 0xB0000);
        break;
    case 10:
        DevilStep(entity);
        if ((s8)--entity->timer <= 2)
        {
            entity->timer = 0x3C;
            entity->subState++;
            entity->stage = 0;
        }
        break;
    case 11:
        DevilStep(entity);
        if (entity->stage == 0 && entity->animDone)
        {
            entity->stage++;
            Anim_StartIndexed(entity, entity->animScriptIndex + 1, 0, 1);
        }
        if (entity->timer == 0x28
            && (entity->unk_518.part8.enemyId == 0x48 || entity->unk_518.part8.enemyId == 0x66))
            DevilChargeEffect_Spawn(entity, 0, -0x300000, entity->timer);
        if (entity->timer == 0x1E)
        {
            s32 dx = Target_GetX(entity) - entity->posX.whole;
            s32 dy = Target_GetY(entity) - ({ s32 t = entity->posY.whole + -0x200000; t; });
            u16 angle = ArcTan2(dx < 0 ? -(-dx >> 16) : dx >> 16, dy < 0 ? -(-dy >> 16) : dy >> 16);
            s32 speed;

            dx = Sin(angle + 0x4000);
            dy = Sin(angle);
            speed = 0x20000;
            dx = FixedMultiply(dx, speed);
            dy = FixedMultiply(dy, speed);
            DevilProjectile_Spawn(entity, entity->flipX ? speed : -0x20000, -0x360000, dx, dy, entity->timer);
            PlaySong(0x78);
        }
        if ((s8)--entity->timer > 0)
            break;
        entity->subState++;
        entity->timer = 1;
        JOINT_POSE(entity) = 3;
        DevilPart_Attach(entity, 1, 0, 0x10000, 0xA0000);
        DevilPart_Attach(entity, 2, 0, 0x10000, 0xD0000);
        DevilPart_Attach(entity, 3, 2, 0x10000, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0x18000, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0x18000, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x18000, 0xB0000);
        break;
    case 12:
        DevilStep(entity);
        if ((s8)--entity->timer <= 0)
        {
            entity->timer = 0x30;
            entity->subState++;
        }
        break;
    case 13:
        DevilStep(entity);
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = 0x16;
        entity->subState++;
        JOINT_POSE(entity) = 2;
        Anim_StartIndexed(entity, entity->animScriptIndex - 2, 0, 1);
        DevilPart_Attach(entity, 1, 0, 0x12A00, 0xA0000);
        DevilPart_Attach(entity, 2, 0, 0x12A00, 0xD0000);
        DevilPart_Attach(entity, 3, 2, 0x14800, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0x15600, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0x15600, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x13800, 0xB0000);
        break;
    case 5:
    case 14:
        DevilStep(entity);
        if ((s8)--entity->timer <= 2)
        {
            entity->timer = 8;
            entity->subState++;
        }
        break;
    case 15:
        DevilStep(entity);
        if ((s8)--entity->timer > 0)
            break;
        Devil_SaveParts(entity);
        entity->timer = 0x16;
        entity->state = 0;
        entity->subState = 1;
        JOINT_POSE(entity) = 1;
        entity->unk_4FC.whole = 0;
        if (entity->flipX)
            entity->unk_4F4.whole = Target_GetX(entity) + ({ s32 t = bg->xPos.whole + -0x280000; t; });
        else
            entity->unk_4F4.whole = Target_GetX(entity) + ({ s32 t = bg->xPos.whole + 0x280000; t; });
        entity->unk_4F8.whole = Target_GetY(entity) - (RandomNumberGenerator() & 0x7FFFF) + bg->yPos.whole;
        DevilPart_Attach(entity, 1, 0, -0xA00, 0xA0000);
        DevilPart_Attach(entity, 2, 0, -0xA00, 0xD0000);
        DevilPart_Attach(entity, 3, 2, -0x800, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0xA00, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0xA00, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x800, 0xB0000);
        break;
    }
}
