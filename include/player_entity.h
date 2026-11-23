#ifndef PLAYER_ENTITY_H
#define PLAYER_ENTITY_H

/*
 * The player entity keeps its work slots in ext.player (solid objects in
 * ext.solidObject). Fields outside the work area:
 * - conveyor velocity: 16.16 x velocity added to posX each frame
 *   (+/- gameState.conveyorSpeed on conveyor tiles), cleared after use;
 * - blink timer: contact immunity frames; the sprite blinks (drawSkip) while
 *   it counts down.
 * The macros expand to member chains or to the ENTITY_* views of the common
 * block (structs/ewram.h) without added parentheses; use them with a plain
 * base expression.
 */
#define PLAYER_CONVEYOR_VELOCITY(e) ENTITY_WORD_510(e)
#define PLAYER_BLINK_TIMER(e) e->hp
/* Julius mode: overlay sprite entity created by Julius_CreateOverlaySprite (whip chain owner). */
#define PLAYER_JULIUS_OVERLAY(e) ENTITY_PTR_514(e)

#endif /* PLAYER_ENTITY_H */
