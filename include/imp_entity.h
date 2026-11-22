#ifndef IMP_ENTITY_H
#define IMP_ENTITY_H

/*
 * Imp / Tiny Devil (code_080C47F8.c, code_080C5200.c; their other slots are
 * TU-local IMP_* / TINY_DEVIL_* aliases): slot index of the linked entity -
 * the imp keeps its grab indicator, a death piece keeps the imp's slot.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define IMP_LINK_INDEX(e) e->unk_4F4.part16.unk_4F6

#endif /* IMP_ENTITY_H */
