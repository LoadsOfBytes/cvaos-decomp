/* Arc flight while the partner attacks. */
#include "code_08009A0.h"
#include "structs/ewram.h"

/* bit 3: target to the left, bit 5: dead */
#define FLAGS(e) ((e)->unk_4FC.part16.unk_4FC)
#define TIMER(e) ((e)->unk_4FC.part16.unk_4FE[0])

/* 8.8 x 8.8 products; the two operand orders give the original registers */
static inline s32 FixMul(s32 a, s32 b)
{
    return (a >> 8) * (b >> 8);
}

static inline s32 FixMulR(s32 a, s32 b)
{
    s32 x = a >> 8;
    return (b >> 8) * x;
}

static inline s32 Mul(s32 a, s32 b)
{
    return a * b;
}

/**
 * @brief Move along an arc while the partner reacts, then await partner state 0 and return to positioning or death according to health.
 */
void Nemesis_UpdatePartnerFlight(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *partner = &gEwramData->entityData[(u8)entity->unk_4F4.part8.unk_4F6];
    s32 angle, speedX, speedY;
    s16 t;

    /* set before the switch: the original rematerialises it later */
    speedY = 0x18000;

    switch (entity->subState)
    {
    case 0:
        entity->timer = 0;
        TIMER(entity) = 0;
        if (gEwramData->playerState.player->posX.whole < entity->posX.whole)
            FLAGS(entity) |= 8;
        else
            FLAGS(entity) &= ~8;
        entity->subState++;
    case 1:
        angle = 0xD000;
        speedX = 0x20000;
        t = TIMER(entity);
        if (!(FLAGS(entity) & 8))
            speedX = -speedX;
        entity->posX.whole += FixMul(speedX, Sin(angle + 0x4000));
        entity->posY.whole += FixMulR(speedY, Sin(angle)) + Mul(t, 0xF00);
        TIMER(entity)++;
        if (partner->state == 4)
            entity->subState++;
        break;
    case 2:
        if (partner->state == 0)
        {
            if ((s16)entity->unk_518.part16.hp <= 0)
            {
                if (entity->state != 0xFF)
                {
                    entity->state = 0xFF;
                    entity->stage = 0;
                    entity->subState = 0;
                }
            }
            else if (entity->state != 1)
            {
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
            }
        }
        break;
    }
    if ((s16)entity->unk_518.part16.hp <= 0)
        FLAGS(entity) |= 0x20;
}
