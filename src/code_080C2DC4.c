/* Ripper projectile hit response: enter falling state 5. */
#include "code_080D73B8.h"
#include "structs/ewram.h"
/**
 * @brief Play the hit sound, set hit flash and flags 2/4, and enter falling
 * state 5 with substate counters reset. This callback belongs to the
 * Ripper projectile, not Ghost Dancer.
 */
void RipperProjectile_OnHit(struct EwramData_EntityData *entity)
{
    s32 zero;
    entity->hitFlashTimer=8;
    zero=0;
    PlaySong(0x13A);
    entity->hit.flags|=4;
    entity->hit.flags|=2;
    entity->state=5;
    entity->stage=zero;
    entity->subState=zero;
}
