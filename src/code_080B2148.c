/* Disc Armor death. */
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void PlaySong(s32);
extern s32 Sin(s32);
extern s32 RandomSigned(s32);
extern struct EwramData_EntityData *Effect_SpawnShared(s32, s32, s32, s32);

/**
 * @brief B2148 | Disc Armor death, a variant of the Axe Armor death
 * AxeArmor_UpdateDeath: state 0 starts animation 8 and an upward velocity; state 1
 * emits rising debris (type 1/7) on even frames and a sound plus effect 16/7
 * every eighth frame, without a frame limit.  Returns 1 once the animation
 * reports flag 4.
 */
s32 DiscArmor_UpdateDeath(struct EwramData_EntityData *entity)
{
    u8 state = entity->subState;
    switch (state)
    {
    case 0:
        Anim_StartIndexed(entity, 8, 0, 0);
        entity->subState = 1;
        entity->velY.whole = -0x280000;
        entity->velX.whole = state;
        PlaySong(0x6F);
        break;
    case 1:
    {
        u32 timer;
        s32 y, phase, x;
        timer = entity->timer + 1;
        entity->timer = timer;
        y = entity->posY.whole + entity->velY.whole;
        entity->velY.whole += 0x12000;
        phase = entity->velX.whole;
        entity->velX.whole = phase + 0x5000;
        if ((timer & state) == 0)
        {
            struct EwramData_EntityData *debris;
            s32 offset = Sin(phase + 0x9000);
            x = entity->posX.whole + (offset << 4) + RandomSigned(0x40000);
            debris = Effect_SpawnShared(x, y, 1, 7);
            if (debris != NULL)
            {
                debris->velY.whole = 0x8000;
                debris->accelY.whole = -0x1000;
            }
        }
        if ((entity->timer & 7) == 0)
        {
            PlaySong(0x6E);
            x = entity->posX.whole + (RandomSigned(16) << 16);
            y += RandomSigned(4) << 16;
            Effect_SpawnShared(x, y, 16, 7);
        }
        if (entity->animFlags & 4)
            return 1;
        break;
    }
    }
    return 0;
}
