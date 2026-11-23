/* Great Armor walk state machine (state == 0): steps with dust and shake, turns at ledges and walls, starts attacks. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code_08011DD0.h"
#include "structs/ewram.h"
extern void *memcpy(void *, const void *, unsigned long);
extern void GreatArmor_FaceTarget(struct EwramData_EntityData *);
extern void GreatArmor_SaveParts(struct EwramData_EntityData *);
extern void GreatArmor_RestoreParts(struct EwramData_EntityData *);
extern void GreatArmor_UpdateParts(struct EwramData_EntityData *, s8 *);
extern void GreatArmor_AttachPartSetAngle(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern s32 Target_GetPixelX(struct EwramData_EntityData *);
extern s32 Tile_Classify(s16, s16);
extern s32 Entity_GetHorizontalRoomEdge(struct EwramData_EntityData *, s32);
extern s32 Entity_MoveWithCollisionDefault(struct EwramData_EntityData *, s32, s32);
extern void Effect_SpawnFadingParticle(s32, s32, s32, s32, s32, s32);
extern s32 Player_GetFocusedActionFlags(void);
extern const s8 sUnk_08118FBE[3], sUnk_08118FC1[3], sUnk_08118FC4[3], sGreatArmorChain[14];

static inline s32 ABS(s32 x)
{
    if (x < 0)
        x = -x;
    return x;
}

#define STATE entity->subState
#define TIMER entity->timer
#define FRAME entity->ext.joint.pose
#define VARIANT ((s8)entity->ext.greatArmor.variant)
#define WAS_HIT (*(s8 *)&entity->ext.greatArmor.hit)
#define HIT_COUNT ((s8)entity->ext.greatArmor.hitCount)
#define UPDATE() GreatArmor_UpdateParts(entity, chain);
#define GRAVITY() Entity_MoveWithCollisionDefault(entity, 0x10000, -0x300000);
#define COUNTDOWN() if ((s8)--TIMER > 0) break;
#define POSE(A,B) \
    GreatArmor_AttachPartSetAngle(entity, 12, 13, A, 0x130000); \
    GreatArmor_AttachPartSetAngle(entity, 13, 11, B, 0xC0000); \
    GreatArmor_AttachPartSetAngle(entity, 10, 11, B, 0x80000);
#define POSE3(A) POSE(A,A)
#define VELX() if (entity->flipX) entity->velX.whole = 0x140000 / (TIMER * 2); else entity->velX.whole = -0x140000 / (TIMER * 2);
#define LAND() entity->velY.whole = 0x10000; entity->accelY.whole = 0x2800;
#define PROBE(D) Tile_Classify(camera->xPos.part16.integer + entity->posX.part16.integer + (D), camera->yPos.part16.integer + entity->posY.part16.integer)
#define NEAR(N) (ABS((s16)Target_GetPixelX(entity) - (s16)entity->posX.part16.integer) <= (N))
#define SHAKE() if (TIMER >= t1[VARIANT] - 10 && !(TIMER & 3)) ScreenShake_RequestOffset(0, 0, 1);
#define ATTACK(S) { entity->state = 1; STATE = S; break; }
#define RECOVER(S) TIMER = 1; STATE = S; entity->state = 0; GreatArmor_RestoreParts(entity);
#define STOMP(DX) \
    dx = DX; \
    Effect_SpawnFadingParticle(entity->posX.whole + dx, entity->posY.whole, (RandomNumberGenerator() & 0xFFFF) + 0x10000, -0x4000, entity->spriteLayer - 1, 1); \
    Effect_SpawnFadingParticle(entity->posX.whole + dx, entity->posY.whole, -0x10000 - (RandomNumberGenerator() & 0xFFFF), -0x4000, entity->spriteLayer - 1, 1); \
    ScreenShake_RequestOffset(0, 1, 2);
/* turn around at a ledge */
#define EDGE_TURN(S, F, A) \
    if (WAS_HIT != 0 && HIT_COUNT > 3) \
    { \
        dx = entity->flipX ? -30 : 30; \
        if (PROBE(dx) == 0) \
        { \
            entity->ext.greatArmor.hopTurns = 0; entity->ext.greatArmor.hit = 0; entity->ext.greatArmor.hitCount = 0; \
            GreatArmor_FaceTarget(entity); \
            TIMER = t0[VARIANT]; VELX() entity->velX.whole = -entity->velX.whole; \
            STATE = S; FRAME = F; POSE3(A) LAND() \
            break; \
        } \
    }
#define SPECIAL() \
    if (entity->enemyId != 0x3C && (RandomNumberGenerator2() & 0x7F) < 0x20) ATTACK(0x28) \
    if (entity->enemyId == 0x67 && (Player_GetFocusedActionFlags() & 0x360)) { STATE = 0x1E; break; }
#define BACK_TURN(S, F, A) \
    { \
        dx = entity->flipX ? -30 : 30; \
        if (PROBE(dx) == 0) \
        { \
            entity->velX.whole = -entity->velX.whole; \
            STATE = S; FRAME = F; POSE3(A) \
        } \
    }
/* walk states 3 and 6 */
#define WALK_A(ES, EF, A, NEXT, NF, TS, TF) \
    SHAKE() COUNTDOWN() \
    EDGE_TURN(ES, EF, A) \
    if (NEAR(0x63)) \
    { \
        if ((RandomNumberGenerator2() & 0x7F) < 0x40) ATTACK(0) \
        if ((RandomNumberGenerator2() & 0x7F) < 0x40) \
        { \
            GreatArmor_FaceTarget(entity); \
            dx = entity->flipX ? -30 : 30; \
            if (PROBE(dx) == 0) \
            { \
                TIMER = t0[VARIANT]; VELX() entity->velX.whole = -entity->velX.whole; \
                STATE = TS; FRAME = TF; POSE3(A) LAND() \
                break; \
            } \
        } \
    } \
    SPECIAL() \
    GreatArmor_FaceTarget(entity); \
    TIMER = t0[VARIANT]; NEXT; FRAME = NF; POSE3(A) VELX() \
    dx = entity->flipX ? 40 : -40; \
    if (!NEAR(0x1D) && PROBE(dx) == 0 \
        && !((!entity->flipX && Entity_GetHorizontalRoomEdge(entity, 0x10) == 1) || (entity->flipX && Entity_GetHorizontalRoomEdge(entity, 0x10) == 2))) \
        ; \
    else \
        BACK_TURN(TS, TF, A) \
    LAND() \
    break;
/* walk states 9 and 12 */
#define WALK_B(ES, EF, A, NS, NF, TS, TF) \
    SHAKE() COUNTDOWN() \
    EDGE_TURN(ES, EF, A) \
    if ((RandomNumberGenerator2() & 0x7F) < 0x40 && NEAR(0x4F)) ATTACK(0) \
    SPECIAL() \
    GreatArmor_FaceTarget(entity); \
    TIMER = t0[VARIANT]; STATE = NS; FRAME = NF; POSE3(A) VELX() \
    dx = entity->flipX ? 30 : -30; \
    if (!NEAR(0x1D) && PROBE(dx) == 0) \
        ; \
    else \
        BACK_TURN(TS, TF, A) \
    LAND() \
    break;
/* last walk step of a jump sequence */
#define HOP_END(S, F, A, NS, NF) \
    SHAKE() COUNTDOWN() \
    GreatArmor_FaceTarget(entity); \
    if (++*(s8 *)&entity->ext.greatArmor.hopTurns <= 1) \
    { \
        dx = entity->flipX ? -30 : 30; \
        if (PROBE(dx) == 0) \
        { \
            TIMER = 20; VELX() entity->velX.whole = -entity->velX.whole; \
            STATE = S; FRAME = F; POSE3(A) LAND() \
            break; \
        } \
    } \
    TIMER = t0[VARIANT]; STATE = NS; FRAME = NF; POSE3(A) VELX() LAND() \
    entity->ext.greatArmor.hopTurns = 0; entity->ext.greatArmor.hit = 0; entity->ext.greatArmor.hitCount = 0; \
    break;

/**
 * @brief Walk state machine (state 0): steps with dust and screen shake, turns at
 * ledges and walls and starts the attacks of state 1.
 */
void GreatArmor_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 dx;
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    s8 t0[3], t1[3];
    s8 chain[14];
    s8 t2[3];
    memcpy(t0, sUnk_08118FBE, 3);
    memcpy(t1, sUnk_08118FC1, 3);
    memcpy(chain, sGreatArmorChain, 14);
    memcpy(t2, sUnk_08118FC4, 3);
    switch (STATE)
    {
    case 0:
        GreatArmor_FaceTarget(entity);
        if (!Target_IsNearEntity(entity, 0xA0, 0xA0) && !Enemy_IsDamaged(entity))
            break;
        STATE = 2; entity->stage = 0; TIMER = t0[VARIANT]; FRAME = 3;
        VELX() LAND()
        break;
    case 1:
        UPDATE() GRAVITY() COUNTDOWN() TIMER = t0[VARIANT]; STATE++; FRAME = 3; POSE3(0x1000)
        break;
    case 2:
        UPDATE() GRAVITY() COUNTDOWN() TIMER = t1[VARIANT]; STATE++; FRAME = 3; POSE3(0)
        STOMP(entity->flipX ? 0xA0000 : -0xA0000) PlaySong(0x7C);
        break;
    case 3:
        WALK_A(0x32, 2, -0x1000, STATE++, 4, 10, 2)
    case 4:
        UPDATE() GRAVITY() COUNTDOWN() TIMER = t0[VARIANT]; STATE++; FRAME = 5; POSE3(-0x1000)
        break;
    case 5:
        UPDATE() GRAVITY() COUNTDOWN() TIMER = t1[VARIANT]; STATE++; FRAME = 5; POSE3(0)
        STOMP(entity->flipX ? 0xA0000 : -0xA0000) PlaySong(0x7C);
        break;
    case 6:
        WALK_A(0x2F, 4, 0x1000, STATE = 1, 2, 7, 4)
    case 7:
        UPDATE() GRAVITY() COUNTDOWN() TIMER = t0[VARIANT]; STATE++; FRAME = 3; POSE3(0x1000)
        break;
    case 8:
        UPDATE() GRAVITY() COUNTDOWN() TIMER = t1[VARIANT]; STATE++; FRAME = 3;
        STOMP(entity->flipX ? -0x80000 : 0x80000) POSE3(0) PlaySong(0x7C);
        break;
    case 9:
        WALK_B(0x32, 2, -0x1000, 4, 4, 10, 2)
    case 10:
        UPDATE() GRAVITY() COUNTDOWN() TIMER = t0[VARIANT]; STATE++; FRAME = 5; POSE3(-0x1000)
        break;
    case 11:
        UPDATE() GRAVITY() COUNTDOWN() TIMER = t1[VARIANT]; STATE++; FRAME = 5;
        STOMP(entity->flipX ? -0x80000 : 0x80000) POSE3(0) PlaySong(0x7C);
        break;
    case 12:
        WALK_B(0x2F, 4, 0x1000, 1, 2, 7, 4)
    case 20:
        GreatArmor_FaceTarget(entity);
        if (!Target_IsNearEntity(entity, 0xC8, 0xA0) && !Enemy_IsDamaged(entity))
            break;
        STATE++; entity->stage = 0; TIMER = 20; FRAME = 1;
        POSE(-0x3300, -0x100) PlaySong(0x64);
        break;
    case 21:
        UPDATE() COUNTDOWN() STATE++; TIMER = 40; POSE3(0)
        ((s32 *)entity->ext.greatArmor.parts)[0x7C] = 0x160000;
        break;
    case 22:
        COUNTDOWN() STATE = 2; TIMER = t0[VARIANT]; FRAME = 3; POSE3(0x1000) VELX() LAND()
        break;
    case 30:
        GreatArmor_FaceTarget(entity); GreatArmor_SaveParts(entity);
        entity->ext.greatArmor.guard = 1;
        if ((s8)FRAME == 3)
        {
            FRAME = 14; STATE = 31; POSE(-0x800, -0x2000)
            entity->ext.jointAttack.phase = 0; TIMER = t2[0];
        }
        else
        {
            FRAME = 15; STATE = 36; POSE(0x1800, 0)
            entity->ext.jointAttack.phase = 0; TIMER = t2[0] / 2 + (u8)t2[0];
        }
        break;
    case 31:
        entity->ext.greatArmor.guard = 1; UPDATE() COUNTDOWN() STATE++; TIMER = 120; POSE3(0)
        break;
    case 32:
        entity->ext.greatArmor.guard = 1; UPDATE() COUNTDOWN() FRAME = 3; STATE++; TIMER = 20; POSE(0x800, 0x2000)
        break;
    case 34:
        entity->ext.greatArmor.guard = 1; UPDATE() COUNTDOWN() RECOVER(3)
        break;
    case 36:
        entity->ext.greatArmor.guard = 1; UPDATE() COUNTDOWN() STATE++; TIMER = 120; POSE3(0)
        break;
    case 37:
        entity->ext.greatArmor.guard = 1; UPDATE() COUNTDOWN() FRAME = 5; STATE++; TIMER = 20; POSE(-0x1800, 0)
        break;
    case 33: case 38:
        entity->ext.greatArmor.guard = 1; UPDATE() if ((s8)--TIMER > 4) break;
        TIMER = 10; STATE++;
        break;
    case 39:
        entity->ext.greatArmor.guard = 1; UPDATE() COUNTDOWN() RECOVER(6)
        break;
    case 47:
        UPDATE() GRAVITY() COUNTDOWN() TIMER = 20; STATE++; FRAME = 0x21; POSE3(0x1000)
        break;
    case 48:
        UPDATE() GRAVITY() COUNTDOWN() if ((s8)entity->ext.greatArmor.hopTurns == 0) TIMER = 14; else TIMER = 30; STATE++; FRAME = 0x21;
        STOMP(entity->flipX ? -0x80000 : 0x80000) POSE3(0) PlaySong(0x7C);
        break;
    case 49:
        HOP_END(0x32, 0x20, -0x1000, 4, 4)
    case 50:
        UPDATE() GRAVITY() COUNTDOWN() TIMER = 20; STATE++; FRAME = 0x23; POSE3(-0x1000)
        break;
    case 51:
        UPDATE() GRAVITY() COUNTDOWN() if ((s8)entity->ext.greatArmor.hopTurns == 0) TIMER = 14; else TIMER = 30; STATE++; FRAME = 0x23;
        STOMP(entity->flipX ? -0x80000 : 0x80000) POSE3(0) PlaySong(0x7C);
        break;
    case 52:
        HOP_END(0x2F, 0x22, 0x1000, 1, 2)
    }
}
