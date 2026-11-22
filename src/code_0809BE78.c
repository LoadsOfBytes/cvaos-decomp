#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/*
 * Stolas cast effect (StolasCastEffect_Update): its owner and the affine
 * angle (turns to +-0x10000). Token-identical expansion.
 */
#define STOLAS_CAST_OWNER(e) e->unk_500.unk_500_entity
#define STOLAS_CAST_ANGLE(e) e->unk_4F8.whole

/**
 * @brief 9BE78 | Effect created by Stolas_SpawnCastEffect: freed (palette and
 * entity) when its owner is gone or no longer entity 0x61; copies the
 * owner's blending 4 or uses its own palette index; after 15 frames the
 * affine angle unk_4F8 turns by 0x800 per frame up to +-0x10000 depending
 * on the facing; freed after 60 frames or when the owner dies; affine
 * matrix from the angle.
 */
void StolasCastEffect_Update(struct EwramData_EntityData *entity)
{
    u8 unused[16];
    struct EwramData_EntityData *owner = STOLAS_CAST_OWNER(entity);

    if (owner == NULL || owner->updateFunc == NULL || owner->unk_518.part8.enemyId != 0x61)
        goto free;
    if (entity->state == 0)
    {
        if (owner->palette == 4)
            entity->palette = 4;
        else
            entity->palette = entity->unk_510.part8.basePalette;
        if (entity->timer > 0xE)
        {
            if (entity->flipX)
            {
                if (STOLAS_CAST_ANGLE(entity) <= 0xFFFF)
                    STOLAS_CAST_ANGLE(entity) += 0x800;
            }
            else if (STOLAS_CAST_ANGLE(entity) > -0x10000)
                STOLAS_CAST_ANGLE(entity) += -0x800;
        }
        if (entity->timer++ > 0x3B || owner->state == 2)
        {
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
        }
    }
    ObjAffine_Set(STOLAS_CAST_ANGLE(entity), 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    return;
free:
    ObjAffine_Free(entity->affineIndex);
    EntityDelete(entity);
}
