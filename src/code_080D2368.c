/* Attached weapon stays inert until its parent starts the active phase. */
#include "structs/ewram.h"
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
/**
 * @brief Remain inert in state 0. Once the owner activates this entity,
 * show the weapon, animate with hit boxes and check player contact.
 */
void BelmontAttachedWeapon_Update(struct EwramData_EntityData *entity)
{
    if(entity->state!=0)
    {
        entity->hidden=0;
        Entity_AnimateWithBoxes(entity);
        Enemy_CheckHitPlayer(entity);
    }
}
