/* Great Armor attack state machine. */
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
extern void GreatArmor_AttachPart(struct EwramData_EntityData *, s32, s32, s32, s32);
extern void Effect_SpawnParticleWithDamping(s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern void Effect_SpawnFallingPiece(s32, s32, s32, s32, s32, s32);
extern s32 Tile_Classify(s16, s16);
extern void GreatArmor_SpawnShockwave(struct EwramData_EntityData *, s32, s32);
extern void GreatArmorSlashEffect_Spawn(struct EwramData_EntityData *, s32, s32, s32, s32, s32);
extern const s8 sUnk_08118FC7[3], sUnk_08118FCA[3], sUnk_08118FCD[3], sUnk_08118FD0[3], sUnk_08118FD3[3], sUnk_08118FD6[3], sUnk_08118FD9[3], sUnk_08118FDC[3], sGreatArmorChain[14];
#define STATE entity->subState
#define TIMER entity->timer
#define FRAME entity->ext.joint.pose
#define VARIANT ((s8)entity->ext.greatArmor.variant)
#define HIT entity->ext.greatArmor.attackHit
#define UPDATE() GreatArmor_UpdateParts(entity, chain);
#define COUNTDOWN() if ((s8)--TIMER > 0) break;
#define STEP(T,F) TIMER = T; STATE++; FRAME = F;
#define ARMS(A) \
    GreatArmor_AttachPartSetAngle(entity, 13, 11, A, 0xC0000); \
    GreatArmor_AttachPartSetAngle(entity, 10, 11, A, 0x80000);
#define POSE(A,B) GreatArmor_AttachPartSetAngle(entity, 12, 13, A, 0x130000); ARMS(B)
#define SLOW_ARMS(A) \
    GreatArmor_AttachPart(entity, 13, 11, A, 0xC0000); \
    GreatArmor_AttachPart(entity, 10, 11, A, 0x80000);
#define SLOW_POSE(A,B) GreatArmor_AttachPart(entity, 12, 13, A, 0x130000); SLOW_ARMS(B)
#define SHAKE() if ((s8)HIT != 0 && !(TIMER & 3)) ScreenShake_RequestOffset(0, 0, 1);
#define RECOVER(S) TIMER = 1; STATE = S; entity->state = 0; GreatArmor_RestoreParts(entity);
#define SPARKS() \
    if ((TIMER & 3) == 2) \
    { \
        s32 magnitude = RandomNumberGenerator() % 40 + 16; \
        s32 dy; register s32 y asm("r0"); /* FAKEMATCH: pinned register */ \
        if (entity->flipX) { dx = Sin(parts[0x7A] + 0x4000); dy = Sin(parts[0x7A]); } \
        else { dx = Sin(parts[0x7A] + 0xC000); dy = Sin(parts[0x7A] + 0x8000); } \
        dx *= magnitude; dy *= magnitude; \
        magnitude = entity->posX.whole + parts[0x78]; y = entity->posY.whole + parts[0x79]; \
        GreatArmorSlashEffect_Spawn(entity, magnitude + dx, y + dy, dx, dy, entity->spriteLayer - 1); \
    }
#define SMASH() \
    if (entity->flipX) dx = 0x440000; else dx = -0x440000; \
    { \
        s32 x = (s16)camera->xPos.part16.integer + entity->posX.part16.integer; s32 probeX; \
        if (dx < 0) probeX = x - ((-dx) >> 16); else probeX = x + (dx >> 16); \
        if (Tile_Classify(probeX, camera->yPos.part16.integer + entity->posY.part16.integer) != 1) HIT = 1; else HIT = 0; \
    } \
    if ((s8)HIT != 0) \
    { \
        \
        Effect_SpawnParticleWithDamping(entity->posX.whole + dx, entity->posY.whole, (RandomNumberGenerator() & 0xFFFF) + 0x10000, -0x4000, entity->spriteLayer - 1, 0, 8, 15, 6); \
        Effect_SpawnParticleWithDamping(entity->posX.whole + dx, entity->posY.whole, 0x10000 - (RandomNumberGenerator() & 0xFFFF), -0x4000, entity->spriteLayer - 1, 0, 8, 15, 6); \
        { s32 vx = (RandomNumberGenerator() & 0xFFFF) - 0x8000; s32 vy = -(RandomNumberGenerator() & 0x1FFFF) - 0x10000; \
        Effect_SpawnFallingPiece(entity->posX.whole + dx, entity->posY.whole, vx, vy, entity->spriteLayer - 1, 0); } \
        { s32 vx = (RandomNumberGenerator() & 0xFFFF) - 0x8000; s32 vy = -(RandomNumberGenerator() & 0x1FFFF) - 0x10000; \
        Effect_SpawnFallingPiece(entity->posX.whole + dx, entity->posY.whole, vx, vy, entity->spriteLayer - 1, 1); } \
        ScreenShake_RequestOffset(0, 1, 2); PlaySong(0x7F); \
        for (i = 0; i <= 2;) \
        { \
            s32 step, vx, vy, rx, ry, delta; s32 next; \
            if (entity->flipX) { dx = 0x440000; step = 0x20000; vx = ({ s32 k = 0x10000; u32 r = RandomNumberGenerator() & 0x7FFF; r + k; }); } \
            else { dx = -0x440000; step = -0x20000; vx = -(RandomNumberGenerator() & 0x7FFF) - 0x10000; } \
            vy = -0x8000 - (RandomNumberGenerator() & 0x7FFF); \
            Effect_SpawnParticleWithDamping(entity->posX.whole + dx + (delta = i * step), entity->posY.whole, vx, vy, entity->spriteLayer - 1, 0, 8, 15, 6); \
            rx = (RandomNumberGenerator() & 0xFFFF) - 0x8000 + vx; \
            ry = -(RandomNumberGenerator() & 0xFFFF); next = i + 1; ry -= next << 16; \
            Effect_SpawnFallingPiece(entity->posX.whole + dx + delta, entity->posY.whole, rx, ry, entity->spriteLayer - 1, RandomNumberGenerator() & 1); i = next; \
        } \
    }
#define SHOCKWAVE() \
    ScreenShake_RequestOffset(0, 1, 2); \
    if (entity->flipX) dx = 0x2A0000; else dx = -0x2A0000; \
    GreatArmor_SpawnShockwave(entity, dx, 0); \
    GreatArmorSlashEffect_Spawn(entity, entity->posX.whole + dx, entity->posY.whole, dx, 0x300000, entity->spriteLayer - 1); \
    PlaySong(0x7F);

/**
 * @brief Attack state machine (state 1): weapon swings posed through the arm chain
 * (parts 12, 13, 10 on 11), slash effects along part 12 and the ground
 * shockwave; returns to the walk state.
 */
void GreatArmor_UpdateAttack(struct EwramData_EntityData *entity)
{
    s32 dx;
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    s8 t0[3], t1[3], t2[3], t3[3], t4[3], t5[3], t6[3], t7[3], t8[3];
    s8 chain[14];
    s32 *parts;
    s32 i;
    memcpy(t0, sUnk_08118FC7, 3);
    memcpy(t1, sUnk_08118FCA, 3);
    memcpy(t2, sUnk_08118FC7, 3);
    memcpy(t3, sUnk_08118FCD, 3);
    memcpy(t4, sUnk_08118FD0, 3);
    memcpy(t5, sUnk_08118FD3, 3);
    memcpy(t6, sUnk_08118FD6, 3);
    memcpy(t7, sUnk_08118FD9, 3);
    memcpy(t8, sUnk_08118FDC, 3);
    memcpy(chain, sGreatArmorChain, 14);
    parts = (s32 *)entity->ext.greatArmor.parts;
    switch (STATE)
    {
    case 0:
        GreatArmor_FaceTarget(entity); GreatArmor_SaveParts(entity);
        if ((RandomNumberGenerator() & 0x7F) <= 0x3F)
        {
            if ((s8)FRAME == 3)
            {
                FRAME = 6; STATE = 1; POSE(0x5800, 0x4800)
                entity->ext.jointAttack.phase = 0; TIMER = t0[VARIANT];
            }
            else
            {
                FRAME = 10; STATE = 11; POSE(0x7800, 0x6800)
                entity->ext.jointAttack.phase = 0; TIMER = t0[VARIANT] / 2 + (u8)t0[VARIANT];
            }
        }
        else
        {
            if ((s8)FRAME == 3)
            {
                FRAME = 16; STATE = 21; SLOW_POSE(0x16800, 0x12000)
                entity->ext.jointAttack.phase = 0; TIMER = t3[VARIANT];
            }
            else
            {
                FRAME = 20; STATE = 31; SLOW_POSE(0x16800, 0x12000)
                entity->ext.jointAttack.phase = 0; TIMER = t3[VARIANT];
            }
        }
        break;
    case 1:
        UPDATE() COUNTDOWN() FRAME = 7; STATE++; TIMER = t1[VARIANT]; POSE(0x400,0x200)
        break;

    case 2:
        UPDATE() COUNTDOWN() TIMER = 4; STATE++; POSE(0,0)
        break;

    case 3:
        UPDATE() COUNTDOWN() STEP(14,8) POSE(-0x6000,-0x6200) PlaySong(0x86);
        break;

    case 4:
        UPDATE() COUNTDOWN() STEP(6,9) POSE(0x400,0) SMASH()
        break;
    case 5:
        UPDATE() SHAKE() COUNTDOWN() STEP(6,8) POSE(-0x400,0)
        break;

    case 6:
        UPDATE() COUNTDOWN() STEP(t2[VARIANT],8) POSE(0,0)
        break;

    case 7:
        UPDATE() COUNTDOWN() STEP(30,3) SLOW_POSE(0x16C00,0x15000)
        break;

    case 11:
        UPDATE() COUNTDOWN() FRAME = 11; STATE++; TIMER = t1[VARIANT]; POSE(0x400,0x200)
        break;

    case 12:
        UPDATE() COUNTDOWN() TIMER = 2; STATE++; POSE(0,0)
        break;

    case 13:
        UPDATE() COUNTDOWN() STEP(14,12) POSE(-0x6000,-0x6200) PlaySong(0x86);
        break;

    case 14:
        UPDATE() COUNTDOWN() STEP(6,13) POSE(0x400,0) SMASH()
        break;
    case 15:
        UPDATE() SHAKE() COUNTDOWN() STEP(6,12) POSE(-0x400,0)
        break;

    case 16:
        UPDATE() COUNTDOWN() STEP(t2[VARIANT],12) POSE(0,0)
        break;

    case 19:
        UPDATE() COUNTDOWN() RECOVER(6)
        break;

    case 21:
        UPDATE() COUNTDOWN() FRAME = 17; STATE++; TIMER = t4[VARIANT]; GreatArmor_AttachPart(entity,12,13,0x16C00,0x130000); ARMS(-0x200)
        break;

    case 22:
        UPDATE() COUNTDOWN() TIMER = 8; STATE++; POSE(0,0)
        break;

    case 23:
        UPDATE() COUNTDOWN() STEP(10,18) SLOW_POSE(0x16800,0x16000) PlaySong(0x86);
        break;

    case 24:
        UPDATE() COUNTDOWN() STEP(6,19) POSE(0,0)
        break;

    case 25:
        UPDATE() COUNTDOWN() STEP(6,18) POSE(0,0)
        break;

    case 26:
        UPDATE() COUNTDOWN() STEP(t5[VARIANT],18) POSE(0,0)
        break;

    case 27:
        UPDATE() COUNTDOWN() STEP(30,3) SLOW_POSE(0x16C00,0x15000)
        break;

    case 31:
        UPDATE() COUNTDOWN() FRAME = 21; STATE++; TIMER = t4[VARIANT]; GreatArmor_AttachPart(entity,12,13,0x16C00,0x130000); ARMS(-0x200)
        break;

    case 32:
        UPDATE() COUNTDOWN() TIMER = 8; STATE++; POSE(0,0)
        break;

    case 33:
        UPDATE() COUNTDOWN() STEP(10,22) SLOW_POSE(0x16800,0x16000) PlaySong(0x86);
        break;

    case 34:
        UPDATE() COUNTDOWN() STEP(6,23) POSE(0,0)
        break;

    case 35:
        UPDATE() COUNTDOWN() STEP(6,22) POSE(0,0)
        break;

    case 36:
        UPDATE() COUNTDOWN() STEP(t5[VARIANT],22) POSE(0,0)
        break;

    case 39:
        UPDATE() COUNTDOWN() RECOVER(6)
        break;

    case 40:
    {
        s32 zero;
        GreatArmor_FaceTarget(entity); GreatArmor_SaveParts(entity);
        if ((s8)FRAME == 3) { zero = 0; FRAME = 24; STATE = 41; }
        else { zero = 0; FRAME = 28; STATE = 51; }
        SLOW_POSE(0x16800, 0x11000)
        entity->ext.jointAttack.phase = zero; TIMER = t6[VARIANT];
        break;
    }
    case 41:
        UPDATE() COUNTDOWN() FRAME = 25; STATE++; TIMER = t7[VARIANT]; GreatArmor_AttachPart(entity,12,13,0x16800,0x130000); ARMS(-0x200) PlaySong(0x75);
        break;

    case 42:
        UPDATE() SPARKS() COUNTDOWN() TIMER = 4; STATE++; POSE(0,0)
        break;
    case 43:
        UPDATE() SPARKS() COUNTDOWN() STEP(10,26) SLOW_POSE(0x16800,0x14000) PlaySong(0x85);
        break;
    case 44:
        UPDATE() COUNTDOWN() STEP(6,27) POSE(0,0) SHOCKWAVE()
        break;
    case 45:
        UPDATE() SHAKE() COUNTDOWN() STEP(6,26) POSE(0,0)
        break;

    case 46:
        UPDATE() COUNTDOWN() STEP(t8[VARIANT],26) POSE(0,0)
        break;

    case 47:
        UPDATE() COUNTDOWN() STEP(30,3) SLOW_POSE(0x16C00,0x15000)
        break;

    case 9: case 29: case 49:
        UPDATE() COUNTDOWN() RECOVER(3)
        break;
    case 51:
        UPDATE() COUNTDOWN() FRAME = 29; STATE++; TIMER = t7[VARIANT]; GreatArmor_AttachPart(entity,12,13,0x16800,0x130000); ARMS(-0x200) PlaySong(0x75);
        break;

    case 52:
        UPDATE() SPARKS() COUNTDOWN() TIMER = 4; STATE++; POSE(0,0)
        break;
    case 53:
        UPDATE() SPARKS() COUNTDOWN() STEP(10,30) SLOW_POSE(0x16800,0x14000) PlaySong(0x85);
        break;
    case 54:
        UPDATE() COUNTDOWN() STEP(6,31) POSE(0,0) SHOCKWAVE()
        break;
    case 55:
        UPDATE() SHAKE() COUNTDOWN() STEP(6,30) POSE(0,0)
        break;

    case 56:
        UPDATE() COUNTDOWN() STEP(t8[VARIANT],30) POSE(0,0)
        break;

    case 17: case 37: case 57:
        UPDATE() COUNTDOWN() STEP(30,5) SLOW_POSE(0x14D00,0x13000)
        break;
    case 8: case 18: case 28: case 38: case 48: case 58:
        UPDATE() if ((s8)--TIMER > 4) break;
        TIMER = 10; STATE++;
        break;
    case 59:
        UPDATE() COUNTDOWN() RECOVER(6)
        break;
    }
}
