/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   LightningDoll_SpawnDeathParts = KillerDoll_SpawnDeathParts */
#include "code_08009A0.h"
#include "doll_entity.h"
#include "structs/ewram.h"

extern void LightningDollPart_Update(struct EwramData_EntityData *entity);

/**
 * @brief Hide the parent and attempt to create five indexed body parts; store the parent slot and increment its live-part count for each success.
 */
void LightningDoll_SpawnDeathParts(struct EwramData_EntityData *entity)
{
    s32 i;

    entity->hidden = 1;
    for (i = 0; i < 5; i++)
    {
        struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)LightningDollPart_Update);

        if (child != NULL)
        {
            ENTITY_PARENT_INDEX(child) = ENTITY_INDEX(entity);
            DOLL_PART_INDEX(child) = i;
            DOLL_DEATH_PART_COUNT(entity)++;
            child->spriteLayer = entity->spriteLayer;
        }
    }
}
