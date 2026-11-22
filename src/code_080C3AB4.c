/* Death sequence. */
#include "code/code_08039340.h"
#include "skeleton_entity.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern void Skeleton_SpawnDeathPieces(struct EwramData_EntityData *);
extern void Enemy_DropRewards(struct EwramData_EntityData *);

/**
 * @brief Hide the entity, spawn death pieces and drop rewards; count down the byte timer, then free the affine slot and request deletion.
 */
void Skeleton_UpdateDeath(struct EwramData_EntityData *entity)
{
    entity->hidden = 1;
    switch (entity->subState)
    {
    case 0:
        Skeleton_SpawnDeathPieces(entity);
        Enemy_DropRewards(entity);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->timer = 0xFF;
        entity->state = 0xFF;
        entity->subState++;
    case 1:
        if (--entity->timer == 0xFF)
        {
            ObjAffine_Free((u16)SKELETON_AFFINE(entity));
            entity->deleteRequested = 1;
        }
        break;
    }
}
