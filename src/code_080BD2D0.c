/* Hitbox entity bound to an owner slot. */
#include "code_08009A0.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern void Hit_CheckWithPlayerSharedHistory(void *, struct EwramData_EntityData *, struct EwramData_EntityData *);
extern void DeathFloatingWeaponHitBox_OnHit(void);

/**
 * @brief BD2D0 | Attached hitbox: the owner is entity slot unk_4F8 (byte).
 * Deleted as soon as the owner's HP (unk_518) drops to 0.  State 0 registers
 * box slot 8 with callback DeathFloatingWeaponHitBox_OnHit (Hit_Enable) and sets bit 1 of
 * flags; state 1 follows the owner's position, copies its current box
 * (Hit_GetEntityBoxes) with HitBox_SetBodyRaw and runs the collision check
 * Hit_CheckWithPlayerSharedHistory against the owner.
 */
void DeathFloatingWeaponHitBox_Update(struct EwramData_EntityData *entity)
{
    u32 buf[4];
    struct EwramData_EntityData *owner = &gEwramData->entityData[(u8)entity->unk_4F8.part8.unk_4F8];

    if ((s16)owner->unk_518.part16.hp <= 0)
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        Hit_Enable(entity, 8, NULL, (s32)DeathFloatingWeaponHitBox_OnHit);
        entity->hit.flags |= 2;
        entity->state++;
        break;
    case 1:
        entity->posX.whole = owner->posX.whole;
        entity->posY.whole = owner->posY.whole;
        HitBox_SetBodyRaw(entity, *(s32 *)Hit_GetEntityBoxes(owner));
        Hit_CheckWithPlayerSharedHistory(buf, entity, owner);
        break;
    }
}
