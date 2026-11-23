/*
 * Chronomage: body and cast effect.
 */
#include "core.h"
#include "graphics.h"
#include "gameplay.h"
#include "in_game.h"
#include "map_screen.h"
#include "map_flags.h"
#include "sound.h"
#include "player_core.h"
#include "player_entity.h"
#include "entity_util.h"
#include "structs/ewram.h"

/* Chronomage affine casting effect. */
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Chronomage_UpdateWait(struct EwramData_EntityData *);
extern void Chronomage_UpdateCast(struct EwramData_EntityData *);
extern void Chronomage_UpdateEscape(struct EwramData_EntityData *);
extern void Chronomage_UpdateBestiaryPreview(struct EwramData_EntityData *);
extern void Chronomage_OnHit(struct EwramData_EntityData *);
extern u8 sChronomageGfx[], sChronomagePalette[], sChronomageSpriteSheet[];

extern u8 sChronomageDestinationRoom[];

/**
 * @brief Allocate an affine sprite, grow its scale, hold for 32 updates, then
 * shrink and release it. Allocation failure removes the effect.
 */
void ChronomageCastEffect_Update(struct EwramData_EntityData *entity)
{
    u16 affine;
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sChronomageGfx, sChronomagePalette, 0, sChronomageSpriteSheet)) goto remove;
        affine = ObjAffine_Alloc(1);
        if ((s16)affine < 0) goto remove;
        entity->affineIndex = affine;
        entity->affine = 1;
        entity->spriteLayer = 0x12;
        Anim_StartIndexed(entity, 6, 0, 0);
        Anim_Update(entity);
        entity->state = 1;
        /* fall through */
    case 1:
        if ((s32)entity->ext.chronomageCast.scale <= 0xFFFF) entity->ext.chronomageCast.scale += 0x1000;
        else
        {
            entity->state = 2;
            entity->timer = 0x20;
        }
        break;
    case 2:
        if (entity->timer != 0) entity->timer--;
        else entity->state = 3;
        break;
    case 3:
        if ((s32)entity->ext.chronomageCast.scale > 0)
        {
            entity->ext.chronomageCast.scale -= 0x2000;
            break;
        }
        /* fall through */
    case 4:
        if (entity->affine) ObjAffine_Free(entity->affineIndex);
remove:
        EntityDelete(entity);
        return;
    }
    ObjAffine_Set(0, entity->ext.chronomageCast.scale, entity->ext.chronomageCast.scale, 0, 0, entity->affineIndex);
}

/* Chronomage waiting-state transitions. */
extern s32 Target_GetX(struct EwramData_EntityData *);
extern u8 sChronomageWalkProfile[];
#define ABS_CHRONOMAGE(v) ((v) < 0 ? -(v) : (v))

/**
 * @brief Wait state (0): enter escape while the global stop timer is active;
 * otherwise begin casting on player proximity or player flags 0x360.
 */
void Chronomage_UpdateWait(struct EwramData_EntityData *entity)
{
    s32 playerX = (s16)gEwramData->playerState.player->posX.part16.integer;
    s16 distance;
    if (playerX - (s16)entity->posX.part16.integer < 0) distance = (u16)entity->posX.part16.integer - playerX;
    else distance = playerX - (u16)entity->posX.part16.integer;
    if (gEwramData->gameState.timeStopTimer != 0)
    {
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
    }
    else if (distance <= 0x6F || (gEwramData->playerState.player->ext.player.actionFlags & 0x360))
    {
        entity->state = 1;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/* Chronomage casting and transition sequence. */
/**
 * @brief Cast state (1): sequence animations, sound and palette changes.
 * Branch on the global stop timer and Player_HasAbilityOneBased(6); either start
 * the transition through RoomJump_SetDestination or spawn the affine effect and escape.
 * The meaning of the player flags and transition data remains unresolved.
 */
void Chronomage_UpdateCast(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *effect;
    switch (entity->subState)
    {
    case 0:
        if (gEwramData->playerState.player->ext.player.actionFlags & 0x360)
        {
            entity->subState = 2;
            break;
        }
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->subState = 1;
        /* fall through */
    case 1:
        if (entity->animFlags & 4) entity->subState = 2;
        break;
    case 2:
        Anim_StartIndexed(entity, 3, 0, 0);
        entity->subState = 3;
        entity->timer = 0x18;
        break;
    case 3:
        if (entity->timer != 0)
        {
            if (--entity->timer == 8) PlaySong(0xEC);
        }
        else
        {
            entity->timer = 0;
            entity->subState = 4;
        }
        break;
    case 4:
        if (gEwramData->gameState.timeStopTimer == 0)
        {
            gEwramData->paletteInvert = entity->timer << 12;
            if (entity->timer <= 0xF)
            {
                entity->timer++;
                break;
            }
            if (!Player_HasAbilityOneBased(6)) goto menu;
        }
        BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x8000, 0x10000, 0x10);
        entity->subState = 6;
        break;
menu:
        entity->subState = 5;
        Game_SetMenusEnabled(0);
        break;
    case 5:
        RoomJump_SetDestination(sChronomageDestinationRoom, 0, 0x100, 0x20, 0x70);
        RoomJump_Begin(&gEwramData->gameState);
        break;
    case 6:
        gEwramData->paletteInvert = 1;
        if (!(entity->animFlags & 4)) break;
        effect = EntityCreateInRange(0x49, 0xCF, (u32 *)ChronomageCastEffect_Update);
        if (effect != NULL)
        {
            effect->posX.part16.integer = entity->posX.part16.integer;
            effect->posY.part16.integer = (u16)entity->posY.part16.integer - 0x20;
            effect->scrollWithBg = 1;
        }
        entity->subState = 7;
        entity->timer = 0x30;
        /* fall through */
    case 7:
        if (entity->timer != 0) entity->timer--;
        else
        {
            entity->state = 2;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    }
}

/* Chronomage escape, bestiary preview, creation and state dispatch. */
/**
 * @brief Escape state (2): disable hit flag 4, animate and accelerate away
 * from the target, jumping at terrain obstacles. Request deletion offscreen.
 */
void Chronomage_UpdateEscape(struct EwramData_EntityData *entity)
{
    s32 speed;
    s32 flags;
    switch (entity->subState)
    {
    case 0:
        entity->hit.flags &= ~4;
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->subState = 1;
        /* fall through */
    case 1:
        if (entity->animFlags & 4) entity->subState = 2;
        break;
    case 2:
        entity->flipX = 0;
        entity->accelY.whole = 0x2000;
        if (Target_GetX(entity) > entity->posX.whole) Anim_StartIndexed(entity, 4, 0, 1);
        else Anim_StartIndexed(entity, 5, 0, 1);
        entity->subState = 3;
        /* fall through */
    case 3:
        if ((s32)entity->ext.chronomage.escapeSpeed <= 0x2FFFF) entity->ext.chronomage.escapeSpeed += 0x2000;
        if (entity->animScriptIndex == 4) { speed = entity->ext.chronomage.escapeSpeed; speed = -speed; }
        else speed = entity->ext.chronomage.escapeSpeed;
        flags = Entity_Walk(entity, speed, sChronomageWalkProfile, 2);
        if ((flags & 8) && (flags & 3)) entity->velY.whole -= 0x30000;
        break;
    }
    if (EntityIsOutsideScreen(entity, 3)) entity->deleteRequested = 1;
}

/**
 * @brief Bestiary state (4): wait 128 updates, then sequence animations 1
 * and 2. Updates the animation internally.
 */
void Chronomage_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->subState = 1;
        entity->timer = 0x80;
        break;
    case 1:
        if (entity->timer != 0) entity->timer--;
        else
        {
            Anim_StartIndexed(entity, 1, 0, 0);
            entity->subState = 2;
        }
        break;
    case 2:
        if (entity->animFlags & 4)
        {
            Anim_StartIndexed(entity, 2, 0, 0);
            entity->subState = 3;
        }
        break;
    }
    Anim_Update(entity);
}

void EnemyChronomageCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sChronomageGfx, sChronomagePalette, 0, sChronomageSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x13;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview) entity->state = 4;
    else
    {
        Hit_Enable(entity, 8, Enemy_OnContact, (s32)Chronomage_OnHit);
        HitBox_SetBodyRaw(entity, 0x2008E0FC);
        HitBox_SetAttackRaw(entity, 0x2008E0FC);
        entity->hit.flags |= 4;
        Entity_SnapToGround(entity);
        entity->posY.part16.integer++;
        entity->scrollWithBg = 1;
        entity->state = 0;
    }
}

void EnemyChronomageUpdate(struct EwramData_EntityData *entity)
{
    u32 box[4];
    s32 done;
    if (entity->state != 3) Enemy_UpdateHitFlash(entity);
    switch (entity->state)
    {
    case 0: Chronomage_UpdateWait(entity); break;
    case 1: Chronomage_UpdateCast(entity); break;
    case 2: Chronomage_UpdateEscape(entity); break;
    case 3:
        switch (entity->subState)
        {
        case 0:
            Anim_StartIndexed(entity, 1, 0, 0);
            entity->subState = 1;
            entity->timer = 0x28;
            goto alive;
        case 1:
            Enemy_UpdateFragmentDeath(entity);
            if (entity->animFlags & 4)
            {
                done = 1;
                break;
            }
        default:
alive:
            done = 0;
            break;
        }
        if (done)
        {
            EntityDelete(entity);
            return;
        }
        break;
    case 4:
        Chronomage_UpdateBestiaryPreview(entity);
        return;
    }
    if (entity->state != 3) Hit_CheckWithPlayer(box, entity);
    Anim_Update(entity);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Chronomage_OnHit = SkullMillioneFamily_OnHit */

extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief Apply enemy damage; on defeat drop rewards, restore the palette
 * and enter death state 3 with the substate counters reset.
 */
void Chronomage_OnHit(struct EwramData_EntityData *entity)
{
    if (Enemy_TakeDamage(entity))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->state = 3;
        entity->stage = 0;
        entity->subState = 0;
    }
}
