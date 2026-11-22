/* Creaking Skull movement: alternating strides, pauses, and attack selection.
 * for compiler constraints and matching evidence. */
#include "code_08009A0.h"
#include "creaking_skull_entity.h"
#include "joint_entity.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
extern void *memcpy(void *, const void *, unsigned long);
extern void CreakingSkull_FaceTarget(struct EwramData_EntityData *);
extern void CreakingSkull_AttachJointSetAngle(struct EwramData_EntityData *, s32, s32, s32, s32);
extern void CreakingSkull_InterpolateJointsInOrder(struct EwramData_EntityData *, s8 *);
extern s32 Entity_MoveWithCollisionDefault(struct EwramData_EntityData *, s32, s32);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern s32 Target_GetPixelX(struct EwramData_EntityData *);
extern s32 Entity_GetHorizontalRoomEdge(struct EwramData_EntityData *, s32);
extern s32 Tile_Classify(s16, s16);
extern void Effect_SpawnParticleWithDamping(s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern const u8 sCreakingSkullTimes0[2], sCreakingSkullTimes1[2], sCreakingSkullTimes2[2], sUnk_08119076[2];
extern const s8 sUnk_08119063[13];
#define PHASE entity->subState
#define SUBPHASE entity->stage
#define TIMER entity->timer
#define FRAME JOINT_POSE(entity)
#define ARMS(A) \
    CreakingSkull_AttachJointSetAngle(entity, 1, 0, A, 0x1E0000); \
    CreakingSkull_AttachJointSetAngle(entity, 2, 1, A, 0x1C0000);
#define POSE(A,B) \
    CreakingSkull_AttachJointSetAngle(entity, 12, 11, A, 0x1F0000); \
    ARMS(B)
#define GRAVITY() entity->velY.whole = 0x10000; entity->accelY.whole = 0x2800;
#define SPEED() \
    if (entity->flipX) entity->velX.whole = 0x190000 / (TIMER * 2 - 4); \
    else entity->velX.whole = -0x190000 / (TIMER * 2 - 4);
#define UPDATE() CreakingSkull_InterpolateJointsInOrder(entity, chain);
#define FALL() Entity_MoveWithCollisionDefault(entity, 0x10000, -0x300000);
#define COUNTDOWN() if ((s8)--TIMER > 0) break;
#define DISTANCE() \
    distance = (s16)Target_GetPixelX(entity) - (s16)entity->posX.part16.integer; \
    if (distance < 0) distance = -distance;
#define TERRAIN() Tile_Classify(camera->xPos.part16.integer + entity->posX.part16.integer + offset, camera->yPos.part16.integer + entity->posY.part16.integer)
/* The extra RNG call is present in the ROM even though its result is unused.
 * Explicit x/vy and arithmetic temporaries preserve agbcc register allocation. */
#define DEBRIS(REVERSE) \
    if (!(TIMER & 15)) \
    { \
        register s32 x asm("r4"); s32 vx; register s32 vy asm("r6"); s32 zero; /* FAKEMATCH: pinned register */ \
        if (entity->flipX) { x = -0x230000; if (REVERSE) vx = -0x4000 - (RandomNumberGenerator() & 0x3FFF); else vx = (RandomNumberGenerator() & 0x3FFF) + 0x4000; } \
        else { x = 0x230000; if (REVERSE) vx = (RandomNumberGenerator() & 0x3FFF) + 0x4000; else vx = -0x4000 - (RandomNumberGenerator() & 0x3FFF); } \
        { register u32 r asm("r1") = RandomNumberGenerator() & 0x3FFF; register s32 k asm("r0") = -0x4000; vy = k - r; } /* FAKEMATCH: pinned register */ \
        RandomNumberGenerator(); zero = 0; \
        Effect_SpawnParticleWithDamping(entity->posX.whole + x, entity->posY.whole, vx, vy, entity->spriteLayer - 1, zero, 0x10, 0xF, 6); \
    } \
    if (!(TIMER & 7)) { if (TIMER & 8) PlaySong(0x118); else if ((RandomNumberGenerator() & 0x7F) <= 0x3F) PlaySong(0x118); }
/* Keep gravity outside the pose macro: duplicating it inside PROBES changes
 * which copies of the branch tails agbcc merges. */
#define BACKWARD_POSE() \
    SPEED() \
    entity->velX.whole = -entity->velX.whole; \
    PHASE = 10; FRAME = 9; CREAKING_SKULL_POSE_DELAY(entity) = TIMER >> 1; \
    POSE(0x1000, 0x1800)
#define BACKWARD_EXISTING() BACKWARD_POSE() GRAVITY()
#define BACKWARD() TIMER = times0[CREAKING_SKULL_VARIANT(entity)]; BACKWARD_EXISTING()
#define STOP() \
    TIMER = times0[CREAKING_SKULL_VARIANT(entity)]; entity->velX.whole = 0; PHASE = 7; FRAME = 8; \
    POSE(-0x1000, 0x800) \
    GRAVITY()
#define FORWARD() \
    TIMER = times0[CREAKING_SKULL_VARIANT(entity)]; PHASE = 1; FRAME = 9; CREAKING_SKULL_POSE_DELAY(entity) = TIMER >> 1; \
    POSE(-0x1000, 0x1800) \
    SPEED()
#define PROBES(BLOCKED) \
    if (entity->flipX) offset = 50; else offset = -50; \
    DISTANCE() \
    if (distance <= 49 || TERRAIN()) \
    { \
        if (entity->flipX) offset = -50; else offset = 50; \
        if (!TERRAIN()) { BLOCKED } \
    } \
    GRAVITY()
#define ATTACK(S) entity->state = 1; PHASE = S;
/**
 * @brief Movement state (0): coordinate joint poses for strides, pauses,
 * turns and attack selection. Shared with Giant Skeleton; retain the
 * unused stack copy, extra RNG call and macro/temporary structure.
 */
void CreakingSkull_UpdateMovement(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    u8 times0[2], times1[2], times2[2], times3[2];
    s8 chain[13];
    u8 unused[2];
    s32 distance, offset, direction;
    memcpy(times0, sCreakingSkullTimes0, 2);
    memcpy(times1, sCreakingSkullTimes1, 2);
    memcpy(times2, sCreakingSkullTimes2, 2);
    memcpy(times3, sUnk_08119076, 2);
    memcpy(chain, sUnk_08119063, 13);
    /* Retain the original unused stack copy. */
    memcpy(unused, sUnk_08119076, 2);
    switch (PHASE)
    {
    case 0:
        CreakingSkull_FaceTarget(entity);
        if (Target_IsNearEntity(entity, 200, 160) || Enemy_IsDamaged(entity))
        {
            PHASE = 2; SUBPHASE = 0; TIMER = times0[CREAKING_SKULL_VARIANT(entity)]; FRAME = 5;
            CreakingSkull_AttachJointSetAngle(entity, 12, 11, -0x1000, 0x1F0000);
            CreakingSkull_AttachJointSetAngle(entity, 1, 0, -0x800, 0x1E0000);
            CreakingSkull_AttachJointSetAngle(entity, 2, 1, -0x1000, 0x1C0000);
            SPEED() GRAVITY()
        }
        break;
    case 1:
        UPDATE() FALL()
        if ((s8)--CREAKING_SKULL_POSE_DELAY(entity) <= 0) FRAME = 4;
        DEBRIS(0)
        COUNTDOWN()
        TIMER = times0[CREAKING_SKULL_VARIANT(entity)]; PHASE++; FRAME = 10; CREAKING_SKULL_POSE_DELAY(entity) = TIMER >> 1;
        POSE(-0x1000, -0x1800)
        break;
    case 2:
        UPDATE() FALL()
        if ((s8)--CREAKING_SKULL_POSE_DELAY(entity) <= 0) FRAME = 5;
        DEBRIS(0)
        if ((s8)--TIMER > 4) break;
        TIMER = times2[CREAKING_SKULL_VARIANT(entity)]; PHASE++; SUBPHASE = 0; FRAME = 5;
        break;
    case 3:
        UPDATE() COUNTDOWN()
        if (SUBPHASE == 0)
        {
            SUBPHASE++; TIMER = 1;
            CreakingSkull_AttachJointSetAngle(entity, 12, 11, 0x13000, 0x1F0000);
            CreakingSkull_AttachJointSetAngle(entity, 1, 0, 0x17000, 0x1E0000);
            CreakingSkull_AttachJointSetAngle(entity, 2, 1, 0x1D000, 0x1C0000);
            break;
        }
        if (entity->unk_518.part8.enemyId == 0x21 && (RandomNumberGenerator2() & 0x7F) <= 0x1F)
        { ATTACK(20) break; }
        DISTANCE()
        if (distance <= 99)
        {
            if ((RandomNumberGenerator2() & 0x7F) <= 0x3F) { ATTACK(0) break; }
            if ((RandomNumberGenerator2() & 0x7F) <= 0x3F)
            { CreakingSkull_FaceTarget(entity); BACKWARD() break; }
        }
        direction = Entity_GetHorizontalRoomEdge(entity, 24);
        if (direction > 0)
        {
            direction--; CreakingSkull_FaceTarget(entity);
            if ((direction == 0 && !entity->flipX) || (direction == 1 && entity->flipX))
            { BACKWARD() break; }
        }
        CreakingSkull_FaceTarget(entity);
        TIMER = times1[CREAKING_SKULL_VARIANT(entity)]; PHASE++; FRAME = 8;
        POSE(0x1000, 0x800)
        entity->velX.whole = 0;
        PROBES(TIMER = times0[CREAKING_SKULL_VARIANT(entity)]; BACKWARD_POSE())
        break;
    case 4:
        UPDATE() FALL() COUNTDOWN()
        TIMER = times1[CREAKING_SKULL_VARIANT(entity)]; PHASE++; FRAME = 7;
        POSE(0x1000, -0x800)
        break;
    case 5:
        UPDATE() FALL()
        if ((s8)--TIMER > 4) break;
        TIMER = times3[CREAKING_SKULL_VARIANT(entity)]; PHASE++; SUBPHASE = 0; FRAME = 7;
        PlaySong(0x7C);
        break;
    case 6:
        UPDATE() COUNTDOWN()
        if (SUBPHASE == 0)
        {
            SUBPHASE++; TIMER = 1;
            CreakingSkull_AttachJointSetAngle(entity, 12, 11, 0x15000, 0x1F0000);
            CreakingSkull_AttachJointSetAngle(entity, 1, 0, 0x17000, 0x1E0000);
            CreakingSkull_AttachJointSetAngle(entity, 2, 1, 0x1D000, 0x1C0000);
            break;
        }
        if (entity->unk_518.part8.enemyId == 0x21 && (RandomNumberGenerator2() & 0x7F) <= 0x1F)
        { ATTACK(20) break; }
        DISTANCE()
        if (distance <= 99)
        {
            if ((RandomNumberGenerator2() & 0x7F) <= 0x3F) { ATTACK(0) break; }
            if ((RandomNumberGenerator2() & 0x7F) <= 0x3F)
            { CreakingSkull_FaceTarget(entity); STOP() break; }
        }
        direction = Entity_GetHorizontalRoomEdge(entity, 24);
        if (direction > 0)
        {
            direction--; CreakingSkull_FaceTarget(entity);
            if ((direction == 0 && !entity->flipX) || (direction == 1 && entity->flipX))
            { STOP() break; }
        }
        CreakingSkull_FaceTarget(entity);
        FORWARD()
        PROBES(entity->velX.whole = 0; PHASE = 7; FRAME = 8; POSE(-0x1000, 0x800))
        break;
    case 7:
        UPDATE() FALL() COUNTDOWN()
        TIMER = times0[CREAKING_SKULL_VARIANT(entity)]; PHASE++; FRAME = 5;
        POSE(-0x1000, -0x800)
        break;
    case 8:
        UPDATE() FALL()
        if ((s8)--TIMER > 4) break;
        TIMER = times2[CREAKING_SKULL_VARIANT(entity)]; PHASE++; SUBPHASE = 0; FRAME = 5;
        break;
    case 9:
        UPDATE() COUNTDOWN()
        if (SUBPHASE == 0)
        {
            SUBPHASE++; TIMER = 1;
            CreakingSkull_AttachJointSetAngle(entity, 12, 11, 0x13000, 0x1F0000);
            CreakingSkull_AttachJointSetAngle(entity, 1, 0, 0x17000, 0x1E0000);
            CreakingSkull_AttachJointSetAngle(entity, 2, 1, 0x1D000, 0x1C0000);
            break;
        }
        if ((RandomNumberGenerator2() & 0x7F) <= 0x3F)
        { DISTANCE() if (distance <= 99) { ATTACK(0) break; } }
        else if ((RandomNumberGenerator2() & 0x7F) <= 0x1F && entity->unk_518.part8.enemyId == 0x21)
        { ATTACK(20) break; }
        CreakingSkull_FaceTarget(entity);
        TIMER = times0[CREAKING_SKULL_VARIANT(entity)]; PHASE = 4; FRAME = 8;
        POSE(0x1000, 0x800)
        entity->velX.whole = 0;
        PROBES(BACKWARD_POSE())
        break;
    case 10:
        UPDATE() FALL()
        if ((s8)--CREAKING_SKULL_POSE_DELAY(entity) <= 0) FRAME = 4;
        DEBRIS(1)
        COUNTDOWN()
        TIMER = times0[CREAKING_SKULL_VARIANT(entity)]; PHASE++; FRAME = 12; CREAKING_SKULL_POSE_DELAY(entity) = TIMER >> 1;
        POSE(0x1000, -0x1800)
        break;
    case 11:
        UPDATE() FALL()
        if ((s8)--CREAKING_SKULL_POSE_DELAY(entity) <= 0) FRAME = 7;
        DEBRIS(1)
        if ((s8)--TIMER > 4) break;
        TIMER = times2[CREAKING_SKULL_VARIANT(entity)]; PHASE++; FRAME = 7;
        break;
    case 12:
        UPDATE() COUNTDOWN()
        if (SUBPHASE == 0)
        {
            SUBPHASE++; TIMER = 1;
            CreakingSkull_AttachJointSetAngle(entity, 12, 11, 0x15000, 0x1F0000);
            CreakingSkull_AttachJointSetAngle(entity, 1, 0, 0x17000, 0x1E0000);
            CreakingSkull_AttachJointSetAngle(entity, 2, 1, 0x1D000, 0x1C0000);
            break;
        }
        if ((RandomNumberGenerator2() & 0x7F) <= 0x3F)
        { DISTANCE() if (distance <= 99) { ATTACK(0) break; } }
        else if ((RandomNumberGenerator2() & 0x7F) <= 0x1F && entity->unk_518.part8.enemyId == 0x21)
        { ATTACK(20) break; }
        CreakingSkull_FaceTarget(entity);
        FORWARD()
        PROBES(entity->velX.whole = 0; PHASE = 7; FRAME = 8; POSE(-0x1000, 0x800))
        break;
    case 20:
        CreakingSkull_FaceTarget(entity);
        POSE(0, 0)
        TIMER = 14; FRAME = 1; PHASE++; SUBPHASE = 0;
        /* Fall through. */
    case 21:
        UPDATE() COUNTDOWN()
        PHASE++; TIMER = 24; FRAME = 2;
        break;
    case 22:
        UPDATE() COUNTDOWN()
        PHASE++; TIMER = 14; FRAME = 3;
        break;
    case 23:
        UPDATE() COUNTDOWN()
        PHASE++; TIMER = 24; FRAME = 0;
        break;
    case 24:
        UPDATE() COUNTDOWN()
        CreakingSkull_FaceTarget(entity);
        CreakingSkull_AttachJointSetAngle(entity, 12, 11, 0x14000, 0x1F0000);
        CreakingSkull_AttachJointSetAngle(entity, 1, 0, 0x17800, 0x1E0000);
        CreakingSkull_AttachJointSetAngle(entity, 2, 1, 0x1E000, 0x1C0000);
        PHASE = 21; TIMER = 14; FRAME = 1;
        if (Target_IsNearEntity(entity, 250, 160) || Enemy_IsDamaged(entity) == 1)
        {
            PHASE = 2; SUBPHASE = 0; TIMER = times0[CREAKING_SKULL_VARIANT(entity)]; FRAME = 5;
            CreakingSkull_AttachJointSetAngle(entity, 12, 11, -0x1000, 0x1F0000);
            CreakingSkull_AttachJointSetAngle(entity, 1, 0, -0x800, 0x1E0000);
            CreakingSkull_AttachJointSetAngle(entity, 2, 1, -0x1000, 0x1C0000);
            SPEED() GRAVITY()
        }
        break;
    }
}
