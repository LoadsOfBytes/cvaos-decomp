#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_080211F0.h"
#include "code/code_08040A38.h"
#include "code_08011DD0.h"
#include "structs/ewram.h"

void PlaySong(u32 song);
extern s32 Target_GetX(struct EwramData_EntityData *entity);

struct BaloreFlightSteeringParameters {
    u16 offsetX;
    u16 offsetY;
    u16 targetAngle;
    u16 turnStep;
    s32 targetRadius;
    s32 radiusStep;
    s32 targetSpeed;
    s32 stepSpeed;
};

extern s32 Entity_SteerToPoint(struct EwramData_EntityData *entity, struct BaloreFlightSteeringParameters *p);

#define SPEED(e) (*(s32 *)&(e)->unk_514)
#define FLIGHT(e) (*(struct BaloreFlightSteeringParameters **)&(e)->unk_50C)
#define UNK_51C_BYTE(e, i) (((u8 *)&(e)->unk_51C)[i])

/* Four-stage flight attack: approach, turn, dive, then emit 16 sparks.
 * Return one when the attack completes. */
s32 BaloreHand_UpdateDive(struct EwramData_EntityData *entity)
{
    struct BaloreFlightSteeringParameters *p = FLIGHT(entity);
    s32 dx;
    s32 i;
    struct EwramData_EntityData *spark;

    switch (entity->subState)
    {
    case 0:
        SPEED(entity) = 0x30000;
        UNK_51C_BYTE(entity, 0) = 0x2E;
        dx = Target_GetX(entity) - entity->posX.whole;
        if (!entity->flipX)
            dx = -dx;
        p->offsetX = (dx < 0 ? -(-dx >> 16) : dx >> 16) - 0x20;
        if ((s16)p->offsetX < 0)
            p->offsetX = 0;
        else if ((s16)p->offsetX > 0x60)
            p->offsetX = 0x60;
        p->offsetY = -0x10;
        p->targetAngle = 0xF000;
        p->turnStep = 0x180;
        p->targetRadius = 0x4C0000;
        p->radiusStep = 0x20000;
        p->targetSpeed = 0x8000;
        p->stepSpeed = 0x1000;
        entity->subState = 1;
    case 1:
        if (Entity_SteerToPoint(entity, p) == 0)
            break;
        SPEED(entity) = 0x8000;
        p->offsetY = -0x20;
        p->targetAngle = 0xE000;
        p->turnStep = 0x200;
        p->targetSpeed = 0x8000;
        entity->subState = 2;
        break;
    case 2:
        if (Entity_SteerToPoint(entity, p) == 0)
            break;
        SPEED(entity) = 0x18000;
        p->offsetY = 0x28;
        p->targetAngle = 0x800;
        p->turnStep = 0x200;
        p->targetSpeed = 0x50000;
        p->stepSpeed = 0x4000;
        /* Set bit 1 while preserving the remaining flight flags. */
        UNK_51C_BYTE(entity, 1) = (UNK_51C_BYTE(entity, 1) & ~2) + 2;
        PlaySong(0x86);
        entity->subState = 3;
        break;
    case 3:
        if (Entity_SteerToPoint(entity, p) == 0)
            break;
        dx = 0x460000;
        if (!entity->flipX)
            dx = -dx;
        dx += entity->posX.whole;
        for (i = 16; i != 0; i--)
        {
            spark = Effect_CreateSharedFrame(dx + (RandomSigned(0x10) << 16), entity->posY.whole + 0x1E0000, (RandomNumberGenerator() & 1) + 0x40, 6);
            if (spark != NULL)
            {
                spark->velY.whole = -0x30000 - ((RandomNumberGenerator() & 0x3F) << 12);
                spark->accelY.whole = 0x3000;
                spark->velX.whole = RandomSigned(0x40) << 11;
            }
        }
        PlaySong(0x80);
        UNK_51C_BYTE(entity, 1) &= ~2;
        ScreenShake_Request(0, 8, 3, 7);
        return 1;
    }
    return 0;
}
