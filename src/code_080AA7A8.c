/* Big Golem attack state machine (state == 1): punch (states 0-19, two variants) and rock throw (20-39). */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern void *memcpy(void *dst, const void *src, unsigned long size);
extern void Effect_SpawnFallingPiece(s32, s32, s32, s32, s32, s32);
extern void BigGolem_AttachPart(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length, s32 duration);
extern void BigGolem_AttachPartEased(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length, s32 duration);
extern void BigGolem_UpdateParts(struct EwramData_EntityData *entity, s8 *order);
extern void BigGolem_SaveParts(struct EwramData_EntityData *entity);
extern void BigGolem_RestoreParts(struct EwramData_EntityData *entity);
extern void BigGolem_SpawnRock(struct EwramData_EntityData *parent, s32 x, s32 y, s32 vx, s32 vy);
extern const s8 sBigGolemOrderA[14], sBigGolemOrderB[14], sBigGolemOrderC[14];

/* Joint A hangs off joint B at ANGLE and distance LEN, reached in T frames (A*: eased). */
#define J(A, B, ANGLE, LEN) BigGolem_AttachPart(entity, A, B, ANGLE, LEN, entity->timer)
#define A(A, B, ANGLE, LEN) BigGolem_AttachPartEased(entity, A, B, ANGLE, LEN, entity->timer)
#define AT(A, B, ANGLE, LEN, T) BigGolem_AttachPartEased(entity, A, B, ANGLE, LEN, T)

#define VARIANT (*(s8 *)&entity->ext.bigGolemAttack.variant)
#define ANIM entity->ext.joint.pose

#define WAIT(ORDER)\
    BigGolem_UpdateParts(entity, ORDER);\
    if ((s8)--entity->timer > 0)\
        break;

/* Legs for a wind-up on foot F (knee K, hip H); the other leg is OH, OK. */
#define WINDUP_LEGS(F, K, H, OH, OK)\
    A(6, 6, 0, 0);\
    A(K, F, 0x19800, 0x300000);\
    A(H, K, 0x1E000, 0x110000);\
    A(3, H, 0x1A800, 0x50000);\
    A(OH, 3, 0x16000, 0x50000);\
    A(OK, OH, 0x18000, 0x110000);\
    A(0, 0, 0, 0);

#define ARMS(ANGLE)\
    A(10, 9, ANGLE, 0x160000);\
    A(11, 10, ANGLE, 0xE0000);\
    A(13, 11, ANGLE, 0x80000);

#define LEAN(F, K, H, OH, OK, OF, A5, A9, ARM)\
    A(F, F, 0, 0);\
    A(K, F, 0x1A800, 0x300000);\
    A(H, K, 0x1D000, 0x110000);\
    A(3, H, 0x1A800, 0x50000);\
    A(OH, 3, 0x15800, 0x50000);\
    A(OK, OH, 0x16400, 0x110000);\
    A(OF, OF, 0, 0);\
    A(4, 3, 0x1A000, 0x80000);\
    A(5, 3, A5, 0x3A0000);\
    A(9, 3, A9, 0x260000);\
    ARMS(ARM)

#define PUNCH_START()\
    J(9, 3, 0x19800, 0x260000);\
    AT(5, 3, 0x18800, 0x3A0000, 0x32);\
    J(10, 9, 0x16000, 0x160000);\
    J(11, 10, 0x16000, 0xE0000);\
    J(13, 11, 0x16000, 0x80000);\
    J(12, 13, 0x16E00, 0x2C0000);

#define PUNCH_START_LOW()\
    J(9, 3, 0x19C00, 0x260000);\
    AT(5, 3, 0x18C00, 0x3A0000, 0x32);\
    J(10, 9, 0x18000, 0x160000);\
    J(11, 10, 0x18000, 0xE0000);\
    J(13, 11, 0x18000, 0x80000);\
    J(12, 13, 0, 0x2C0000);

#define PUNCH_BACK(A9)\
    A(9, 3, A9, 0x260000);\
    ARMS(0x13000)\
    A(12, 13, -0x200, 0x2C0000);

#define PUNCH_BACK_HIGH()\
    A(9, 3, 0x19C00, 0x260000);\
    ARMS(0x12000)\
    A(12, 13, 0x17000, 0x2C0000);

#define ROCK(STEP, CALL)\
    if (entity->flipX)\
    {\
        x = 0x200000;\
        vx = (RandomNumberGenerator() & 0xFFFF) + STEP;\
    }\
    else\
    {\
        x = -0x200000;\
        vx = (RandomNumberGenerator() & 0xFFFF) - 0x10000 - STEP;\
    }\
    CALL;

#define THROW_PREPARE(K, F, H, OH, OK, AOH)\
    J(9, 3, 0x1B000, 0x260000);\
    AT(5, 3, 0x1A000, 0x3A0000, 0x28);\
    AT(10, 9, 0x13000, 0x160000, 0x28);\
    J(11, 10, 0x13000, 0xE0000);\
    J(13, 11, 0x13000, 0x80000);\
    AT(12, 13, -0x3400, 0x2C0000, 0x28);\
    A(6, 6, 0, 0);\
    A(K, F, 0x19C00, 0x300000);\
    A(H, K, 0x1DC00, 0x110000);\
    A(3, H, 0x1AC00, 0x50000);\
    A(OH, 3, AOH, 0x50000);\
    A(OK, OH, 0x17C00, 0x110000);\
    A(0, 0, 0, 0);

#define SETTLE(ORDER)\
    BigGolem_UpdateParts(entity, ORDER);\
    if ((s8)--entity->timer > 4)\
        break;\
    entity->timer = 1;\
    entity->subState++;

#define FINISH(NEXT)\
    entity->timer = 1;\
    entity->subState = NEXT;\
    entity->state = 0;\
    BigGolem_RestoreParts(entity);

/**
 * @brief Attack state machine (state 1): punch (substates 0-19, two variants
 * chosen at random) and rock throw (substates 20-39).
 */
void BigGolem_UpdateAttack(struct EwramData_EntityData *entity)
{
    s8 orderA[14];
    s8 orderB[14];
    s8 orderC[14];
    s32 x;
    s32 vx;

    memcpy(orderA, sBigGolemOrderA, sizeof(orderA));
    memcpy(orderB, sBigGolemOrderB, sizeof(orderB));
    memcpy(orderC, sBigGolemOrderC, sizeof(orderC));

    switch (entity->subState)
    {
    case 0:
        BigGolem_SaveParts(entity);
        if ((RandomNumberGenerator() & 0x7F) < 0x2A)
            entity->ext.bigGolemAttack.variant = 0;
        else if ((RandomNumberGenerator() & 0x7F) < 0x2A)
            entity->ext.bigGolemAttack.variant = 1;
        else
        {
            entity->subState = 0x14;
            break;
        }
        if ((s8)ANIM == 3)
        {
            entity->subState = 1;
            entity->ext.jointAttack.phase = 0;
            entity->timer = 0x78;
            WINDUP_LEGS(6, 8, 7, 1, 2)
            A(4, 3, 0x19000, 0x80000);
            A(5, 3, 0x19400, 0x3A0000);
            A(9, 3, 0x1A000, 0x260000);
            if (!VARIANT)
            {
                ANIM = 6;
                ARMS(0x12000)
                A(12, 13, 0x17000, 0x2C0000);
            }
            else
            {
                ANIM = 0xE;
                ARMS(0x13000)
                A(12, 13, 0x1000, 0x2C0000);
            }
            entity->timer = 0x28;
        }
        else
        {
            entity->subState = 0xB;
            entity->ext.jointAttack.phase = 0;
            entity->timer = 0x78;
            WINDUP_LEGS(0, 2, 1, 7, 8)
            A(4, 3, 0x19000, 0x80000);
            A(5, 3, 0x19400, 0x3A0000);
            A(9, 3, 0x1A000, 0x260000);
            if (!VARIANT)
            {
                ANIM = 0xA;
                ARMS(0x12000)
                A(12, 13, 0x17000, 0x2C0000);
            }
            else
            {
                ANIM = 0x12;
                ARMS(0x13000)
                A(12, 13, 0x6000, 0x2C0000);
            }
            entity->timer = 0x28;
        }
        break;
    case 1:
        WAIT(orderA)
        entity->subState++;
        entity->timer = 0x28;
        AT(5, 3, 0x19400, 0x3A0000, 0x50);
        if (!VARIANT)
            ANIM = 7;
        else
            ANIM = 0xF;
        break;
    case 2:
        WAIT(orderA)
        entity->timer = 0x28;
        entity->subState++;
        AT(5, 3, 0x19400, 0x3A0000, 0x3C);
        if (!VARIANT)
            ANIM = 6;
        else
            ANIM = 0xE;
        break;
    case 3:
        WAIT(orderA)
        entity->timer = 0x18;
        entity->subState++;
        PlaySong(0x86);
        if (!VARIANT)
        {
            ANIM = 8;
            PUNCH_START()
        }
        else
        {
            ANIM = 8;
            PUNCH_START_LOW()
        }
        entity->timer = 0xF;
        break;
    case 4:
        WAIT(orderA)
        entity->timer = 6;
        entity->subState++;
        if (!VARIANT)
        {
            ANIM = 9;
            J(12, 13, 0x17000, 0x2C0000);
        }
        else
        {
            ANIM = 0x11;
            J(12, 13, 0x200, 0x2C0000);
        }
        break;
    case 5:
        WAIT(orderA)
        entity->timer = 0x20;
        entity->subState++;
        if (!VARIANT)
        {
            PUNCH_BACK_HIGH()
        }
        else
        {
            PUNCH_BACK(0x19E00)
        }
        entity->timer = 0x18;
        break;
    case 6:
        WAIT(orderA)
        entity->timer = 8;
        entity->subState++;
        if (!VARIANT)
            ANIM = 9;
        else
            ANIM = 0x11;
        break;
    case 7:
        WAIT(orderA)
        entity->timer = 0x3C;
        entity->subState++;
        ANIM = 3;
        LEAN(6, 8, 7, 1, 2, 0, 0x1A400, 0x1B000, 0x15000)
        if (!VARIANT)
            A(12, 13, 0x18000, 0x2C0000);
        else
            A(12, 13, -0x1000, 0x2C0000);
        break;
    case 8:
        SETTLE(orderA)
        break;
    case 11:
        WAIT(orderB)
        entity->subState++;
        entity->timer = 0x28;
        AT(5, 3, 0x19400, 0x3A0000, 0x50);
        if (!VARIANT)
            ANIM = 0xB;
        else
            ANIM = 0x13;
        break;
    case 12:
        WAIT(orderB)
        entity->timer = 0x28;
        AT(5, 3, 0x19400, 0x3A0000, 0x3C);
        entity->subState++;
        if (!VARIANT)
            ANIM = 0xA;
        else
            ANIM = 0x12;
        break;
    case 13:
        WAIT(orderB)
        entity->timer = 0x18;
        entity->subState++;
        PlaySong(0x86);
        if (!VARIANT)
        {
            ANIM = 0xC;
            PUNCH_START()
        }
        else
        {
            ANIM = 0x14;
            PUNCH_START_LOW()
        }
        entity->timer = 0xF;
        break;
    case 14:
        WAIT(orderB)
        entity->timer = 6;
        entity->subState++;
        if (!VARIANT)
        {
            ANIM = 0xD;
            J(12, 13, 0x17000, 0x2C0000);
        }
        else
        {
            ANIM = 0x15;
            J(12, 13, 0x200, 0x2C0000);
        }
        break;
    case 15:
        WAIT(orderB)
        entity->timer = 0x20;
        entity->subState++;
        if (!VARIANT)
        {
            PUNCH_BACK_HIGH()
        }
        else
        {
            PUNCH_BACK(0x19C00)
        }
        entity->timer = 0x18;
        break;
    case 16:
        WAIT(orderB)
        entity->timer = 8;
        entity->subState++;
        if (!VARIANT)
            ANIM = 0xD;
        else
            ANIM = 0x15;
        break;
    case 17:
        WAIT(orderB)
        entity->timer = 0x64;
        entity->subState++;
        ANIM = 5;
        LEAN(0, 2, 1, 7, 8, 6, 0x1B400, 0x1C000, 0x12000)
        if (!VARIANT)
            A(12, 13, 0x13000, 0x2C0000);
        else
            A(12, 13, -0x6000, 0x2C0000);
        break;
    case 19:
        WAIT(orderB)
        FINISH(6)
        break;
    case 20:
        BigGolem_SaveParts(entity);
        if ((s8)ANIM == 3)
        {
            entity->subState = 0x15;
            entity->ext.jointAttack.phase = 0;
            entity->timer = 0x78;
            WINDUP_LEGS(6, 8, 7, 1, 2)
            A(4, 3, 0x1B000, 0x80000);
            AT(5, 3, 0x1B400, 0x3A0000, 0x78);
            A(9, 3, 0x1C000, 0x260000);
            ANIM = 0x16;
            ARMS(0x15000)
            A(12, 13, 0x2800, 0x2C0000);
            entity->timer = 0x1E;
        }
        else
        {
            entity->subState = 0x1F;
            entity->ext.jointAttack.phase = 0;
            entity->timer = 0x78;
            WINDUP_LEGS(0, 2, 1, 7, 8)
            A(4, 3, 0x1B000, 0x80000);
            A(5, 3, 0x1B400, 0x3A0000);
            A(9, 3, 0x1C000, 0x260000);
            ANIM = 0x1A;
            ARMS(0x15000)
            A(12, 13, 0x7800, 0x2C0000);
            entity->timer = 0x50;
        }
        break;
    case 21:
        WAIT(orderA)
        entity->subState++;
        entity->timer = 0x5A;
        A(5, 3, 0x1B400, 0x3A0000);
        ANIM = 0x17;
        break;
    case 22:
        BigGolem_UpdateParts(entity, orderA);
        if (entity->timer <= 0x2C && !(entity->timer & 0xF))
        {
            ROCK(0, Effect_SpawnFallingPiece(entity->posX.whole + x, entity->posY.whole - 0x680000, vx, 0x1000, entity->spriteLayer + 1, 1))
        }
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = 0x14;
        entity->subState++;
        break;
    case 23:
        WAIT(orderA)
        entity->timer = 0x14;
        entity->subState++;
        ANIM = 0x18;
        THROW_PREPARE(8, 6, 7, 1, 2, 0x16000)
        entity->timer = 0xC;
        break;
    case 24:
        WAIT(orderA)
        entity->timer = 0x28;
        entity->subState++;
        ANIM = 0x19;
        PlaySong(0x71);
        break;
    case 25:
        BigGolem_UpdateParts(entity, orderA);
        if (!(entity->timer & 7))
        {
            ROCK(0x18000, BigGolem_SpawnRock(entity, entity->posX.whole + x, entity->posY.whole - 0x680000, vx, 0x1000))
        }
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = 8;
        entity->subState++;
        break;
    case 26:
        WAIT(orderA)
        entity->timer = 8;
        entity->subState++;
        ANIM = 0x19;
        break;
    case 27:
        WAIT(orderA)
        entity->timer = 0x3C;
        entity->subState++;
        ANIM = 3;
        LEAN(6, 8, 7, 1, 2, 0, 0x1A400, 0x1B000, 0x15000)
        A(12, 13, 0x18000, 0x2C0000);
        break;
    case 28:
        SETTLE(orderA)
        break;
    case 9:
    case 29:
        WAIT(orderA)
        FINISH(3)
        break;
    case 31:
        WAIT(orderB)
        entity->subState++;
        entity->timer = 0x5A;
        A(5, 3, 0x1B400, 0x3A0000);
        ANIM = 0x1B;
        break;
    case 32:
        BigGolem_UpdateParts(entity, orderB);
        if (entity->timer <= 0x2C && !(entity->timer & 0xF))
        {
            ROCK(0, Effect_SpawnFallingPiece(entity->posX.whole + x, entity->posY.whole - 0x680000, vx, 0x1000, entity->spriteLayer + 1, 1))
        }
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = 0x14;
        entity->subState++;
        break;
    case 33:
        WAIT(orderB)
        entity->timer = 0x14;
        entity->subState++;
        ANIM = 0x1C;
        THROW_PREPARE(2, 0, 1, 7, 8, 0x15C00)
        entity->timer = 0xC;
        break;
    case 34:
        WAIT(orderB)
        entity->timer = 0x28;
        entity->subState++;
        ANIM = 0x1D;
        PlaySong(0x71);
        break;
    case 35:
        BigGolem_UpdateParts(entity, orderB);
        if (!(entity->timer & 7))
        {
            ROCK(0x18000, BigGolem_SpawnRock(entity, entity->posX.whole + x, entity->posY.whole - 0x680000, vx, 0x1000))
        }
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = 8;
        entity->subState++;
        break;
    case 36:
        WAIT(orderB)
        entity->timer = 8;
        entity->subState++;
        ANIM = 0x1D;
        break;
    case 37:
        WAIT(orderB)
        entity->timer = 0x3C;
        entity->subState++;
        ANIM = 5;
        LEAN(0, 2, 1, 7, 8, 6, 0x1B400, 0x1C000, 0x12000)
        A(12, 13, 0x13000, 0x2C0000);
        break;
    case 18:
    case 38:
        SETTLE(orderB)
        break;
    case 39:
        WAIT(orderB)
        FINISH(6)
        break;
    }
}
