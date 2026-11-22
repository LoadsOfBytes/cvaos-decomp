#ifndef BOSS_PICKUP_ENTITY_H
#define BOSS_PICKUP_ENTITY_H

/*
 * Boss recovery pickup (BossRecoveryPickup_Update - TU aliases PHASE /
 * LANDED / SCALE): bob phase, landed flag and the affine scale derived from
 * the phase. Its particles (BossRecoveryPickupParticle_Update - TU aliases
 * ANGLE / STEP / RADIUS / SPEED): angle, angle step, 16.16 radius (they spiral
 * in until it is <= 0) and the accelerating step growth.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define BOSS_PICKUP_PHASE(e) e->unk_4F8.part16.unk_4F8
#define BOSS_PICKUP_LANDED(e) e->unk_4F8.part16.unk_4FA
#define BOSS_PICKUP_SCALE(e) e->unk_4FC.whole
#define BOSS_PICKUP_PARTICLE_ANGLE(e) e->unk_500.part16.unk_500
#define BOSS_PICKUP_PARTICLE_STEP(e) e->unk_500.part16.unk_502
#define BOSS_PICKUP_PARTICLE_RADIUS(e) e->unk_504.whole
#define BOSS_PICKUP_PARTICLE_SPEED(e) e->unk_508.whole

#endif /* BOSS_PICKUP_ENTITY_H */
