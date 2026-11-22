#include "code_08009A0.h"
#include "une_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern s32 Enemy_IsStunned(struct EwramData_EntityData *entity);
extern s32 Entity_IsOutsideRoomMap(struct EwramData_EntityData *entity);
extern s32 Entity_GetVisibilityPattern(s32 step);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern void Une_UpdateGrow(struct EwramData_EntityData *entity);
extern void Une_UpdateIdle(struct EwramData_EntityData *entity);
extern s32 Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);

/**
 * @brief 74130 | Une update: unless state 3 (waiting for its parent), the
 * on-screen test (except while dying), hidden off screen (hidden) and
 * Entity_IsOutsideRoomMap starting the dying state 2; states 0 / 1 (Une_UpdateGrow /
 * Une_UpdateIdle), 2 blinks one frame then sets deleteRequested and decrements the
 * count of the parent (unk_51A 0x3D, counter unk_4FC), 3 finishes once its
 * own counter unk_4FC is zero, 4 animation only.
 */
void EnemyUneUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct EwramData_EntityData *parent;

    if (entity->state != 3)
    {
        if (entity->state != 2 && Entity_IsInRect(entity, -16, 0, 272, 176))
            Player_TrackEntity(entity);
        if (EntityIsOutsideScreen(entity, 4))
            entity->hidden = 1;
        else
            entity->hidden = 0;
        if (Entity_IsOutsideRoomMap(entity))
        {
            entity->state = 2;
            entity->timer = 1;
        }
    }
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 2)
            Hit_CheckWithPlayer(collision, entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        Une_UpdateGrow(entity);
        break;
    case 1:
        Une_UpdateIdle(entity);
        break;
    case 2:
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0)
            entity->deleteRequested = 1;
        if (entity->deleteRequested)
        {
            parent = UNE_SPAWNER(entity);
            if (parent != NULL && parent->updateFunc != NULL && parent->unk_518.part8.enemyId == 0x3D
                && UNE_COUNT(parent) != 0)
                UNE_COUNT(parent)--;
        }
        break;
    case 3:
        if (UNE_COUNT(entity) == 0)
            entity->deleteRequested = 1;
        break;
    case 4:
        Anim_Update(entity);
        return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}
