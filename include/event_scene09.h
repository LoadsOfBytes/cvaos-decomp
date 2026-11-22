#ifndef EVENT_SCENE09_H
#define EVENT_SCENE09_H

/*
 * Event scene 9 controller: the two helper entities created from the object
 * table (kinds 5 and 1; actor 5 has an affine sprite that is squashed by the
 * scale x / scale y slots and then hidden), as in event scene 3 the actors
 * are named after their object table kind.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define EVENT09_ACTOR5(e) e->unk_4F8.unk_4F8_entity
#define EVENT09_ACTOR1(e) e->unk_4FC.unk_4FC_entity
#define EVENT09_SCALE_X(e) e->unk_500.whole
#define EVENT09_SCALE_Y(e) e->unk_504.whole

#endif /* EVENT_SCENE09_H */
