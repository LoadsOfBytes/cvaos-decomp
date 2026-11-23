/* Golem attack state machine (state == 1): overhead punches (states 0-17), ground smashes with rocks
   (20-35) and the repeated jab (40-56). */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code_08011DD0.h"
#include "structs/ewram.h"
extern void *memcpy(void *, const void *, unsigned long);
extern void Golem_FaceTarget(struct EwramData_EntityData *);
extern void Golem_AttachPart(struct EwramData_EntityData *, s32, s32, s32, s32);
extern void Golem_UpdateParts(struct EwramData_EntityData *, s8 *);
extern void Golem_ResetPartAngles(struct EwramData_EntityData *, s32);
extern void Effect_SpawnRisingParticle(s32, s32, s32, s32);
extern void Effect_SpawnFallingPiece(s32, s32, s32, s32, s32, s32);
extern void Golem_ThrowDebris(struct EwramData_EntityData *, s32, s32, s32, s32);
extern const s8 sGolemChain[11];
extern const u8 sUnk_08118EEB[4], sUnk_08118EEF[4], sUnk_08118EF3[4], sUnk_08118EF7[4];

#define STATE entity->subState
#define TIMER entity->timer
#define FRAME entity->ext.joint.pose
#define SWING entity->ext.joint.hurtMask
#define COUNT entity->ext.raw.unk_504.part8.unk_505
#define IDX entity->ext.golem.variant
#define UPDATE() Golem_UpdateParts(entity, chain);
#define COUNTDOWN() if ((s8)--TIMER > 0) break;
#define J(A, B, ANGLE, LEN) Golem_AttachPart(entity, A, B, ANGLE, LEN);
#define R(A) Golem_ResetPartAngles(entity, A);
#define DONE(S) STATE = S; entity->state = 0;
#define DEBRIS() \
    { \
        s32 vx = (RandomNumberGenerator() & 0xFFFF) - 0x8000; \
        s32 vy = -0x10000 - (RandomNumberGenerator() & 0x1FFFF); \
        Effect_SpawnFallingPiece(entity->posX.whole + dx, entity->posY.whole, vx, vy, entity->spriteLayer - 1, 0); \
    }
#define STOMP() \
    { ScreenShake_RequestOffset(0, 1, 2); \
    dx = entity->flipX ? 0x340000 : -0x340000; \
    Effect_SpawnRisingParticle(entity->posX.whole + dx, entity->posY.whole, entity->spriteLayer - 1, 1); \
    DEBRIS() DEBRIS() \
    PlaySong(0x80); }
#define ROCK() \
    { \
        s32 vx; \
        if (entity->flipX) { dx = 0x10000; vx = RandomNumberGenerator() & 0x1FFFF; } else { dx = -0x10000; vx = -(RandomNumberGenerator() & 0x1FFFF); } \
        Effect_SpawnFallingPiece(entity->posX.whole + dx, entity->posY.whole, vx + dx, -0x20000 - (RandomNumberGenerator() & 0x1FFFF), entity->spriteLayer - 1, 0); \
    }
#define SMASH() \
    if (TIMER == 6) \
    { \
        s32 v; \
        ScreenShake_RequestOffset(0, 1, 2); \
        dx = entity->flipX ? 0x10000 : -0x10000; \
        Effect_SpawnRisingParticle(entity->posX.whole + dx, entity->posY.whole, entity->spriteLayer - 1, 1); \
        ROCK() ROCK() \
        if (entity->flipX) { dx = 0x10000; v = 0x48000; } else { dx = -0x10000; v = -0x48000; } \
        Golem_ThrowDebris(entity, entity->posX.whole + dx, entity->posY.whole, v, -0x30000); \
        PlaySong(0x80); \
    }
#define P8(N) (*(s32 *)((u8 *)parts + (N) * 0x20 + 8))
#define SHAKE() if (!(TIMER & 3)) ScreenShake_RequestOffset(0, 0, 1);

/**
 * @brief Attack state 1: coordinate joint poses for overhead strikes,
 * ground smashes with debris and repeated jabs, then return to
 * movement. Preserve all joint macros and signed countdowns.
 */
void Golem_UpdateAttack(struct EwramData_EntityData *entity)
{
    s8 chain[11];
    u8 t0[4], t1[4], t2[4], t3[4], t4[4]; /* state timers, indexed by the difficulty variant */
    struct GolemPart *parts;
    s32 dx; /* ground offset of the fist, shared by all impacts */

    memcpy(chain, sGolemChain, 11);
    memcpy(t0, sUnk_08118EEB, 4);
    memcpy(t1, sUnk_08118EEB, 4);
    memcpy(t2, sUnk_08118EEF, 4);
    memcpy(t3, sUnk_08118EF3, 4);
    memcpy(t4, sUnk_08118EF7, 4);
    parts = entity->ext.golem.parts;
    switch (STATE)
    {
    case 0:
        if (entity->enemyId != 0x65) { STATE = 0x28; break; }
        Golem_FaceTarget(entity);
        if ((s8)FRAME == 2)
        {
            FRAME = 11; STATE = 1;
            J(3, 2, 0x1C00, 0) J(9, 5, 0, 0) J(10, 9, 0x4800, 0) J(4, 5, 0x400, 0)
            SWING = 0x400; entity->ext.golem.attackPhase = 0;
        }
        else
        {
            FRAME = 16; STATE = 10;
            J(3, 2, 0x4800, 0) J(10, 9, 0x1000, 0) J(4, 5, 0x400, 0)
            SWING = 8; entity->ext.golem.attackPhase = 0;
        }
        TIMER = t0[IDX];
        break;
    case 1:
        UPDATE() COUNTDOWN() TIMER = t1[IDX]; STATE++; FRAME = 12;
        J(3, 2, 0x1C00, 0) J(9, 5, 0, 0) J(10, 9, 0x4800, 0) J(4, 5, 0x400, 0)
        break;
    case 2:
        UPDATE() COUNTDOWN() TIMER = t2[IDX]; STATE++;
        break;
    case 3:
        UPDATE() COUNTDOWN() TIMER = 6; STATE++; FRAME = 13;
        J(3, 2, -0x3400, 0) J(4, 5, -0x1400, 0) J(2, 5, -0x1400, 0) J(9, 5, -0x2200, 0) J(10, 9, -0x4000, 0)
        PlaySong(0x87);
        break;
    case 4:
        UPDATE() COUNTDOWN() TIMER = 6; STATE++; FRAME = 14;
        J(3, 2, -0x3400, 0) J(4, 5, -0x1400, 0) J(2, 5, -0x1400, 0) J(9, 5, -0x2200, 0) J(10, 9, -0x4000, 0)
        break;
    case 5:
        UPDATE() COUNTDOWN() TIMER = 1; STATE++; FRAME = 15;
        J(3, 2, 0, 0) J(4, 5, 0, 0) J(2, 5, 0, 0) J(9, 5, 0, 0) J(10, 9, 0, 0)
        STOMP()
        break;
    case 6:
        UPDATE() COUNTDOWN() TIMER = 60; STATE++;
        R(3) R(4) R(2) J(9, 5, 0x2200, 0) J(10, 9, 0x4000, 0)
        break;
    case 7:
        if (TIMER > 0x27 && !(TIMER & 3)) ScreenShake_RequestOffset(0, 0, 1);
        COUNTDOWN() TIMER = 40; STATE++; FRAME = 2; SWING = 0;
        R(9) R(10)
        break;
    case 8:
        UPDATE() COUNTDOWN() TIMER = 20; DONE(3)
        break;
    case 10:
        UPDATE() COUNTDOWN() TIMER = t1[IDX]; STATE++; FRAME = 17;
        J(3, 2, 0x4800, 0) J(10, 9, 0x1000, 0) J(4, 5, 0x400, 0)
        break;
    case 11:
        UPDATE() COUNTDOWN() TIMER = t2[IDX]; STATE++; FRAME = 17;
        J(3, 2, 0, 0) J(10, 9, 0, 0)
        break;
    case 12:
        UPDATE() COUNTDOWN() TIMER = 6; STATE++; FRAME = 18;
        J(3, 2, -0x4000, 0) J(4, 5, -0x1400, 0) J(2, 5, -0x2400, 0) J(9, 5, -0x1400, 0) J(10, 9, -0x2C00, 0)
        PlaySong(0x87);
        break;
    case 13:
        UPDATE() COUNTDOWN() TIMER = 6; STATE++; FRAME = 19;
        J(3, 2, -0x4000, 0) J(4, 5, -0x1400, 0) J(2, 5, -0x2400, 0) J(9, 5, -0x1400, 0) J(10, 9, -0x2C00, 0)
        break;
    case 14:
        UPDATE() COUNTDOWN() TIMER = 1; STATE++; FRAME = 20;
        J(3, 2, 0, 0) J(4, 5, 0, 0) J(2, 5, 0, 0) J(9, 5, 0, 0) J(10, 9, 0, 0)
        STOMP()
        break;
    case 15:
        UPDATE() COUNTDOWN() TIMER = 60; STATE++;
        R(3) R(4) R(2) R(9) R(10)
        break;
    case 16:
        if (TIMER > 0x27 && !(TIMER & 3)) ScreenShake_RequestOffset(0, 0, 1);
        COUNTDOWN() TIMER = 40; STATE++; FRAME = 4; SWING = 0;
        break;
    case 20:
        Golem_FaceTarget(entity);
        if ((s8)FRAME == 2)
        {
            FRAME = 21; STATE = 31;
            J(8, 7, -0x1000, 0xD0000) J(7, 6, 0x14000, 0x50000) J(5, 6, -0x1000, 0) J(4, 5, -0x1000, 0)
            J(9, 6, -0x1000, 0) J(10, 9, -0x2000, 0) J(2, 6, -0x1000, 0) J(3, 2, -0x1000, 0)
            SWING = 0x100; entity->ext.golem.attackPhase = 0;
        }
        else
        {
            FRAME = 23; STATE = 21;
            J(1, 6, -0x1000, 0x120000) J(5, 6, -0x1000, 0) J(4, 5, -0x1000, 0) J(9, 6, -0x1000, 0) J(10, 9, -0x1000, 0)
            SWING = 2; entity->ext.golem.attackPhase = 0;
        }
        TIMER = t3[IDX];
        break;
    case 21:
        UPDATE() COUNTDOWN() TIMER = t4[IDX]; STATE++;
        J(1, 6, 0, 0) J(5, 6, 0, 0) J(4, 5, 0, 0) J(9, 6, 0, 0) J(10, 9, 0, 0)
        break;
    case 22:
        COUNTDOWN() TIMER = 10; STATE++;
        J(1, 6, 0x4000, 0x130000) J(5, 6, 0x1800, 0) J(4, 5, 0x1800, 0) J(9, 6, 0x1800, 0) J(10, 9, 0x1000, 0)
        FRAME = 24;
        break;
    case 23:
        UPDATE() SMASH() COUNTDOWN() TIMER = 12; STATE++; SWING = 0;
        J(1, 6, 0, 0) J(5, 6, 0, 0) J(4, 5, 0, 0) J(9, 6, 0, 0) J(10, 9, 0, 0)
        break;
    case 24:
        SHAKE() COUNTDOWN() TIMER = 30; STATE++; FRAME = 4;
        R(1) R(5) R(4) R(9) R(10)
        break;
    case 31:
        UPDATE() COUNTDOWN() TIMER = t4[IDX]; STATE++;
        J(8, 7, 0, 0) J(7, 6, 0x14000, 0x50000) J(5, 6, 0, 0) J(4, 5, 0, 0)
        J(9, 6, 0, 0) J(10, 9, 0, 0) J(2, 6, 0, 0) J(3, 2, 0, 0)
        break;
    case 32:
        COUNTDOWN() TIMER = 10; STATE++;
        J(8, 7, 0x4800, 0xD0000) J(7, 6, 0x14000, 0x50000) J(5, 6, 0x1800, 0) J(4, 5, 0x1800, 0)
        J(9, 6, 0x1800, 0) J(10, 9, 0x1000, 0) J(2, 6, 0x1800, 0) J(3, 2, 0x2000, 0)
        FRAME = 22;
        break;
    case 33:
        UPDATE() SMASH() COUNTDOWN() TIMER = 12; STATE++; SWING = 0;
        J(8, 7, 0, 0) J(7, 6, 0x14000, 0x50000) J(5, 6, 0, 0) J(4, 5, 0, 0)
        J(9, 6, 0, 0) J(10, 9, 0, 0) J(2, 6, 0, 0) J(3, 2, 0, 0)
        break;
    case 34:
        SHAKE() COUNTDOWN() TIMER = 30; STATE++; FRAME = 2;
        R(8) R(7) R(5) R(4) R(9) R(10) R(3) R(2)
        break;
    case 35:
        UPDATE() COUNTDOWN() TIMER = 20; DONE(3)
        break;
    case 40:
        Golem_FaceTarget(entity);
        COUNT = 0;
        if ((s8)FRAME == 2)
        {
            FRAME = 25; STATE = 41;
            J(2, 5, 0, 0) J(9, 5, 0, 0) J(10, 9, 0x8000, 0)
            SWING = 0x400;
        }
        else
        {
            FRAME = 29; STATE = 51;
            J(9, 5, 0, 0) J(2, 5, 0, 0) J(3, 2, 0x8000, 0)
            SWING = 8;
        }
        entity->ext.golem.attackPhase = 1; TIMER = 12;
        break;
    case 41:
        UPDATE() COUNTDOWN() TIMER = 12 - (s8)COUNT * 2; STATE++; FRAME = 26;
        J(10, 9, 0x8000, 0) PlaySong(0x87);
        break;
    case 42:
        UPDATE() COUNTDOWN()
        COUNT++; if ((s8)COUNT > 1)
        {
            if (entity->flipX) P8(10) += 0x10000; else P8(10) -= 0x10000;
            TIMER = 6; STATE++; FRAME = 27;
            J(3, 2, -0x2000, 0) J(10, 9, 0x4000, 0) J(9, 5, -0x1800, 0)
        }
        else
        {
            if (entity->flipX) P8(10) += 0x10000; else P8(10) -= 0x10000;
            TIMER = 12 - (s8)COUNT * 2; STATE--; FRAME = 25;
            J(10, 9, 0x8000, 0)
        }
        break;
    case 43:
        UPDATE() COUNTDOWN() TIMER = 3; STATE++; FRAME = 28;
        J(9, 5, 0, 0) J(10, 9, 0, 0) PlaySong(0x87);
        break;
    case 44:
        UPDATE() COUNTDOWN() TIMER = 30; STATE++;
        R(9) R(10)
        break;
    case 45:
        COUNTDOWN() TIMER = 40; STATE++; FRAME = 2; SWING = 0;
        R(2) R(3) R(9) R(10)
        break;
    case 46:
        UPDATE() COUNTDOWN() TIMER = 20; DONE(3)
        break;
    case 51:
        UPDATE() COUNTDOWN() TIMER = 12 - (s8)COUNT * 2; STATE++; FRAME = 30;
        J(3, 2, 0x8000, 0) PlaySong(0x87);
        break;
    case 52:
        UPDATE() COUNTDOWN()
        COUNT++; if ((s8)COUNT > 1)
        {
            if (entity->flipX) P8(3) += 0x10000; else P8(3) -= 0x10000;
            TIMER = 6; STATE++; FRAME = 31;
            J(10, 9, -0x2000, 0) J(3, 2, 0x4000, 0) J(2, 5, -0x1800, 0)
        }
        else
        {
            if (entity->flipX) P8(3) += 0x10000; else P8(3) -= 0x10000;
            TIMER = 12 - (s8)COUNT * 2; STATE--; FRAME = 29;
            J(3, 2, 0x8000, 0)
        }
        break;
    case 53:
        UPDATE() COUNTDOWN() TIMER = 3; STATE++; FRAME = 32;
        J(2, 5, 0, 0) J(3, 2, 0, 0) PlaySong(0x87);
        break;
    case 54:
        UPDATE() COUNTDOWN() TIMER = 30; STATE++;
        R(2) R(3)
        break;
    case 55:
        COUNTDOWN() TIMER = 40; STATE++; FRAME = 4; SWING = 0;
        R(2) R(3) R(9) R(10)
        break;
    case 17: case 25: case 56:
        UPDATE() COUNTDOWN() TIMER = 20; DONE(6)
        break;
    }
}
