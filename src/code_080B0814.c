#include "code_08009A0.h"
#include "structs/ewram.h"
extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern s32 Minotaur_CheckAttackChance(struct EwramData_EntityData *, s32);

/* Minotaur idle: choose approach or attack using two distance windows. */
void Minotaur_UpdateIdle(struct EwramData_EntityData *entity)
{
    Entity_FaceTarget(entity);
    if (entity->animTimer == 0)
        Entity_FaceTarget(entity);
    if (Target_IsNearEntity(entity, 200, 120) || Enemy_IsDamaged(entity))
    {
        s32 result = Minotaur_CheckAttackChance(entity, 1);
        if (result == 0)
        {
            entity->state = 2;
            entity->stage = result;
            entity->subState = result;
        }
        else if (entity->unk_4FC.whole & 2)
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
        }
    }
    else if (Target_IsNearEntity(entity, 280, 120) || Enemy_IsDamaged(entity))
    {
        s32 result = Minotaur_CheckAttackChance(entity, 63);
        if (result == 0)
        {
            entity->state = 2;
            entity->stage = result;
            entity->subState = result;
        }
    }
}
