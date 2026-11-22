/* Soul helpers 0x0802D9B4..0x0802DAD3. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern void RedSoulAnchor_Update(struct EwramData_EntityData *entity);
extern void EntityAccelerateAndMoveX(struct EwramData_EntityData *entity, s32 maxSpeed);
extern void EntityAccelerateAndMoveY(struct EwramData_EntityData *entity, s32 maxSpeed);
extern s32 EntityIsOutsideScreen(struct EwramData_EntityData *entity, s32 margin);

/**
 * @brief Hit callback of soul attack projectiles (Hit_Enable slot 5): set the
 * hit marker unk_4FC to 1.
 */
void SoulAttack_MarkHit(struct EwramData_EntityData *entity)
{
    entity->unk_4FC.part8.unk_4FC = 1;
}

/**
 * @brief Hit callback: unless already anchored, create a RedSoulAnchor_Update
 * entity linking the projectile to the target, set a 0x78-update timer
 * and attack box 0x1014F8F6; otherwise clear unk_50A.
 */
void RedSoul_AnchorToHitTargetTimed(struct EwramData_EntityData *entity, struct EwramData_EntityData *target)
{
    struct EwramData_EntityData *child;

    if (entity->unk_504.unk_504_entity == NULL)
    {
        child = EntityCreateInRange(0x49, 0xCF, (u32 *)RedSoulAnchor_Update);
        if (child == NULL)
            return;
        entity->unk_504.unk_504_entity = child;
        child->unk_4F8.unk_4F8_entity = entity;
        child->unk_504.unk_504_entity = target;
        child->unk_4F4.part8.unk_4F4 = entity->unk_4F4.part8.unk_4F4;
        entity->timer = 0x78;
        HitBox_SetAttackRaw(entity, 0x1014F8F6);
        child->accelX.whole = entity->posX.whole - target->posX.whole;
        child->accelY.whole = entity->posY.whole - target->posY.whole;
    }
    else
    {
        entity->unk_508.part16.unk_50A = 0;
    }
}

/**
 * @brief Contact callback of the homing soul projectile: switch it to its
 * release state 3.
 */
void HomingSoulProjectile_OnPlayerContact(struct EwramData_EntityData *entity)
{
    entity->state = 3;
}

/**
 * @brief Draw callback (SkillChronomageUse): outside mode 6 and with unk_A074
 * bit 0 copy unk_50C to paletteInvert, then draw.
 */
void Soul_DrawWithPaletteInvert(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram = gEwramData;

    if (ewram->gameState.subMode != 6 && ewram->paused)
        ewram->paletteInvert = entity->unk_50C.whole;
    Sprite_Draw(entity);
}

/**
 * @brief 2DA5C | A random non-empty entry of trackedEntities[4] (NULL if all empty).
 */
struct EwramData_EntityData *Player_GetRandomTrackedEntity(void)
{
    s32 i = 0;
    u32 slot = RandomNumberGenerator() % 4;

    for (; i <= 3; i++)
    {
        if (gEwramData->playerState.trackedEntities[slot] != NULL)
            return gEwramData->playerState.trackedEntities[slot];
        slot = (slot + 1) % 4;
    }
    return NULL;
}

/**
 * @brief Move with acceleration (velocity limit 0x80000) and end outside
 * the screen.
 */
void Effect_UpdateBallisticDebris(struct EwramData_EntityData *entity)
{
    EntityAccelerateAndMoveX(entity, 0x80000);
    EntityAccelerateAndMoveY(entity, 0x80000);
    if (EntityIsOutsideScreen(entity, 4))
        entity->deleteRequested = 1;
}

/**
 * @brief Contact callback of the bone soul projectile: set its hit flag
 * (unk_504).
 */
void BoneSoul_OnPlayerContact(struct EwramData_EntityData *entity)
{
    entity->unk_504.part16.unk_504 = 1;
}
