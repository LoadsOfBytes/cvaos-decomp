#ifndef PEEPING_EYE_ENTITY_H
#define PEEPING_EYE_ENTITY_H

/*
 * Peeping Eye: its pupil, eyeball and chain links keep the eye (the other
 * slots are TU-local PEEPING_EYE_* names in code_08087DC0.c).
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define PEEPING_EYE_OWNER(e) e->unk_500.unk_500_entity

#endif /* PEEPING_EYE_ENTITY_H */
