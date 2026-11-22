/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Ripper_UpdateDeath = Fleaman_UpdateDeath */
#include "code/code_08039340.h"
#include "altair_entity.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern void Enemy_DropRewards(struct EwramData_EntityData *entity);
extern void Enemy_UpdateFragmentDeathWithSpread(struct EwramData_EntityData *entity, s32 interval, s32 width, s32 height);

/**
 * @brief Death state (0xFF): drop rewards, restore the palette and update
 * the linked spawner count once, then run fragment death.
 */
void Ripper_UpdateDeath(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            PlaySong(0x72);
            Enemy_DropRewards(entity);
            Anim_StartIndexed(entity, 2, 0, 1);
            entity->unk_4F8.part8.unk_4F9 = 2;
            entity->palette = entity->unk_510.part8.basePalette;
            entity->timer = 0x32;
            entity->state = 0xFF;
            entity->velX.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = 0;
            entity->accelY.whole = 0x2000;
            entity->subState = 1;
        case 1:
            if (ALTAIR_DROP_COUNTED(entity))
            {
                struct EwramData_EntityData *spawner = &gEwramData->entityData[ALTAIR_DROP_SPAWNER_INDEX(entity)];

                ALTAIR_DROP_COUNT(spawner)--;
            }
            entity->subState = 2;
        case 2:
            break;
    }
    Enemy_UpdateFragmentDeathWithSpread(entity, 0xF, 0x80000, 0x60000);
}
