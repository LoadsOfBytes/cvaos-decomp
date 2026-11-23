/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   HeadhunterBurstShot_DamagePlayerOnContact = PoisonWorm_DamagePlayerOnContact
 *   HeadhunterAttackTip_DamagePlayerOnContact = Object28_OnPlayerContact */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief Apply stored burst-shot damage with effect type 0x40
 * and flags 0.
 */
void HeadhunterBurstShot_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0x40;
    contactDamage.unused = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief Apply stored attack-tip damage with effect type 0
 * and flags 1.
 */
void HeadhunterAttackTip_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}
