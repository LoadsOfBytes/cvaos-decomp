/* Death second-phase hover and attack selection. State numbers are
 * observed dispatcher values, not evidence of original source names. */
#include "code_08009A0.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern void EntityDampVelocityY(struct EwramData_EntityData *, s32);

#define PLAYER (gEwramData->playerState.player)

/* Keep these inline boundaries: they separate the address and random-mask
 * lifetimes. A byte match does not prove the original helper organization. */
static inline s32 PlayerX(void)
{
    return PLAYER->posX.whole;
}

static inline u8 RandomChoice(void)
{
    return RandomNumberGenerator() & 0x3F;
}

/**
 * @brief Hover state (3): track player X and oscillating height, then
 * choose dash (4), throw (5) or wall attack (6). Retain the empty substate
 * case and reuse of the zero-valued target temporary required for matching.
 */
void Death_UpdateHover(struct EwramData_EntityData *entity)
{
    s32 ty, s, acc, q;
    u32 r;

    switch (entity->subState)
    {
        case 0:
            entity->timer = 0;
            entity->subState = 1;
            /* Fall through. */
        case 1:
            ty = PLAYER->posY.whole;
            s = Sin(entity->timer << 8);
            if (ty < 0)
                ty += 7;
            q = ty >> 3;
            s = (s << 4) + 0x500000;
            ty = q + s;
            if (ty <= 0x3FFFFF)
                ty = 0x400000;
            entity->timer++;
            {
                s32 px = PlayerX();
                acc = 0x400;
                {
                    s32 x = entity->posX.whole;
                    if (px < x - 0x400000)
                        entity->accelX.whole = -0x400;
                    else if (px > x + 0x400000)
                        entity->accelX.whole = acc;
                    else
                    {
                        entity->accelX.whole = 0;
                        EntityDampVelocityX(entity, 0x28);
                    }
                }
            }
            {
                s32 y = entity->posY.whole;
                if (ty < y - 0x100000)
                    entity->accelY.whole = -0x400;
                else if (ty > y + 0x100000)
                    entity->accelY.whole = acc;
                else
                {
                    entity->accelY.whole = 0;
                    EntityDampVelocityY(entity, 0x28);
                }
            }
            EntityAccelerateAndMove(entity, 0x14000);
            /* Reuse the target temporary for the zero passed into states 4/6;
             * separate zero locals change allocation and tail sharing. */
            do { ty = entity->timer & 0x1F; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
            if (ty == 0 && (RandomNumberGenerator() & 0x3F) <= 0x2F)
            {
                r = RandomChoice();
                entity->ext.death.unusedChoice = r;
                if (r <= 0x14)
                {
                    entity->state = 4;
                    entity->stage = ty;
                    entity->subState = ty;
                    break;
                }
                else if (r <= 0x29 && (s16)entity->posY.part16.integer > 0x60)
                {
                    entity->state = 6;
                    entity->stage = ty;
                    entity->subState = ty;
                    break;
                }
                else
                {
                    entity->state = 5;
                    entity->stage = 0;
                    entity->subState = 0;
                    break;
                }
            }
            break;
        /* Retain this empty case for the original switch decision tree. */
        case 2:
            break;
    }
}
