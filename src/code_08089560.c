/* Sweep the attack direction while emitting effects on alternate frames. */
#include "code/code_08039340.h"
#include "gorgon_entity.h"
#include "code_080D73B8.h"
extern s32 Sin(s32);
extern s32 FixedMultiply(s32, s32);
extern void GorgonBreath_Spawn(struct EwramData_EntityData *, s32, s32, s32, s32);
/**
 * @brief Attack state machine (state 1): play animation 2 and, from its first step
 * on, emit breath puffs (GorgonBreath_Spawn) for the attack duration, then
 * return to state 0.
 */
void Gorgon_UpdateAttack(struct EwramData_EntityData *entity)
{
    s32 phase = entity->subState;
    switch (phase)
    {
    case 0:
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->subState++;
        break;
    case 1:
        if (entity->animStep != 0)
        {
            entity->subState = 2;
            if (entity->flipX)
            {
                GORGON_BREATH_ANGLE(entity) = 0;
                GORGON_BREATH_DIR(entity) = phase;
            }
            else
            {
                GORGON_BREATH_ANGLE(entity) = 0x8000;
                GORGON_BREATH_DIR(entity) = 0;
            }
            GORGON_BREATH_STEP(entity) = 3;
        }
        break;
    case 2:
        entity->timer++;
        if (entity->timer == 30 || entity->timer == 50) PlaySong(0x67);
        if ((u8)(entity->timer - 30) <= 40 && !(entity->timer & 1))
        {
            s32 x, y, vx, vy, speed;
            if (entity->flipX) x = 0x2A0000;
            else x = -0x2A0000;
            y = -0x120000;
            vx = Sin((u16)GORGON_BREATH_ANGLE(entity) + 0x4000);
            vy = Sin((u16)GORGON_BREATH_ANGLE(entity));
            speed = 0x30000;
            vx = FixedMultiply(vx, speed);
            vy = FixedMultiply(vy, speed);
            GorgonBreath_Spawn(entity, x, y, vx, vy);
            if ((s8)GORGON_BREATH_DIR(entity) == 0)
                GORGON_BREATH_ANGLE(entity) += 0x400;
            else GORGON_BREATH_ANGLE(entity) -= 0x400;
            if ((s8)++GORGON_BREATH_STEP(entity) > 6)
            {
                GORGON_BREATH_STEP(entity) = 0;
                GORGON_BREATH_DIR(entity) ^= 1;
            }
        }
        if (entity->animDone)
        {
            s32 zero = 0;
            entity->state = zero;
            entity->subState = 1;
            entity->unk_4FC.part8.unk_4FF = zero;
            entity->unk_4FC.part8.unk_4FD = zero;
            entity->velX.whole = zero;
            entity->accelX.whole = zero;
            entity->accelY.whole = zero;
            entity->timer = zero;
        }
        break;
    }
}
