#include "code_08009A0.h"
#include "code/code_08040A38.h"
extern void Witch_SpawnGroundProjectile(struct EwramData_EntityData *, s32, s32, s32);

/**
 * @brief Death state (3): emit rising particles, spawn the ground projectile
 * in mode 1 and return nonzero for removal.
 */
s32 Witch_UpdateDeath(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *particle;
    s32 y;
    switch (entity->subState)
    {
    case 0:
        entity->subState = 1;
        entity->drawSkip = 1;
        entity->timer = 0x20;
        /* fall through */
    case 1:
        if (!(entity->timer & 1))
        {
            u32 timer = entity->timer;
            y = (-(timer >> 1) + 4) << 16;
            particle = Effect_SpawnShared(entity->posX.whole + (Sin(timer << 13) << 3), entity->posY.whole + y, 5, 6);
            if (particle != NULL)
            {
                particle->velY.whole = -0x18000;
                particle->accelY.whole = 0x800;
                particle->spriteLayer = 3;
            }
        }
        if (entity->timer != 0) entity->timer--;
        else entity->subState = 2;
        break;
    case 2:
        Witch_SpawnGroundProjectile(entity, 0, -0x10, 1);
        return 1;
    }
    return 0;
}
