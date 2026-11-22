#ifndef SKY_FISH_ENTITY_H
#define SKY_FISH_ENTITY_H

/*
 * Sky Fish (EnemySkyFishCreate, SkyFish_UpdateFly): darting flag (1 while it
 * flies its fast pass, 0 when stopped by a time stop), start side (0 = left,
 * 1 = right edge) and the random delay before it appears.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define SKY_FISH_DARTING(e) e->unk_4FC.part8.unk_4FC
#define SKY_FISH_FROM_RIGHT(e) e->unk_4FC.part8.unk_4FD
#define SKY_FISH_DELAY(e) e->unk_4FC.part16.unk_4FE[0]

#endif /* SKY_FISH_ENTITY_H */
