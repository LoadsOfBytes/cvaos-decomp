/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   KillerMantle_OnHit = SkullMillioneFamily_OnHit */
#include "code_08009A0.h"
#include "cockatrice_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief Apply enemy damage; on defeat drop rewards, restore the palette
 * and enter death state 3 with the substate counters reset.
 */
void KillerMantle_OnHit(struct EwramData_EntityData *entity)
{
    if (Enemy_TakeDamage(entity))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->state = 3;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/* Killer Mantle update and part placement 0x080BB824..0x080BB923. */
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *entity);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *entity, u16 width, u16 height);
extern void KillerMantle_UpdateHover(struct EwramData_EntityData *entity);
extern void KillerMantle_UpdateJumpAttack(struct EwramData_EntityData *entity);
extern s32 KillerMantle_UpdateDeath(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern void Sprite_GetPartCornerOffset(struct EwramData_EntityData *entity, s32 index, s16 *offset);

/* x offset of the Cockatrice/Basilisk part (CockatriceFamilyPart_FollowOwner). */
extern const u16 sUnk_08119490[];

/**
 * @brief BB824 | Cockatrice/Basilisk update: unless Enemy_PreUpdate handles the
 * frame, state 0 waits for the target within 0xDC x 0xA0 (then state 1),
 * states 1/2 run KillerMantle_UpdateHover / KillerMantle_UpdateJumpAttack, state 3 deletes the enemy
 * when KillerMantle_UpdateDeath says so, state 4 only animates; then Enemy_CheckHitPlayer and
 * animate.
 */
void EnemyKillerMantleUpdate(struct EwramData_EntityData *entity)
{
    if (Enemy_PreUpdate(entity))
        return;
    switch (entity->state)
    {
        case 0:
            if (Target_IsNearEntity(entity, 0xDC, 0xA0))
            {
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
        case 1:
            KillerMantle_UpdateHover(entity);
            break;
        case 2:
            KillerMantle_UpdateJumpAttack(entity);
            break;
        case 3:
            if (KillerMantle_UpdateDeath(entity))
            {
                EntityDelete(entity);
                return;
            }
            break;
        case 4:
            Anim_Update(entity);
            return;
    }
    Enemy_CheckHitPlayer(entity);
    Anim_Update(entity);
}

/**
 * @brief Place the shared Cockatrice/Basilisk part at owner sprite point 4
 * plus a facing-dependent horizontal offset.
 */
void CockatriceFamilyPart_FollowOwner(struct EwramData_EntityData *entity, struct EwramData_EntityData *owner)
{
    struct { s16 x; s16 y; } point;
    s16 offset = sUnk_08119490[0];

    entity->flipX = owner->flipX;
    if (entity->flipX)
        offset = -offset;
    Sprite_GetPartCornerOffset(owner, 4, &point.x);
    entity->posX.part16.integer = offset + (owner->posX.part16.integer + point.x);
    entity->posY.part16.integer = point.y + owner->posY.part16.integer;
}

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, u8 *resource, void *palette, u8 variant, void *sheet);

/* Graphics of the Cockatrice/Basilisk part (CockatriceFamilyPart_Update). */
extern u8 sCockatriceFamilyGfx[];
extern u8 sCockatriceFamilyPalette[];
extern u8 sCockatriceFamilySpriteSheet[];

/**
 * @brief Load an affine part and follow the owner with angle and scale
 * wobble; increase amplitude while it attacks. Free the affine slot
 * after the owner termination flag. Exact visual identity is unresolved.
 */
void CockatriceFamilyPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    u16 slot;

    switch (entity->state)
    {
        case 0:
            if (!Entity_LoadGraphics(entity, sCockatriceFamilyGfx, sCockatriceFamilyPalette, COCKATRICE_PALETTE_SLOT(parent), sCockatriceFamilySpriteSheet))
                goto delete;
            slot = ObjAffine_Alloc(1);
            if ((s16)slot < 0)
                goto delete;
            entity->affineIndex = slot;
            entity->affine = 1;
            entity->animFrame = 2;
            entity->spriteLayer = parent->spriteLayer;
            COCKATRICE_PART_SCALE_X(entity) = 0x10000;
            COCKATRICE_PART_AMPLITUDE(entity) = 8;
            entity->state = 1;
        case 1:
            CockatriceFamilyPart_FollowOwner(entity, parent);
            if (parent->state == 2)
                entity->state = 2;
            if (gEwramData->gameState.timeStopTimer == 0)
                COCKATRICE_PART_PHASE(entity) -= 0x2B0;
            break;
        case 2:
            CockatriceFamilyPart_FollowOwner(entity, parent);
            COCKATRICE_PART_PHASE(entity) -= 0x17B0;
            COCKATRICE_PART_AMPLITUDE(entity) = 16;
            break;
        case 3:
            ObjAffine_Free(entity->affineIndex);
        delete:
            EntityDelete(entity);
            return;
    }
    if (!entity->flipX)
        COCKATRICE_PART_ANGLE(entity) = Sin(COCKATRICE_PART_PHASE(entity))
                                              * COCKATRICE_PART_AMPLITUDE(entity) / 256;
    else
        COCKATRICE_PART_ANGLE(entity) = Sin(COCKATRICE_PART_PHASE(entity))
                                              * -COCKATRICE_PART_AMPLITUDE(entity) / 256;
    COCKATRICE_PART_SCALE_X(entity) = (Sin(COCKATRICE_PART_PHASE(entity) + 0x2000) << 13 >> 16) + 0xE000;
    entity->palette = parent->palette;
    if (COCKATRICE_DEATH_FLAGS(parent) & 1)
        entity->state = 3;
    ObjAffine_Set(COCKATRICE_PART_ANGLE(entity), COCKATRICE_PART_SCALE_X(entity), 0x10000, entity->flipX, 0,
                 entity->affineIndex);
}

// The offsets are passed sign-extended here (u16 in the definition).
extern void Entity_PlaceRelativeToCopy(struct EwramData_EntityData *entity, struct EwramData_EntityData *anchor, s32 offsetX, s32 offsetY);
extern s32 Hit_Enable(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);
extern s32 HitBox_SetAttackRaw(struct EwramData_EntityData *entity, s32 box);
extern s32 Sound_PlayTracked(s32 song);
extern s32 Sound_StopTracked(s32 song);
extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern void CockatriceFamilyAttack_Draw(struct EwramData_EntityData *entity);
extern void CockatriceFamilyAttack_DamagePlayerOnContact(struct EwramData_EntityData *entity);

/**
 * @brief Create the shared affine attack effect, play its tracked sound,
 * rotate and scale it for 32 updates and check player contact at update 16.
 * Stop the sound and free the affine slot on removal.
 */
void CockatriceFamilyAttack_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    u16 slot;
    s32 scale;
    s32 growth;

    switch (entity->state)
    {
        case 0:
            if (!Entity_LoadGraphics(entity, sCockatriceFamilyGfx, sCockatriceFamilyPalette, COCKATRICE_PALETTE_SLOT(parent), sCockatriceFamilySpriteSheet))
                goto delete;
            entity->animFrame = 1;
            entity->spriteLayer = parent->spriteLayer + 1;
            entity->drawFunc = (u32 *)CockatriceFamilyAttack_Draw;
            slot = ObjAffine_Alloc(1);
            if ((s16)slot < 0)
                goto delete;
            entity->affineIndex = slot;
            entity->affine = 1;
            COCKATRICE_BEAM_LENGTH(entity) = 0x1000000;
            Entity_PlaceRelativeToCopy(entity, parent, -24, -33);
            COCKATRICE_BEAM_ANGLE(entity) = 0x6C00;
            if (entity->flipX)
            {
                s32 angle = -0xEC00; // 0x8000 - 0x6C00 in 16 bits

                COCKATRICE_BEAM_ANGLE(entity) = angle;
            }
            Hit_Enable(entity, 8, CockatriceFamilyAttack_DamagePlayerOnContact, 0);
            HitBox_SetAttackRaw(entity, 0x10440CBC);
            entity->hit.flags |= 4;
            entity->objMode = 1;
            entity->state = 1;
            Sound_PlayTracked(0x65);
        case 1:
            COCKATRICE_BEAM_FRAME(entity)++;
            COCKATRICE_BEAM_SCALE_Y(entity) = Sin(COCKATRICE_BEAM_FRAME(entity) << 10);
            if (COCKATRICE_BEAM_FRAME(entity) <= 15)
                entity->posY.whole -= 0x2000;
            if (!entity->flipX)
                COCKATRICE_BEAM_ANGLE(entity) += 8;
            else
                COCKATRICE_BEAM_ANGLE(entity) -= 8;
            if (COCKATRICE_BEAM_FRAME(entity) == 16)
                Hit_CheckWithPlayer(collision, entity);
            if (COCKATRICE_BEAM_FRAME(entity) <= 31)
                break;
        case 2:
            Sound_StopTracked(0x65);
            ObjAffine_Free(entity->affineIndex);
        delete:
            EntityDelete(entity);
            return;
    }
    growth = COCKATRICE_BEAM_LENGTH(entity);
    if (growth > 0x1000000)
        growth = 0x1000000;
    growth /= 16;
    scale = growth / 16;
    COCKATRICE_BEAM_STEP_X(entity) = Sin(COCKATRICE_BEAM_ANGLE(entity) + 0x4000) << 4;
    COCKATRICE_BEAM_STEP_Y(entity) = Sin(COCKATRICE_BEAM_ANGLE(entity)) << 4;
    COCKATRICE_BEAM_STEP_X(entity) = COCKATRICE_BEAM_STEP_X(entity) / 0x10000 * scale;
    COCKATRICE_BEAM_STEP_Y(entity) = COCKATRICE_BEAM_STEP_Y(entity) / 0x10000 * scale;
    ObjAffine_Set(COCKATRICE_BEAM_ANGLE(entity), 0x10000, COCKATRICE_BEAM_SCALE_Y(entity), 0, 0, entity->affineIndex);
}
