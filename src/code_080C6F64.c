/* Turning attack: three passes, then return to the ordinary state. */
#include "code_08009A0.h"
#include "nemesis_entity.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern const u16 sNemesisTurn[3];
extern void Nemesis_UpdateRotatedBoxes(struct EwramData_EntityData *);
extern void EntitySetVelocityFromAngle(struct EwramData_EntityData *, s32, s32);
extern void *memcpy(void *, const void *, unsigned long);

#define FLAGS(e) (NEMESIS_FLAGS((e)))
#define LEFT(e) ((e)->unk_4F4.part8.unk_4F7)

static inline s16 GetTurn(struct EwramData_EntityData *e)
{ return NEMESIS_TURN(e); }
/**
 * @brief Run three table-driven turning passes with translation and affine rotation, then pause and return to positioning state 1.
 */
void Nemesis_UpdateTurningAttack(struct EwramData_EntityData *entity)
{
    s32 frames = 14;
    u16 turn[3];

    memcpy(turn, sNemesisTurn, sizeof(turn));
    Nemesis_UpdateRotatedBoxes(entity);
    switch (entity->subState)
    {
    case 0:
        LEFT(entity) = 3;
        entity->subState++;
    case 1:
        NEMESIS_TURN(entity) = turn[3 - LEFT(entity)];
        if (!(FLAGS(entity) & 2))
            NEMESIS_TURN(entity) = -GetTurn(entity);
        EntitySetVelocityFromAngle(entity, NEMESIS_ANGLE(entity) + 0x4000, 0x10000);
        entity->timer = 0;
        entity->subState++;
    case 2:
        if (entity->timer++ > frames)
        {
            entity->timer = 0;
            entity->subState++;
            PlaySong(0x85);
            break;
        }
        entity->posX.whole += entity->velX.whole * 3;
        entity->posY.whole += entity->velY.whole * 3;
        break;
    case 3:
        if (LEFT(entity) == 1)
        {
            entity->subState = 4;
            entity->timer = 0;
            break;
        }
        if (entity->timer++ > frames / 2)
        {
            entity->timer = 0;
            /* Re-read the remaining-pass byte after resetting the timer,
             * as the original does; do not forward the earlier comparison. */
            LEFT(entity) = *(volatile u8 *)&LEFT(entity) - 1; /* FAKEMATCH: volatile read forces the reload of the original */
            if (LEFT(entity) == 0)
                entity->subState = 4;
            else
                entity->subState = 1;
            break;
        }
        NEMESIS_ANGLE(entity) += NEMESIS_TURN(entity);
        ObjAffine_Set(NEMESIS_ANGLE(entity), 0x10000, 0x10000, 0, 0, entity->affineIndex);
        EntitySetVelocityFromAngle(entity, NEMESIS_ANGLE(entity) + 0x4000, 0x10000);
        entity->posX.whole -= entity->velX.whole * 6;
        entity->posY.whole -= entity->velY.whole * 6;
        break;
    case 4:
        if (entity->timer++ > 0x28)
        {
            entity->timer = 0;
            if (entity->state != 1)
            {
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
            }
            LEFT(entity) = 0;
        }
        break;
    }
}
