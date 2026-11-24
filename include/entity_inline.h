#ifndef ENTITY_INLINE_H
#define ENTITY_INLINE_H

/* Small entity helpers shared by several C files. */

#include "types.h"
#include "structs/ewram.h"
#include "core.h"
#include "math_anim.h"
#include "syscalls.h"

/* Bounce away from a hit, using the direction between the entities. */
static inline void Bounce(struct EwramData_EntityData *entity, struct EwramData_EntityData *other)
{
    s32 x = entity->posX.whole - other->posX.whole;
    s32 y = entity->posY.whole - other->posY.whole;
    s32 pixelX, pixelY;
    u16 direction;
    s32 parameter;
    if (x < 0) pixelX = (s16)(-((-x) >> 16));
    else pixelX = x >> 16;
    if (y < 0) pixelY = (s16)(-((-y) >> 16));
    else pixelY = y >> 16;
    direction = ArcTan2(pixelX, pixelY);
    parameter = direction;
    x = Sin(parameter + 0x4000);
    y = Sin(parameter);
    /* The direction has been consumed; reuse the temporary for speed. */
    parameter = 0x20000;
    entity->velX.whole = FixedMultiply(x, parameter);
    entity->velY.whole = FixedMultiply(y, parameter);
}

static inline void SetAffine(struct EwramData_EntityData *entity, s32 index)
{
    entity->affineIndex = index;
}

/* Sine sweep driven by the entity timer, amplitude 24 (16.16 units of Sin). */
static inline s32 SweepWave(struct EwramData_EntityData *entity)
{
    return Sin(entity->timer * 0xA00) * 24;
}

/* Screen coordinate -> room coordinate (adds the scroll of BG layer 1). */
static inline s32 ToRoomX(s32 pos)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    return pos + (s16)bg->xPos.part16.integer;
}

static inline s32 ToRoomY(s32 pos)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    return pos + (s16)bg->yPos.part16.integer;
}

static inline s32 IsNotFlippedX(struct EwramData_EntityData *entity)
{
    return entity->flipX == 0;
}

/* Enter a state at the given stage, sub state 0. */
static inline void Entity_SetStateAndStage(struct EwramData_EntityData *entity, s8 stage, u8 state)
{
    entity->state = state;
    entity->stage = stage;
    entity->subState = 0;
}

#endif // ENTITY_INLINE_H
