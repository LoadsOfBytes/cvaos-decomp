/* Creaking Skull main state machine: arm swings, ground smash debris, bone spit. */
#include "code_08009A0.h"
#include "creaking_skull_entity.h"
#include "joint_entity.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void *memcpy(void *dst, const void *src, unsigned long size);
extern void CreakingSkull_FaceTarget(struct EwramData_EntityData *entity);
extern void CreakingSkull_SaveJointPositions(struct EwramData_EntityData *entity);
extern void CreakingSkull_RestoreJointPositions(struct EwramData_EntityData *entity);
extern void CreakingSkull_AttachJointSetAngle(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length);
extern void CreakingSkull_AttachJoint(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length);
extern void CreakingSkull_InterpolateJointsInOrder(struct EwramData_EntityData *entity, s8 *order);
extern void CreakingSkull_SpawnGuidedEffect(struct EwramData_EntityData *entity, s32 lateTargetX, s32 lateTargetY, s32 earlyTargetX, s32 earlyTargetY, s32 x, s32 y);
extern void CreakingSkull_SpawnProjectile(struct EwramData_EntityData *owner, s32 x, s32 y, s32 vx, s32 vy);
extern void Effect_SpawnParticleWithDamping(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 variant, s32 damping, s32 flags, s32 palette);
extern void Effect_SpawnFallingPiece(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 piece);
extern void Effect_SpawnParticleAnim11(s32 x, s32 y, s32 vx, s32 vy, s32 layer);
extern void ScreenShake_RequestOffset(u8 priority, s8 param_1, s8 param_2);
extern s32 Entity_GetVisibilityPattern(s32);
extern const u8 sCreakingSkullSwingTimes[3], sCreakingSkullArmTimes[3], sCreakingSkullSpitTimes[3];
extern const s8 sUnk_08119063[13];

#define ARMS(A, B, C, D)\
    CreakingSkull_AttachJointSetAngle(entity, 1, 0, A, 0x1E0000);\
    CreakingSkull_AttachJointSetAngle(entity, 2, 1, B, 0x1C0000);

#define COUNTDOWN()\
    CreakingSkull_InterpolateJointsInOrder(entity, chain);\
    if ((s8)--entity->timer > 0)\
        break;

/* Smash: three debris bursts along the arm. */
#define SMASH_DEBRIS(BX)\
    for (i = 0; i <= 2; i++)\
    {\
        s32 bx, vx, vy, rvx, rvy;\
\
        if (entity->flipX)\
        {\
            bx = BX;\
            step = 0xC0000;\
            vx = (RandomNumberGenerator() & 0x7FFF) + 0x10000;\
        }\
        else\
        {\
            bx = -(BX);\
            step = -0xC0000;\
            vx = -(RandomNumberGenerator() & 0x7FFF) + -0x10000;\
        }\
        vy = -0x8000 - (RandomNumberGenerator() & 0x7FFF);\
        Effect_SpawnParticleWithDamping(entity->posX.whole + bx + i * step, entity->posY.whole, vx, vy,\
                     entity->spriteLayer - 1, 0, 8, 0xF, 6);\
        rvx = (RandomNumberGenerator() & 0xFFFF) + -0x8000;\
        rvy = -(RandomNumberGenerator() & 0xFFFF) + -0x30000;\
        Effect_SpawnFallingPiece(entity->posX.whole + bx + i * step, entity->posY.whole, rvx, rvy,\
                     entity->spriteLayer - 1, RandomNumberGenerator() & 1);\
    }

/* Dragging the arm: dust along it and one rock. */
#define DRAG_DEBRIS(BX)\
    if (!(entity->timer & 3))\
        ScreenShake_RequestOffset(0, 0, 1);\
    if (entity->timer > 3)\
    {\
        for (i = 0; i <= 3; i++)\
        {\
\
\
            if (entity->flipX)\
            {\
                bx = BX;\
                step = 0xC0000;\
            }\
            else\
            {\
                bx = -(BX);\
                step = -0xC0000;\
            }\
            vx = (RandomNumberGenerator() & 0xFFFF) + -0x8000;\
            vy = -0x8000 - (RandomNumberGenerator() & 0xFFFF);\
            Effect_SpawnParticleWithDamping(entity->posX.whole + bx + i * step, entity->posY.whole, vx, vy,\
                         entity->spriteLayer - 1, 0, 0x10, 0xF, 6);\
        }\
        for (i = 0; i <= 0; i++)\
        {\
                s32 base, vx, vy;\
\
            if (entity->flipX)\
            {\
                bx = 0x700000;\
                step = 0x100000;\
                base = 0x8000;\
            }\
            else\
            {\
                bx = -0x700000;\
                step = -0x100000;\
                base = -0x8000;\
            }\
            vx = (RandomNumberGenerator() & 0xFFFF) + ({ s32 t = base + -0x8000; t; });\
            vy = -(RandomNumberGenerator() & 0xFFFF) - ((entity->timer - 2) << 16);\
            Effect_SpawnFallingPiece(entity->posX.whole + bx + i * step, entity->posY.whole, vx, vy,\
                         entity->spriteLayer - 1, RandomNumberGenerator() & 1);\
        }\
    }

/* Spit bones from the mouth. */
#define SPIT_BONES()\
    if (Entity_GetVisibilityPattern(entity->timer) == 1)\
    {\
        register s32 y0 asm("r5"); /* FAKEMATCH: pinned register */ \
        s32 x1, y1;\
\
        if (entity->flipX)\
        {\
            step = entity->posX.whole + -0xB0000;\
            bx = step - RandomNumberGenerator() % 0x280000 + 0xC0000;\
            x1 = entity->posX.whole + -0xF0000;\
        }\
        else\
        {\
            step = entity->posX.whole + 0xB0000;\
            bx = step + RandomNumberGenerator() % 0x280000 + -0xC0000;\
            x1 = entity->posX.whole + 0xF0000;\
        }\
        { register s32 py asm("r0") = entity->posY.whole; y0 = py + -0x2B0000; } /* FAKEMATCH: pinned register */ \
        y1 = y0 - (RandomNumberGenerator() & 0x1FFFFF) + -0x80000;\
        vy = entity->posY.whole + -0x2F0000;\
        CreakingSkull_SpawnGuidedEffect(entity, step, y0, x1, vy, bx, y1);\
    }

/* Breath puffs, then the fireball. */
#define BREATH(SONG)\
    if (Entity_GetVisibilityPattern(entity->timer) == 1)\
    {\
\
        if (entity->flipX)\
            bx = RandomNumberGenerator() % 0x60000 + entity->posX.whole + 0x1D0000;\
        else\
            bx = RandomNumberGenerator() % 0x60000 + entity->posX.whole + -0x230000;\
        { register s32 by asm("r0") = RandomNumberGenerator() % 0x60000 + entity->posY.whole; Effect_SpawnParticleAnim11(bx, by + -0x1F0000, 0, 0, entity->spriteLayer - 1); } /* FAKEMATCH: pinned register */ \
    }\
    if (!(entity->timer & 7))\
        PlaySong(SONG);\
    if ((s8)--entity->timer > 0)\
        break;\
    {\
        s32 vx;\
\
        if (entity->flipX)\
        {\
            bx = 0x240000;\
            vx = 0x30000;\
        }\
        else\
        {\
            bx = -0x240000;\
            vx = -0x30000;\
        }\
        CreakingSkull_SpawnProjectile(entity, bx, -0x190000, vx, 0);\
    }

#define RECOVER(STATE)\
    entity->timer = 1;\
    entity->subState = STATE;\
    entity->stage = 0;\
    entity->state = 0;\
    CreakingSkull_RestoreJointPositions(entity);\
    ARMS(0x17000, 0x1D000, , )\
    CreakingSkull_InterpolateJointsInOrder(entity, chain);

/**
 * @brief Attack state (1): coordinate arm swings, ground-smash/drag effects
 * and ranged attack sequences through joint poses and variant timing
 * tables. Preserve macros, signed counters and pinned registers.
 */
void CreakingSkull_UpdateAttack(struct EwramData_EntityData *entity)
{
    u8 swingTimes[3];
    u8 armTimes[3];
    u8 waitTimes[3];
    s8 spitTimes[3];
    s8 chain[13];
    s32 i;
    s32 step;
    s32 vx, vy;
    register s32 bx asm("r9"); /* FAKEMATCH: pinned register */
    s32 zero;

    memcpy(swingTimes, sCreakingSkullSwingTimes, sizeof(swingTimes));
    memcpy(armTimes, sCreakingSkullArmTimes, sizeof(armTimes));
    memcpy(waitTimes, sCreakingSkullSwingTimes, sizeof(waitTimes));
    memcpy(spitTimes, sCreakingSkullSpitTimes, sizeof(spitTimes));
    memcpy(chain, sUnk_08119063, sizeof(chain));

    switch (entity->subState)
    {
    case 0:
        CreakingSkull_FaceTarget(entity);
        CreakingSkull_SaveJointPositions(entity);
        if ((s8)JOINT_POSE(entity) == 5)
        {
            zero = 0;
            JOINT_POSE(entity) = 0xD;
            entity->subState = 1;
        }
        else
        {
            zero = 0;
            JOINT_POSE(entity) = 0x11;
            entity->subState = 0xB;
        }
        CreakingSkull_AttachJointSetAngle(entity, 0xC, 0xB, 0, 0x1F0000);
        ARMS(0x5000, 0x1000, , )
        JOINT_ATTACK_PHASE(entity) = zero;
        entity->timer = swingTimes[CREAKING_SKULL_VARIANT(entity)];
        break;
    case 1:
        COUNTDOWN()
        JOINT_POSE(entity) = 0xE;
        entity->subState++;
        entity->timer = armTimes[CREAKING_SKULL_VARIANT(entity)];
        ARMS(0x200, 0x200, , )
        break;
    case 2:
        COUNTDOWN()
        entity->timer = 4;
        entity->subState++;
        ARMS(0, 0, , )
        break;
    case 3:
        COUNTDOWN()
        entity->timer = 0xE;
        entity->subState++;
        JOINT_POSE(entity) = 0xF;
        ARMS(-0x5A00, -0x7200, , )
        PlaySong(0x86);
        break;
    case 4:
        COUNTDOWN()
        entity->timer = 6;
        entity->subState++;
        JOINT_POSE(entity) = 0x10;
        ARMS(0, 0x400, , )
        SMASH_DEBRIS(0x4C0000)
        ScreenShake_RequestOffset(0, 1, 2);
        PlaySong(0x80);
        break;
    case 5:
        CreakingSkull_InterpolateJointsInOrder(entity, chain);
        DRAG_DEBRIS(0x4C0000)
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = 6;
        entity->subState++;
        JOINT_POSE(entity) = 0xF;
        ARMS(0, -0x400, , )
        break;
    case 6:
        COUNTDOWN()
        entity->timer = waitTimes[CREAKING_SKULL_VARIANT(entity)];
        entity->subState++;
        JOINT_POSE(entity) = 0xF;
        ARMS(0, 0, , )
        break;
    case 7:
        COUNTDOWN()
        entity->timer = 0x32;
        entity->subState++;
        JOINT_POSE(entity) = 5;
        ARMS(0x800, 0x6000, , )
        break;
    case 11:
        COUNTDOWN()
        JOINT_POSE(entity) = 0x12;
        entity->subState++;
        entity->timer = armTimes[CREAKING_SKULL_VARIANT(entity)];
        ARMS(0x200, 0x200, , )
        break;
    case 12:
        COUNTDOWN()
        entity->timer = 2;
        entity->subState++;
        ARMS(0, 0, , )
        break;
    case 13:
        COUNTDOWN()
        entity->timer = 0xE;
        entity->subState++;
        JOINT_POSE(entity) = 0x13;
        ARMS(-0x5A00, -0x7200, , )
        PlaySong(0x86);
        break;
    case 14:
        COUNTDOWN()
        entity->timer = 6;
        entity->subState++;
        JOINT_POSE(entity) = 0x14;
        ARMS(0, 0x400, , )
        SMASH_DEBRIS(0x4C0000)
        ScreenShake_RequestOffset(0, 1, 2);
        PlaySong(0x80);
        break;
    case 15:
        CreakingSkull_InterpolateJointsInOrder(entity, chain);
        DRAG_DEBRIS(0x500000)
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = 6;
        entity->subState++;
        JOINT_POSE(entity) = 0x13;
        ARMS(0, -0x400, , )
        break;
    case 16:
        COUNTDOWN()
        entity->timer = waitTimes[CREAKING_SKULL_VARIANT(entity)];
        entity->subState++;
        JOINT_POSE(entity) = 0x13;
        ARMS(0, 0, , )
        break;
    case 17:
        COUNTDOWN()
        entity->timer = 0x1E;
        entity->subState++;
        JOINT_POSE(entity) = 7;
        ARMS(0x800, 0x6000, , )
        break;
    case 8:
    case 18:
        CreakingSkull_InterpolateJointsInOrder(entity, chain);
        if ((s8)--entity->timer > 4)
            break;
        entity->timer = 0xA;
        entity->subState++;
        CreakingSkull_AttachJoint(entity, 1, 0, 0x17000, 0x1E0000);
        CreakingSkull_AttachJoint(entity, 2, 1, 0x1D000, 0x1C0000);
        break;
    case 19:
        COUNTDOWN()
        RECOVER(6)
        break;
    case 20:
        CreakingSkull_FaceTarget(entity);
        CreakingSkull_SaveJointPositions(entity);
        if ((s8)JOINT_POSE(entity) == 5)
        {
            zero = 0;
            JOINT_POSE(entity) = 0x16;
            entity->subState = 0x15;
        }
        else
        {
            zero = 0;
            JOINT_POSE(entity) = 0x1A;
            entity->subState = 0x1F;
        }
        CreakingSkull_AttachJointSetAngle(entity, 0xC, 0xB, 0, 0x1F0000);
        ARMS(0, 0, , )
        JOINT_ATTACK_PHASE(entity) = zero;
        entity->timer = spitTimes[CREAKING_SKULL_VARIANT(entity)];
        CREAKING_SKULL_POSE_DELAY(entity) = entity->timer / 2;
        break;
    case 21:
        if ((s8)--*(s8 *)&CREAKING_SKULL_POSE_DELAY(entity) <= 0)
            JOINT_POSE(entity) = 0x15;
        COUNTDOWN()
        entity->subState++;
        entity->timer = 0x29;
        PlaySong(0x77);
        break;
    case 22:
        SPIT_BONES()
        if (!(entity->timer & 7))
            PlaySong(0x84);
        if ((s8)--entity->timer > 0)
            break;
        entity->subState++;
        entity->timer = 0xA;
        CREAKING_SKULL_POSE_DELAY(entity) = 5;
        JOINT_POSE(entity) = 0x17;
        break;
    case 24:
        BREATH(0x12E)
        entity->timer = 3;
        entity->subState++;
        JOINT_POSE(entity) = 0x18;
        PlaySong(0x7A);
        break;
    case 25:
        COUNTDOWN()
        entity->subState++;
        entity->timer = 0x1E;
        JOINT_POSE(entity) = 0x17;
        break;
    case 26:
        COUNTDOWN()
        JOINT_POSE(entity) = 5;
        entity->subState++;
        entity->timer = 0x3C;
        break;
    case 9:
    case 27:
        COUNTDOWN()
        RECOVER(3)
        break;
    case 31:
        if ((s8)--*(s8 *)&CREAKING_SKULL_POSE_DELAY(entity) <= 0)
            JOINT_POSE(entity) = 0x19;
        COUNTDOWN()
        entity->subState++;
        entity->timer = 0x29;
        PlaySong(0x77);
        break;
    case 32:
        SPIT_BONES()
        if (!(entity->timer & 7))
            PlaySong(0x84);
        if ((s8)--entity->timer > 0)
            break;
        entity->subState++;
        entity->timer = 0xA;
        CREAKING_SKULL_POSE_DELAY(entity) = 5;
        JOINT_POSE(entity) = 0x1B;
        break;
    case 23:
    case 33:
        COUNTDOWN()
        entity->subState++;
        entity->timer = 0x28;
        break;
    case 34:
        BREATH(0x77)
        entity->timer = 3;
        entity->subState++;
        JOINT_POSE(entity) = 0x1C;
        PlaySong(0x7A);
        break;
    case 35:
        COUNTDOWN()
        entity->subState++;
        entity->timer = 0x1E;
        JOINT_POSE(entity) = 0x1B;
        break;
    case 36:
        COUNTDOWN()
        JOINT_POSE(entity) = 7;
        entity->subState++;
        entity->timer = 0x3C;
        break;
    case 37:
        COUNTDOWN()
        RECOVER(6)
        break;
    }
}
