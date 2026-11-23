/* 2D724. */
#include "types.h"
#include "structs/ewram.h"

/* Remove a red soul entity from the active list (ext.redSoul.slot). */
void RedSoul_RemoveActiveEntity(struct EwramData_EntityData *soul)
{
    struct EwramData *ewram = gEwramData;

    ewram->playerState.activeRedSoulEntities[soul->ext.redSoul.slot] = NULL;
    ewram->playerState.nbrActiveRedSoulEntities--;
}
