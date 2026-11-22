#ifndef ZOMBIE_ENTITY_H
#define ZOMBIE_ENTITY_H

/*
 * Zombie (EnemyZombie*, Zombie_*):
 * - timer: frames it walks before sinking back (random 600..1111), then the
 *   sinking frame counter;
 * - debris / sprite spawned: the falling debris and the temporary sprite of
 *   the current animation step were spawned;
 * - a spawner keeps its live zombies in child count, each child links back
 *   to it through spawner.
 */
#define ZOMBIE_TIMER(e) e->unk_500.whole
#define ZOMBIE_DEBRIS_SPAWNED(e) e->unk_4F8.part8.unk_4F8
#define ZOMBIE_SPRITE_SPAWNED(e) e->unk_4F8.part8.unk_4F9
#define ZOMBIE_CHILD_COUNT(e) e->unk_4FC.whole
#define ZOMBIE_SPAWNER(e) e->unk_4FC.unk_4FC_entity

/*
 * Body pieces of Zombie / Zombie Officer / Zombie Soldier (*_SpawnPiece,
 * *Piece_Update): floor y where the falling piece stops.
 * Temporary sprite (Zombie_SpawnTemporarySprite): lifetime in frames.
 */
#define ZOMBIE_PIECE_FLOOR_Y(e) e->unk_500.whole
#define ZOMBIE_SPRITE_LIFETIME(e) e->unk_500.whole

/*
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

#endif /* ZOMBIE_ENTITY_H */
