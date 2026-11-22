#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Entity_IsTargetRight(struct EwramData_EntityData *entity);

/**
 * @brief 8EFD8 | Enemy attack sub-state (file 0x0808ECF8): faces the
 * player and plays animation 2, after 44 frames animation 3, then
 * animation 4 with song 0x85 on frame 11, after 30 frames animation 3 for
 * 20 more frames; then back to state 0 (animation 1, velY 0x10000, accelY
 * 0x2800).
 */
void SkeletonKnight_UpdateAttack(struct EwramData_EntityData *entity)
{
    u8 sub = entity->subState;

    switch (sub)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->subState++;
        entity->timer = sub;
        break;
    case 1:
        if (entity->timer++ > 0x2B)
        {
            Anim_StartIndexed(entity, 3, 0, 0);
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 2:
        if (entity->timer == 0xB)
            PlaySong(0x85);
        if (entity->timer++ > 0x1D)
        {
            Anim_StartIndexed(entity, 4, 0, 0);
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 3:
        if (entity->timer++ > 0x13)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->state = 0;
            entity->subState = 1;
            entity->timer = 0;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
        }
        break;
    }
}
