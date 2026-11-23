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

/*
 * Double-drawn particles (Effect_UpdateTimedDoubleParticle, save sparkles;
 * drawn by Effect_DrawAtOffsetPair / SaveSparkle_Draw): 16.16 offset of the
 * second copy (random +-16 px).
 */
#define PARTICLE_PAIR_OFFSET_X(e) e->unk_500.whole
#define PARTICLE_PAIR_OFFSET_Y(e) e->unk_504.whole

/*
 * Table effects (Effect_CreateFromTable, Effect_UpdateTimedDoubleParticle,
 * Effect_UpdateTableAnimation): index into sUnk_085251A8 (frame, lifetime,
 * animation). Attached animations (Effect_SpawnAttachedAnimation,
 * Effect_UpdateAttachedAnimation): the entity they follow and the animation
 * index. Timed particles drawn by Effect_DrawTimedParticle: lifetime in frames
 * and the mode (0 = rise, 1 = step back through sTimedParticleFrames).
 */
#define PARTICLE_TABLE_INDEX(e) e->unk_4FC.whole
#define PARTICLE_ATTACH_PARENT(e) e->unk_4FC.unk_4FC_entity
#define PARTICLE_ATTACH_ANIM(e) e->unk_4F8.part8.unk_4F8
#define PARTICLE_LIFETIME(e) e->unk_4FC.part8.unk_4FD
#define PARTICLE_STEP_FRAMES(e) e->unk_4FC.part8.unk_4FE

/*
 * Player effect sets (Player_SpawnEffectSet, PlayerEffectSetPart_Update):
 * animation index of the part. Surface effects (Player_SpawnSurfaceEffect,
 * Effect_UpdatePlayerSurfaceEffect): animation index from
 * sPlayerSurfaceEffects.
 */
#define PARTICLE_EFFECT_SET_ANIM(e) e->unk_4F8.part8.unk_4F8
#define PARTICLE_SURFACE_ANIM(e) e->unk_4FC.part16.unk_4FC

/*
 * Hit effects (HitEffect_Spawn, HitEffect_SpawnAtContact, the hit spark of
 * ResolveDamage): effect type (0 plain hit, 1..6 attribute index); also
 * stored in subState, which HitEffect_Update uses.
 */
#define PARTICLE_HIT_TYPE(e) e->unk_4F8.part16.unk_4F8

/*
 * Player flash (Effect_CreatePlayerFlash, Effect_DrawPlayerFlash): its row
 * of sPlayerFlashInfo, the kind (table index) and the duration in frames
 * (info byte 1; the game is paused while it grows).
 */
#define PLAYER_FLASH_INFO(e) e->unk_4F8.whole
#define PLAYER_FLASH_KIND(e) e->unk_4FC.part8.unk_4FC
#define PLAYER_FLASH_DURATION(e) e->unk_4FC.part8.unk_4FD

/* Fragment sets (Entity_SpawnFragmentSet, FragmentSet_Update): the owner
 * counts its live fragments. */
#define FRAGMENT_SET_COUNT(e) e->unk_4F4.part8.unk_4F7

/* Soul acquisition (SoulAcquisition_CreateEffect / _DrawAndWait): frames it
 * holds the game paused (0x40). */
#define SOUL_ACQUIRE_DURATION(e) e->unk_4FC.part8.unk_4FD

#endif /* PARTICLE_EFFECT_H */
