/* Hopping minion (sibling of Fleaman_UpdateMovement): jumps toward the player, flaps while airborne. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Collision_GetTileOnScreen(s32, s32);
extern s16 Collision_GetPushDown(s32, s32);
extern s16 Collision_GetPushLeft(s32, s32);
extern s16 Collision_GetPushRight(s32, s32);
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern struct EwramData_EntityData *Ripper_CreateProjectile(struct EwramData_EntityData *owner);
extern s16 GetEntityRoomXPositionInteger(struct EwramData_EntityData *entity);
extern s16 GetEntityRoomYPositionInteger(struct EwramData_EntityData *entity);

/**
 * @brief Alive state (1): handle carried release and random target-relative
 * hops. Near the jump apex, select a throwing animation and create
 * a projectile at its event. Preserve terrain tests and substate fallthrough.
 */
void Ripper_UpdateMovement(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *link = &gEwramData->entityData[entity->ext.altairDrop.spawnerIndex];
    u8 st;
    u16 x;
    u16 y;
    s32 xs;
    s32 ys;
    s32 d;

    switch (entity->subState)
    {
    case 0:
        entity->spriteLayer = 0x14;
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->ext.ripper.anim = 0;
        {
            s32 v = 0x8000;
            entity->velX.whole = v;
            if (!entity->flipX)
                entity->velX.whole = -v;
        }
        st = entity->ext.ripper.airborne;
        if (st != 1)
            st = 4;
        entity->subState = st;
    case 1:
        entity->velY.whole = 0x2000;
        entity->accelY.whole = 0x2000;
        if (link != NULL)
            break;
        entity->subState++;
        break;
    case 2:
        if (entity->ext.ripper.anim != 3)
        {
            Anim_StartIndexed(entity, 3, 0, 1);
            entity->ext.ripper.anim = 3;
            entity->animTimer = 0;
            entity->animStep = 3;
            entity->animFlags |= 2;
        }
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += entity->accelY.whole;
        if (Collision_GetTileOnScreen((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer))
        {
            Entity_SnapToGround(entity);
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 3:
        entity->animTimer = 0;
        entity->animStep = 0;
        entity->animFlags |= 2;
        if (++entity->timer > 0xF)
        {
            entity->subState++;
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->ext.ripper.anim = 0;
        }
        break;
    case 4:
        entity->posX.whole += entity->velX.whole;
        if ((s16)Entity_GetTargetDistanceX(entity) > 0)
            entity->flipX = 0;
        else
            entity->flipX = 1;
        switch (RandomNumberGenerator() % 6)
        {
        case 0: case 1: case 2: default:
            entity->velX.whole = 0xD000;
            entity->velY.whole = -0x30000;
            entity->accelY.whole = 0xA000;
            Anim_StartIndexed(entity, 0, 0, 1);
            st = 6;
            break;
        case 3:
            entity->velX.whole = 0x1A000;
            entity->velY.whole = -0x30000;
            entity->accelY.whole = 0x2800;
            Anim_StartIndexed(entity, 3, 0, 1);
            entity->ext.ripper.anim = 3;
            entity->animFlags |= 2;
            st = 5;
            break;
        case 4:
            entity->velX.whole = 0xD000;
            entity->velY.whole = -0x30000;
            entity->accelY.whole = 0x1400;
            Anim_StartIndexed(entity, 3, 0, 1);
            entity->ext.ripper.anim = 3;
            entity->animFlags |= 2;
            st = 5;
            break;
        }
        entity->subState = st;
        if (!entity->flipX)
            entity->velX.whole = -entity->velX.whole;
        {
            s32 dist = (s16)Entity_GetTargetDistanceX(entity);
            if (dist < 0)
                dist = -dist;
            if (dist <= 0x3F)
            {
                s16 p = entity->posX.part16.integer;
                if ((u32)(p - entity->velX.part16.integer - 1) <= 0xEE)
                    entity->velX.whole = -entity->velX.whole;
            }
        }
        break;
    case 5:
    case 6:
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += entity->accelY.whole;
        if (entity->subState == 5)
        {
            if (entity->ext.ripper.anim == 3)
            {
                if (entity->velY.whole >= -entity->accelY.whole && entity->velY.whole <= entity->accelY.whole)
                {
                    entity->animTimer = 0;
                    entity->animStep = 2;
                    entity->animFlags |= 2;
                }
                else if (entity->animStep == 0)
                {
                    entity->animTimer = 0;
                    entity->animStep = 1;
                    entity->animFlags |= 2;
                }
                else if (entity->animStep == 2)
                {
                    entity->animTimer = 0;
                    entity->animStep = 3;
                    entity->animFlags |= 2;
                }
            }
            if (entity->ext.ripper.anim == 1)
            {
                /* m doubles as the mask and the second reading (register allocation) */
                u16 *p = (u16 *)&entity->animStep;
                s32 m = 0xFFFF;
                s32 a = *p & m;
                if (a == 3)
                {
                    Anim_StartIndexed(entity, 3, 0, 1);
                    entity->ext.ripper.anim = a;
                }
                if ((m &= *p) == 2 && entity->ext.ripper.thrown != 0)
                {
                    Ripper_CreateProjectile(entity);
                    entity->ext.ripper.thrown = 0;
                }
            }
            if (entity->ext.ripper.anim == 3 && RandomNumberGenerator() % 3 == 0
                && (u32)(entity->velY.whole + 0x1FFF) <= 0x3FFE
                && entity->ext.ripper.thrown == 0)
            {
                entity->ext.ripper.thrown = 1;
                Anim_StartIndexed(entity, 1, 0, 0);
                entity->ext.ripper.anim = 1;
            }
        }
        y = GetEntityRoomYPositionInteger(entity);
        x = GetEntityRoomXPositionInteger(entity);
        xs = (s16)x;
        ys = (s16)y;
        d = Collision_GetPushDown(xs, ys - 10);
        if (d)
            { u32 t = (u16)entity->posY.part16.integer; entity->posY.part16.integer = d + t; }
        if (entity->velX.whole > 0)
        {
            d = Collision_GetPushLeft(xs + 8, ys - 8);
            if (d)
                { u32 t = (u16)entity->posX.part16.integer; entity->posX.part16.integer = d + t; }
        }
        if (entity->velX.whole < 0)
        {
            d = Collision_GetPushRight((s16)x - 8, (s16)y - 8);
            if (d)
                { u32 t = (u16)entity->posX.part16.integer; entity->posX.part16.integer = d + t; }
        }
        if (Collision_GetTileOnScreen((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer))
        {
            if (entity->subState == 5)
            {
                entity->animTimer = 0;
                entity->animStep = 0;
            }
            entity->subState = 4;
        }
        break;
    }
}
