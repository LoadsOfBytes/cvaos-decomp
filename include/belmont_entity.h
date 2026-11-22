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
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define BELMONT_ATTACK(e) e->unk_500.part8.unk_500
#define BELMONT_ATTACK_OFF(e) e->unk_500.part8.unk_501
#define BELMONT_FLAGS(e) e->unk_4F8.part8.unk_4F8
#define BELMONT_FLASH(e) e->unk_4F8.part8.unk_4F9
#define BELMONT_HITS(e) e->unk_504.part8.unk_504
#define BELMONT_WEAPON_INDEX(e) e->unk_4F4.part8.unk_4F6
#define BELMONT_POWER_STEP(e) e->unk_4F8.part8.unk_4FA

/* Scene object (Belmont_CreateSceneObject / _UpdateIntro / _UpdateSceneBoundary): its entity index. */
#define BELMONT_SCENE_OBJECT_INDEX(e) e->unk_4F8.part8.unk_4FB

/*
 * Subweapons 2 / 3 (BelmontSubweapon2_Update / 3_Update): 16.16 walk speed
 * of the ground flame, stored in the pointer-typed slot (NULL = stopped).
 */
#define BELMONT_SUBWEAPON_SPEED(e) e->unk_514.unk_514_32

#endif /* BELMONT_ENTITY_H */
