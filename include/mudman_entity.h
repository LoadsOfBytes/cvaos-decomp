#ifndef MUDMAN_ENTITY_H
#define MUDMAN_ENTITY_H

/*
 * Mudman shot (MudmanShot_DamagePlayerOnContact, MudmanShot_Update): after a
 * hit it sticks to the entity it hit (ENTITY_OWNER) and keeps that entity's
 * 16.16 room position of the previous frame (the shot moves along with it).
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define MUDMAN_SHOT_OWNER_X(e) e->unk_4FC.whole
#define MUDMAN_SHOT_OWNER_Y(e) e->unk_500.whole

#endif /* MUDMAN_ENTITY_H */
