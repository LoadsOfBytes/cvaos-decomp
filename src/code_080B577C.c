/* Hovering phase of the Balore-area body. */
#include "code_08009A0.h"
#include "structs/ewram.h"

extern s32 Target_GetX(struct EwramData_EntityData *);

/* lean towards the target, 0..8 with 4 = centred */
#define TILT(e) ((e)->unk_4F4.part8.unk_4F6)

/**
 * @brief B577C | Hover phase.  State 1 bobs vertically (sine of
 * timer << 8, / 8 per frame), leans 0..8 (unk_4F6) after the horizontal
 * distance to the target (Target_GetX) in 16-pixel steps and sets
 * unk_4FC = 5; at HP (unk_518) <= 1999 it moves on to state 2 (unk_4FC = 7),
 * which waits for bit 5 of unk_502, clears it, recentres the lean and
 * switches to state = 3.  State 3 counts timer down and restarts
 * state 1 with unk_4FC = 1 and timer = 0xF0.
 */
void Balore_UpdateHover(struct EwramData_EntityData *entity)
{
    s32 dx;

    switch (entity->subState)
    {
    case 0:
        entity->subState = 1;
    case 1:
        entity->posY.whole -= Sin(entity->timer << 8) / 8;
        entity->timer++;
        if (Target_GetX(entity) - entity->posX.whole < 0)
            dx = -((entity->posX.whole - Target_GetX(entity)) >> 16);
        else
            dx = (Target_GetX(entity) - entity->posX.whole) >> 16;
        TILT(entity) = dx / 16 + 4;
        if ((s8)TILT(entity) < 0)
            TILT(entity) = 0;
        else if ((s8)TILT(entity) > 8)
            TILT(entity) = 8;
        entity->unk_4FC.part8.unk_4FC = 5;
        if ((s16)entity->unk_518.part16.hp <= 1999)
            entity->subState = 2;
        break;
    case 2:
        entity->unk_4FC.part8.unk_4FC = 7;
        if (entity->unk_500.part16.unk_502 & 0x20)
        {
            /* u16 view of the flags keeps the 0xFFDF mask */
            entity->unk_500.part16.unk_502 = (u16)entity->unk_500.part16.unk_502 & 0xFFDF;
            TILT(entity) = 4;
            entity->state = 3;
            entity->subState = 0;
        }
        break;
    case 3:
        if (entity->timer != 0)
            entity->timer--;
        else
        {
            entity->unk_4FC.part8.unk_4FC = 1;
            entity->subState = 1;
            entity->timer = 0xF0;
        }
        break;
    }
}
