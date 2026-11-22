#include "code_08009A0.h"

/*
 * Number watch (NumberWatch_Update): width of the watched value in bits
 * (8 / 16 / 32), its address and the last shown value. Token-identical
 * expansion.
 */
#define NUMBER_WATCH_BITS(e) e->unk_508.part8.unk_509
#define NUMBER_WATCH_ADDRESS(e) e->unk_50C.whole
#define NUMBER_WATCH_LAST(e) e->unk_510.whole
extern void NumberIndicator_Format(struct EwramData_EntityData *, s32);

/**
 * @brief Read the configured signed memory value and reformat only when it differs from the cached value.
 */
void NumberWatch_Update(struct EwramData_EntityData *entity)
{
    s32 value;
    switch (NUMBER_WATCH_BITS(entity))
    {
    case 8: value = *(s8 *)NUMBER_WATCH_ADDRESS(entity); break;
    case 16: value = *(s16 *)NUMBER_WATCH_ADDRESS(entity); break;
    default: value = *(s32 *)NUMBER_WATCH_ADDRESS(entity); break;
    }
    if (value != NUMBER_WATCH_LAST(entity))
    {
        NumberIndicator_Format(entity, value);
        NUMBER_WATCH_LAST(entity) = value;
    }
}
