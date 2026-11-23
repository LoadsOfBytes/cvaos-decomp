/* Needles 0x080CB6BC..0x080CB94B (Needles_UpdateBestiaryPreview / Needles_OnHit are copies of the Blue Crow
 * BlueCrow_UpdateBestiaryPreview / BlueCrow_OnHit). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern void Enemy_DropRewards(struct EwramData_EntityData *entity);
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern void Needles_UpdateIdle(struct EwramData_EntityData *entity);
extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity);

/* Five 6-byte needle records for Needles_SpawnDeathParts (a struct would be padded to
 * 8 bytes by agbcc): offset x, offset y, animFrame, velX / 0x1000,
 * -velY / 0x1000, timer (timer). */
enum {
    NEEDLE_OFFSET_X,
    NEEDLE_OFFSET_Y,
    NEEDLE_UNK_2,
    NEEDLE_VEL_X,
    NEEDLE_VEL_Y,
    NEEDLE_TIMER
};

extern const s8 sNeedlesDeathParts[5][6];
extern u8 sNeedlesSpriteSheet[];

void NeedlesPart_Update(struct EwramData_EntityData *entity);
void Needles_UpdateDeath(struct EwramData_EntityData *entity);
void Needles_UpdateBestiaryPreview(struct EwramData_EntityData *entity);

/**
 * @brief Spawn up to five table-configured needle parts with mirrored
 * velocity and offsets; count the live parts in the owner field.
 */
void Needles_SpawnDeathParts(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;
    const s8 *needle;
    s32 offsetX;
    s32 i;

    for (i = 0; i < 5; i++)
    {
        child = EntityCreateInRange(0x49, 0xCF, (u32 *)NeedlesPart_Update);
        if (child == NULL)
            continue;
        needle = sNeedlesDeathParts[i];
        entity->ext.needlesDeath.parts++;
        ENTITY_PARENT_INDEX(child) = ENTITY_INDEX(entity);
        child->spriteLayer = entity->spriteLayer;
        child->unk_510.part8.basePalette = entity->unk_510.part8.basePalette;
        child->flipX = entity->flipX;
        Sprite_Attach(child, sNeedlesSpriteSheet, Sprite_GetTileSlot(entity), child->unk_510.part8.basePalette);
        child->scrollWithBg = 1;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
        child->posX.whole = entity->posX.whole;
        child->posY.whole = entity->posY.whole;
        offsetX = needle[NEEDLE_OFFSET_X];
        child->posY.part16.integer += needle[NEEDLE_OFFSET_Y];
        child->animFrame = needle[NEEDLE_UNK_2];
        child->velX.whole = needle[NEEDLE_VEL_X] << 12;
        child->velY.whole = -(needle[NEEDLE_VEL_Y] << 12);
        child->timer = needle[NEEDLE_TIMER];
        if (child->flipX)
        {
            child->velX.whole = -child->velX.whole;
            offsetX = -offsetX;
        }
        child->posX.part16.integer += offsetX;
    }
}

/**
 * @brief Move a needle part under gravity. On bounds, timer-underflow or
 * bottom-Y completion, decrement the owner part count and request deletion.
 */
void NeedlesPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];

    if (!Entity_IsInRect(entity, -0x80, -0x40, 0x1F0, 0x120) || (u8)--entity->timer == 0xFF
        || entity->posY.part16.integer > 0xAA)
    {
        parent->ext.needlesDeath.parts--;
        entity->deleteRequested = 1;
    }
    else
    {
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += 0x3000;
    }
}

/**
 * @brief CB854 | Needles update: state 3 -> Needles_UpdateBestiaryPreview, state 2 ->
 * Needles_UpdateDeath, otherwise Needles_UpdateIdle unless Enemy_PreUpdate says otherwise.
 */
void EnemyNeedlesUpdate(struct EwramData_EntityData *entity)
{
    if (entity->state == 3)
    {
        Needles_UpdateBestiaryPreview(entity);
    }
    else if (entity->state == 2)
    {
        Needles_UpdateDeath(entity);
    }
    else if (!Enemy_PreUpdate(entity))
    {
        Needles_UpdateIdle(entity);
        Anim_Update(entity);
        Enemy_CheckHitPlayer(entity);
    }
}

/**
 * @brief Death state (2): hide, drop rewards and spawn the five parts.
 * Request deletion after the live-part count reaches zero.
 */
void Needles_UpdateDeath(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            PlaySong(0x70);
            entity->hidden = 1;
            Enemy_DropRewards(entity);
            entity->palette = entity->unk_510.part8.basePalette;
            Needles_SpawnDeathParts(entity);
            entity->subState++;
        case 1:
            if (entity->ext.needlesDeath.parts == 0)
                entity->deleteRequested = 1;
            break;
    }
}

/**
 * @brief Bestiary state (3): select animation 0 once, then update it.
 */
void Needles_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        if (entity->animScriptIndex != 0)
            Anim_StartIndexed(entity, 0, 0, 1);
        entity->subState++;
    }
    Anim_Update(entity);
}

/**
 * @brief Apply enemy damage; on defeat enter death state 2 with substate
 * counters reset. Reward drops belong to the death helper.
 */
void Needles_OnHit(struct EwramData_EntityData *entity)
{
    if (Enemy_TakeDamage(entity))
    {
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
    }
}
