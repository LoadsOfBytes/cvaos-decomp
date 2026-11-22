/* Play the Merman attack, emit its projectile and restart movement. */
#include "code/code_08039340.h"
#include "code_080D73B8.h"
extern void Merman_SpawnShot(struct EwramData_EntityData *);
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
static inline void RestartJump(struct EwramData_EntityData *entity)
{
    entity->state = 0;
    entity->subState = 10;
    entity->accelX.whole = 0;
    entity->velY.whole = 0x40000;
    entity->accelY.whole = 0x1800;
    entity->unk_4F8.part8.unk_4F8 = 0;
    entity->unk_4F8.part8.unk_4F9 = 0;
    entity->unk_4F8.part8.unk_4FA = 0;
    Anim_StartIndexed(entity, 2, 0, 1);
    entity->flipX = Entity_IsTargetRight(entity);
    if (entity->flipX) entity->velX.whole = 0x8000;
    else entity->velX.whole = -0x8000;
    entity->timer = 0;
}
static inline void RestartSwim(struct EwramData_EntityData *entity)
{
    entity->state = 0;
    entity->subState = 6;
    entity->accelX.whole = 0;
    entity->velY.whole = 0x20000;
    entity->accelY.whole = 0x1800;
    entity->unk_4F8.part8.unk_4F8 = 0;
    entity->unk_4F8.part8.unk_4F9 = 0;
    entity->unk_4F8.part8.unk_4FA = 0;
    Anim_StartIndexed(entity, 2, 0, 1);
    entity->flipX = Entity_IsTargetRight(entity);
    if (entity->flipX) entity->velX.whole = 0x8000;
    else entity->velX.whole = -0x8000;
    entity->timer = 0;
}
/**
 * @brief Attack state machine (state 1): animation 3 and, from its fourth step,
 * sound 0x7A and Merman_SpawnShot; then back to state 0.
 */
void Merman_UpdateAttack(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 3, 0, 0);
        entity->subState++;
        break;
    case 1:
        if (entity->animStep > 3)
        {
            PlaySong(0x7A);
            Merman_SpawnShot(entity);
            entity->subState++;
        }
        break;
    case 2:
        if (entity->animDone)
        {
            if (entity->unk_514.part16.spawnParam0 == 0)
                RestartJump(entity);
            else
                RestartSwim(entity);
        }
        break;
    }
}
