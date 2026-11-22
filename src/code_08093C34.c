#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern s32 Enemy_IsStunned(struct EwramData_EntityData *entity);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *entity);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *entity);
extern void Succubus_UpdateWalk(struct EwramData_EntityData *entity);
extern void Succubus_UpdateAttack(struct EwramData_EntityData *entity);
extern s32 Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);

/**
 * @brief 93C34 | Succubus update (as EnemyDevilUpdate): skipped while off
 * screen unless dying, hidden off screen (hidden), states 0 / 1
 * (Succubus_UpdateWalk / Succubus_UpdateAttack), 2 dying, 3 animation only; contact test
 * unless dying or unk_4FF is set. The "previous frame" compared before
 * Entity_UpdateBoxes is uninitialised in the original.
 */
void EnemySuccubusUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    u8 previous;

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
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 2 && (s8)entity->unk_4FC.part8.unk_4FF == 0)
            Hit_CheckWithPlayer(collision, entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        Succubus_UpdateWalk(entity);
        break;
    case 1:
        Succubus_UpdateAttack(entity);
        break;
    case 2:
        Enemy_UpdateFragmentDeath(entity);
        break;
    case 3:
        Anim_Update(entity);
        return;
    }
    Anim_Update(entity);
    if (previous != entity->animFrame)
        Entity_UpdateBoxes(entity);
    if (entity->state != 2 && (s8)entity->unk_4FC.part8.unk_4FF == 0)
        Hit_CheckWithPlayer(collision, entity);
}
