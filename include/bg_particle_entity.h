#ifndef BG_PARTICLE_ENTITY_H
#define BG_PARTICLE_ENTITY_H

/*
 * Floating background particle (BgParticle_SpawnAll, BgParticle_Update):
 * - kind: 0..3 (size class; animation index and the speed shift; kind 3
 *   uses the second angle);
 * - owner index: entity index of the spawning entity;
 * - angle x / y: phases of the sine drift.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define BG_PARTICLE_KIND(e) e->unk_500.part8.unk_500
#define BG_PARTICLE_OWNER_INDEX(e) e->unk_500.part8.unk_501
#define BG_PARTICLE_ANGLE_X(e) e->unk_4F8.whole
#define BG_PARTICLE_ANGLE_Y(e) e->unk_4FC.whole

#endif /* BG_PARTICLE_ENTITY_H */
