#include "code_08009A0.h"
extern void NumberIndicator_Format(struct EwramData_EntityData *, s32);

/**
 * @brief Read the configured signed memory value and reformat only when it differs from the cached value.
 */
void NumberWatch_Update(struct EwramData_EntityData *entity)
{
    s32 value;
    switch (entity->unk_508.part8.unk_509)
    {
    case 8: value = *(s8 *)entity->unk_50C.whole; break;
    case 16: value = *(s16 *)entity->unk_50C.whole; break;
    default: value = *(s32 *)entity->unk_50C.whole; break;
    }
    if (value != entity->unk_510.whole)
    {
        NumberIndicator_Format(entity, value);
        entity->unk_510.whole = value;
    }
}
