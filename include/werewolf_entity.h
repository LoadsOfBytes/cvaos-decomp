#ifndef WEREWOLF_ENTITY_H
#define WEREWOLF_ENTITY_H

/*
 * Werewolf: set by Werewolf_DamagePlayerOnContact when the dash (state 1,
 * sub-states 1..2) hit the player; the dash then stops (state 3).
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define WEREWOLF_DASH_HIT(e) e->unk_4F4.whole

#endif /* WEREWOLF_ENTITY_H */
