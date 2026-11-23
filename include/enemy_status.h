#ifndef ENEMY_STATUS_H
#define ENEMY_STATUS_H

/*
 * Enemy status word (low half of the entity field enemyStatus): ResolveDamage
 * stores the special attributes (mask 0xFC0) of the first hit an enemy is weak
 * to. Bits seen in the code: 0x40 = weakened (defense 0, attack -1/4), 0x100 =
 * petrified (frozen with palette 14 by Enemy_IsStunned, takes double damage),
 * 0x800 = HP swapped with the entity field mp.
 * The macros expand to the original tokens without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define ENEMY_STATUS(e) *(u16 *)&e->enemyStatus
#define ENEMY_STATUS_PETRIFIED 0x100

#endif /* ENEMY_STATUS_H */
