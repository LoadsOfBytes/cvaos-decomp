#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_Walk(struct EwramData_EntityData *, s32, void *, s32);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);
extern u8 sMinotaurWalkProfile[];

/* Minotaur approach animation, movement, then close-range attack. */
void Minotaur_UpdateApproach(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->subState = 1;
        break;
    case 1:
        if (entity->animFlags & 4)
        {
            Anim_StartIndexed(entity, 3, 0, 1);
            entity->subState = 2;
        }
        break;
    case 2:
        Entity_Walk(entity, 0x8000, sMinotaurWalkProfile, 13);
        if (Target_IsNearEntity(entity, 100, 80))
        {
            Anim_StartIndexed(entity, 4, 0, 0);
            entity->subState = 3;
        }
        break;
    case 3:
        if (*(u16 *)&entity->animStep == 3)
            PlaySong(0x86);
        if (entity->animFlags & 4)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->state = 0;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 4:
        break;
    }
}
