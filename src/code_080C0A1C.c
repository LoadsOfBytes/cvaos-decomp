/* Slime crawling and transitions between floors, walls and ceilings. */
#include "code_08009A0.h"
#include "code_08001194.h"
#include "code/code_08039340.h"
extern void Slime_SpawnParticles(struct EwramData_EntityData *, s32, s32, s32);
extern void Slime_BeginSurfaceTurn(struct EwramData_EntityData *, u8, s32, u16);
extern s32 Slime_UpdateSurfaceSupport(struct EwramData_EntityData *, u16);
extern s32 Slime_GetFloorCorrection(s32, s32);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);
extern u8 Angle_TurnTowards(u16 *, u16, u16);
extern void EntityMoveX(struct EwramData_EntityData *);
extern void EntityMoveY(struct EwramData_EntityData *);
extern const u16 sSlimeSurfaceTurnTargets[];
#define SLIME_FIXED_INTEGER(v) ((u32)((v) < 0 ? -((-(v)) >> 16) << 16 : (v)) >> 16)
#define SLIME_PROBE_X ((s16)roomX + (s16)dx)
#define SLIME_PROBE_Y ((s16)roomY + (s16)dy)

/**
 * @brief Crawl state (0): react to hits, move along the current surface and
 * begin floor/wall/ceiling turns at corners. On non-floor surfaces,
 * proximity and randomness may select the detach state.
 */
void Slime_UpdateCrawl(struct EwramData_EntityData *entity)
{
    u16 angle = entity->ext.slime.angle;
    u16 roomX;
    u16 roomY;
    u16 dx;
    u16 dy;
    u16 normalX;
    u16 normalY;
    s32 lostGround;
    if (entity->ext.slime.flags & 1)
    {
        entity->ext.slime.flags = (entity->ext.slime.flags & ~1) | 2;
        Anim_StartIndexed(entity, 4, 0, 0);
        Slime_SpawnParticles(entity, 4, 0, -0x18000);
    }
    if ((entity->ext.slime.flags & 2) && (entity->animFlags & 4))
    {
        entity->ext.slime.flags &= ~2;
        Anim_StartIndexed(entity, 0, 0, 1);
    }
    switch (entity->subState)
    {
    case 0:
        entity->ext.slime.speed = 0x4000;
        if (entity->flipX) entity->ext.slime.speed = -entity->ext.slime.speed;
        if ((s32)entity->ext.slime.scaleX <= 0xFFFF) entity->ext.slime.scaleX += 0x200;
        if (entity->ext.slime.turnCooldown != 0) goto countdown;
        if (entity->stage != 0 && (RandomNumberGenerator() & 0xFF) <= 3 && Target_IsNearEntity(entity, 0x78, 0xB4))
        {
            entity->state = 1;
            entity->subState = 0;
            return;
        }
        break;
    case 1:
        {
            s32 trig = Sin((s16)angle + 0x4000);
            s32 speed = entity->ext.slime.speed;
            trig >>= 8;
            speed >>= 8;
            trig *= speed;
            entity->velX.whole = trig;
        }
        {
            s32 trig = Sin((s16)angle);
            s32 speed = entity->ext.slime.speed;
            trig >>= 8;
            speed >>= 8;
            trig *= speed;
            entity->velY.whole = trig;
        }
        EntityMoveX(entity);
        EntityMoveY(entity);
        angle = sSlimeSurfaceTurnTargets[entity->ext.slime.turnSurface];
        if ((s32)entity->ext.slime.scaleX > 0x8000) entity->ext.slime.scaleX -= 0x200;
        if (Angle_TurnTowards((u16 *)&entity->ext.slime.angle, entity->ext.slime.turnRate, angle))
        {
            entity->subState = 0;
            entity->stage = entity->ext.slime.turnSurface;
            entity->ext.slime.turnCooldown = 0x18;
        }
        return;
    }
    if (entity->ext.slime.turnCooldown != 0)
    {
countdown:
        entity->ext.slime.turnCooldown--;
    }
    EntityMoveX(entity);
    EntityMoveY(entity);
    {
        s32 trig = Sin((s16)angle + 0x4000);
        s32 speed = entity->ext.slime.speed;
        trig >>= 8;
        speed >>= 8;
        trig *= speed;
        entity->velX.whole = trig;
    }
    {
        s32 trig = Sin((s16)angle);
        s32 speed = entity->ext.slime.speed;
        trig >>= 8;
        speed >>= 8;
        trig *= speed;
        entity->velY.whole = trig;
    }
    roomX = GetEntityRoomXPositionInteger(entity);
    roomY = GetEntityRoomYPositionInteger(entity);
    dx = SLIME_FIXED_INTEGER(9 * Sin((s16)angle + 0x4000));
    dy = SLIME_FIXED_INTEGER(9 * Sin((s16)angle));
    if (entity->ext.slime.speed <= 0)
    {
        dx = -(u32)((u32)dx << 16) >> 16;
        dy = -(u32)((u32)dy << 16) >> 16;
    }
    angle = sSlimeSurfaceTurnTargets[entity->stage] - 0x4000;
    normalX = SLIME_FIXED_INTEGER(3 * Sin((s16)angle + 0x4000));
    normalY = SLIME_FIXED_INTEGER(3 * Sin((s16)angle));
    dx = (u32)(((s16)dx + (s16)normalX) << 16) >> 16;
    dy = (u32)(((s16)dy + (s16)normalY) << 16) >> 16;
    lostGround = Slime_UpdateSurfaceSupport(entity, 0x100);
    switch (entity->stage)
    {
    case 0:
        if (lostGround)
        {
            if (entity->ext.slime.turnCooldown != 0) break;
            if (entity->ext.slime.speed > 0) Slime_BeginSurfaceTurn(entity, 3, 0x2000, 0);
            else Slime_BeginSurfaceTurn(entity, 1, 0x2000, 0);
        }
        else
        {
            if (entity->ext.slime.turnCooldown != 0) break;
            if (entity->ext.slime.speed > 0)
            {
                if ((s16)Collision_GetPushLeft(SLIME_PROBE_X, SLIME_PROBE_Y)) Slime_BeginSurfaceTurn(entity, 1, 0, 0);
            }
            else if ((s16)Collision_GetPushRight(SLIME_PROBE_X, SLIME_PROBE_Y)) Slime_BeginSurfaceTurn(entity, 3, 0, 0);
        }
        break;
    case 2:
        if (lostGround)
        {
            if (entity->ext.slime.turnCooldown != 0) break;
            if (entity->ext.slime.speed > 0) Slime_BeginSurfaceTurn(entity, 1, 0x2000, 0);
            else Slime_BeginSurfaceTurn(entity, 3, 0x2000, 0);
        }
        else
        {
            if (entity->ext.slime.turnCooldown != 0) break;
            if (entity->ext.slime.speed > 0)
            {
                if ((s16)Collision_GetPushRight(SLIME_PROBE_X, SLIME_PROBE_Y)) Slime_BeginSurfaceTurn(entity, 3, 0, 0);
            }
            else if ((s16)Collision_GetPushLeft(SLIME_PROBE_X, SLIME_PROBE_Y)) Slime_BeginSurfaceTurn(entity, 1, 0, 0);
        }
        break;
    case 1:
        if (lostGround)
        {
            if (entity->ext.slime.turnCooldown != 0) break;
            if (entity->ext.slime.speed > 0) Slime_BeginSurfaceTurn(entity, 0, 0x2000, 0);
            else Slime_BeginSurfaceTurn(entity, 2, 0x2000, 0);
        }
        else
        {
            if (entity->ext.slime.turnCooldown != 0) break;
            if (entity->ext.slime.speed > 0)
            {
                if ((s16)Collision_GetPushDown(SLIME_PROBE_X, SLIME_PROBE_Y)) Slime_BeginSurfaceTurn(entity, 2, 0, 0);
            }
            else if ((s16)Slime_GetFloorCorrection(SLIME_PROBE_X, SLIME_PROBE_Y)) Slime_BeginSurfaceTurn(entity, 0, 0, 0);
        }
        break;
    case 3:
        if (lostGround)
        {
            if (entity->ext.slime.turnCooldown != 0) break;
            if (entity->ext.slime.speed > 0) Slime_BeginSurfaceTurn(entity, 2, 0x2000, 0);
            else Slime_BeginSurfaceTurn(entity, 0, 0x2000, 0);
        }
        else
        {
            if (entity->ext.slime.turnCooldown != 0) break;
            if (entity->ext.slime.speed > 0)
            {
                if ((s16)Slime_GetFloorCorrection(SLIME_PROBE_X, SLIME_PROBE_Y)) Slime_BeginSurfaceTurn(entity, 0, 0, 0);
            }
            else if ((s16)Collision_GetPushDown(SLIME_PROBE_X, SLIME_PROBE_Y)) Slime_BeginSurfaceTurn(entity, 2, 0, 0);
        }
        break;
    }
}
