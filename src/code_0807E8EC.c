/* Golem main state machine: hops towards the player (turning at walls), landing dust, attacks. */
#include "code_08009A0.h"
#include "joint_entity.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void *memcpy(void *dst, const void *src, unsigned long size);
extern void Golem_FaceTarget(struct EwramData_EntityData *entity);
extern void Golem_UpdateAllParts(struct EwramData_EntityData *entity);
extern void Golem_SpawnIdleEffect(struct EwramData_EntityData *, s32, s32);
extern s32 Entity_MoveWithCollisionDefault(struct EwramData_EntityData *, s32, s32);
extern s32 Target_IsInRect(s16, s16, s16, s16);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern s32 Target_GetPixelX(struct EwramData_EntityData *);
extern s32 Tile_Classify(s16, s16);
extern void Effect_SpawnRisingParticle(s32, s32, s32, s32);
extern void ScreenShake_RequestOffset(u8 priority, s8 param_1, s8 param_2);
extern const u8 sGolemHopTimes[4], sGolemLandTimes[4];

static inline s32 ABS(s32 x)
{
    if (x < 0)
        x = -x;
    return x;
}

#define GOLEM_HOP_SPEED(entity)\
    if ((entity)->flipX)\
        (entity)->velX.whole = 0x100000 / ((entity)->timer * 2);\
    else\
        (entity)->velX.whole = -0x100000 / ((entity)->timer * 2);

/* Wait on the ground, then attack when the player is close or hop on (turning at walls). */
#define GOLEM_WAIT(NEXT, ANIM, TURN_STATE, TURN_ANIM)\
    if (entity->timer >= (s8)landTimes[entity->unk_508.whole] - 10 && !(entity->timer & 3))\
        ScreenShake_RequestOffset(0, 0, 1);\
    if ((s8)--entity->timer > 0)\
        break;\
    r = RandomNumberGenerator() & 3;\
    if (r <= 1 && ABS((s16)Target_GetPixelX(entity) - (s16)entity->posX.part16.integer) <= 0x3B)\
    {\
        entity->state = 1;\
        entity->subState = 0;\
        break;\
    }\
    if (r == 2 && ABS((s16)Target_GetPixelX(entity) - (s16)entity->posX.part16.integer) <= 0x77)\
    {\
        entity->state = 1;\
        entity->subState = 0x14;\
        break;\
    }\
    entity->timer = hopTimes[entity->unk_508.whole];\
    NEXT;\
    JOINT_POSE(entity) = ANIM;\
    Golem_FaceTarget(entity);\
    GOLEM_HOP_SPEED(entity)\
    dx = entity->flipX ? 0x20 : -0x20;\
    if (ABS((s16)Target_GetPixelX(entity) - (s16)entity->posX.part16.integer) > 0x17\
        && !Tile_Classify(bg->xPos.part16.integer + entity->posX.part16.integer + dx,\
                         bg->yPos.part16.integer + entity->posY.part16.integer))\
        goto hop;\
    dx = entity->flipX ? -0x20 : 0x20;\
    if (Tile_Classify(bg->xPos.part16.integer + entity->posX.part16.integer + dx,\
                     bg->yPos.part16.integer + entity->posY.part16.integer))\
        goto hop;\
    entity->velX.whole = -entity->velX.whole;\
    entity->subState = TURN_STATE;\
    JOINT_POSE(entity) = TURN_ANIM;\
    goto hop;

#define GOLEM_LAND(TIMES)\
    Golem_UpdateAllParts(entity);\
    Entity_MoveWithCollisionDefault(entity, 0x10000, -0x300000);\
    if ((s8)--entity->timer > 0)\
        break;\
    entity->timer = TIMES[entity->unk_508.whole];\
    entity->subState++;

#define GOLEM_PAUSE()\
    Golem_FaceTarget(entity);\
    Golem_UpdateAllParts(entity);\
    if ((s8)--entity->timer > 0)\
        break;

/**
 * @brief Movement state 0 for the shared Golem family: wait, hop,
 * turn at obstacles and select nearby attacks. The Flesh Golem idle
 * branch may spawn its visual effect. Preserve macros and shared labels.
 */
void Golem_UpdateMovement(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    u8 hopTimes[4];
    u8 landTimes[4];
    u32 r;
    s32 dx; /* wall probe offset, also the dust offset */

    memcpy(hopTimes, sGolemHopTimes, sizeof(hopTimes));
    memcpy(landTimes, sGolemLandTimes, sizeof(landTimes));

    switch (entity->subState)
    {
    case 0:
        Golem_FaceTarget(entity);
        entity->velX.whole = 0;
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0x2800;
        Entity_MoveWithCollisionDefault(entity, 0x10000, -0x300000);
        if (!Target_IsInRect(entity->posX.part16.integer - 0x50, entity->posY.part16.integer - 0x50, 0xA0, 0xA0)
            && !Enemy_IsDamaged(entity))
            break;
        entity->subState = 2;
        entity->timer = hopTimes[entity->unk_508.whole];
        JOINT_POSE(entity) = 2;
        GOLEM_HOP_SPEED(entity)
        goto hop;
    case 2:
        GOLEM_LAND(landTimes)
        PlaySong(0x7C);
        JOINT_POSE(entity) = 2;
        if (entity->flipX)
            dx = 0x80000;
        else
            dx = -0x80000;
        Effect_SpawnRisingParticle(entity->posX.whole + dx, entity->posY.whole, entity->spriteLayer - 1, 1);
        ScreenShake_RequestOffset(0, 1, 2);
        break;
    case 3:
        GOLEM_WAIT(entity->subState++, 3, 10, 1)
    case 5:
        GOLEM_LAND(landTimes)
        PlaySong(0x7C);
        JOINT_POSE(entity) = 4;
        if (entity->flipX)
            dx = 0x80000;
        else
            dx = -0x80000;
        Effect_SpawnRisingParticle(entity->posX.whole + dx, entity->posY.whole, entity->spriteLayer - 1, 1);
        ScreenShake_RequestOffset(0, 1, 2);
        break;
    case 6:
        GOLEM_WAIT(entity->subState = 1, 1, 7, 3)
    case 1:
    case 7:
        GOLEM_LAND(hopTimes)
        JOINT_POSE(entity) = 2;
        break;
    case 8:
        GOLEM_LAND(landTimes)
        PlaySong(0x7C);
        JOINT_POSE(entity) = 2;
        dx = entity->flipX ? -0x60000 : 0x60000;
        Effect_SpawnRisingParticle(entity->posX.whole + dx, entity->posY.whole, entity->spriteLayer - 1, 1);
        ScreenShake_RequestOffset(0, 1, 2);
        break;
    case 9:
        GOLEM_WAIT(entity->subState = 4, 3, 10, 1)
    case 4:
    case 10:
        GOLEM_LAND(hopTimes)
        JOINT_POSE(entity) = 4;
        break;
    case 11:
        GOLEM_LAND(landTimes)
        PlaySong(0x7C);
        JOINT_POSE(entity) = 4;
        dx = entity->flipX ? -0x60000 : 0x60000;
        Effect_SpawnRisingParticle(entity->posX.whole + dx, entity->posY.whole, entity->spriteLayer - 1, 1);
        ScreenShake_RequestOffset(0, 1, 2);
        break;
    case 12:
        GOLEM_WAIT(entity->subState = 1, 1, 7, 3)
    case 20:
        Golem_FaceTarget(entity);
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0x2800;
        Entity_MoveWithCollisionDefault(entity, 0x10000, -0x300000);
        if (!Target_IsInRect(entity->posX.part16.integer - 0x50, entity->posY.part16.integer - 0x50, 0xA0, 0xA0)
            && !Enemy_IsDamaged(entity))
            break;
        entity->subState++;
        entity->timer = 0x14;
        JOINT_POSE(entity) = 6;
        break;
    case 21:
        GOLEM_PAUSE()
        entity->timer = 4;
        entity->subState++;
        JOINT_POSE(entity) = 8;
        PlaySong(0x7E);
        break;
    case 22:
        GOLEM_PAUSE()
        entity->timer = 4;
        entity->subState++;
        JOINT_POSE(entity) = 6;
        break;
    case 24:
        GOLEM_PAUSE()
        entity->subState++;
        entity->timer = 0x14;
        JOINT_POSE(entity) = 5;
        break;
    case 25:
        GOLEM_PAUSE()
        entity->timer = 4;
        entity->subState++;
        JOINT_POSE(entity) = 9;
        PlaySong(0x7E);
        break;
    case 26:
        GOLEM_PAUSE()
        entity->timer = 4;
        entity->subState++;
        JOINT_POSE(entity) = 5;
        break;
    case 28:
        GOLEM_PAUSE()
        entity->subState++;
        entity->timer = 0x14;
        JOINT_POSE(entity) = 0;
        break;
    case 29:
        GOLEM_PAUSE()
        entity->timer = 4;
        entity->subState++;
        JOINT_POSE(entity) = 10;
        PlaySong(0x7E);
        break;
    case 30:
        GOLEM_PAUSE()
        entity->timer = 4;
        entity->subState++;
        JOINT_POSE(entity) = 0;
        break;
    case 23:
    case 27:
    case 31:
        GOLEM_PAUSE()
        entity->timer = 0x14;
        entity->subState++;
        break;
    case 32:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = hopTimes[entity->unk_508.whole];
        entity->subState = 2;
        JOINT_POSE(entity) = 2;
        GOLEM_HOP_SPEED(entity)
        goto hop;
    case 40:
        Golem_FaceTarget(entity);
        entity->velX.whole = 0;
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0x2800;
        Entity_MoveWithCollisionDefault(entity, 0x10000, -0x300000);
        if (Target_IsInRect(entity->posX.part16.integer - 0x50, entity->posY.part16.integer - 0x50, 0xA0, 0xA0)
            || Enemy_IsDamaged(entity))
        {
            entity->subState = 2;
            entity->timer = hopTimes[entity->unk_508.whole];
            JOINT_POSE(entity) = 2;
            GOLEM_HOP_SPEED(entity)
        hop:
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
            break;
        }
        if (RandomNumberGenerator() % 60 != 0)
            break;
        entity->subState++;
        Golem_SpawnIdleEffect(entity, 0, 0);
        entity->timer = 0;
        break;
    case 41:
        Golem_FaceTarget(entity);
        entity->velX.whole = 0;
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0x2800;
        Entity_MoveWithCollisionDefault(entity, 0x10000, -0x300000);
        if (entity->timer++ > 0x13)
        {
            entity->subState = 0x28;
            entity->timer = 0;
        }
        break;
    }
}
