#ifndef BELMONT_ENTITY_H
#define BELMONT_ENTITY_H

/*
 * Belmont (boss) work slots:
 * - attack: contact attack power from the template, restored after moves;
 *   attack off: always 0, copied to attack to disable contact damage during
 *   dodges, slides and reactions;
 * - flags: state bits (8 = ignore hit reactions, 0x10 = scene objects made);
 * - flash: frames of the hit flash / reaction;
 * - hits: hits taken since the last reaction (reacts after 3);
 * - weapon index: entity index of his attached weapon;
 * - power step: counter of the powered phase.
 * The macros expand to member chains or to the ENTITY_* views of the common
 * block (structs/ewram.h) without added parentheses; use them with a plain
 * base expression.
 */

/* Scene object (Belmont_CreateSceneObject / _UpdateIntro / _UpdateSceneBoundary): its entity index. */

/*
 * Subweapons 2 / 3 (BelmontSubweapon2_Update / 3_Update): 16.16 walk speed
 * of the ground flame, stored in the pointer-typed slot (NULL = stopped).
 */
#define BELMONT_SUBWEAPON_SPEED(e) ENTITY_PTR_514(e)

/* Power particles (Belmont_CreatePowerParticle, BelmontPowerParticle_Update):
 * step (set to the powered phase state by Belmont_UpdatePoweredPhase). */

#endif /* BELMONT_ENTITY_H */
