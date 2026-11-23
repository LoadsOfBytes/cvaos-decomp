/* Bael-fight helper functions 0x080CC3C8..0x080CC623. */
#include "code_08009A0.h"
#include "wheel_enemy_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void BuerChargeEffect_Update(struct EwramData_EntityData *entity);
extern void Buer_MoveAndRotate(struct EwramData_EntityData *entity, s32 mode);
extern s32 Entity_IsOffsetOutsideRoom(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY);
extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);
extern s32 Entity_IsOutsideRoomMap(struct EwramData_EntityData *entity);

s32 Buer_CheckCleanup(struct EwramData_EntityData *entity);

/**
 * @brief While state is 0, copy the linked body position. Death advances the state so the head can shake independently.
 */
void BuerHead_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *body = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];

    if (entity->state == 0)
    {
        entity->posX.whole = body->posX.whole;
        entity->posY.whole = body->posY.whole;
    }
}

/**
 * @brief Allocate and initialize an affine slot; delete the entity on allocation failure.
 */
void Buer_AllocateAffine(struct EwramData_EntityData *entity)
{
    u16 affineIndex = ObjAffine_Alloc(1);

    if ((s16)affineIndex < 0)
    {
        EntityDelete(entity);
        return;
    }
    entity->affineIndex = affineIndex;
    entity->affine = 1;
    ObjAffine_Set(0x10000, 0x10000, 0x10000, 0, 0, entity->affineIndex);
}

/**
 * @brief Check cleanup, move with mode 1 and enter active state 2 when the downward probe at offset 92 succeeds.
 */
void Buer_UpdateEntry(struct EwramData_EntityData *entity)
{
    s32 cleanedUp = Buer_CheckCleanup(entity);

    if (cleanedUp == 0)
    {
        Buer_MoveAndRotate(entity, 1);
        if (Entity_IsOffsetOutsideRoom(entity, 0, 0x5C))
        {
            entity->state = 2;
            entity->stage = cleanedUp;
            entity->subState = cleanedUp;
        }
    }
}

/**
 * @brief Move upward 32 pixels once and rotate frame 0 by 0x100 per tick. The gameplay purpose of state 4 remains unresolved.
 */
void Buer_UpdateState4Rotation(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            entity->posY.part16.integer -= 0x20;
            entity->animFrame = 0;
            entity->subState++;
        case 1:
            WHEEL_ANGLE(entity) += 0x100;
            ObjAffine_Set(WHEEL_ANGLE(entity), 0x10000, 0x10000, 0, 0, entity->affineIndex);
            break;
    }
}

/**
 * @brief CC4E4 | Player_ApplyContactDamage with (attack, 1, 0, 1) at the entity position.
 */
void Buer_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 1;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief Apply damage; on death drop rewards and enter state 3. A hit in state 2/substate 0 advances to substate 1.
 */
void Buer_OnHit(struct EwramData_EntityData *entity)
{
    if (Enemy_TakeDamage(entity))
    {
        Enemy_DropRewards(entity);
        entity->state = 3;
        entity->stage = 0;
        entity->subState = 0;
    }
    // The original reads state and subState as one halfword here.
    if (entity->state != 1 && *(u16 *)&entity->state == 2)
    {
        entity->subState = 1;
        entity->stage = 0;
    }
}

/**
 * @brief Add the supplied step when the value is positive; subtract it when the value is zero or negative.
 */
s32 Buer_OffsetBySign(s32 value, s32 offset)
{
    if (value > 0)
        value += offset;
    else
        value -= offset;
    return value;
}

/**
 * @brief When Entity_IsOutsideRoomMap succeeds, free the affine slot and delete the body and linked head; return 1, otherwise 0.
 */
s32 Buer_CheckCleanup(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *head = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 deleted = 0;

    if (Entity_IsOutsideRoomMap(entity))
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
        EntityDelete(head);
        entity->state = 3;
        entity->stage = deleted;
        entity->subState = deleted;
        deleted = 1;
    }
    return deleted;
}

/**
 * @brief Probe 24 pixels along the signed movement direction; on success restore the supplied fixed-point X coordinate.
 */
void Buer_RestoreXAtProbe(struct EwramData_EntityData *entity, s32 savedX)
{
    s32 probeX = 0x18;

    if ((s32)WHEEL_SPEED(entity) < 0)
        probeX = -probeX;
    if (Entity_IsOffsetOutsideRoom(entity, probeX, 0))
        entity->posX.whole = savedX;
}

/**
 * @brief Create a charge effect at pixel coordinates with the supplied animation index and inverted facing flag.
 */
void Buer_SpawnChargeEffect(u16 x, u16 y, u8 kind, u32 flip)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)BuerChargeEffect_Update);

    if (child != NULL)
    {
        WHEEL_EFFECT_ANIM(child) = kind;
        child->posX.part16.integer = x;
        child->posY.part16.integer = y;
        child->flipX = flip ^ 1;
    }
}
