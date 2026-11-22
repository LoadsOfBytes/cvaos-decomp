#include "macros.h"
/* Dryad activation and attack selection. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_IsInRect(s32, s32, s32, s32);
extern void Dryad_ThrowSeed(struct EwramData_EntityData *);

void Dryad_UpdateAttack(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        if (entity->animFlags & 4)
        {
            Anim_StartIndexed(entity, 2, 0, 1);
            entity->subState = 1;
        }
        break;
    case 1:
        if (!Entity_IsInRect(entity, -0x80, -0x80, 0x1F0, 0x1A0))
        {
            u8 *base = (u8 *)gEwramData;
            u32 event = entity->unk_518.part8.spawnIndex;
            s32 index = event >> 5;
            s32 *word;
            index <<= 2;
            base += OFFSET_OF(struct EwramData, gameState.roomEntitySpawn.spawnedFlags);
            word = (s32 *)(base + index);
            {
                s32 bit = 31;
                bit &= event;
                *word &= ~(1 << bit);
            }
            entity->deleteRequested = 1;
        }
        else
        {
            entity->flipX = Entity_IsTargetRight(entity);
            if (!(RandomNumberGenerator() & 7))
            {
                Anim_StartIndexed(entity, 3, 0, 0);
                entity->timer = 0x12;
                entity->subState = 2;
            }
        }
        break;
    case 2:
        if (entity->timer != 0)
        {
            if (--entity->timer <= 2)
            {
                PlaySong(0x89);
                Dryad_ThrowSeed(entity);
            }
        }
        if (entity->animFlags & 4) entity->subState = 1;
        break;
    }
}
