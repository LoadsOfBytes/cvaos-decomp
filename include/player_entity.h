#ifndef PLAYER_ENTITY_H
#define PLAYER_ENTITY_H

/*
 * The player entity keeps its work slots in ext.player (solid objects in
 * ext.solidObject). Fields outside the work area:
 * - conveyor velocity: 16.16 x velocity added to posX each frame
 *   (+/- gameState.conveyorSpeed on conveyor tiles), cleared after use;
 * - blink timer: contact immunity frames; the sprite blinks (drawSkip) while
 *   it counts down.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define PLAYER_CONVEYOR_VELOCITY(e) e->unk_510.whole
#define PLAYER_BLINK_TIMER(e) e->unk_518.part16.hp

#endif /* PLAYER_ENTITY_H */
