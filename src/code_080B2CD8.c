#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Sin(s32);
extern s32 RandomSigned(s32);
extern struct EwramData_EntityData *Effect_SpawnShared(s32, s32, s32, s32);

s32 BomberRockArmor_UpdateDeath(struct EwramData_EntityData *entity)
{
    s32 state = entity->subState;
    switch (state)
    {
    case 0:
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->subState = 1;
        entity->velY.whole = -0x280000;
        entity->velX.whole = state;
        PlaySong(0x6F);
        break;
    case 1:
        {
            s32 x, y;
            entity->timer++;
            y = entity->posY.whole + entity->velY.whole;
            entity->velY.whole += 0x8000;
            entity->velX.whole += 0x5000;
            if (!(entity->timer & 3))
            {
                struct EwramData_EntityData *effect;
                s32 offset;
                x = Sin(entity->velX.whole + 0x4000);
                offset = RandomSigned(0x40000);
                x <<= 4; x = entity->posX.whole + x + offset;
                effect = Effect_SpawnShared(x, y, 0xF, 7);
                if (effect != NULL)
                {
                    effect->velY.whole = 0x8000;
                    effect->accelY.whole = -0x2000;
                }
            }
            if (!(entity->timer & 7))
            {
                PlaySong(0x72);
                x = entity->posX.whole + (RandomSigned(0x10) << 16);
                y += RandomSigned(4) << 16;
                Effect_SpawnShared(x, y, 0x10, 6);
            }
            if (entity->animFlags & 4)
                return 1;
        }
        break;
    }
    return 0;
}
