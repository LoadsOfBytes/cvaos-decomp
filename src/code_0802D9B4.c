/* Soul helpers 0x0802D9B4..0x0802DAD3. */
#include "code_08009A0.h"
#include "soul_skill_entity.h"
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
    entity->ext.redSoul.hit = 1;
}

/**
 * @brief Hit callback: unless already anchored, create a RedSoulAnchor_Update
 * entity linking the projectile to the target, set a 0x78-update timer
 * and attack box 0x1014F8F6; otherwise clear unk_50A.
 */
void RedSoul_AnchorToHitTargetTimed(struct EwramData_EntityData *entity, struct EwramData_EntityData *target)
{
    struct EwramData_EntityData *child;

    if (entity->ext.redSoul.anchor == NULL)
    {
        child = EntityCreateInRange(0x49, 0xCF, (u32 *)RedSoulAnchor_Update);
        if (child == NULL)
            return;
        entity->ext.redSoul.anchor = child;
        ENTITY_OWNER(child) = entity;
        child->ext.redSoulAnchor.target = target;
        ENTITY_INDEX(child) = ENTITY_INDEX(entity);
        entity->timer = 0x78;
        HitBox_SetAttackRaw(entity, 0x1014F8F6);
        child->accelX.whole = entity->posX.whole - target->posX.whole;
        child->accelY.whole = entity->posY.whole - target->posY.whole;
    }
    else
    {
        entity->ext.redSoul.anchorTimer = 0;
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
 * bit 0 copy ext.skillChronomage.invert to paletteInvert, then draw.
 */
void Soul_DrawWithPaletteInvert(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram = gEwramData;

    if (ewram->gameState.subMode != 6 && ewram->paused)
        ewram->paletteInvert = entity->ext.skillChronomage.invert;
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
 * (ext.skillBone.hit).
 */
void BoneSoul_OnPlayerContact(struct EwramData_EntityData *entity)
{
    entity->ext.skillBone.hit = 1;
}
