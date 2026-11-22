/* 2D724. */
#include "types.h"
#include "red_soul_entity.h"
#include "structs/ewram.h"

/* Remove a red soul entity from the active list (RED_SOUL_SLOT). */
void RedSoul_RemoveActiveEntity(struct EwramData_EntityData *soul)
{
    struct EwramData *ewram = gEwramData;

    ewram->playerState.activeRedSoulEntities[RED_SOUL_SLOT(soul)] = NULL;
    ewram->playerState.nbrActiveRedSoulEntities--;
}
