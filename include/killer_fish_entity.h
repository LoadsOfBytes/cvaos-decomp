#ifndef KILLER_FISH_ENTITY_H
#define KILLER_FISH_ENTITY_H

/*
 * Killer Fish (big fish, KillerFish_*):
 * - stall timer / last x: room x of the previous frame and the frames spent
 *   at it; after 0x3C stalled frames the fish turns around;
 * - effect timer: countdown to the next oscillating swim effect (random
 *   0x78..0xEF frames);
 * - attack x / y: room pixel position sampled as the attack target.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define KILLER_FISH_STALL_TIMER(e) e->unk_4F8.whole
#define KILLER_FISH_LAST_X(e) e->unk_4FC.part16.unk_4FC
#define KILLER_FISH_EFFECT_TIMER(e) e->unk_4FC.part16.unk_4FE[0]
#define KILLER_FISH_ATTACK_X(e) e->unk_500.part16.unk_500
#define KILLER_FISH_ATTACK_Y(e) e->unk_500.part16.unk_502

/*
 * Swarm fish (KillerFishSwarm_*): bite flag, set by the contact handler
 * (KillerFishSwarm_DamagePlayerOnContact) and ending the dart.
 */
#define KILLER_FISH_SWARM_BIT_PLAYER(e) e->unk_500.whole

/*
 * Swarm fish movement (code_08070218.c - TU aliases SCATTER / OWNER_LOST /
 * STEER_TIMER): scatter flag, set when the leader (ENTITY_CREATOR) is gone,
 * and the frames until the next steering change.
 */
#define KILLER_FISH_SWARM_SCATTER(e) e->unk_4FC.part8.unk_4FC
#define KILLER_FISH_SWARM_OWNER_LOST(e) e->unk_4FC.part8.unk_4FD
#define KILLER_FISH_SWARM_STEER_TIMER(e) e->unk_4F8.whole

#endif /* KILLER_FISH_ENTITY_H */
