/* Falling-away death of a linked part. */
#include "code/code_08039340.h"
#include "nemesis_entity.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern void Nemesis_DeleteAfterimages(struct EwramData_EntityData *);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);

/**
 * @brief Delete afterimages, fall and emit effects; free the affine slot and request deletion of self and partner at the Y or timer limit.
 */
void Nemesis_UpdateDeath(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        PlaySong(0x72);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->velY.whole = 0x8000;
        entity->accelY.whole = 0x800;
        entity->timer = 0;
        entity->subState++;
        Nemesis_DeleteAfterimages(entity);
    case 1:
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += entity->accelY.whole;
        if ((s16)entity->posY.part16.integer > 0xAA || entity->timer++ > 0x40)
        {
            ObjAffine_Free(entity->affineIndex);
            entity->deleteRequested = 1;
            (gEwramData->entityData + NEMESIS_PARTNER_INDEX(entity))->deleteRequested = 1;
        }
        break;
    }
    if ((u8)((u32)entity->timer % 7) == 0)
        Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0x100000, 0x100000, entity->spriteLayer - 1);
    entity->hidden = entity->timer % 2;
}
