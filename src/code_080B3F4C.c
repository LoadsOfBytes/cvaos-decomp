/* Damage helper. */
#include "code/code_080211F0.h"
#include "balore_entity.h"
#include "structs/ewram.h"

/**
 * @brief B3F4C | Damage request at the entity's position: strength 90 / type 3
 * with unk_51C byte 1 bit 0, else 80 or 55 / type 1 (bit 1).  Type 3 also
 * pushes the player (Player_SetKnockbackVelocity) by +-0x60000 along the entity's velX
 * direction and -0x20000 vertically.
 */
void BaloreHandHitBox_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage hit;

    hit.attributes = 0;
    hit.unused = 0;
    if (BALORE_HAND_HIT_FLAGS(entity) & 1)
    {
        hit.attack = 90;
        hit.reaction = 3;
    }
    else if (BALORE_HAND_HIT_FLAGS(entity) & 2)
    {
        hit.attack = 80;
        hit.reaction = 1;
    }
    else
    {
        hit.attack = 55;
        hit.reaction = 1;
    }
    hit.sourceX = entity->posX.whole;
    hit.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&hit);
    if (hit.reaction == 3)
    {
        s32 vx = 0x60000;
        if (entity->velX.whole < 0)
            vx = -vx;
        Player_SetKnockbackVelocity(vx, -0x20000);
    }
}
