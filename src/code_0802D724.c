/* 2D724. */
#include "types.h"
#include "structs/ewram.h"

/* Remove a red soul entity from the active list (slot unk_4FE). */
void RedSoul_RemoveActiveEntity(struct EwramData_EntityData *soul)
{
    struct EwramData *ewram = gEwramData;

    ewram->playerState.activeRedSoulEntities[soul->unk_4FC.part16.unk_4FE[0]] = NULL;
    ewram->playerState.nbrActiveRedSoulEntities--;
}
