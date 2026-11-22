#ifndef PLAYER_ENTITY_H
#define PLAYER_ENTITY_H

/*
 * Work slots of the player entity (entity slot 0, Soma and Julius code):
 * - anim palette: palette index last loaded for the current animation
 *   (sSomaAnimPalettes[anim] is compared with it before ObjPalette_LoadRange);
 * - invulnerable timer: nonzero blocks Player_ApplyContactDamage; souls and
 *   damage set it, Soma_Update/Julius_Update count it down;
 * - floor kind: 0xFF = none yet this frame, else the floor/slope kind found by
 *   the collision probes; reset to 0xFF after use;
 * - conveyor velocity: 16.16 x velocity added to posX each frame
 *   (+/- gameState.conveyorSpeed on conveyor tiles), cleared after use.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define PLAYER_ANIM_PALETTE(e) e->unk_508.part8.unk_50A
#define PLAYER_INVULNERABLE_TIMER(e) e->unk_500.part8.unk_502
#define PLAYER_FLOOR_KIND(e) e->unk_50C.part8.unk_50D
#define PLAYER_CONVEYOR_VELOCITY(e) e->unk_510.whole

#endif /* PLAYER_ENTITY_H */
