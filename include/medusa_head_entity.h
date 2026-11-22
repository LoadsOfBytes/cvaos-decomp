#ifndef MEDUSA_HEAD_ENTITY_H
#define MEDUSA_HEAD_ENTITY_H

/*
 * Medusa Head (EnemyMedusaHead*, MedusaHead_*):
 * - wave up: 1 while the vertical acceleration is reversed;
 * - wave frame / wave length: frames of the current wave half and its
 *   length (random), wave up flips when it is reached;
 * - a spawner keeps its live heads in child count, each head links back to
 *   it through spawner.
 */
#define MEDUSA_HEAD_WAVE_UP(e) e->unk_504.part8.unk_504
#define MEDUSA_HEAD_WAVE_FRAME(e) e->unk_504.part8.unk_505
#define MEDUSA_HEAD_WAVE_LENGTH(e) e->unk_504.part8.unk_506
#define MEDUSA_HEAD_CHILD_COUNT(e) e->unk_4FC.whole
#define MEDUSA_HEAD_SPAWNER(e) e->unk_500.unk_500_entity

/*
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

#endif /* MEDUSA_HEAD_ENTITY_H */
