/* KillerDoll_SpawnDeathParts. */
#include "code_08009A0.h"
#include "structs/ewram.h"

extern void KillerDollPart_Update(struct EwramData_EntityData *entity);

/**
 * @brief Hide the parent and attempt to create five indexed body parts; store the parent slot and increment its live-part count for each success.
 */
void KillerDoll_SpawnDeathParts(struct EwramData_EntityData *entity)
{
    s32 i;

    entity->hidden = 1;
    for (i = 0; i < 5; i++)
    {
        struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)KillerDollPart_Update);

        if (child != NULL)
        {
            child->unk_4F4.part8.unk_4F5 = entity->unk_4F4.part8.unk_4F4;
            child->unk_500.part8.unk_501 = i;
            entity->unk_500.part8.unk_500++;
            child->spriteLayer = entity->spriteLayer;
        }
    }
}
