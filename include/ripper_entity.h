#ifndef RIPPER_ENTITY_H
#define RIPPER_ENTITY_H

/*
 * Ripper (EnemyRipperCreate, Ripper_*; same template as the Fleaman):
 * airborne (1 when both spawn params are 0xFF: starts in the air instead of
 * snapping to the ground), the index of the animation it last started, and
 * the thrown flag (a knife was thrown during the current jump).
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define RIPPER_AIRBORNE(e) e->unk_4F8.part8.unk_4F8
#define RIPPER_ANIM(e) e->unk_4F8.part8.unk_4F9
#define RIPPER_THROWN(e) e->unk_504.part8.unk_504

/* Projectile (Ripper_CreateProjectile, RipperProjectile_Update): its Ripper. */
#define RIPPER_PROJECTILE_OWNER(e) e->unk_508.unk_508_entity

#endif /* RIPPER_ENTITY_H */
