#ifndef AFTERIMAGE_ENTITY_H
#define AFTERIMAGE_ENTITY_H

/*
 * Afterimages (Afterimage_Create, Afterimage_Update): owner, heap buffer of
 * the owner's recorded positions (24 entries; as a pointer in the update),
 * start delay, and flags (4 / 0x10 / 0x80 drawing options, 0x20 = end when
 * the owner's hp runs out).
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define AFTERIMAGE_OWNER(e) e->unk_4FC.unk_4FC_entity
#define AFTERIMAGE_HISTORY(e) e->unk_4F8.whole
#define AFTERIMAGE_HISTORY_PTR(e) e->unk_4F8.unk_4F8_entity
#define AFTERIMAGE_DELAY(e) e->unk_500.part16.unk_502
#define AFTERIMAGE_FLAGS(e) e->unk_504.whole

#endif /* AFTERIMAGE_ENTITY_H */
