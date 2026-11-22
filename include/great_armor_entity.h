#ifndef GREAT_ARMOR_ENTITY_H
#define GREAT_ARMOR_ENTITY_H

/*
 * Great Armor (EnemyGreatArmor*, GreatArmor_*; joint slots in joint_entity.h):
 * - variant: index into the per-variant timing tables (0..2 by enemy id;
 *   the TU macros VARIANT read it as s8);
 * - hit / hit count: set and incremented by GreatArmor_OnHit, the flag is
 *   cleared by the update (the walk TU reads them as WAS_HIT / HIT_COUNT);
 * - attack hit: the TU macro HIT of GreatArmor_UpdateAttack.
 * Falling parts after death (GreatArmorPart_Update): spin, affine angle and
 * floor y. Slash effect (GreatArmorSlashEffect_*): the armor it follows.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define GREAT_ARMOR_VARIANT(e) e->unk_508.part8.unk_508
#define GREAT_ARMOR_ATTACK_HIT(e) e->unk_508.part8.unk_509
#define GREAT_ARMOR_HIT(e) e->unk_4FC.part8.unk_4FE
#define GREAT_ARMOR_HIT_COUNT(e) e->unk_4FC.part8.unk_4FD
#define GREAT_ARMOR_PART_SPIN(e) e->unk_4F4.whole
#define GREAT_ARMOR_PART_ANGLE(e) e->unk_4FC.whole
#define GREAT_ARMOR_PART_FLOOR_Y(e) e->unk_508.whole
#define GREAT_ARMOR_SLASH_OWNER(e) e->unk_500.unk_500_entity

/*
 * Walk / attack AI: guard flag (1 while the shield arm is raised in the attack
 * states, makes the second hit box harmless; cleared every frame by
 * EnemyGreatArmorUpdate) and the number of back turns in a hop sequence.
 */
#define GREAT_ARMOR_GUARD(e) e->unk_504.part8.unk_505
#define GREAT_ARMOR_HOP_TURNS(e) e->unk_4FC.part8.unk_4FF

#endif /* GREAT_ARMOR_ENTITY_H */
