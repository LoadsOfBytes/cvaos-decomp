/* Killer Fish part: follow its owner, steer toward targets and handle reactions.
 *
 * Compiler-sensitive constructs:
 * - Target_GetPixelX/Target_GetPixelY return s16 pixel coordinates (their bodies end
 *   in a halfword); with the halfword call value as a register and the camera
 *   field first, the truncated sum is done in HImode and combine folds the
 *   camera load into a (subreg:SI (mem:HI)) operand, so reload loads it into
 *   the camera pointer's reload register (ldr rX, [sp]; ldrh rX, [rX, #6]).
 * - `distance` is reused for the collision flags of Entity_MoveWithBoxCollision; the longer
 *   live range moves it to r5 and velocityX to r4. */
#include "code_08009A0.h"
#include "syscalls.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code/code_080211F0.h"
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern s32 Entity_MoveWithBoxCollision(struct EwramData_EntityData *, s32, s32, s32, s32, s32, s32);
extern s32 Collision_FindWaterSurfaceOffset(s32, s32);
extern s32 Target_IsInRect(s16, s16, s16, s16);
extern s16 Target_GetPixelX(struct EwramData_EntityData *);
extern s16 Target_GetPixelY(struct EwramData_EntityData *);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern s32 Tile_Classify(s16, s16);
extern s32 Entity_GetVisibilityPattern(s32);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
#define FLAG_A(e) ((e)->unk_4FC.part8.unk_4FC)
#define FLAG_B(e) (*(s8 *)&(e)->unk_4FC.part8.unk_4FD)
#define TIMER(e) ((e)->unk_4F8.whole)
#define ROOM_X(e) ((s16)(camera->xPos.part16.integer + Target_GetPixelX(e)))
#define ROOM_Y(e) ((s16)(camera->yPos.part16.integer + Target_GetPixelY(e)))

static inline s32 DeltaPixels(s32 target, s32 base)
{
    s32 delta = target - base;
    if (delta < 0) return -((base - target) >> 16);
    return delta >> 16;
}

static inline s32 CoordinateDelta(s32 target, s32 base)
{
    if (target < 0) return -((-target) >> 16) - base;
    return (target >> 16) - base;
}

/**
 * @brief Swarm fish update: swim to random points around the owner (type 7) and,
 * when the target is close and in water, dart at it; scatter when the owner
 * attacks (state 4) or dies; state 2 blinks away.
 */
void KillerFishSwarm_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->unk_4F4.unk_4F4_entity;
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    u32 bounds[4];
    s32 targetX, targetY, dx, dy, velocityX, velocityY;
    s32 distance;

    if (entity->state != 2)
    {
        if (EntityIsOutsideScreen(entity, 4)) return;
        if (Entity_IsInRect(entity, -16, 0, 272, 176)) Player_TrackEntity(entity);
    }
    if (EntityIsOutsideScreen(entity, 4)) entity->hidden = 1;
    else entity->hidden = 0;
    if (owner && owner->updateFunc && owner->unk_518.part8.enemyId == 7)
    {
        if (owner->state == 4) FLAG_A(entity) = 1;
        if ((s16)owner->unk_518.part16.hp <= 0 || entity->unk_518.part8.spawnIndex != owner->unk_518.part8.spawnIndex)
            FLAG_B(entity) = 1;
    }
    if (entity->state != 2 && gEwramData->gameState.timeStopTimer)
    {
        Hit_CheckWithPlayer(bounds, entity);
        return;
    }
restart:
    switch (entity->state)
    {
    case 0:
        entity->state = 1;
        entity->velX.whole = 0;
        entity->velY.whole = 0;
        entity->accelX.whole = 0;
        entity->accelY.whole = 0;
        TIMER(entity) = 0;
        entity->subState = 0;
        /* fall through */
    case 1:
    case 4:
        if (FLAG_A(entity) == 1 || FLAG_B(entity) == 1)
        {
            entity->state = 3;
            entity->subState = 0;
            goto restart;
        }
        if (--TIMER(entity) <= 0)
        {
            TIMER(entity) = RandomNumberGenerator() % 20 + 20;
            {
                s32 random = RandomNumberGenerator();
                s32 base = owner->posX.whole;
                base -= 0x80000;
                targetX = base + (random & 0xFFFFF);
            }
            {
                s32 random = RandomNumberGenerator();
                s32 base = owner->posY.whole;
                base -= 0x80000;
                targetY = base + (random & 0xFFFFF);
            }
            dx = DeltaPixels(targetX, entity->posX.whole);
            dx *= dx;
            dy = DeltaPixels(targetY, entity->posY.whole);
            dy *= dy;
            distance = Sqrt(dx + dy);
            velocityX = (DeltaPixels(targetX, entity->posX.whole) << 17) / distance;
            velocityY = (DeltaPixels(targetY, entity->posY.whole) << 17) / distance;
            if (entity->state == 4 && distance <= 9) entity->state = 1;
            entity->velX.whole = velocityX;
            entity->velY.whole = velocityY;
        }
        if (entity->velX.whole > 0) entity->flipX = 1;
        else entity->flipX = 0;
        EntityDampVelocityX(entity, 20);
        EntityDampVelocityY(entity, 20);
        distance = Entity_MoveWithBoxCollision(entity, 0, -4, -4, 8, 8, 0x60000) & 0x80;
        if (!distance)
        {
            s32 correction = Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, entity->posY.part16.integer - 3);
            entity->posY.part16.integer += correction;
            entity->velY.whole = distance;
        }
        if (entity->state != 4 && Target_IsInRect((s16)(entity->posX.part16.integer - 80), (s16)(entity->posY.part16.integer - 32), 160, 64) && gEwramData->playerState.enemyTarget)
        {
            s32 x = ROOM_X(entity);
            s32 y = ROOM_Y(entity);
            if (Tile_Classify(x, y) == 3)
            {
                entity->state = 3;
                entity->subState = 0;
            }
        }
        Hit_CheckWithPlayer(bounds, entity);
        break;
    case 2:
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if (--entity->timer == 0) entity->deleteRequested = 1;
        break;
    case 3:
        switch (entity->subState)
        {
        case 0:
            entity->unk_500.whole = 0;
            entity->subState++;
            TIMER(entity) = 0;
            entity->velX.whole = 0;
            entity->velY.whole = 0;
            entity->accelX.whole = 0;
            entity->accelY.whole = 0;
            entity->timer = 0;
            /* fall through */
        case 1:
            if (--TIMER(entity) <= 0)
            {
                entity->timer++;
                TIMER(entity) = RandomNumberGenerator() % 20 + 20;
                targetX = 0;
                if (gEwramData->playerState.enemyTarget)
                {
                    s32 x = ROOM_X(entity);
                    s32 y = ROOM_Y(entity);
                    if (Tile_Classify(x, y) == 3)
                    {
                        s32 random = RandomNumberGenerator();
                        {
                            s32 base = Target_GetX(entity);
                            random &= 0xFFFFF;
                            base -= 0x80000;
                            targetX = random + base;
                        }
                        {
                            s32 base = Target_GetY(entity);
                            base -= RandomNumberGenerator() & 0xFFFFF;
                            targetY = -0x100000 + base;
                        }
                    }
                }
                if (!targetX || FLAG_A(entity) == 1)
                {
                    s32 random = RandomNumberGenerator() & 0xFFFFF;
                    { s32 base = entity->posX.whole; base -= 0x80000; targetX = random + base; }
                    random = RandomNumberGenerator() & 0xFFFFF;
                    { s32 base = entity->posY.whole; base -= 0x80000; targetY = random + base; }
                }
                dx = CoordinateDelta(targetX, (s16)entity->posX.part16.integer);
                dx *= dx;
                dy = CoordinateDelta(targetY, entity->posY.part16.integer);
                dy *= dy;
                distance = Sqrt(dx + dy);
                velocityX = (CoordinateDelta(targetX, (s16)entity->posX.part16.integer) << 17) / distance;
                velocityY = (CoordinateDelta(targetY, entity->posY.part16.integer) << 17) / distance;
                entity->velX.whole = velocityX;
                entity->velY.whole = velocityY;
            }
            if (entity->velX.whole > 0) entity->flipX = 1;
            else if (entity->velX.whole < 0) entity->flipX = 0;
            EntityDampVelocityX(entity, 20);
            EntityDampVelocityY(entity, 20);
            distance = Entity_MoveWithBoxCollision(entity, 0, -4, -4, 8, 8, 0x60000) & 0x80;
            if (!distance)
            {
                s32 correction = Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, entity->posY.part16.integer - 3);
                entity->posY.part16.integer += correction;
                entity->velY.whole = distance;
            }
            if (entity->timer > 5) entity->state = 4;
            if (entity->unk_500.whole) entity->state = 4;
            Hit_CheckWithPlayer(bounds, entity);
            break;
        }
        break;
    }
    Anim_Update(entity);
}
