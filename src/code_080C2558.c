/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   EnemyRipperUpdate = EnemyFleamanUpdate */
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void Ripper_UpdateMovement(struct EwramData_EntityData *entity);
extern void Ripper_UpdateDeath(struct EwramData_EntityData *entity);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *entity);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);

/**
 * @brief C2558 | Same as EnemyFleamanUpdate.
 */
void EnemyRipperUpdate(struct EwramData_EntityData *entity)
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
            Ripper_UpdateMovement(entity);
            break;
        case 0xFF:
            Ripper_UpdateDeath(entity);
            break;
    }
    Entity_AnimateWithBoxes(entity);
    if (entity->state != 0xFF)
        Hit_CheckWithPlayer(collision, entity);
}
