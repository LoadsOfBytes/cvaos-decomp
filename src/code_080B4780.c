/* Flight state machine on Entity_SteerToPoint. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern s32 Entity_SteerToPoint(struct EwramData_EntityData *, void *);
extern u8 sBaloreHandSteeringParams2[], sBaloreHandSwipeParams[], sBaloreHandSteeringParams3[];

#define SPEED(e) (*(s32 *)&(e)->unk_514)
/* unk_51C used as bytes by this entity */
struct BaloreHandAttackBytes {
    u8 animationIndex;
    u8 flags;
};
#define UNK_51C(e) ((struct BaloreHandAttackBytes *)&(e)->unk_51C)

/**
 * @brief B4780 | Flight phase built on Entity_SteerToPoint.  State 0 sets speed
 * 0x30000 and a 0x20-frame counter (timer), whose value / 16 + 0x2E goes
 * to byte 0 of unk_51C while it runs.  State 1 flies with sBaloreHandSteeringParams2;
 * on arrival speed 0x54000, sound 0x86, bit 0 of byte 1 of unk_51C, and
 * randomly (1/2) state 3 with a 0x2F-frame counter or state 2.  States 2
 * and 3 fly with sBaloreHandSwipeParams / sBaloreHandSteeringParams3; on arrival the bit is
 * cleared and 1 is returned, otherwise 0.
 */
s32 BaloreHand_UpdateSwipe(struct EwramData_EntityData *entity)
{
    u8 *params;

    switch (entity->subState)
    {
    case 0:
        SPEED(entity) = 0x30000;
        entity->subState = 1;
        entity->timer = 0x20;
    case 1:
        if (entity->timer != 0)
        {
            entity->timer--;
            UNK_51C(entity)->animationIndex = (*(volatile u8 *)&entity->timer >> 4) + 0x2E;
        }
        if (Entity_SteerToPoint(entity, sBaloreHandSteeringParams2))
        {
            SPEED(entity) = 0x54000;
            PlaySong(0x86);
            if ((RandomNumberGenerator2() & 0x3F) < 0x20)
            {
                entity->subState = 3;
                entity->timer = 0x2F;
            }
            else
                entity->subState = 2;
            UNK_51C(entity)->flags |= 1;
        }
        break;
    case 2:
        params = sBaloreHandSwipeParams;
        goto move;
    case 3:
        if (entity->timer != 0)
        {
            entity->timer--;
            UNK_51C(entity)->animationIndex = (*(volatile u8 *)&entity->timer >> 4) + 0x2E;
        }
        params = sBaloreHandSteeringParams3;
    move:
        if (Entity_SteerToPoint(entity, params))
        {
            UNK_51C(entity)->flags &= 0xFE;
            return 1;
        }
        break;
    }
    return 0;
}
