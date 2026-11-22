/* Enter the special reaction state and remember the attacker's world position. */
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
/**
 * @brief Contact: Player_ApplyContactDamage(attack, 0, 0, 0), remember the entity
 * hit and its room position (unk_4F8 / unk_4FC / unk_500) and stick to it
 * (state 2).
 */
void MudmanShot_DamagePlayerOnContact(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    struct PlayerContactDamage effect;
    s32 zero;
    effect.attack = entity->attack;
    zero = 0;
    effect.attributes = 0;
    effect.unk_4 = 0;
    effect.reaction = 0;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
    entity->unk_4F8.unk_4F8_entity = attacker;
    entity->unk_4FC.whole = attacker->posX.whole + background->xPos.whole;
    entity->unk_500.whole = attacker->posY.whole + background->yPos.whole;
    entity->state = 2;
    entity->subState = zero;
    Anim_StartIndexed(entity, 6, 0, 0);
    Anim_Update(entity);
    entity->animDone = 0;
}
