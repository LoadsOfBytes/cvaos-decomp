/* Big Golem walk state machine (state == 0): shifts weight from foot to foot, stomps (dust, shake), turns at walls. */
#include "code_08009A0.h"
#include "joint_entity.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern void *memcpy(void *dst, const void *src, unsigned long size);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern s32 Target_GetPixelX(struct EwramData_EntityData *);
extern s32 Tile_Classify(s16, s16);
extern s32 Entity_GetHorizontalRoomEdge(struct EwramData_EntityData *, s32);
extern void Effect_SpawnFadingParticleCopy(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 variant);
extern void ScreenShake_RequestOffset(u8 priority, s8 param_1, s8 param_2);
extern void BigGolem_AttachPart(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length, s32 duration);
extern void BigGolem_AttachPartEased(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length, s32 duration);
extern void BigGolem_UpdateParts(struct EwramData_EntityData *entity, s8 *order);
extern void BigGolem_RecenterOnParts(struct EwramData_EntityData *entity, s32 partIndex0, s32 partIndex1);
extern void BigGolem_SetPartYOffset(struct EwramData_EntityData *entity, s32 partIndex, s32 offsetY);
extern const s8 sBigGolemOrderA[14], sBigGolemOrderB[14];

static inline s32 ABS(s32 x)
{
    if (x < 0)
        x = -x;
    return x;
}

/* Joint A hangs off joint B at ANGLE and distance LEN, reached in timer frames. */
#define J(A, B, ANGLE, LEN) BigGolem_AttachPart(entity, A, B, ANGLE, LEN, entity->timer)
#define JT(A, B, ANGLE, LEN, T) BigGolem_AttachPart(entity, A, B, ANGLE, LEN, T)
#define ARM(A, B, ANGLE, LEN) BigGolem_AttachPartEased(entity, A, B, ANGLE, LEN, 0x96)

/* Standing pose on foot F (knee K, hip H); the other leg is OH, OK, OF. */
#define STAND(F, K, H, OH, OK, OF)\
    J(F, F, 0, 0);\
    J(K, F, 0x1B800, 0x300000);\
    J(H, K, 0x1D800, 0x110000);\
    J(3, H, 0x1B000, 0x50000);\
    J(OH, 3, 0x15000, 0x50000);\
    J(OK, OH, 0x16800, 0x110000);\
    J(OF, OK, 0x14000, 0x300000);\
    J(4, 3, 0x1A000, 0x80000);\
    J(5, 3, 0x1AC00, 0x3A0000);\
    J(9, 3, 0x1B800, 0x260000);

#define ARMS_A()\
    ARM(10, 9, 0x15000, 0x160000);\
    ARM(11, 10, 0x15000, 0xE0000);\
    ARM(13, 11, 0x15000, 0x80000);\
    ARM(12, 13, 0x18000, 0x2C0000);

#define ARMS_B()\
    ARM(10, 9, 0x12000, 0x160000);\
    ARM(11, 10, 0x12000, 0xE0000);\
    ARM(13, 11, 0x12000, 0x80000);\
    ARM(12, 13, 0x13000, 0x2C0000);

#define LEAN(F, K, H, OH, OK, OF, A5, A9)\
    J(F, F, 0, 0);\
    J(K, F, 0x1A800, 0x300000);\
    J(H, K, 0x1D000, 0x110000);\
    J(3, H, 0x1A800, 0x50000);\
    J(OH, 3, 0x15800, 0x50000);\
    J(OK, OH, 0x16400, 0x110000);\
    J(OF, OK, 0x15000, 0x300000);\
    J(4, 3, 0x1A000, 0x80000);\
    J(5, 3, A5, 0x3A0000);\
    J(9, 3, A9, 0x260000);

#define STRIDE(F, K, H, OH, OK, OF, A5, A9)\
    J(F, F, 0, 0);\
    J(K, F, 0x1D000, 0x300000);\
    J(H, K, 0x1E400, 0x110000);\
    J(3, H, 0x1D800, 0x50000);\
    J(OH, 3, 0x12800, 0x50000);\
    J(OK, OH, 0x15000, 0x110000);\
    J(OF, OK, 0x12800, 0x300000);\
    J(4, 3, 0x1A000, 0x80000);\
    J(5, 3, A5, 0x3A0000);\
    J(9, 3, A9, 0x260000);

#define DUST(DX)\
    Effect_SpawnFadingParticleCopy(entity->posX.whole + (DX), entity->posY.whole, (RandomNumberGenerator() & 0x7FFF) + 0x8000,\
                 -0x2000 - (RandomNumberGenerator() & 0x3FFF), entity->spriteLayer - 1, 1);\
    Effect_SpawnFadingParticleCopy(entity->posX.whole + (DX), entity->posY.whole, -0x8000 - (RandomNumberGenerator() & 0x7FFF),\
                 -0x2000 - (RandomNumberGenerator() & 0x3FFF), entity->spriteLayer - 1, 1);

#define WAIT(ORDER)\
    BigGolem_UpdateParts(entity, ORDER);\
    if ((s8)--entity->timer > 0)\
        break;

#define STOMP(ORDER, ANIM)\
    WAIT(ORDER)\
    entity->timer = 0x14;\
    entity->subState++;\
    entity->stage = 0;\
    BigGolem_RecenterOnParts(entity, 0, 6);\
    BigGolem_SetPartYOffset(entity, 0, -0xD0000);\
    BigGolem_SetPartYOffset(entity, 6, -0xD0000);\
    JOINT_POSE(entity) = ANIM;

#define STOMP_FX()\
    ScreenShake_RequestOffset(0, 1, 4);\
    PlaySong(0x7C);

#define SHAKE(ORDER)\
    BigGolem_UpdateParts(entity, ORDER);\
    if (entity->timer > 3 && !(entity->timer & 3))\
        ScreenShake_RequestOffset(0, 0, 2);

#define ATTACK()\
    {\
        entity->state = 1;\
        entity->subState = 0;\
        break;\
    }

#define NEAR() (ABS((s16)Target_GetPixelX(entity) - (s16)entity->posX.part16.integer) <= 0x8B)

/* res = 0: walk on, res = 1: turn around. */
#define WALL_CHECK()\
    if ((s8)entity->unk_504.part8.unk_505 == entity->flipX)\
    {\
        dx = entity->flipX ? 0x6E : -0x6E;\
        if (Tile_Classify(bg->xPos.part16.integer + entity->posX.part16.integer + dx,\
                         bg->yPos.part16.integer + entity->posY.part16.integer)\
            || (!entity->flipX && Entity_GetHorizontalRoomEdge(entity, 0x18) == 1)\
            || (entity->flipX && Entity_GetHorizontalRoomEdge(entity, 0x18) == 2))\
        {\
            res = 1;\
            entity->unk_504.part8.unk_505 = entity->flipX ^ 1;\
        }\
        else\
            res = 0;\
    }\
    else\
    {\
        s32 hit;\
        dx = entity->flipX ? -0x6E : 0x6E;\
        hit = Tile_Classify(bg->xPos.part16.integer + entity->posX.part16.integer + dx,\
                           bg->yPos.part16.integer + entity->posY.part16.integer);\
        res = 1;\
        if (hit)\
        {\
            res = 0;\
            entity->unk_504.part8.unk_505 = entity->flipX;\
        }\
    }

#define RANDOM_TURN(CHANCE)\
    if (NEAR())\
    {\
        if ((RandomNumberGenerator2() & 0x7F) < 0x30)\
            ATTACK()\
        if ((RandomNumberGenerator2() & 0x7F) < CHANCE)\
            entity->unk_504.part8.unk_505 = entity->flipX ^ 1;\
    }\
    if (*(s8 *)&entity->unk_4FC.part8.unk_4FE && (s8)entity->unk_4FC.part8.unk_4FD > 3)\
    {\
        entity->unk_4FC.part8.unk_4FE = 0;\
        entity->unk_4FC.part8.unk_4FD = 0;\
        entity->unk_504.part8.unk_505 = entity->flipX ^ 1;\
    }

/**
 * @brief Walk state machine (state 0): wake when the target is near or the golem
 * is damaged, shift weight from foot to foot, stomp (dust, shake) and turn
 * at walls.
 */
void BigGolem_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 dx, dx2; /* dust offsets; dx is also the wall probe offset. Declared before bg: dx2 gets the lower spill slot. */
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s8 orderA[14];
    s8 orderB[14];
    s32 res;

    memcpy(orderA, sBigGolemOrderA, sizeof(orderA));
    memcpy(orderB, sBigGolemOrderB, sizeof(orderB));

    switch (entity->subState)
    {
    case 0:
        if (!Target_IsNearEntity(entity, 0xF0, 0xA0) && !Enemy_IsDamaged(entity))
            break;
        entity->subState = 1;
        entity->stage = 0;
        entity->timer = 0x28;
        JOINT_POSE(entity) = 2;
        STAND(6, 8, 7, 1, 2, 0)
        ARMS_A()
        break;
    case 1:
        WAIT(orderA)
        entity->timer = 0x32;
        entity->subState++;
        JOINT_POSE(entity) = 3;
        LEAN(6, 8, 7, 1, 2, 0, 0x1A400, 0x1B000)
        /* fallthrough */
    case 2:
        STOMP(orderA, 3)
        {
            dx = entity->flipX ? 0x180000 : -0x180000;
            DUST(dx)
            DUST(dx * 2)
        }
        STOMP_FX()
        JT(7, 8, 0x1D400, 0x110000, 10);
        JT(1, 3, 0x15C00, 0x50000, 10);
        JT(8, 6, 0x1A600, 0x300000, 10);
        JT(2, 1, 0x16A00, 0x110000, 10);
        JT(0, 2, 0x14D00, 0x300000, 10);
        break;
    case 3:
        SHAKE(orderA)
        if (entity->timer == 10)
        {
            JT(7, 8, 0x1D000, 0x110000, 10);
            JT(1, 3, 0x15800, 0x50000, 10);
            JT(8, 6, 0x1A800, 0x300000, 10);
            JT(2, 1, 0x16400, 0x110000, 10);
            JT(0, 2, 0x15000, 0x300000, 10);
        }
        if ((s8)--entity->timer > 0)
            break;
        RANDOM_TURN(0x20)
        WALL_CHECK()
        if (res == 0)
        {
            entity->timer = 0x50;
            entity->subState++;
            entity->stage = 0;
            JOINT_POSE(entity) = 4;
            STAND(0, 2, 1, 7, 8, 6)
            ARMS_B()
        }
        else if (res == 1)
        {
            entity->subState = 10;
            JOINT_POSE(entity) = 4;
            entity->timer = 0x32;
            STAND(6, 8, 7, 1, 2, 0)
            ARMS_B()
        }
        break;
    case 4:
        WAIT(orderB)
        entity->timer = 0x32;
        entity->subState++;
        JOINT_POSE(entity) = 5;
        LEAN(0, 2, 1, 7, 8, 6, 0x1B400, 0x1C000)
        /* fallthrough */
    case 5:
        STOMP(orderB, 5)
        {
            dx = entity->flipX ? 0x180000 : -0x180000;
            DUST(dx)
            DUST(dx * 2)
        }
        STOMP_FX()
        JT(1, 2, 0x1D400, 0x110000, 10);
        JT(7, 3, 0x15C00, 0x50000, 10);
        JT(2, 0, 0x1A600, 0x300000, 10);
        JT(8, 7, 0x16A00, 0x110000, 10);
        JT(6, 8, 0x14D00, 0x300000, 10);
        break;
    case 6:
        SHAKE(orderB)
        if (entity->timer == 10)
        {
            JT(1, 2, 0x1D000, 0x110000, 10);
            JT(7, 3, 0x15800, 0x50000, 10);
            JT(2, 0, 0x1A800, 0x300000, 10);
            JT(8, 7, 0x16400, 0x110000, 10);
            JT(6, 8, 0x15000, 0x300000, 10);
        }
        if ((s8)--entity->timer > 0)
            break;
        RANDOM_TURN(0x40)
        WALL_CHECK()
        if (res == 0)
        {
            entity->timer = 0x50;
            entity->subState = 1;
            entity->stage = 0;
            JOINT_POSE(entity) = 2;
            STAND(6, 8, 7, 1, 2, 0)
            ARMS_A()
        }
        else if (res == 1)
        {
            entity->subState = 7;
            entity->timer = 0x32;
            JOINT_POSE(entity) = 2;
            STAND(0, 2, 1, 7, 8, 6)
            ARMS_A()
        }
        break;
    case 7:
        WAIT(orderB)
        entity->timer = 0x50;
        entity->subState++;
        JOINT_POSE(entity) = 3;
        STRIDE(0, 2, 1, 7, 8, 6, 0x1A400, 0x1B000)
        break;
    case 8:
        STOMP(orderB, 3)
        {
            if (entity->flipX)
            {
                dx = -0x60000;
                dx2 = -0x1E0000;
            }
            else
            {
                dx = 0x60000;
                dx2 = 0x1E0000;
            }
            DUST(dx)
            DUST(dx2)
        }
        STOMP_FX()
        JT(1, 2, 0x1E800, 0x110000, 10);
        JT(8, 7, 0x15400, 0x110000, 10);
        break;
    case 9:
        SHAKE(orderB)
        if (entity->timer == 10)
        {
            JT(2, 0, 0x1D000, 0x300000, 10);
            JT(1, 2, 0x1E400, 0x110000, 10);
            JT(7, 3, 0x12800, 0x50000, 10);
            JT(8, 7, 0x15000, 0x110000, 10);
            JT(6, 8, 0x12800, 0x300000, 10);
        }
        if ((s8)--entity->timer > 0)
            break;
        if (NEAR() && (RandomNumberGenerator2() & 0x7F) < 0x30)
            ATTACK()
        WALL_CHECK()
        if (res == 0)
        {
            entity->timer = 0x50;
            entity->subState = 4;
            JOINT_POSE(entity) = 4;
            STAND(0, 2, 1, 7, 8, 6)
            ARMS_B()
        }
        else if (res == 1)
        {
            entity->subState = 10;
            entity->timer = 0x32;
            JOINT_POSE(entity) = 4;
            STAND(6, 8, 7, 1, 2, 0)
            ARMS_B()
        }
        break;
    case 10:
        WAIT(orderA)
        entity->timer = 0x50;
        entity->subState++;
        JOINT_POSE(entity) = 5;
        STRIDE(6, 8, 7, 1, 2, 0, 0x1B400, 0x1C000)
        break;
    case 11:
        STOMP(orderA, 5)
        {
            if (entity->flipX)
            {
                dx = -0x60000;
                dx2 = -0x1E0000;
            }
            else
            {
                dx = 0x60000;
                dx2 = 0x1E0000;
            }
            DUST(dx)
            DUST(dx2)
        }
        STOMP_FX()
        JT(7, 8, 0x1E800, 0x110000, 10);
        JT(2, 1, 0x15400, 0x110000, 10);
        break;
    case 12:
        SHAKE(orderA)
        if (entity->timer == 10)
        {
            JT(8, 6, 0x1D000, 0x300000, 10);
            JT(7, 8, 0x1E400, 0x110000, 10);
            JT(1, 3, 0x12800, 0x50000, 10);
            JT(2, 1, 0x15000, 0x110000, 10);
            JT(0, 2, 0x12800, 0x300000, 10);
        }
        if ((s8)--entity->timer > 0)
            break;
        if (NEAR() && (RandomNumberGenerator2() & 0x7F) < 0x30)
            ATTACK()
        WALL_CHECK()
        if (res == 0)
        {
            entity->timer = 0x50;
            entity->subState = 1;
            JOINT_POSE(entity) = 2;
            STAND(6, 8, 7, 1, 2, 0)
            ARMS_A()
        }
        else if (res == 1)
        {
            entity->subState = 7;
            entity->timer = 0x32;
            JOINT_POSE(entity) = 2;
            STAND(0, 2, 1, 7, 8, 6)
            ARMS_A()
        }
        break;
    }
}
