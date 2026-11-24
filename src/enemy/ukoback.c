/*
 * Ukoback.
 * Built without -f2003-patch (the Ukoback animation / contact file needs it off; the rest compiles the same either way).
 */
#include "ukoback.h"
#include "bg_and_collision.h"
#include "core.h"
#include "player_core.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "nemesis.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "math_inline.h"
#include "damage_numbers.h"
#include "sound.h"
#include "entity_hit.h"

extern s32 Entity_IsInRect(struct EwramData_EntityData *entity, s16 left, s16 top, u16 width, u16 height);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);

s32 Ukoback_SpawnFireball(struct EwramData_EntityData *entity);
s32 Ukoback_SpawnDeathPiece(struct EwramData_EntityData *entity);
void UkobackFireball_OnContact(struct EwramData_EntityData *entity);
void UkobackFireball_OnHit(struct EwramData_EntityData *entity);
void UkobackSparkle_Update(struct EwramData_EntityData *entity);
void Ukoback_UpdateState1Animation(struct EwramData_EntityData *entity);
void Nemesis_UpdateAppearance(struct EwramData_EntityData *entity);
s32 Nemesis_LoadGraphics(struct EwramData_EntityData *entity);

/* Ukoback flight: room-edge and tile collision, facing and timer updates. */

/* 1 when the tile coordinate lies outside a room of the given size in
 * screens (one screen: single tiles, otherwise 32 tiles per screen). */
static inline s32 IsTileOutsideRoom(s32 tile, u8 screens, s32 single)
{
    s32 limit = single;

    if (screens != 1)
        limit = screens << 5;
    if (tile < 0)
        return 1;
    return tile >= limit;
}

/**
 * @brief Fly diagonally, bounce at room boundaries and collision probes, and periodically spawn a fireball while fewer than eight are counted.
 */
void Ukoback_UpdateFlight(struct EwramData_EntityData *entity)
{
    u16 probe;
    u8 cells;
    /* Keep collision displacement in r2; the local sums below use r0. */
    register s32 push asm("r2"); /* FAKEMATCH: pinned register */
    s16 roomY;

    if (entity->ext.ukoback.attackTimer != 0)
        entity->ext.ukoback.attackTimer--;
    else
        entity->ext.ukoback.attackTimer = RandomNumberGenerator() % 360 + 80;
    switch (entity->subState)
    {
        case 0:
            entity->velX.whole = entity->velY.whole = 0xC000;
            if ((s16)Entity_GetTargetDistanceX(entity) > 0)
                entity->velX.whole = -entity->velX.whole;
            if ((s16)Entity_GetTargetDistanceY(entity) < 0)
                entity->velY.whole = -entity->velY.whole;
            entity->subState++;
        case 1:
            if (entity->velX.whole < 0)
                entity->flipX = 0;
            else
                entity->flipX = 1;

            probe = GetEntityRoomYPositionInteger(entity) + entity->velY.part16.integer;
            cells = gEwramData->bgInfo[1].pBgMetadata->widthCells;
            if (IsTileOutsideRoom(GetEntityRoomXPositionInteger(entity) / 8, cells, 30))
            {
                entity->velX.whole = -entity->velX.whole;
                entity->posX.whole += entity->velX.whole * 2;
            }
            else
            {
                push = Collision_GetPushRightFiltered(GetEntityRoomXPositionInteger(entity) - 8, (s16)probe, 1);
                if (push == 0)
                    push = Collision_GetPushLeftFiltered(GetEntityRoomXPositionInteger(entity) + 8, (s16)probe, 1);
                if (push != 0)
                {
                    { register s32 position asm("r0") = push + (u16)entity->posX.part16.integer; entity->posX.part16.integer = position; } /* FAKEMATCH: pinned register */
                    entity->velX.whole = -entity->velX.whole;
                }
            }

            probe = GetEntityRoomXPositionInteger(entity) + entity->velX.part16.integer;
            cells = gEwramData->bgInfo[1].pBgMetadata->heightCells;
            if (IsTileOutsideRoom(GetEntityRoomYPositionInteger(entity) / 8, cells, 26))
            {
                entity->velY.whole = -entity->velY.whole;
                entity->posY.whole += entity->velY.whole * 2;
            }
            else
            {
                roomY = GetEntityRoomYPositionInteger(entity);
                push = Collision_GetPushDownFiltered((s16)probe, roomY - 5, 1);
                if (push == 0)
                {
                    roomY = GetEntityRoomYPositionInteger(entity);
                    push = Collision_GetPushUpFiltered((s16)probe, roomY + 1, 1);
                }
                if (push != 0)
                {
                    { register s32 position asm("r0") = push + (u16)entity->posY.part16.integer; entity->posY.part16.integer = position; } /* FAKEMATCH: pinned register */
                    entity->velY.whole = -entity->velY.whole;
                }
            }
            break;
    }
    if (entity->ext.ukoback.attackTimer == 0 && entity->ext.ukoback.fireballCount <= 7
        && Ukoback_SpawnFireball(entity))
        entity->ext.ukoback.fireballCount++;
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
}

/* Ukoback functions from 0x080C5F10. */

/**
 * @brief Drop rewards, spawn a falling piece and fall while running the fragment-death effect; request deletion below Y 170.
 */
void Ukoback_UpdateDeath(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            PlaySong(0x72);
            Enemy_DropRewards(entity);
            entity->palette = entity->basePalette;
            entity->timer = 50;
            entity->state = 0xFF;
            entity->subState++;
            entity->animFrame = 9;
            entity->velY.whole = 0x8000;
            Ukoback_SpawnDeathPiece(entity);
        case 1:
            entity->posY.whole += entity->velY.whole;
            if (entity->posY.part16.integer > 170)
                entity->deleteRequested = 1;
            break;
    }
    Enemy_UpdateFragmentDeathWithSpread(entity, 7, 0x100000, 0xC0000);
}

/**
 * @brief Initialize owner fields, graphics, collision callbacks and affine scaling; return 0 if affine allocation fails, otherwise 1.
 */
s32 UkobackFireball_Initialize(struct EwramData_EntityData *entity, struct EwramData_EntityData *ukoback)
{
    void *palette = sUkobackPalette;
    void *sprite = sUkobackSpriteSheet;
    u8 *graphics = sUkobackGfx;
    s32 result = 1;
    s32 gfx;
    s32 slot;

    entity->scrollWithBg = 1;
    entity->spriteLayer = ukoback->spriteLayer + 2;
    entity->timer = 0;
    entity->attack = ukoback->attack;
    entity->hp = ukoback->hp;
    entity->ext.ukobackFireball.scaleX = 0x10000;
    entity->ext.ukobackFireball.scaleY = 0x10000;
    entity->ext.ukobackFireball.frame = 0;
    entity->basePalette = ObjPalette_FindOrLoad(palette);
    entity->ext.ukobackFireball.ownerIndex = ENTITY_INDEX_PAIR(ukoback);
    entity->posX.whole = ukoback->posX.whole;
    entity->posY.whole = ukoback->posY.whole;
    entity->velX.whole = 0;
    entity->accelX.whole = -0x100;
    entity->velY.whole = 0;
    entity->accelY.whole = -0x100;
    gfx = ObjGfx_Find(graphics);
    Sprite_Attach(entity, sprite, gfx, (s8)ObjPalette_FindOrLoad(palette));
    entity->drawFunc = Sprite_DrawEntity;
    Anim_StartIndexed(entity, 1, 0, 1);
    Anim_Update(entity);
    Hit_Enable(entity, 8, UkobackFireball_OnContact, UkobackFireball_OnHit);
    HitBox_SetAttackRaw(entity, 0x0C08F6FC);
    HitBox_SetBodyRaw(entity, 0x0C08F6FC);
    slot = ObjAffine_Alloc(1);
    if (slot < 0)
    {
        result = 0;
    }
    else
    {
        entity->affineIndex = slot;
        ObjAffine_Set(0, entity->ext.ukobackFireball.scaleX, entity->ext.ukobackFireball.scaleY, 0, 0, entity->affineIndex);
        entity->affine = 1;
    }
    if (Entity_IsInRect(entity, -128, -64, 496, 288))
        PlaySong(0x78);
    return result;
}

/**
 * @brief Check collisions and lifetime; dissolve by changing affine scales, then free the slot, decrement the owner fireball count and request deletion.
 */
void UkobackFireball_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    struct EwramData_EntityData *other = &gEwramData->entityData[entity->ext.ukobackFireball.ownerIndex];

    Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity) && entity->state != 0xFF)
    {
        Hit_CheckWithPlayer(&result, entity);
        return;
    }
    if (entity->state == 0xFF)
    {
        ObjAffine_Set(0, entity->ext.ukobackFireball.scaleX, entity->ext.ukobackFireball.scaleY, 0, 0, entity->affineIndex);
        if (entity->ext.ukobackFireball.scaleX > 0)
        {
            entity->ext.ukobackFireball.scaleX += entity->velX.whole;
            entity->velX.whole += entity->accelX.whole;
        }
        else
        {
            entity->ext.ukobackFireball.scaleX = 0;
        }
        entity->ext.ukobackFireball.scaleY += entity->velY.whole;
        entity->velY.whole += entity->accelY.whole;
        if (entity->ext.ukobackFireball.scaleX == 0)
        {
            ObjAffine_Free(entity->affineIndex);
            entity->deleteRequested = 1;
            other->ext.ukoback.fireballCount--;
            return;
        }
    }
    else if ((s16)entity->ext.ukobackFireball.frame++ > 0x3FF)
    {
        entity->state |= 0xFF;
    }
    if (entity->state != 0xFF)
    {
        Anim_Update(entity);
        Hit_CheckWithPlayer(&result, entity);
    }
}

/**
 * @brief Start four pixels below the owner with 1.5 times its vertical velocity, fall and apply the visibility pattern until deletion.
 */
void UkobackDeathPiece_Update(struct EwramData_EntityData *entity)
{
    void *palette = sUkobackPalette;
    void *sprite = sUkobackSpriteSheet;
    u8 *graphics = sUkobackGfx;
    struct EwramData_EntityData *other = &gEwramData->entityData[entity->ext.ukobackPiece.ownerIndex];
    s32 gfx;

    switch (entity->state)
    {
        case 0:
            entity->scrollWithBg = 1;
            gfx = ObjGfx_Find(graphics);
            Sprite_Attach(entity, sprite, gfx, (s8)ObjPalette_FindOrLoad(palette));
            entity->drawFunc = Sprite_DrawEntity;
            entity->animFrame = 10;
            entity->spriteLayer = other->spriteLayer + 1;
            entity->posX.whole = other->posX.whole;
            entity->posY.whole = other->posY.whole + 0x40000;
            entity->velY.whole = other->velY.whole * 3 / 2;
            entity->timer = 0;
            entity->state++;
            break;
        case 1:
            entity->posY.whole += entity->velY.whole;
            if (entity->timer++ > 50 || entity->posY.part16.integer > 170)
                entity->state = 2;
            break;
        case 2:
            entity->deleteRequested = 1;
            break;
    }
    entity->hidden = Entity_GetVisibilityPattern(entity->timer);
}

/**
 * @brief Create a player-following sparkle with random offsets; initial Y deliberately uses player X minus 16 pixels, as in the original.
 */
void Ukoback_SpawnContactSparkle(void)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, UkobackSparkle_Update);

    if (child != NULL)
    {
        s32 x = gEwramData->playerState.player->posX.whole;
        s32 y = x - 0x100000;
        s32 range = 0x100000;

        child->ext.ukobackSparkle.offsetX = RandomOffset(range);
        child->ext.ukobackSparkle.offsetY = RandomOffset(range);
        child->posX.whole = x + child->ext.ukobackSparkle.offsetX;
        child->posY.whole = y + child->ext.ukobackSparkle.offsetY;
        child->flipX = RandomNumberGenerator() % 2;
        child->spriteLayer = 9;
    }
}

/**
 * @brief Attach cached sparkle graphics, wait a random hidden interval, then show and follow the player offset until the animation finishes.
 */
void UkobackSparkle_Update(struct EwramData_EntityData *entity)
{
    s32 gfx;

    switch (entity->state)
    {
        case 0:
            gfx = ObjGfx_Find(sCommonObjectGfx);
            Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, 7);
            Anim_StartIndexed(entity, 0, 0, 0);
            entity->scrollWithBg = 1;
            entity->drawFunc = Sprite_DrawEntity;
            entity->state++;
            entity->animFlags |= 2;
            entity->hidden = 1;
            entity->timer = RandomNumberGenerator() % 64;
            break;
        case 1:
            if ((s8)entity->timer-- == 0)
            {
                entity->animFlags &= ~2;
                entity->hidden = 0;
                entity->state++;
            }
            break;
        case 2:
            break;
    }
    entity->posX.whole = gEwramData->playerState.player->posX.whole + entity->ext.ukobackSparkle.offsetX;
    entity->posY.whole = gEwramData->playerState.player->posY.whole - 0x100000 + entity->ext.ukobackSparkle.offsetY;
    if (Anim_Update(entity) == 3)
        EntityDelete(entity);
}

/**
 * @brief C6480 | Ukoback update: state 1 only runs Ukoback_UpdateState1Animation.
 * Otherwise Player_TrackEntity, collision and done when Enemy_IsStunned holds
 * outside state 0xFF; run the state (0 Ukoback_UpdateFlight, 0xFF Ukoback_UpdateDeath),
 * Entity_AnimateWithBoxes unless dying, and Enemy_CheckHitPlayer.
 */
void EnemyUkobackUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;

    if (entity->state == 1)
    {
        Ukoback_UpdateState1Animation(entity);
        return;
    }
    Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity) && entity->state != 0xFF)
    {
        Hit_CheckWithPlayer(&result, entity);
        return;
    }
    switch (entity->state)
    {
        case 0:
            Ukoback_UpdateFlight(entity);
            break;
        case 0xFF:
            Ukoback_UpdateDeath(entity);
            break;
    }
    if (entity->state != 0xFF)
        Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}

/* Ukoback helpers 0x080C64E8..0x080C664B. */

/**
 * @brief Start animation 0 once and then advance it. The gameplay purpose of state 1 remains unresolved.
 */
void Ukoback_UpdateState1Animation(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->subState++;
    }
    Anim_Update(entity);
}

/**
 * @brief Submit the entity attack contact record with mode fields 0, 0, 0.
 */
void Ukoback_OnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief Resolve damage, subtract health, show damage and flash; start the death sequence on lethal damage.
 */
void Ukoback_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    {
        u16 hp = entity->hp;
        damage = (damage << 16) >> 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, (s16)damage, 0);
    entity->hitFlashTimer = 8;
    if ((s16)entity->hp <= 0)
    {
        entity->subState = 0;
        Ukoback_UpdateDeath(entity);
    }
}

/**
 * @brief Allocate and initialize a fireball; delete it if initialization fails and return 1 only on success.
 */
s32 Ukoback_SpawnFireball(struct EwramData_EntityData *entity)
{
    s32 created = 0;
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, UkobackFireball_Update);

    if (child != NULL)
    {
        if (!UkobackFireball_Initialize(child, entity))
            EntityDelete(child);
        else
            created = 1;
    }
    return created;
}

/**
 * @brief Submit the attack contact record, create ten player-following sparkles and enter dissolving state 0xFF with hit flash.
 */
void UkobackFireball_OnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;
    s32 i;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
    for (i = 9; i >= 0; i--)
        Ukoback_SpawnContactSparkle();
    entity->hitFlashTimer = 8;
    entity->state = 0xFF;
}

/**
 * @brief Set an eight-tick hit flash and enter dissolving state 0xFF.
 */
void UkobackFireball_OnHit(struct EwramData_EntityData *entity)
{
    entity->hitFlashTimer = 8;
    entity->state = 0xFF;
}

/**
 * @brief Create a falling death piece with the owner slot stored in its parent field; return 1 on success, otherwise 0.
 */
s32 Ukoback_SpawnDeathPiece(struct EwramData_EntityData *entity)
{
    s32 created = 0;
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, UkobackDeathPiece_Update);

    if (child != NULL)
    {
        child->ext.ukobackPiece.ownerIndex = ENTITY_INDEX_PAIR(entity);
        created = 1;
    }
    return created;
}

/* Nemesis create and graphics loader 0x080C664C..0x080C67CB. The loader only matches without
 * -f2003-patch. */

/**
 * @brief C664C | Nemesis create: load the graphics (delete the enemy when
 * that fails), hit boxes, layer 3; state 7 when bestiaryPreview is set.
 * Otherwise ends (0x53D bit 3) when Nemesis_CreatePartner fails, sets the hit box
 * callbacks and boxes, unk_4FA 0, unk_4FB 9, unk_508 3, faces left and
 * runs Nemesis_UpdateAppearance, Nemesis_CreateAfterimages and Nemesis_SetAfterimagesHidden(1).
 */
void EnemyNemesisCreate(struct EwramData_EntityData *entity)
{
    if (!Nemesis_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 3;
    if (gEwramData->gameState.bestiaryPreview)
    {
        if (entity->state != 7)
        {
            entity->state = 7;
            entity->stage = 0;
            entity->subState = 0;
        }
        return;
    }
    if (!Nemesis_CreatePartner(entity))
        entity->deleteRequested = 1;
    Hit_Enable(entity, 8, Nemesis_OnContact, Nemesis_OnHit);
    HitBox_SetAttackRaw(entity, 0x080804FC);
    HitBox_SetBodyRaw(entity, 0x1010F8F8);
    entity->animFrame = 0;
    entity->ext.nemesis.attackSet = 0;
    entity->ext.nemesis.attackIndex = 9;
    entity->ext.nemesis.hitsBeforeReact = 3;
    entity->flipX = 0;
    Nemesis_UpdateAppearance(entity);
    Nemesis_CreateAfterimages(entity);
    Nemesis_SetAfterimagesHidden(entity, 1);
}

/**
 * @brief Find or load graphics and palette, attach the sprite and allocate an affine slot; return 0 on failure, 1 on success.
 */
s32 Nemesis_LoadGraphics(struct EwramData_EntityData *entity)
{
    u8 *graphics = sNemesisGfx;
    s32 gfx = ObjGfx_Find(graphics);
    s8 paletteIndex;
    s32 slot;

    if (gfx < 0)
    {
        gfx = ObjGfx_LoadResource(graphics, 0);
        if (gfx < 0)
            return 0;
    }
    paletteIndex = ObjPalette_FindOrLoadVariant(sNemesisPalette, 0);
    if (paletteIndex == -1)
        return 0;
    entity->basePalette = paletteIndex;
    if (!Sprite_Attach(entity, sNemesisSpriteSheet, gfx, entity->basePalette))
        return 0;
    slot = ObjAffine_Alloc(1);
    if (slot < 0)
        return 0;
    entity->affineIndex = slot;
    entity->affine = 1;
    ObjAffine_Set(0, 0x10000, 0x10000, 0, 0, entity->affineIndex);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief C67CC | Nemesis update: state 7 Nemesis_UpdateState7, 0xFF Nemesis_UpdateDeath.
 * Otherwise Player_TrackEntity; when Enemy_IsStunned holds, it waits in state 0
 * unless Entity_IsTargetWithinRange(0x50 pixels) and else only collides (unless unk_4FC
 * bit 5). Runs the state (0..6), then collision unless unk_4FC bit 5.
 */
void EnemyNemesisUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;

    if (entity->state == 7)
    {
        Nemesis_UpdateState7(entity);
        return;
    }
    if (entity->state == 0xFF)
    {
        Nemesis_UpdateDeath(entity);
        return;
    }
    Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity))
    {
        if (!Entity_IsTargetWithinRange(entity, 0x500000) && entity->state == 0)
            return;
        if (!(entity->ext.nemesis.flags & 0x20) && entity->state != 0)
        {
            Hit_CheckWithPlayer(&result, entity);
            return;
        }
    }
    switch (entity->state)
    {
        case 0:
            Nemesis_UpdateAppearance(entity);
            break;
        case 1:
            Nemesis_UpdatePositioning(entity);
            break;
        case 2:
            Nemesis_UpdateAttackSelection(entity);
            break;
        case 3:
            Nemesis_UpdateTurningAttack(entity);
            break;
        case 4:
            Nemesis_UpdateScriptedAttack(entity);
            break;
        case 5:
            Nemesis_UpdateSpinLunge(entity);
            break;
        case 6:
            Nemesis_UpdatePartnerFlight(entity);
            break;
    }
    if (!(entity->ext.nemesis.flags & 0x20))
        Hit_CheckWithPlayer(&result, entity);
}

/* (u8)(t % 3) != 0 (hidden on two of three frames), written so that agbcc
 * emits `negs` + `lsrs #31` like the original. */
#define IS_BLINK_FRAME(t) (-(s32)(u8)((t) % 3) < 0)

/**
 * @brief Hide the entity and partner until the proximity check succeeds, then run the timed blinking appearance sequence and enter state 1.
 */
void Nemesis_UpdateAppearance(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *other = &gEwramData->entityData[entity->ext.nemesis.partnerIndex];
    s32 x;

    switch (entity->subState)
    {
        case 0:
            entity->ext.nemesis.angle = 0x1000;
            entity->hidden = 1;
            other->hidden = 1;
            Nemesis_SetAfterimagesHidden(entity, 1);
            other->state = 5;
            entity->timer = 0;
            entity->subState++;
        case 1:
            if (!Entity_IsTargetWithinRange(entity, 0x500000))
                break;
            x = entity->posX.whole;
            other->posX.whole = x + 0xA0000;
            if (!other->flipX)
                other->posX.whole = x + 0xD0000;
            other->posY.whole = entity->posY.whole + 0x100000;
            ObjAffine_Set(entity->ext.nemesis.angle, 0x10000, 0x10000, 0, 0, entity->affineIndex);
            entity->subState++;
            break;
        case 2:
            entity->hidden = IS_BLINK_FRAME(entity->timer);
            other->hidden = IS_BLINK_FRAME(entity->timer);
            entity->objMode = 1;
            if (entity->timer++ > 40)
            {
                entity->hidden = 0;
                other->hidden = 0;
                Nemesis_SetAfterimagesHidden(entity, 0);
                entity->objMode = 0;
                entity->timer = 0;
                entity->subState++;
            }
            break;
        case 3:
            if (entity->timer++ > 20)
            {
                entity->timer = 0;
                entity->subState++;
            }
            break;
        case 4:
            other->hidden = IS_BLINK_FRAME(entity->timer);
            if (entity->timer++ > 40)
            {
                other->hidden = 1;
                if (other->state != 0)
                {
                    other->state = 0;
                    other->stage = 0;
                    other->subState = 0;
                }
                entity->timer = 0;
                entity->subState++;
            }
            break;
        case 5:
            if (entity->timer++ > 20)
            {
                entity->timer = 0;
                if (entity->state != 1)
                {
                    entity->state = 1;
                    entity->stage = 0;
                    entity->subState = 0;
                }
            }
            break;
    }
}
