/*
 * Blue Crow and Red Crow.
 */
#include "core.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "sound.h"
#include "player_core.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "entity_util.h"
#include "hitbox.h"
#include "entity_hit.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, u8 *resource, void *palette, u8 variant, void *sheet);
extern void Entity_FaceTarget(struct EwramData_EntityData *entity);

void BlueCrow_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
void BlueCrow_SpawnDeathParts(struct EwramData_EntityData *entity);
void BlueCrowDeathPart_Update(struct EwramData_EntityData *entity);
void BlueCrow_UpdateBestiaryPreview(struct EwramData_EntityData *entity);
void RedCrow_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
void RedCrow_SpawnDeathParts(struct EwramData_EntityData *entity);
void RedCrowDeathPart_Update(struct EwramData_EntityData *entity);
void RedCrow_UpdateBestiaryPreview(struct EwramData_EntityData *entity);
void RedCrow_UpdateDeath(struct EwramData_EntityData *entity);

/* EnemyBlueCrowCreate. */

/**
 * @brief C9A6C | Blue Crow create: load the graphics (delete the enemy when
 * that fails), start the animation, layer 3, state 1, hit boxes; state 3
 * instead when bestiaryPreview is set.
 */
void EnemyBlueCrowCreate(struct EwramData_EntityData *entity)
{
    u8 *palette = sBlueCrowRedCrowPalette;
    u8 *sheet = sBlueCrowRedCrowSpriteSheet;

    if (!Entity_LoadGraphics(entity, sBlueCrowRedCrowGfx, palette, 0, sheet))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 3;
    entity->state = 1;
    entity->stage = 0;
    entity->subState = 0;
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        entity->stage = 0;
        entity->subState = 0;
        return;
    }
    Entity_EnableBoxes(entity, Enemy_OnContact, BlueCrow_OnHit);
}

/* Blue Crow flight and attack states, reconstructed from the original ROM. */

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define ANIM(e,n,loop) do { if ((e)->animScriptIndex != (n)) Anim_StartIndexed((e),(n),0,(loop)); } while (0)

/**
 * @brief Alive movement: activate near the target, manage flight substates
 * and aim at a point above and beside the player. Preserve the existing
 * substate reachability, division and timer-underflow behavior.
 */
void BlueCrow_UpdateMovement(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 targetX = Target_GetX(entity) + bg->xPos.whole;
    s32 targetY = Target_GetY(entity) + bg->yPos.whole - 0x2A0000;
    s32 zero = 0;

    switch (entity->subState)
    {
        case 0:
            entity->ext.blueCrow.unusedDiveCount = zero;
            Entity_FaceTarget(entity);
            ANIM(entity, 0, 1);
            entity->subState = 1;
            entity->stage = zero;
            entity->timer = zero;
        case 1:
            if (Entity_IsTargetWithinRange(entity, 0x3C0000))
            {
                entity->subState = 3;
                entity->stage = 0;
                PlaySong(0x8B);
            }
            break;
        case 2:
            switch (entity->stage)
            {
                case 0:
                    ANIM(entity, 1, 1);
                    if (!entity->flipX)
                    {
                        entity->velX.whole = -ABS(entity->velX.whole);
                        entity->accelX.whole = -ABS(entity->accelX.whole);
                    }
                    {
                        s32 duration = entity->velY.whole / entity->accelY.whole;
                        entity->timer = ABS(duration) * 2;
                    }
                    entity->stage++;
                case 1:
                    Entity_FaceTarget(entity);
                    if (Entity_AimVelocityAtPoint(entity, targetX, targetY, 0x18000))
                    {
                        entity->subState = 3;
                        entity->stage = 0;
                    }
                    break;
            }
            break;
        case 3:
            Entity_FaceTarget(entity);
            switch (entity->stage)
            {
                case 0:
                    entity->ext.blueCrow.unusedDiveCount++;
                    ANIM(entity, 2, 1);
                    entity->accelX.whole = zero;
                    entity->velX.whole = zero;
                    entity->accelY.whole = zero;
                    entity->velY.whole = zero;
                    entity->timer = 0x20;
                    entity->stage++;
                case 1:
                    if (--entity->timer == 0xFF)
                        entity->stage++;
                    break;
                case 2:
                    if (ABS(Entity_GetTargetOffsetX(entity)) > 0x480000 && ABS(Entity_GetTargetOffsetY(entity)) > 0x100000)
                        entity->stage++;
                    break;
                case 3:
                {
                    u8 *animation = &entity->animScriptIndex;
                    if (*animation != 1)
                        Anim_StartIndexed(entity, 1, 0, 1);
                    zero = -0x200000;
                    if (gEwramData->playerState.player->flipX)
                        zero = -zero;
                    if (Entity_AimVelocityAtPoint(entity, targetX + zero, targetY, 0x1A000))
                    {
                        if (*animation != 2)
                            Anim_StartIndexed(entity, 2, 0, 1);
                        entity->accelX.whole = 0;
                        entity->velX.whole = 0;
                        entity->accelY.whole = 0;
                        entity->velY.whole = 0;
                        entity->stage--;
                    }
                    break;
                }
            }
            break;
    }
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
}

/* Blue Crow 0x080C9D68..0x080CA04B. The Red Crow functions at 0x080CA308.. are machine-identical
 * copies (see this file). */

/* Graphics of the two feather (?) entities spawned by BlueCrow_SpawnDeathParts. */

/**
 * @brief Death state (2): spawn two parts, drop rewards and fall with
 * hit/fragment effects. Restore the palette and update animation internally.
 */
void BlueCrow_UpdateDeath(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            PlaySong(0x72);
            BlueCrow_SpawnDeathParts(entity);
            if (entity->animScriptIndex != 3)
                Anim_StartIndexed(entity, 3, 0, 0);
            entity->accelX.whole = 0;
            entity->velX.whole = 0;
            entity->accelY.whole = 0;
            entity->velY.whole = 0x8000;
            entity->palette = entity->basePalette;
            entity->timer = 0x40;
            entity->subState = 1;
            entity->stage = 0;
            Enemy_DropRewards(entity);
            HitEffect_Spawn((s16)entity->posX.part16.integer, entity->posY.part16.integer, 1);
        case 1:
            entity->posY.whole += entity->velY.whole;
            if ((entity->timer % 16) == 0)
                HitEffect_Spawn((s16)entity->posX.part16.integer, entity->posY.part16.integer + 4, 1);
            Enemy_UpdateFragmentDeathWithSpread(entity, 6, 0x80000, 0x80000);
            Anim_Update(entity);
            break;
    }
}

/**
 * @brief Spawn two frame-8 death parts at opposite offsets, copying the
 * owner palette and selecting mirrored movement by index.
 */
void BlueCrow_SpawnDeathParts(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;
    s32 i;

    for (i = 0; i <= 1; i++)
    {
        child = EntityCreateInRange(0x1B, 0x48, BlueCrowDeathPart_Update);
        if (child == NULL)
            continue;
        if (i == 0)
        {
            child->posX.part16.integer = entity->posX.part16.integer + 8;
            child->posY.part16.integer = entity->posY.part16.integer + 4;
        }
        else
        {
            child->posX.part16.integer = entity->posX.part16.integer - 8;
            child->posY.part16.integer = entity->posY.part16.integer - 4;
        }
        child->spriteLayer = entity->spriteLayer - 1;
        child->scrollWithBg = 1;
        Sprite_Attach(child, sBlueCrowRedCrowSpriteSheet, ObjGfx_Find(sBlueCrowRedCrowGfx), entity->basePalette);
        child->animFrame = 8;
        child->drawFunc = Sprite_DrawEntity;
        child->spawnParam0 = i;
    }
}

/**
 * @brief Move a mirrored death part with horizontal acceleration, blink
 * from the timer bit, then request deletion after two countdowns.
 * Preserve decrement-to-0xFF completion tests. Exact visual unresolved.
 */
void BlueCrowDeathPart_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
            entity->velX.whole = 0x4000;
            entity->velY.whole = 0x2000;
            entity->accelX.whole = -0x300;
            if (entity->spawnParam0 != 0)
            {
                entity->velX.whole = -entity->velX.whole;
                entity->accelX.whole = -entity->accelX.whole;
                entity->flipX = 1;
            }
            entity->timer = 0x30;
            entity->state++;
        case 1:
            if ((u8)--entity->timer == 0xFF)
            {
                entity->objMode = 1;
                entity->timer = 0x18;
                entity->state++;
            }
            break;
        case 2:
            if ((u8)--entity->timer == 0xFF)
                entity->deleteRequested = 1;
            break;
    }
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
    entity->hidden = entity->timer % 2;
}

/**
 * @brief C9FA4 | Blue Crow update: state 3 -> BlueCrow_UpdateBestiaryPreview, state 2 ->
 * BlueCrow_UpdateDeath, otherwise the normal behaviour (BlueCrow_UpdateMovement) unless
 * Enemy_IsStunned says otherwise.
 */
void EnemyBlueCrowUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;

    if (entity->state == 3)
    {
        BlueCrow_UpdateBestiaryPreview(entity);
    }
    else if (entity->state == 2)
    {
        BlueCrow_UpdateDeath(entity);
    }
    else
    {
        Player_TrackEntity(entity);
        if (Enemy_IsStunned(entity))
        {
            Hit_CheckWithPlayer(&result, entity);
        }
        else
        {
            BlueCrow_UpdateMovement(entity);
            Entity_AnimateWithBoxes(entity);
            Hit_CheckWithPlayer(&result, entity);
        }
    }
}

/**
 * @brief Bestiary state (3): select animation 0 once and update it.
 */
void BlueCrow_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
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
 * @brief Apply enemy damage; on defeat enter death state 2 with the
 * substate counters reset. The death helper owns reward drops.
 */
void BlueCrow_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/* Decompiled locally from the original assembly (variant of an already
 * decompiled function). */

/**
 * @brief CA04C | Red Crow create: like EnemyBlueCrowCreate, but palette argument 1
 * for Entity_LoadGraphics.
 */
void EnemyRedCrowCreate(struct EwramData_EntityData *entity)
{
    u8 *palette = sBlueCrowRedCrowPalette;
    u8 *sheet = sBlueCrowRedCrowSpriteSheet;

    if (!Entity_LoadGraphics(entity, sBlueCrowRedCrowGfx, palette, 1, sheet))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 3;
    entity->state = 1;
    entity->stage = 0;
    entity->subState = 0;
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        entity->stage = 0;
        entity->subState = 0;
        return;
    }
    Entity_EnableBoxes(entity, Enemy_OnContact, RedCrow_OnHit);
}

/* Red Crow flight and attack states, reconstructed from the original ROM. */
/**
 * @brief Alive movement: activate near the target, alternate timed waits
 * and accelerating flight, optionally play a reset animation when out
 * of range. Preserve facing changes and timer-underflow completion.
 */
void RedCrow_UpdateMovement(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
        {
            s32 zero = 0;
            entity->ext.redCrow.phase = zero;
            Entity_FaceTarget(entity);
            ANIM(entity, 0, 1);
            entity->subState = 1;
            entity->stage = zero;
            entity->timer = zero;
        }
        case 1:
            if (Entity_IsTargetWithinRange(entity, 0x3C0000))
            {
                entity->subState = 3;
                entity->stage = 0;
            }
            break;
        case 2:
            switch (entity->stage)
            {
                case 0:
                    ANIM(entity, 1, 1);
                    entity->velX.whole = 0x8000;
                    entity->velY.whole = 0x20000;
                    entity->accelX.whole = 0x1000;
                    entity->accelY.whole = -0x1000;
                    if (!entity->flipX)
                    {
                        entity->velX.whole = -entity->velX.whole;
                        entity->accelX.whole = -entity->accelX.whole;
                    }
                    {
                        s32 duration = entity->velY.whole / entity->accelY.whole;
                        entity->timer = ABS(duration) * 2;
                    }
                    entity->stage++;
                    PlaySong(0x8B);
                case 1:
                    if (--entity->timer == 0xFF)
                    {
                        entity->subState = 3;
                        entity->stage = 0;
                    }
                    break;
            }
            break;
        case 3:
            switch (entity->stage)
            {
                case 0:
                    entity->ext.redCrow.phase++;
                    ANIM(entity, 2, 1);
                    entity->accelX.whole = 0;
                    entity->velX.whole = 0;
                    entity->accelY.whole = 0;
                    entity->velY.whole = 0;
                    entity->timer = 0x80;
                    entity->stage++;
                case 1:
                    if (entity->timer == 0x40)
                    {
                        if (entity->ext.redCrow.phase & 1)
                            Entity_FaceTarget(entity);
                        else
                            entity->flipX = 1 - entity->flipX;
                    }
                    if (--entity->timer == 0xFF)
                    {
                        entity->subState = 2;
                        entity->stage = 0;
                        if ((entity->ext.redCrow.phase & 1) && entity->ext.redCrow.phase != 1)
                        {
                            s32 inRange = (s8)Entity_IsTargetWithinRange(entity, 0x780000);
                            if (inRange == 0)
                            {
                                entity->subState = 4;
                                entity->stage = inRange;
                                entity->animDone = 0;
                            }
                        }
                    }
                    break;
            }
            break;
        case 4:
            switch (entity->stage)
            {
                case 0:
                    if (entity->animDone)
                        entity->stage = 1;
                    break;
                case 1:
                    ANIM(entity, 4, 0);
                    entity->stage++;
                case 2:
                    if (entity->animDone)
                    {
                        entity->subState = 0;
                        entity->stage = 0;
                    }
                    break;
            }
            break;
    }
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
}

/* Red Crow 0x080CA308..0x080CA5EB: machine-identical copies of the Blue Crow
 * functions in this file, generated with a cloning script. */

/**
 * @brief Death state (2): spawn two parts, drop rewards and fall with
 * hit/fragment effects. Restore the palette and update animation internally.
 */
void RedCrow_UpdateDeath(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            PlaySong(0x72);
            RedCrow_SpawnDeathParts(entity);
            if (entity->animScriptIndex != 3)
                Anim_StartIndexed(entity, 3, 0, 0);
            entity->accelX.whole = 0;
            entity->velX.whole = 0;
            entity->accelY.whole = 0;
            entity->velY.whole = 0x8000;
            entity->palette = entity->basePalette;
            entity->timer = 0x40;
            entity->subState = 1;
            entity->stage = 0;
            Enemy_DropRewards(entity);
            HitEffect_Spawn((s16)entity->posX.part16.integer, entity->posY.part16.integer, 1);
        case 1:
            entity->posY.whole += entity->velY.whole;
            if ((entity->timer % 16) == 0)
                HitEffect_Spawn((s16)entity->posX.part16.integer, entity->posY.part16.integer + 4, 1);
            Enemy_UpdateFragmentDeathWithSpread(entity, 6, 0x80000, 0x80000);
            Anim_Update(entity);
            break;
    }
}

/**
 * @brief Spawn two frame-8 death parts at opposite offsets, copying the
 * owner palette and selecting mirrored movement by index.
 */
void RedCrow_SpawnDeathParts(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;
    s32 i;

    for (i = 0; i <= 1; i++)
    {
        child = EntityCreateInRange(0x1B, 0x48, RedCrowDeathPart_Update);
        if (child == NULL)
            continue;
        if (i == 0)
        {
            child->posX.part16.integer = entity->posX.part16.integer + 8;
            child->posY.part16.integer = entity->posY.part16.integer + 4;
        }
        else
        {
            child->posX.part16.integer = entity->posX.part16.integer - 8;
            child->posY.part16.integer = entity->posY.part16.integer - 4;
        }
        child->spriteLayer = entity->spriteLayer - 1;
        child->scrollWithBg = 1;
        Sprite_Attach(child, sBlueCrowRedCrowSpriteSheet, ObjGfx_Find(sBlueCrowRedCrowGfx), entity->basePalette);
        child->animFrame = 8;
        child->drawFunc = Sprite_DrawEntity;
        child->spawnParam0 = i;
    }
}

/**
 * @brief Move a mirrored death part with horizontal acceleration, blink
 * from the timer bit, then request deletion after two countdowns.
 * Preserve decrement-to-0xFF tests. Exact visual unresolved.
 */
void RedCrowDeathPart_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
            entity->velX.whole = 0x4000;
            entity->velY.whole = 0x2000;
            entity->accelX.whole = -0x300;
            if (entity->spawnParam0 != 0)
            {
                entity->velX.whole = -entity->velX.whole;
                entity->accelX.whole = -entity->accelX.whole;
                entity->flipX = 1;
            }
            entity->timer = 0x30;
            entity->state++;
        case 1:
            if ((u8)--entity->timer == 0xFF)
            {
                entity->objMode = 1;
                entity->timer = 0x18;
                entity->state++;
            }
            break;
        case 2:
            if ((u8)--entity->timer == 0xFF)
                entity->deleteRequested = 1;
            break;
    }
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
    entity->hidden = entity->timer % 2;
}

/**
 * @brief CA544 | Same as EnemyBlueCrowUpdate.
 */
void EnemyRedCrowUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;

    if (entity->state == 3)
    {
        RedCrow_UpdateBestiaryPreview(entity);
    }
    else if (entity->state == 2)
    {
        RedCrow_UpdateDeath(entity);
    }
    else
    {
        Player_TrackEntity(entity);
        if (Enemy_IsStunned(entity))
        {
            Hit_CheckWithPlayer(&result, entity);
        }
        else
        {
            RedCrow_UpdateMovement(entity);
            Entity_AnimateWithBoxes(entity);
            Hit_CheckWithPlayer(&result, entity);
        }
    }
}

/**
 * @brief Bestiary state (3): select animation 0 once and update it.
 */
void RedCrow_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
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
 * @brief Apply enemy damage; on defeat enter death state 2 with the
 * substate counters reset. The death helper owns reward drops.
 */
void RedCrow_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
    }
}
