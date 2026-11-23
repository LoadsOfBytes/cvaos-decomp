#include "code_08009A0.h"

/**
 * @brief Clear projectile HP so its update bursts and removes it.
 */
void EvilButcherProjectile_OnHit(struct EwramData_EntityData *entity)
{
    entity->hp = 0;
}

