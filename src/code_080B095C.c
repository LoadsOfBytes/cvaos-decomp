#include "code_08009A0.h"
#include "minotaur_entity.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_Walk(struct EwramData_EntityData *, s32, void *, s32);
extern void Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);
extern u8 sMinotaurWalkProfile[];

/* Minotaur attack: windup, decelerating charge, recovery or follow-up. */
void Minotaur_UpdateAttack(struct EwramData_EntityData *entity)
{
    s32 remaining;
    switch (entity->subState)
    {
    case 0:
        {
            s32 flag = MINOTAUR_FLAGS(entity) & 2;
            if (flag == 0)
            {
                Anim_StartIndexed(entity, 5, 0, 1);
                entity->subState = 3;
                entity->timer = flag;
            }
            else
            {
                MINOTAUR_FLAGS(entity) &= ~1;
                Anim_StartIndexed(entity, 7, 0, 0);
                entity->subState = 1;
            }
        }
        break;
    case 1:
        if (entity->animFlags & 4)
        {
            entity->subState = 2;
            entity->timer = 8;
        }
        break;
    case 2:
        remaining = entity->timer;
        if (remaining == 0)
        {
            Anim_StartIndexed(entity, 5, 0, 1);
            entity->subState = 3;
            Afterimage_Create(entity, 40, 12, 1);
            Afterimage_Create(entity, 40, 6, 0);
        }
        else
            goto decrementTimer;
        break;
    case 3:
        Entity_Walk(entity, 0x34000 - entity->timer * 0x1400, sMinotaurWalkProfile, 12);
        if (++entity->timer == 24)
            Anim_StartIndexed(entity, 6, 0, 0);
        if (entity->timer > 40)
        {
            if (!(MINOTAUR_FLAGS(entity) & 2))
            {
                entity->subState = 4;
                entity->timer = 48;
            }
            else
            {
                entity->subState = 5;
                Anim_StartIndexed(entity, 4, 0, 0);
                MINOTAUR_FLAGS(entity) |= 1;
            }
        }
        break;
    case 4:
        {
            s32 timer;
            remaining = entity->timer;
            timer = (u8)remaining;
            if (timer != 0)
            {
decrementTimer:
                entity->timer = remaining - 1;
            }
            else
            {
                Anim_StartIndexed(entity, 1, 0, 1);
                entity->state = timer;
                entity->stage = timer;
                entity->subState = timer;
            }
        }
        break;
    case 5:
        if (*(u16 *)&entity->animStep == 3)
            PlaySong(0x86);
        if (entity->animFlags & 4)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->state = 0;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    }
}
