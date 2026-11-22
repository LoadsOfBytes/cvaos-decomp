#include "code_08009A0.h"
#include "manticore_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern s32 Enemy_IsStunned(struct EwramData_EntityData *entity);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *entity);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *entity);
extern void Manticore_UpdateWalk(struct EwramData_EntityData *entity);
extern void Manticore_UpdateAttack(struct EwramData_EntityData *entity);
extern s32 Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);

/**
 * @brief 72768 | Manticore update (pattern of EnemyKillerFishUpdate): wake
 * when on screen (Player_TrackEntity), hidden (hidden) more than 0x20 px above
 * or 0xF0 px below the screen, states 0 / 1 (Manticore_UpdateWalk / Manticore_UpdateAttack),
 * 2 dying (Enemy_UpdateFragmentDeath), 3 animation only; contact tests for itself and
 * its tail part (unk_4F8) except while dying.
 */
void EnemyManticoreUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[48];

    if (entity->state != 2 && Entity_IsInRect(entity, -16, 0, 272, 176))
        Player_TrackEntity(entity);
    if ((u16)(entity->posY.part16.integer + 0x20) > 0x110)
        entity->hidden = 1;
    else
        entity->hidden = 0;
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 2)
        {
            Hit_CheckWithPlayer(collision, entity);
            Hit_CheckWithPlayer(collision, MANTICORE_TAIL_TIP(entity));
        }
        return;
    }
    switch (entity->state)
    {
    case 0:
        Manticore_UpdateWalk(entity);
        break;
    case 1:
        Manticore_UpdateAttack(entity);
        break;
    case 2:
        Enemy_UpdateFragmentDeath(entity);
        break;
    case 3:
        Anim_Update(entity);
        return;
    }
    {
        u8 *frame = &entity->animFrame;
        u8 previous = *frame;
        Anim_Update(entity);
        if (previous != *frame)
            Entity_UpdateBoxes(entity);
    }
    if (entity->state != 2)
    {
        Hit_CheckWithPlayer(collision, entity);
        Hit_CheckWithPlayer(collision, MANTICORE_TAIL_TIP(entity));
    }
}
