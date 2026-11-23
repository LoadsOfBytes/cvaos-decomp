/* Attack phase spawning a projectile. */
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern void SkeletonProjectile_Update(struct EwramData_EntityData *);

/* attack flags: bit 3 alternate animation, bits 1/2 pending shots */
#define FLAGS(e) (e->ext.skeleton.flags)

/**
 * @brief Start animation 3 or 4, create a counted projectile on step 2 and return to state 1 when the animation finishes.
 */
void Skeleton_UpdateThrow(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *e;

    switch (entity->subState)
    {
    case 0:
        entity->subState = 1;
    case 1:
        if (FLAGS(entity) & 8)
        {
            Anim_StartIndexed(entity, 3, 0, 0);
            FLAGS(entity) &= ~8;
        }
        else
            Anim_StartIndexed(entity, 4, 0, 0);
        entity->subState++;
    case 2:
        if (entity->animStep == 2)
        {
            e = Entity_CreateObject((u32 *)SkeletonProjectile_Update);
            if (e != NULL)
            {
                e->ext.skeletonPiece.ownerIndex = ENTITY_INDEX_PAIR(entity);
                if (FLAGS(entity) & 2)
                    e->ext.skeletonBone.kind = 1;
                entity->ext.skeleton.throwCount++;
            }
            entity->subState++;
            if (FLAGS(entity) & 2)
                FLAGS(entity) &= ~2;
            else
                FLAGS(entity) &= ~4;
        }
        break;
    case 3:
        if (entity->animFlags & 4)
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    }
}
