/* Resolve the record family and stride for a one-based item identifier. */
#include "types.h"
extern u8 sConsumableInfo[], sUnk_085059BC[], sUnk_08505C94[];
/**
 * @brief Resolve a one-based global item identifier to its record family and stride; return NULL for zero or above the final range, preserving signed comparisons.
 */
void *Item_GetInfoByGlobalId(s32 id)
{
    void *record = NULL;
    if (id == 0) return NULL;
    else
    {
        id--;
        if (id <= 31) record = sConsumableInfo + id * 16;
        else if (id <= 90) record = sUnk_085059BC + id * 28;
        else if (id <= 135) record = sUnk_08505C94 + id * 20;
    }
    return record;
}
