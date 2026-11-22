/* EnemyFleamanUpdate. */
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *entity);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern void Fleaman_UpdateMovement(struct EwramData_EntityData *entity);
extern void Fleaman_UpdateDeath(struct EwramData_EntityData *entity);

/**
 * @brief C1F1C | Fleaman update. State 3: start the animation once, then
 * only animate. Otherwise (Enemy_UpdateHitFlash unless state 0xFF) skip the state
 * and only run Hit_CheckWithPlayer while Entity_IsTimeStopped says so or unk_511 is set
 * (not in states 2 / 0xFF); states 1 Fleaman_UpdateMovement, 0xFF Fleaman_UpdateDeath.
 */
void EnemyFleamanUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[0x10];

    if (entity->state == 3)
    {
        switch (entity->subState)
        {
            case 0:
                Anim_StartIndexed(entity, 0, 0, 1);
                entity->subState++;
            case 1:
                Anim_Update(entity);
                break;
        }
        return;
    }
    if (entity->state != 0xFF)
        Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity) && entity->state != 0xFF)
    {
        Hit_CheckWithPlayer(collision, entity);
        return;
    }
    if (entity->unk_510.part8.hitFlashTimer && entity->state != 2 && entity->state != 0xFF)
    {
        Hit_CheckWithPlayer(collision, entity);
        return;
    }
    switch (entity->state)
    {
        case 1:
            Fleaman_UpdateMovement(entity);
            break;
        case 0xFF:
            Fleaman_UpdateDeath(entity);
            break;
    }
    Entity_AnimateWithBoxes(entity);
    if (entity->state != 0xFF)
        Hit_CheckWithPlayer(collision, entity);
}
