/* Mandragora 0x080CF804..0x080CFAAF. Mandragora_SpawnDeathParts / MandragoraDeathPart_Update / Mandragora_UpdateBestiaryPreview are machine-identical
 * copies of the Needles / Blue Crow functions (cloning script). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern s32 Enemy_PreUpdate(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern s32 Enemy_TakeDamage(struct EwramData_EntityData *param_0);
extern void Mandragora_UpdateWait(struct EwramData_EntityData *entity);
extern void Mandragora_UpdateScream(struct EwramData_EntityData *entity);
extern void Mandragora_UpdateDeath(struct EwramData_EntityData *entity);

/* Same layout as sNeedlesDeathParts in code_080CB6BC.c. */
enum {
    NEEDLE_OFFSET_X,
    NEEDLE_OFFSET_Y,
    NEEDLE_UNK_2,
    NEEDLE_VEL_X,
    NEEDLE_VEL_Y,
    NEEDLE_TIMER
};

void MandragoraDeathPart_Update(struct EwramData_EntityData *entity);
extern const s8 sMandragoraDeathParts[5][6];
extern u8 sMandragoraSpriteSheet[];

/**
 * @brief Spawn up to five table-configured sprite parts with mirrored
 * velocity and offsets; count their lifetimes in the root owner field.
 */
void Mandragora_SpawnDeathParts(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;
    const s8 *needle;
    s32 offsetX;
    s32 i;

    for (i = 0; i < 5; i++)
    {
        child = EntityCreateInRange(0x49, 0xCF, (u32 *)MandragoraDeathPart_Update);
        if (child == NULL)
            continue;
        needle = sMandragoraDeathParts[i];
        entity->unk_4F4.part8.unk_4F7++;
        child->unk_4F4.part8.unk_4F5 = entity->unk_4F4.part8.unk_4F4;
        child->spriteLayer = entity->spriteLayer;
        child->unk_510.part8.basePalette = entity->unk_510.part8.basePalette;
        child->flipX = entity->flipX;
        Sprite_Attach(child, sMandragoraSpriteSheet, Sprite_GetTileSlot(entity), child->unk_510.part8.basePalette);
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
 * @brief Move under gravity; decrement the root live-part count and
 * request deletion on bounds, bottom-Y or timer-underflow completion.
 */
void MandragoraDeathPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = &gEwramData->entityData[entity->unk_4F4.part8.unk_4F5];

    if (!Entity_IsInRect(entity, -0x80, -0x40, 0x1F0, 0x120) || (u8)--entity->timer == 0xFF
        || entity->posY.part16.integer > 0xAA)
    {
        parent->unk_4F4.part8.unk_4F7--;
        entity->deleteRequested = 1;
    }
    else
    {
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += 0x3000;
    }
}

void Mandragora_UpdateBestiaryPreview(struct EwramData_EntityData *entity);

/**
 * @brief CF99C | Mandragora update: state 3 -> Mandragora_UpdateBestiaryPreview, state 0xF0 ->
 * Mandragora_UpdateDeath; otherwise (unless Enemy_PreUpdate intervenes while animation
 * is not 2) run state 1 / 2 and animate.
 */
void EnemyMandragoraUpdate(struct EwramData_EntityData *entity)
{
    if (entity->state == 3)
    {
        Mandragora_UpdateBestiaryPreview(entity);
    }
    else if (entity->state == 0xF0)
    {
        Mandragora_UpdateDeath(entity);
    }
    else if (entity->animScriptIndex == 2 || !Enemy_PreUpdate(entity))
    {
        switch (entity->state)
        {
            case 1:
                Mandragora_UpdateWait(entity);
                break;
            case 2:
                Mandragora_UpdateScream(entity);
                break;
        }
        Anim_Update(entity);
        Enemy_CheckHitPlayer(entity);
    }
}

/**
 * @brief Apply enemy damage to the intermediate part; on defeat enter
 * its state 0xF0 with substate counters reset.
 */
void MandragoraPart_OnHit(struct EwramData_EntityData *entity)
{
    if (Enemy_TakeDamage(entity))
    {
        entity->state = 0xF0;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/**
 * @brief Bestiary state (3): select animation 0 once, then update it.
 */
void Mandragora_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
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
 * @brief Apply enemy damage to the root owner; on defeat enter death
 * state 0xF0 with substate counters reset.
 */
void Mandragora_OnHit(struct EwramData_EntityData *entity)
{
    if (Enemy_TakeDamage(entity))
    {
        entity->state = 0xF0;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/**
 * @brief Request deletion of the root owner, intermediate part and
 * terminal part through their stored slot links.
 */
void Mandragora_RequestLinkedDeletion(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *linked = &gEwramData->entityData[entity->unk_4F4.part8.unk_4F6];
    struct EwramData_EntityData *linked2 = &gEwramData->entityData[linked->unk_4F4.part8.unk_4F6];

    linked2->deleteRequested = 1;
    linked->deleteRequested = 1;
    entity->deleteRequested = 1;
}
