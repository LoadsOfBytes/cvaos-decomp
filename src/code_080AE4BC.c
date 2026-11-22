/* Dryad activation and attack selection. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_IsInRect(s32, s32, s32, s32);
extern void Dryad_ThrowSeed(struct EwramData_EntityData *);

void Dryad_UpdateIdle(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        if (Target_IsInRect((s16)(entity->posX.part16.integer - 0x6E),
                        (s16)(entity->posY.part16.integer - 0x28), 0xDC, 0x50))
            entity->subState = 1;
        break;
    case 1:
        entity->flipX = Entity_IsTargetRight(entity);
        if (!(RandomNumberGenerator() & 3))
        {
            entity->state = 1;
            entity->subState = 0;
            entity->stage = 0;
            Anim_StartIndexed(entity, 1, 0, 0);
        }
        break;
    }
}

