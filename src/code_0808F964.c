#include "code_08009A0.h"
#include "devil_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern s32 Enemy_IsStunned(struct EwramData_EntityData *entity);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *entity);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *entity);
extern void Devil_UpdateApproach(struct EwramData_EntityData *entity);
extern void Devil_UpdateAttack(struct EwramData_EntityData *entity);
extern void Devil_UpdateHover(struct EwramData_EntityData *entity);
extern s32 Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);

/**
 * @brief 8F964 | Devil update (pattern of EnemyKillerFishUpdate): wake when
 * on screen, states 0 / 1 (Devil_UpdateApproach / Devil_UpdateAttack), 2 dying, 3
 * (Devil_UpdateHover + animation); contact test unless dying or unk_50A is set.
 * The original compares the frame against an uninitialised "previous
 * frame" register before calling Entity_UpdateBoxes.
 */
void EnemyDevilUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    u8 previous;

    if (entity->state != 2 && Entity_IsInRect(entity, -16, 0, 272, 176))
        Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 2 && (s8)DEVIL_INTANGIBLE(entity) == 0)
            Hit_CheckWithPlayer(collision, entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        Devil_UpdateApproach(entity);
        break;
    case 1:
        Devil_UpdateAttack(entity);
        break;
    case 2:
        Enemy_UpdateFragmentDeath(entity);
        break;
    case 3:
        Devil_UpdateHover(entity);
        Anim_Update(entity);
        return;
    }
    Anim_Update(entity);
    if (previous != entity->animFrame)
        Entity_UpdateBoxes(entity);
    if (entity->state != 2 && (s8)DEVIL_INTANGIBLE(entity) == 0)
        Hit_CheckWithPlayer(collision, entity);
}
