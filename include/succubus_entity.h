#ifndef SUCCUBUS_ENTITY_H
#define SUCCUBUS_ENTITY_H

/*
 * Grab attack slots of the Succubus (Succubus_UpdateAttack):
 * - lunging: set while it dashes at the player; contact then grabs instead of
 *   dealing contact damage (Succubus_DamagePlayerOnContact);
 * - grabbed / holding: set by that contact; while holding there is no contact
 *   damage check;
 * - escape presses: buttons pressed by the player while held (frees at 30);
 * - drain timer / drain count: frames until the next drain hit and the number
 *   of hits dealt (the hold ends after 5).
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define SUCCUBUS_LUNGING(e) e->unk_500.part8.unk_500
#define SUCCUBUS_GRABBED(e) e->unk_500.part8.unk_501
#define SUCCUBUS_ESCAPE_PRESSES(e) e->unk_500.part8.unk_502
#define SUCCUBUS_DRAIN_COUNT(e) e->unk_500.part8.unk_503
#define SUCCUBUS_DRAIN_TIMER(e) e->unk_504.part8.unk_504
#define SUCCUBUS_HOLDING(e) e->unk_4FC.part8.unk_4FF

/*
 * Walk (Succubus_UpdateWalk; same template as Lilith): provoked (set when hit,
 * makes it attack at once), seen (the target came near, sound 0x1C7 played
 * once) and dodge checked (the dodge chance for the current player attack was
 * already rolled).
 */
#define SUCCUBUS_PROVOKED(e) e->unk_4FC.part8.unk_4FC
#define SUCCUBUS_SEEN(e) e->unk_504.part8.unk_505
#define SUCCUBUS_DODGE_CHECKED(e) e->unk_4FC.part8.unk_4FD

/*
 * Intro: set when it starts in its intro pose (spawn parameter 0, cleared
 * when the intro animation ends); a hit then only sets holding and cannot
 * bring its HP below 1 (Succubus_OnHit).
 */
#define SUCCUBUS_INTRO(e) e->unk_4FC.part8.unk_4FE

#endif /* SUCCUBUS_ENTITY_H */
