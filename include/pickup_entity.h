#ifndef PICKUP_ENTITY_H
#define PICKUP_ENTITY_H

/*
 * Pickups (Pickup_Update): bob frame (the low byte drives the sine bob),
 * lifetime (300 frames, then it disappears) and the delay before the player
 * can collect it.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define PICKUP_BOB_FRAME(e) e->unk_4F8.whole
#define PICKUP_BOB_FRAME_LOW(e) e->unk_4F8.part8.unk_4F8
#define PICKUP_LIFETIME(e) e->unk_4FC.part16.unk_4FC
#define PICKUP_DELAY(e) e->unk_4FC.part16.unk_4FE[0]

#endif /* PICKUP_ENTITY_H */
