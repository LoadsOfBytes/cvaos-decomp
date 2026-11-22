#ifndef YELLOW_SOUL_ENTITY_H
#define YELLOW_SOUL_ENTITY_H

/*
 * Work slots of yellow soul entities (created from YellowSoulInfo.updateFunc
 * by YellowSoul_CreateEquippedEntity / YellowSoul_Restart; one at a time in
 * playerState.activeYellowSoulEntity):
 * - owner: the player;
 * - param / value: YellowSoulInfo.effectParam / effectValue (which stat or
 *   flag the soul changes and by how much).
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define YELLOW_SOUL_OWNER(e) e->unk_4F8.unk_4F8_entity
#define YELLOW_SOUL_PARAM(e) e->unk_4FC.part16.unk_4FE[0]
#define YELLOW_SOUL_VALUE(e) e->unk_500.whole

#endif /* YELLOW_SOUL_ENTITY_H */
