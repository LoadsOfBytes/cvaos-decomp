/* Fleaman_UpdateDeath (Fleaman death). */
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern void Enemy_DropRewards(struct EwramData_EntityData *entity);
extern void Enemy_UpdateFragmentDeathWithSpread(struct EwramData_EntityData *entity, s32 interval, s32 width, s32 height);

/**
 * @brief C2388 | Death: sub-state 0 plays song 0x72, animation 2, palette
 * into 0x53E bits 3-6, timer 50, state 0xFF, no velocity, accelY 0x2000;
 * sub-state 1 decrements unk_4FD of the entity unk_4F6 (counter of the
 * spawner) when unk_4FD is set. Then Enemy_UpdateFragmentDeathWithSpread (fall).
 */
void Fleaman_UpdateDeath(struct EwramData_EntityData *entity)
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
            if (entity->unk_4FC.part8.unk_4FD)
            {
                struct EwramData_EntityData *spawner = &gEwramData->entityData[entity->unk_4F4.part16.unk_4F6];

                spawner->unk_4FC.part8.unk_4FD--;
            }
            entity->subState = 2;
        case 2:
            break;
    }
    Enemy_UpdateFragmentDeathWithSpread(entity, 0xF, 0x80000, 0x60000);
}
