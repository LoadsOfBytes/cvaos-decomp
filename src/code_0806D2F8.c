/* Entity motion helpers 0x0806D2F8..0x0806D5BF (no upstream C existed).
 *
 * Entity fields used (16.16 fixed point): 0x524/0x528 position x/y,
 * 0x52C/0x530 velocity x/y, 0x534/0x538 acceleration x/y. */
#include <string.h>
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void RisingEffect_Update(struct EwramData_EntityData *entity);

/* Same layout as struct EnemyTemplateView in src/code_0800F1FC.c, extended by the
 * field at 0x1C read here. */
/* Local 0x24-byte template-table view: retain this TU's types and padding. */
struct EnemyTemplateFlagsView {
    u32 *createFunc;
    u32 *updateFunc;
    u8 pad_8[0xC - 0x8];
    u16 hp;
    u8 pad_E[0x1C - 0xE];
    u16 flags_1C;
    u8 pad_1E[0x24 - 0x1E];
}; /* size = 0x24 */
extern const struct EnemyTemplateFlagsView sEnemyData[];

/* 0x28 signed bytes at ROM 0x08118CE0, copied by Entity_GetVisibilityPattern. Declared as
 * bytes: the original calls memcpy instead of an inline copy. */
extern const u8 sEntityVisibilityPattern[];

static inline void AccelerateX(struct EwramData_EntityData *entity, s32 maxSpeed)
{
    entity->velX.whole += entity->accelX.whole;
    if (maxSpeed != 0)
    {
        if (entity->accelX.whole >= 0)
        {
            if (entity->velX.whole > maxSpeed)
                entity->velX.whole = maxSpeed;
        }
        else if (entity->velX.whole < -maxSpeed)
        {
            entity->velX.whole = -maxSpeed;
        }
    }
}

static inline void AccelerateY(struct EwramData_EntityData *entity, s32 maxSpeed)
{
    entity->velY.whole += entity->accelY.whole;
    if (maxSpeed != 0)
    {
        if (entity->accelY.whole >= 0)
        {
            if (entity->velY.whole > maxSpeed)
                entity->velY.whole = maxSpeed;
        }
        else if (entity->velY.whole < -maxSpeed)
        {
            entity->velY.whole = -maxSpeed;
        }
    }
}

static inline void MoveX(struct EwramData_EntityData *entity)
{
    entity->posX.whole += entity->velX.whole;
}

static inline void MoveY(struct EwramData_EntityData *entity)
{
    entity->posY.whole += entity->velY.whole;
}

/**
 * @brief 6D2F8 | Set the velocity from an angle (0x10000 = full turn) and a
 * 16.16 speed, using Sin (sine) and FixedMultiply (fixed multiply).
 */
void EntitySetVelocityFromAngle(struct EwramData_EntityData *entity, s32 angle, s32 speed)
{
    if (speed == 0x10000)
    {
        entity->velX.whole = Sin(angle + 0x4000);
        entity->velY.whole = Sin(angle);
    }
    else
    {
        entity->velX.whole = FixedMultiply(Sin(angle + 0x4000), speed);
        entity->velY.whole = FixedMultiply(Sin(angle), speed);
    }
}

void EntityAccelerateX(struct EwramData_EntityData *entity, s32 maxSpeed);
void EntityAccelerateY(struct EwramData_EntityData *entity, s32 maxSpeed);

/**
 * @brief 6D344 | Accelerate in both axes, clamping the speed to maxSpeed
 * (0 = no limit).
 */
void EntityAccelerate(struct EwramData_EntityData *entity, s32 maxSpeed)
{
    EntityAccelerateX(entity, maxSpeed);
    EntityAccelerateY(entity, maxSpeed);
}

/**
 * @brief 6D35C | Accelerate horizontally, clamping to maxSpeed (0 = no limit).
 */
void EntityAccelerateX(struct EwramData_EntityData *entity, s32 maxSpeed)
{
    AccelerateX(entity, maxSpeed);
}

/**
 * @brief 6D384 | Accelerate vertically, clamping to maxSpeed (0 = no limit).
 */
void EntityAccelerateY(struct EwramData_EntityData *entity, s32 maxSpeed)
{
    AccelerateY(entity, maxSpeed);
}

/**
 * @brief 6D3AC | Move by the velocity in both axes.
 */
void EntityMove(struct EwramData_EntityData *entity)
{
    MoveX(entity);
    MoveY(entity);
}

/**
 * @brief 6D3C0 | Move horizontally by the velocity.
 */
void EntityMoveX(struct EwramData_EntityData *entity)
{
    MoveX(entity);
}

/**
 * @brief 6D3CC | Move vertically by the velocity.
 */
void EntityMoveY(struct EwramData_EntityData *entity)
{
    MoveY(entity);
}

/**
 * @brief 6D3D8 | Accelerate (clamped) and move in both axes.
 */
void EntityAccelerateAndMove(struct EwramData_EntityData *entity, s32 maxSpeed)
{
    AccelerateX(entity, maxSpeed);
    AccelerateY(entity, maxSpeed);
    MoveX(entity);
    MoveY(entity);
}

/**
 * @brief 6D430 | Accelerate (clamped) and move horizontally.
 */
void EntityAccelerateAndMoveX(struct EwramData_EntityData *entity, s32 maxSpeed)
{
    AccelerateX(entity, maxSpeed);
    MoveX(entity);
}

/**
 * @brief 6D460 | Accelerate (clamped) and move vertically.
 */
void EntityAccelerateAndMoveY(struct EwramData_EntityData *entity, s32 maxSpeed)
{
    AccelerateY(entity, maxSpeed);
    MoveY(entity);
}

/**
 * @brief 6D490 | Damp the horizontal velocity: v = v * (factor - 1) / factor.
 */
void EntityDampVelocityX(struct EwramData_EntityData *entity, s32 factor)
{
    entity->velX.whole = entity->velX.whole * (factor - 1) / factor;
}

/**
 * @brief 6D4A8 | Damp the vertical velocity: v = v * (factor - 1) / factor.
 */
void EntityDampVelocityY(struct EwramData_EntityData *entity, s32 factor)
{
    entity->velY.whole = entity->velY.whole * (factor - 1) / factor;
}

/**
 * @brief 6D4C0 | 1 when EWRAM 0x8E bit 6 is clear, timeStopTimer is non-zero and
 * the entity type's info has bit 10 of unk_1C clear; otherwise 0.
 */
s32 Entity_IsTimeStopped(struct EwramData_EntityData *entity)
{
    if (!gEwramData->gameState.bestiaryPreview && gEwramData->gameState.timeStopTimer != 0
        && !(sEnemyData[entity->enemyId].flags_1C & 0x400))
        return 1;
    return 0;
}

/**
 * @brief Return the signed byte visibility-table entry for index 0..39, or zero outside that range.
 */
s32 Entity_GetVisibilityPattern(s32 step)
{
    s8 pattern[0x28];

    memcpy(pattern, sEntityVisibilityPattern, sizeof(pattern));
    if (step <= 0x27)
    {
        if (step >= 0)
            return pattern[step];
    }
    return 0;
}

/**
 * @brief 6D54C | 1 when the entity's 0x518 value is below unk_C of its type
 * info, else 0.
 */
s32 Enemy_IsDamaged(struct EwramData_EntityData *entity)
{
    if ((s16)entity->hp < sEnemyData[entity->enemyId].hp)
        return 1;
    return 0;
}

/**
 * @brief 6D578 | Spawn an RisingEffect_Update entity in slots 0x49..0xCF at
 * (x, y) with a random horizontal flip and spriteLayer = layer.
 */
void Effect_SpawnAt(s32 x, s32 y, s32 layer)
{
    struct EwramData_EntityData *child;

    child = EntityCreateInRange(0x49, 0xCF, (u32 *)RisingEffect_Update);
    if (child != NULL)
    {
        child->posX.whole = x;
        child->posY.whole = y;
        child->flipX = RandomNumberGenerator() % 2;
        child->spriteLayer = layer;
    }
}
