/* Dispatch Killer Fish states and refresh its frame-dependent collision data. */
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code/code_080211F0.h"
extern s32 Enemy_IsStunned(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void KillerFish_UpdateSwimming(struct EwramData_EntityData *);
extern void KillerFish_UpdateAttack(struct EwramData_EntityData *);
extern void KillerFish_UpdateDeath(struct EwramData_EntityData *);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *);
void EnemyKillerFishUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    if (entity->state != 2)
    {
        if (EntityIsOutsideScreen(entity, 4)) goto finished;
        if (Entity_IsInRect(entity, -16, 0, 272, 176))
            Player_TrackEntity(entity);
    }
    if (EntityIsOutsideScreen(entity, 4)) entity->hidden = 1;
    else entity->hidden = 0;
    if (Enemy_IsStunned(entity))
    {
        if ((u8)(entity->state - 2) > 1)
            Hit_CheckWithPlayer(collision, entity);
        goto finished;
    }
    switch (entity->state)
    {
    case 0: KillerFish_UpdateSwimming(entity); break;
    case 1: KillerFish_UpdateAttack(entity); break;
    case 2: KillerFish_UpdateDeath(entity); break;
    case 3: Enemy_UpdateFragmentDeath(entity); break;
    case 4: entity->deleteRequested = 1; goto finished;
    case 5: Anim_Update(entity); goto finished;
    }
    {
        u8 *frame = &entity->animFrame;
        u8 previous = *frame;
        if ((u8)(entity->state - 2) > 1)
            Anim_Update(entity);
        if (previous != *frame)
            Entity_UpdateBoxes(entity);
    }
    if ((u8)(entity->state - 2) > 1)
        Hit_CheckWithPlayer(collision, entity);
finished:
    return;
}

