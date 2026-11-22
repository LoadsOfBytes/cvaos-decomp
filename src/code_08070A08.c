/* Spawn the part's death sprite and start its forty-tick death state. */
#include "code/code_080211F0.h"
extern void KillerFish_SpawnRisingSprite(struct EwramData_EntityData *, s32, s32, s32);
/**
 * @brief Hit: rising sprite 15 at the fish position, then state 2 (40 frames).
 */
void KillerFishSwarm_OnHit(struct EwramData_EntityData *entity, void *attacker, u16 *hit)
{
    Player_CalculateAttackPower(hit[2]);
    KillerFish_SpawnRisingSprite(entity, entity->posX.whole, entity->posY.whole, 15);
    entity->state = 2;
    entity->timer = 40;
    entity->subState = 0;
}
