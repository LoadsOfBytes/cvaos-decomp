#include "code_08009A0.h"
#include "gate_entity.h"
#include "code_08001194.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief 4DD84 | Door step: once roomActive is clear, set deleteRequested and free
 * both affine slots (unk_4FC / unk_4FD). Otherwise while flag unk_514 of
 * the flag words at gameFlagsTail is set open (unk_4F8 += 0x400 up to 0x4000,
 * Collision_ClearBlock on the three tiles above it), else close (unk_4F8 -= 0x400
 * down to 0, Collision_SetBlock on the tiles).
 */
void Gate_UpdateFlagControlled(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg;
    s32 x, y, px, py;

    px = (s16)entity->posX.part16.integer;
    bg = &gEwramData->bgInfo[1];
    x = px + (s16)bg->xPos.part16.integer;
    py = (s16)entity->posY.part16.integer;
    y = py + (s16)bg->yPos.part16.integer;
    if (!entity->roomActive)
    {
        entity->deleteRequested = 1;
        ObjAffine_Free((u8)GATE_AFFINE_0(entity));
        ObjAffine_Free(GATE_AFFINE_1(entity));
        return;
    }
    if (gEwramData->gameState.gameFlagsTail[entity->unk_514.part16.spawnParam0 >> 5] & (1 << (entity->unk_514.part16.spawnParam0 & 0x1F)))
    {
        GATE_OPEN_ANGLE(entity) += 0x400;
        if (GATE_OPEN_ANGLE(entity) > 0x4000)
            GATE_OPEN_ANGLE(entity) = 0x4000;
        Collision_ClearBlock(x, y - 8);
        Collision_ClearBlock(x, y - 0x18);
        Collision_ClearBlock(x, y - 0x28);
    }
    else
    {
        GATE_OPEN_ANGLE(entity) -= 0x400;
        if (GATE_OPEN_ANGLE(entity) < 0)
            GATE_OPEN_ANGLE(entity) = 0;
        Collision_SetBlock(x, y - 8);
        Collision_SetBlock(x, y - 0x18);
        Collision_SetBlock(x, y - 0x28);
    }
}
