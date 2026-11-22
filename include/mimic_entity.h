#ifndef MIMIC_ENTITY_H
#define MIMIC_ENTITY_H

/*
 * Mimic (EnemyMimic*, Mimic_*): per-frame flags (bit 0 = no contact damage,
 * bit 1 = show the held frame of sMimicAnimation) and the held frame.
 * Its trigger (MimicTrigger_*) links back to it through trigger owner.
 */
#define MIMIC_FLAGS(e) e->unk_4FC.part8.unk_4FC
#define MIMIC_HELD_FRAME(e) e->unk_4FC.part8.unk_4FD
#define MIMIC_TRIGGER_OWNER(e) e->unk_500.unk_500_entity

/*
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

#endif /* MIMIC_ENTITY_H */
