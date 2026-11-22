#ifndef PARTICLE_EFFECT_H
#define PARTICLE_EFFECT_H

/*
 * Work slots of the generic particle effects updated by Effect_UpdateParticle,
 * Effect_UpdateRandomlyAnimatedParticle, Effect_UpdateTimedParticle and
 * Effect_UpdateParticleWithDamping:
 * - flags (unk_4FC byte): bit 0 / bit 1 damp the x / y velocity, bit 2 blink
 *   out with Entity_GetVisibilityPattern, bit 3 animate (timed particle) or
 *   animate every other update (damped particle);
 * - flip flags (unk_4FD byte): bit 0 / bit 1 randomize flipX / flipY each frame;
 * - damping (unk_4FE byte): damping amount of Effect_UpdateParticleWithDamping
 *   (the other updates damp by 8).
 * Expansion is token-identical (no added parentheses); plain identifiers only.
 */
#define PARTICLE_FLAGS(e) e->unk_4FC.part8.unk_4FC
#define PARTICLE_FLIP_FLAGS(e) e->unk_4FC.part8.unk_4FD
#define PARTICLE_DAMPING(e) e->unk_4FC.part8.unk_4FE

#endif /* PARTICLE_EFFECT_H */
