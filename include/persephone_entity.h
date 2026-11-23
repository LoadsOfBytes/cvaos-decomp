#ifndef PERSEPHONE_ENTITY_H
#define PERSEPHONE_ENTITY_H

/*
 * Persephone: contact hit (set on the owner by Persephone_DamagePlayerOnContact,
 * from the body or an attack box; a jump attack that hit lands at once);
 * attack box (Persephone_SpawnAttackBox): frames it stays.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define PERSEPHONE_CONTACT_HIT(e) e->unk_504.part8.unk_504
#define PERSEPHONE_BOX_TIME(e) e->unk_500.whole

#endif /* PERSEPHONE_ENTITY_H */
