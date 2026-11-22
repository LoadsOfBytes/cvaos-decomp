/* MenuSlotIcon_SetHidden (matches by widening the value first). */
#include "code_08009A0.h"

/**
 * @brief Set the hidden flag of slot icon entity 0xD0 + index to bit 0 of value.
 */
void MenuSlotIcon_SetHidden(u8 value, s32 index)
{
    struct EwramData_EntityData *e;
    s32 v = value;
    e = &gEwramData->entityData[index];
    e[0xD0].hidden = v;
}
