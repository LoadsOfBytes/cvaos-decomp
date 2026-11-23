/*
 * Ukoback: flight, fireballs, death pieces and sparkles.
 */
#include "code_08001194.h"
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/* Ukoback flight: room-edge and tile collision, facing and timer updates. */
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *entity);
extern s32 Entity_GetTargetDistanceY(struct EwramData_EntityData *entity);
extern s32 Ukoback_SpawnFireball(struct EwramData_EntityData *entity);

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
extern void PlaySong(u16 song);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);
extern void Enemy_UpdateFragmentDeathWithSpread(struct EwramData_EntityData *entity, s32 interval, s32 width, s32 height);
extern s32 Ukoback_SpawnDeathPiece(struct EwramData_EntityData *entity);
extern s32 ObjPalette_FindOrLoad(void *palette);
extern s32 Hit_Enable(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);
extern s32 HitBox_SetAttackRaw(struct EwramData_EntityData *entity, s32 box);
extern s32 HitBox_SetBodyRaw(struct EwramData_EntityData *entity, s32 box);
extern s32 Entity_IsInRect(struct EwramData_EntityData *entity, s16 left, s16 top, u16 width, u16 height);
extern void UkobackFireball_OnContact(struct EwramData_EntityData *entity);
extern void UkobackFireball_OnHit(struct EwramData_EntityData *entity);

/* Graphics of the Ukoback fireball (UkobackFireball_Initialize). */
extern u8 sUkobackGfx[];
extern u8 sUkobackPalette[];
extern u8 sUkobackSpriteSheet[];

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
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    Anim_StartIndexed(entity, 1, 0, 1);
    Anim_Update(entity);
    Hit_Enable(entity, 8, UkobackFireball_OnContact, (s32)UkobackFireball_OnHit);
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

extern s32 Enemy_IsStunned(struct EwramData_EntityData *entity);
extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern s32 Entity_GetVisibilityPattern(s32 step);

/**
 * @brief Check collisions and lifetime; dissolve by changing affine scales, then free the slot, decrement the owner fireball count and request deletion.
 */
void UkobackFireball_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct EwramData_EntityData *other = &gEwramData->entityData[entity->ext.ukobackFireball.ownerIndex];

    Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity) && entity->state != 0xFF)
    {
        Hit_CheckWithPlayer(collision, entity);
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
        Hit_CheckWithPlayer(collision, entity);
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
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
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

extern void UkobackSparkle_Update(struct EwramData_EntityData *entity);
extern void Ukoback_UpdateState1Animation(struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);

/* Random value in -range / 2 .. range / 2 - 1. */
static inline s32 RandomOffset(s32 range)
{
    return RandomNumberGenerator() % range - range / 2;
}

/* Graphics of the sparkle (UkobackSparkle_Update). */
extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];

/**
 * @brief Create a player-following sparkle with random offsets; initial Y deliberately uses player X minus 16 pixels, as in the original.
 */
void Ukoback_SpawnContactSparkle(void)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)UkobackSparkle_Update);

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
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
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
    u8 collision[16];

    if (entity->state == 1)
    {
        Ukoback_UpdateState1Animation(entity);
        return;
    }
    Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity) && entity->state != 0xFF)
    {
        Hit_CheckWithPlayer(collision, entity);
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
