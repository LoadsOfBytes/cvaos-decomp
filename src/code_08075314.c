#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern s32 Entity_MoveWithPointCollision(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset);
extern s32 Entity_GetVisibilityPattern(s32 step);
extern s32 Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);

/**
 * @brief 75314 | Projectile spawned by WingedSkeleton_ThrowShot: while flying the hit
 * box cycles through three boxes (0x0404FC00 / 0x0404F00C / 0x0404E418)
 * and it moves with Entity_MoveWithPointCollision (0x10000, -0x20000), testing contact
 * (Hit_CheckWithPlayer); on landing (bits 0-2) it lies 60 frames with its box
 * disabled, blinking (Entity_GetVisibilityPattern -> hidden) for the last 40. Deleted
 * off screen or at the end.
 */
void WingedSkeletonShot_Update(struct EwramData_EntityData *entity)
{
    u32 contact[4];
    u32 box0 = 0x0404FC00;
    u32 box1 = 0x0404F00C;
    u32 box2 = 0x0404E418;
    s32 r;
    s8 t;
    u8 m;
    switch (entity->state)
    {
    case 0:
        entity->timer++;
        m = entity->timer % 3;
        if (m == 0)
            HitBox_SetAttack(entity, &box0);
        else if (m == 1)
            HitBox_SetAttack(entity, &box1);
        else
            HitBox_SetAttack(entity, &box2);
        r = Entity_MoveWithPointCollision(entity, 0x10000, -0x20000);
        if (EntityIsOutsideScreen(entity, 1))
            goto delete;
        if (r & 7)
        {
            entity->state = 1;
            entity->subState = 0;
            entity->timer = 0x3C;
            entity->hit.flags |= 4;
            entity->hit.flags |= 2;
            return;
        }
        break;
    case 1:
        t = entity->timer--;
        if (t <= 0x28)
        {
            entity->hidden = Entity_GetVisibilityPattern(entity->timer);
            if ((s8)entity->timer <= 0)
            {
            delete:
                EntityDelete(entity);
                return;
            }
        }
        break;
    }
    if (entity->state == 0)
        Hit_CheckWithPlayer(contact, entity);
    Anim_Update(entity);
}
