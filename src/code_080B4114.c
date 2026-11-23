/* Affine overlay following its parent. */
#include "code_08009A0.h"
#include "balore_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 BaloreOverlay_SetupSprite(struct EwramData_EntityData *);
extern void BaloreOverlay_UpdateSprite(struct EwramData_EntityData *);

/**
 * @brief B4114 | Overlay that follows its parent unk_4F8.  State 0 sets up
 * the sprite data (BaloreOverlay_SetupSprite) and an affine matrix (affineIndex, flag
 * affine); on failure the entity is deleted.  State 1 runs BaloreOverlay_UpdateSprite,
 * copies the parent's position and palette bits palette.  Bit 2 of the
 * parent's unk_502 forces state 2, which frees the matrix and deletes the
 * entity.  Each living frame the matrix gets the parent's angle unk_500 and
 * scale unk_504 (both axes) with BALORE_OVERLAY_MIRROR.
 */
void BaloreOverlay_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);

    if (BALORE_FLAGS(parent) & 4)
        entity->state = 2;
    switch (entity->state)
    {
    case 0:
        if (BaloreOverlay_SetupSprite(entity) == 0)
        {
            u16 matrix = ObjAffine_Alloc(1);

            if ((s16)matrix >= 0)
            {
                entity->affineIndex = matrix;
                entity->affine = 1;
                entity->state = 1;
                goto active;
            }
        }
        goto delete;
    case 1:
    active:
        BaloreOverlay_UpdateSprite(entity);
        entity->posX.whole = parent->posX.whole;
        entity->posY.whole = parent->posY.whole;
        entity->palette = parent->palette;
        break;
    case 2:
        ObjAffine_Free(entity->affineIndex);
    delete:
        EntityDelete(entity);
        return;
    }
    ObjAffine_Set(BALORE_BODY_ANGLE(parent), BALORE_BODY_SCALE(parent), BALORE_BODY_SCALE(parent),
                 BALORE_OVERLAY_MIRROR(entity), 0, entity->affineIndex);
}
