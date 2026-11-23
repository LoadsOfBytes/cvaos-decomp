#include "code_08009A0.h"
#include "dead_crusader_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *entity);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *entity);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern void DeadCrusader_UpdateWalk(struct EwramData_EntityData *entity);
extern void DeadCrusader_UpdateAttack(struct EwramData_EntityData *entity);
extern s32 Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);

/**
 * @brief 8F1F4 | Dead Crusader update: clears DEAD_CRUSADER_GUARDING, wakes when on screen
 * (Player_TrackEntity; skipped entirely while off screen unless dying), hidden
 * off screen (hidden), Enemy_UpdateHitFlash unless dying, states 0 / 1
 * (DeadCrusader_UpdateWalk / DeadCrusader_UpdateAttack), 2 dying (Enemy_UpdateFragmentDeath), 3 animation only;
 * then Entity_AnimateWithBoxes and Enemy_CheckHitPlayer.
 */
void EnemyDeadCrusaderUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];

    DEAD_CRUSADER_GUARDING(entity) = 0;
    if (entity->state != 2)
    {
        if (EntityIsOutsideScreen(entity, 4))
            return;
        if (Entity_IsInRect(entity, -16, 0, 272, 176))
            Player_TrackEntity(entity);
    }
    if (EntityIsOutsideScreen(entity, 4))
        entity->hidden = 1;
    else
        entity->hidden = 0;
    if (entity->state != 2)
        Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity) && entity->state != 2)
    {
        Hit_CheckWithPlayer(collision, entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        DeadCrusader_UpdateWalk(entity);
        break;
    case 1:
        DeadCrusader_UpdateAttack(entity);
        break;
    case 2:
        Enemy_UpdateFragmentDeath(entity);
        break;
    case 3:
        Anim_Update(entity);
        return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}
