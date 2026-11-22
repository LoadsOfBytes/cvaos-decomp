/* Death_UpdateThrowAttack. */
#include "code_08009A0.h"
#include "death_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "syscalls.h"

extern s32 Entity_AccelerateAlongAngle(struct EwramData_EntityData *entity, u16 angle, s32 speed, s32 target, s16 acceleration, u16 time);

/* Move towards (tx, ty) (16.16) at a speed proportional to the distance;
 * velX/velY get the step taken. Returns the speed. */
static inline s32 MoveToward(struct EwramData_EntityData *entity, s32 tx, s32 ty)
{
    s32 x = entity->posX.whole;
    s32 y = entity->posY.whole;
    u32 dist = Sqrt(((x - tx) >> 16) * ((x - tx) >> 16) + ((y - ty) >> 16) * ((y - ty) >> 16)) << 16;
    u16 angle = ArcTan2(((tx < 0) ? -(-tx >> 16) : (tx >> 16)) - (s16)entity->posX.part16.integer,
                        ((ty < 0) ? -(-ty >> 16) : (ty >> 16)) - entity->posY.part16.integer);
    u32 speed = dist >> 4;
    Entity_AccelerateAlongAngle(entity, angle, speed, 0, 0, 0);
    entity->velX.whole = entity->posX.whole - x;
    entity->velY.whole = entity->posY.whole - y;
    return speed;
}

/* Cutscene step: fly to a point 0x50 px beside the player (screen y 0x78),
 * then play the landing animations and hand back to state 3. */
/**
 * @brief Throw attack state (5): move beside the player, signal weapon
 * throw and return with owner flags, then restore hover state once the
 * weapon clears the return flag and the recovery animation finishes.
 */
void Death_UpdateThrowAttack(struct EwramData_EntityData *entity)
{
    s32 tx, ty;
    switch (entity->subState)
    {
    case 0:
    {
        struct EwramData *ewram = gEwramData;
        struct EwramData_EntityData **player = &ewram->playerState.player;
        struct BgLayerState *camera;
        {
            s32 px = (*player)->posX.whole;
            u8 right = 0;
            if (entity->posX.whole < px)
                right = 1;
            entity->flipX = right;
        }
        tx = -0x50;
        { s32 px = (*player)->posX.whole; if (entity->posX.whole < px) tx = 0x50; }
        {
            s32 px = (s16)(*player)->posX.part16.integer;
            camera = &ewram->bgInfo[1];
            DEATH_TARGET_X(entity) = px + camera->xPos.part16.integer - tx;
        }
        DEATH_TARGET_Y(entity) = camera->yPos.part16.integer + 0x78;
        entity->subState = 1;
        break;
    }
    case 1:
    {
        struct BgLayerState *camera;
        tx = (DEATH_TARGET_X(entity) - (s16)(camera = &gEwramData->bgInfo[1])->xPos.part16.integer) << 16;
        ty = (DEATH_TARGET_Y(entity) - (s16)camera->yPos.part16.integer) << 16;
        if (MoveToward(entity, tx, ty) <= 0x3FFF)
        {
            Anim_StartIndexed(entity, 0x19, 2, 0);
            DEATH_FLAGS(entity) |= 8;
            entity->subState = 2;
        }
        break;
    }
    case 2:
        if (entity->animFlags & 4)
        {
            Anim_StartIndexed(entity, 0x1A, 2, 0);
            DEATH_FLAGS(entity) |= 0x84;
            entity->subState = 3;
        }
        break;
    case 3:
        if (!(DEATH_FLAGS(entity) & 4))
        {
            DEATH_FLAGS(entity) &= 0xFF7F;
            entity->flipX = 0;
            Anim_StartIndexed(entity, 0x19, 2, 0);
            entity->subState = 4;
        }
        break;
    case 4:
        if (entity->animFlags & 4)
        {
            Anim_StartIndexed(entity, 7, 2, 1);
            entity->state = 3;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    }
}
