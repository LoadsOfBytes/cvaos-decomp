/* Box callbacks and hit-effect setup, 0x08073E58..0x08073EE7. */
#include "code/code_080211F0.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

void ManticoreTail_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void ManticoreTail_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

void ManticoreTail_EnableHitBoxes(struct EwramData_EntityData *entity)
{
    Hit_Enable(entity, 8, ManticoreTail_DamagePlayerOnContact, (s32)ManticoreTail_OnHit);
    HitBox_SetBodyRaw(entity, 0x0C0CFAFA); /* box {-6, -6, 12, 12} */
    HitBox_SetAttackRaw(entity, 0x0C0CFAFA);
    entity->hit.flags &= ~4;
    entity->hit.flags &= ~2;
}

/**
 * @brief Contact of a tail part: Player_ApplyContactDamage with attribute 0x40,
 * the owner's full attack during the sting (state 1, sub-states <= 4), else
 * half of it.
 */
void ManticoreTail_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage effect;
    struct EwramData_EntityData *parent = entity->unk_4F8.unk_4F8_entity;
    if (parent->state == 1 && parent->subState <= 4)
        effect.attack = parent->attack;
    else
        effect.attack = parent->attack >> 1;
    effect.attributes = 0x40;
    effect.unk_4 = 0;
    effect.reaction = 1;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
}
