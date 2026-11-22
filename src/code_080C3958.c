/* Idle choice between two attacks. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern struct EwramData_EntityData *Entity_CreateLinkedAndHide(struct EwramData_EntityData *, u32 *);
extern void SkeletonAttachedPart_Update(struct EwramData_EntityData *);

/* attack flags, see Skeleton_UpdateThrow */
#define FLAGS(e) ((e)->unk_4FC.part8.unk_4FE)

/**
 * @brief Select preparation animation 5 or 6 and pending attack flags; optionally create an attached part, then return to state 1.
 */
void Skeleton_UpdateAttackPreparation(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        if ((RandomNumberGenerator() & 7) == 0 && (FLAGS(entity) & 1))
        {
            Anim_StartIndexed(entity, 5, 0, 0);
            FLAGS(entity) = (FLAGS(entity) & ~1) | 2;
        }
        else
        {
            Anim_StartIndexed(entity, 6, 0, 0);
            if (Entity_CreateLinkedAndHide(entity, (u32 *)SkeletonAttachedPart_Update) == NULL)
            {
                EntityDelete(entity);
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
                break;
            }
            FLAGS(entity) |= 4;
        }
        entity->subState++;
    case 1:
        if (entity->animFlags & 4)
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    }
}
