#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern s16 Entity_GetTargetDistance(struct EwramData_EntityData *entity);
extern void WingedSkeleton_ThrowShot(struct EwramData_EntityData *entity);

/**
 * @brief 75078 | Winged Skeleton attack sub-state machine (subState): animation 5
 * (player farther than 0x50) or 6, then on frame 3 of the throw animation
 * spawn the projectile WingedSkeleton_ThrowShot (song 0x85); states 10..12 do the same
 * with animation 7. When the animation ends (animDone) animation 1 is set
 * and the main state returns to 0 with sub-state 4.
 */
void WingedSkeleton_UpdateAttack(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        if (Entity_GetTargetDistance(entity) > 0x50)
        {
            entity->subState = 1;
            Anim_StartIndexed(entity, 5, 0, 0);
            entity->animDone = 0;
        }
        else
        {
            entity->subState = 2;
            Anim_StartIndexed(entity, 6, 0, 0);
            entity->animDone = 0;
        }
        break;
    case 1:
        if (entity->animDone)
        {
            entity->subState = 2;
            Anim_StartIndexed(entity, 6, 0, 0);
            entity->animDone = 0;
        }
        break;
    case 2:
        if (entity->animStep > 2)
        {
            entity->subState++;
            WingedSkeleton_ThrowShot(entity);
            entity->animDone = 0;
            PlaySong(0x85);
        }
        break;
    case 10:
        Anim_StartIndexed(entity, 7, 0, 1);
        entity->subState = 11;
        break;
    case 11:
        if (entity->animStep > 2)
        {
            entity->subState++;
            entity->animDone = 0;
            WingedSkeleton_ThrowShot(entity);
            PlaySong(0x85);
        }
        break;
    case 3:
    case 12:
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->state = 0;
            entity->subState = 4;
        }
        break;
    }
}
