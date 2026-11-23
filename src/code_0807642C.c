/* Report a projectile hit at its current position and enter cleanup. */
#include "code/code_080211F0.h"
#include "structs/ewram.h"
void HarpyFeather_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage hit;
    s32 zero;
    hit.attack=entity->attack;
    zero=0;
    hit.attributes=0;
    hit.unused=0;
    hit.reaction=0;
    hit.sourceX=entity->posX.whole;
    hit.sourceY=entity->posY.whole;
    Player_ApplyContactDamage(&hit);
    entity->state=2;
    entity->subState=zero;
}
