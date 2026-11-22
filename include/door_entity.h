#ifndef DOOR_ENTITY_H
#define DOOR_ENTITY_H

/*
 * Doors (objects 0x02 / 0x04 / 0x06, Door_Draw / Door_SetupGraphics):
 * - open height: 0 = closed .. 0x30 = fully raised (Door_Draw draws the door
 *   that many pixels higher; stepped by 1 or 2 per frame);
 * - on right: the door is in the right half of the screen (x > 0x78), the
 *   direction the player is walked through it;
 * - palette slot.
 */
#define DOOR_OPEN_HEIGHT(e) e->unk_4F8.whole
#define DOOR_ON_RIGHT(e) e->unk_4FC.part8.unk_4FE
#define DOOR_PALETTE(e) e->unk_4FC.part8.unk_4FF

/*
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

#endif /* DOOR_ENTITY_H */
