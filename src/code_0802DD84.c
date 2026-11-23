#include "code_08009A0.h"
#include "player_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

#include "structs/entity_render_flags.h"

static inline void ReleaseAffineSlot(struct EwramData_EntityData *entity)
{
    if (ENTITY_RENDER_FLAGS16(entity) & 0x101F) ObjAffine_Free(entity->affineIndex);
}
/**
 * @brief Release root affine state, clear root slot, delete children one through seven, unload graphics and clear flags; leave root entity deletion to the caller.
 */
void BlueSoul_ReleaseActiveResources(struct EwramData_EntityData *entity)
{
    s32 i;
    ReleaseAffineSlot(entity);
    gEwramData->playerState.activeBlueSoulEntities[0] = NULL;
    for (i = 1; i <= 7; i++)
    {
        struct EwramData_EntityData *child = gEwramData->playerState.activeBlueSoulEntities[i];
        if (child != NULL)
        {
            ReleaseAffineSlot(child);
            EntityDelete(child);
            gEwramData->playerState.activeBlueSoulEntities[i] = NULL;
        }
    }
    if (gEwramData->playerState.blueSoulGfx != -1)
    {
        ObjGfx_Unload(gEwramData->playerState.blueSoulGfx);
        gEwramData->playerState.blueSoulGfx = -1;
    }
    gEwramData->playerStatus.soulStateFlags &= ~0x3F00;
    gEwramData->playerState.player->ext.player.actionFlags &= ~0x200;
}
