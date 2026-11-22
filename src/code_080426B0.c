/* Entity box-slot helpers 0x080426B0..0x0804291F (no upstream C existed). */
#include "code/code_08040A38.h"
#include "code_08009A0.h"
#include "structs/entity_boxes.h"
#include "gba.h"

/**
 * @brief 426B0 | Count down the three hit timers of an entity
 *
 * A recent hit (hitBy[i]) is forgotten when its timer reaches zero. Only
 * while the entity's hit block is active (flags bit 0).
 *
 * @param entity Entity
 */
void Hit_UpdateTimers(struct EwramData_EntityData *entity)
{
    struct EntityHit *hit = &entity->hit;

    if (hit->flags & 1)
    {
        if (hit->hitTimer[0] != 0)
        {
            if (--hit->hitTimer[0] == 0)
                hit->hitBy[0] = 0;
        }
        if (hit->hitTimer[1] != 0)
        {
            if (--hit->hitTimer[1] == 0)
                hit->hitBy[1] = 0;
        }
        if (hit->hitTimer[2] != 0)
        {
            if (--hit->hitTimer[2] == 0)
                hit->hitBy[2] = 0;
        }
    }
}

/**
 * @brief 42700 | Record an attack type in hitBy[] (or restart its entry)
 *
 * @param entity Entity that was hit
 * @param type Attack type
 * @return 1 on success, 0 when all three entries hold other types
 */
s32 Hit_AddHitBy(struct EwramData_EntityData *entity, s32 type)
{
    struct EntityHit *hit = &entity->hit;

    if (hit->hitBy[0] == type)
    {
        hit->hitTimer[0] = 0;
    }
    else if (hit->hitBy[1] == type)
    {
        hit->hitTimer[1] = 0;
    }
    else if (hit->hitBy[2] == type)
    {
        hit->hitTimer[2] = 0;
    }
    else if (hit->hitBy[0] == 0)
    {
        hit->hitBy[0] = type;
        hit->hitTimer[0] = 0;
    }
    else if (hit->hitBy[1] == 0)
    {
        hit->hitBy[1] = type;
        hit->hitTimer[1] = 0;
    }
    else if (hit->hitBy[2] == 0)
    {
        hit->hitBy[2] = type;
        hit->hitTimer[2] = 0;
    }
    else
    {
        return 0;
    }
    return 1;
}

/**
 * @brief 42754 | Clear all entity boxes (gEntityBoxes)
 */
void HitBox_ClearAll(void)
{
    DMA_FILL_32(3, 0, gEntityBoxes, sizeof(gEntityBoxes));
}

/**
 * @brief 4277C | Activate the entity's hit block; only its attack box hurts
 *
 * @param entity Entity
 * @param type Attack type
 * @param onHit Callback when this entity hits another one
 * @param onHurt Callback when this entity is hit (a function address)
 * @return Always 1
 */
s32 Hit_Enable(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt)
{
    struct EntityHit *hit = &entity->hit;

    hit->type = type;
    hit->noContactDamage = 1;
    hit->flags = 1;
    hit->boxSlot = EntityGetSlot(entity);
    hit->hitBy[2] = 0;
    hit->hitBy[1] = 0;
    hit->hitBy[0] = 0;
    hit->hitTimer[2] = 0;
    hit->hitTimer[1] = 0;
    hit->hitTimer[0] = 0;
    hit->onHit = onHit;
    hit->onHurt = (void *)onHurt;
    return 1;
}

/**
 * @brief 427B8 | Activate the entity's hit block; its body also hurts (contact damage)
 *
 * @param entity Entity
 * @param type Attack type
 * @param onHit Callback when this entity hits another one
 * @param onHurt Callback when this entity is hit (a function address)
 * @return Always 1
 */
s32 Hit_EnableContact(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt)
{
    struct EntityHit *hit = &entity->hit;

    hit->type = type;
    hit->noContactDamage = 0;
    hit->flags = 1;
    hit->boxSlot = EntityGetSlot(entity);
    hit->hitBy[2] = 0;
    hit->hitBy[1] = 0;
    hit->hitBy[0] = 0;
    hit->hitTimer[2] = 0;
    hit->hitTimer[1] = 0;
    hit->hitTimer[0] = 0;
    hit->onHit = onHit;
    hit->onHurt = (void *)onHurt;
    return 1;
}

/**
 * @brief 427F4 | Like Hit_EnableContact with an extra unused argument
 *
 * @param entity Entity
 * @param type Attack type
 * @param unused Ignored
 * @param onHit Callback when this entity hits another one
 * @param onHurt Callback when this entity is hit
 * @return Always 1
 */
s32 Hit_EnableContactEx(struct EwramData_EntityData *entity, s32 type, s32 unused, void *onHit, s32 onHurt)
{
    struct EntityHit *hit = &entity->hit;

    hit->type = type;
    hit->noContactDamage = 0;
    hit->flags = 1;
    hit->boxSlot = EntityGetSlot(entity);
    hit->hitBy[2] = 0;
    hit->hitBy[1] = 0;
    hit->hitBy[0] = 0;
    hit->hitTimer[2] = 0;
    hit->hitTimer[1] = 0;
    hit->hitTimer[0] = 0;
    hit->onHit = onHit;
    hit->onHurt = (void *)onHurt;
    return 1;
}

/**
 * @brief 42828 | Deactivate the entity's hit block
 *
 * @param entity Entity
 */
void Hit_Disable(struct EwramData_EntityData *entity)
{
    struct EntityHit *hit = &entity->hit;

    hit->noContactDamage = 0;
    hit->flags = 0;
    hit->boxSlot = 0xFF;
    hit->hitBy[2] = 0;
    hit->hitBy[1] = 0;
    hit->hitBy[0] = 0;
    hit->hitTimer[2] = 0;
    hit->hitTimer[1] = 0;
    hit->hitTimer[0] = 0;
    hit->onHit = NULL;
    hit->onHurt = NULL;
}

/**
 * @brief 42848 | Set the entity's attack box
 *
 * @param entity Entity
 * @param box SpriteBox (byte aligned, copied byte by byte)
 * @return 0 when the hit block is inactive, otherwise 1
 */
s32 HitBox_SetAttack(struct EwramData_EntityData *entity, void *box)
{
    struct EntityHit *hit = &entity->hit;

    if (hit->flags & 1)
    {
        struct EntityBoxes *dst = &gEntityBoxes[entity->hit.boxSlot];
        struct SpriteBox *src = box;

        // Byte-wise copy (the record has byte alignment).
        dst->attack.box.x = src->x;
        dst->attack.box.y = src->y;
        dst->attack.box.width = src->width;
        dst->attack.box.height = src->height;
        return 1;
    }
    return 0;
}

/**
 * @brief 42884 | Set the entity's attack box from a packed 32-bit SpriteBox
 *
 * @param entity Entity
 * @param box Packed box (x, y, width, height bytes)
 * @return 0 when the hit block is inactive, otherwise 1
 */
s32 HitBox_SetAttackRaw(struct EwramData_EntityData *entity, s32 box)
{
    struct EntityHit *hit = &entity->hit;

    if (hit->flags & 1)
    {
        struct EntityBoxes *dst = &gEntityBoxes[entity->hit.boxSlot];
        dst->attack.raw = box;
        return 1;
    }
    return 0;
}

/**
 * @brief 428B4 | Set the entity's body box
 *
 * @param entity Entity
 * @param box SpriteBox (byte aligned, copied byte by byte)
 * @return 0 when the hit block is inactive, otherwise 1
 */
s32 HitBox_SetBody(struct EwramData_EntityData *entity, void *box)
{
    struct EntityHit *hit = &entity->hit;

    if (hit->flags & 1)
    {
        struct SpriteBox *dst = &(gEntityBoxes + entity->hit.boxSlot)->body.box;
        struct SpriteBox *src = box;

        dst->x = src->x;
        dst->y = src->y;
        dst->width = src->width;
        dst->height = src->height;
        return 1;
    }
    return 0;
}

/**
 * @brief 428F0 | Set the entity's body box from a packed 32-bit SpriteBox
 *
 * @param entity Entity
 * @param box Packed box (x, y, width, height bytes)
 * @return 0 when the hit block is inactive, otherwise 1
 */
s32 HitBox_SetBodyRaw(struct EwramData_EntityData *entity, s32 box)
{
    struct EntityHit *hit = &entity->hit;

    if (hit->flags & 1)
    {
        union EntityBox *dst = &(gEntityBoxes + entity->hit.boxSlot)->body;

        dst->raw = box;
        return 1;
    }
    return 0;
}
