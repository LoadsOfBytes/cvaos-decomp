/* Convert a global item id to its type/index before updating the list. */
#include "types.h"

extern s32 Item_GetTypeByGlobalId(s32);
extern s32 Item_GetIndexByGlobalId(s32);
extern void EquipmentList_Insert(s32, s32);

void EquipmentList_InsertGlobalItem(s32 item)
{
    s32 type = Item_GetTypeByGlobalId(item);
    s32 index = Item_GetIndexByGlobalId(item);
    EquipmentList_Insert(type, index);
}
