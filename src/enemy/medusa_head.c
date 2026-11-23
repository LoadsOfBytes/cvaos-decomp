/*
 * Medusa Head.
 */
#include "core.h"
#include "player_core.h"
#include "graphics.h"
#include "entity_util.h"
#include "structs/ewram.h"
#include "structs/entity_template.h"
#include "gameplay.h"
#include "sound.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"

extern s32 Target_GetPixelX(struct EwramData_EntityData *entity);
extern void MedusaHead_UpdateFly(struct EwramData_EntityData *entity);
extern void MedusaHead_UpdateIdle(struct EwramData_EntityData *entity);
extern void EnemyMedusaHeadCreate(struct EwramData_EntityData *entity);
extern s32 Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);

/**
 * @brief 93714 | Medusa Head update: a head (states 0 / 1, MedusaHead_UpdateFly /
 * MedusaHead_UpdateIdle) dies off screen (state 2) and then decrements the head
 * count of its spawner (unk_500, enemyId 0x39); the spawner (state 3)
 * stays at x 120 and with 1/32 chance creates a head below the limit
 * spawnParam0: x off the left or right room edge depending on Target_GetPixelX /
 * Target_GetFlipXOrOpposite, y 64..127, template unk_C / unk_E from sEnemyData.
 */
void EnemyMedusaHeadUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct EwramData_EntityData *parent;
    struct EwramData_EntityData *head;
    s32 x;

    if (entity->state != 3)
    {
        if (entity->state != 2 && Entity_IsInRect(entity, -16, 0, 272, 176))
            Player_TrackEntity(entity);
        if (EntityIsOutsideScreen(entity, 4))
        {
            entity->state = 2;
            entity->timer = 1;
        }
    }
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 2)
            Hit_CheckWithPlayer(collision, entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        MedusaHead_UpdateFly(entity);
        break;
    case 1:
        MedusaHead_UpdateIdle(entity);
        break;
    case 2:
        Enemy_UpdateFragmentDeath(entity);
        if (entity->deleteRequested)
        {
            parent = entity->ext.medusaHead.spawner;
            if (parent != NULL && parent->updateFunc != NULL && parent->enemyId == 0x39
                && parent->ext.medusaHead.childCount != 0)
                parent->ext.medusaHead.childCount--;
        }
        break;
    case 3:
        entity->posX.whole = 0x780000;
        if ((RandomNumberGenerator() & 0x1F) == 0 && (s32)entity->ext.medusaHead.childCount < entity->spawnParam0)
        {
            head = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
            if (head != NULL)
            {
                entity->ext.medusaHead.childCount++;
                if ((s16)Target_GetPixelX(entity) <= 0x67)
                    head->posX.whole = 0x1000000;
                else if ((s16)Target_GetPixelX(entity) <= 0x88 && (u8)Target_GetFlipXOrOpposite(entity) == 0)
                    head->posX.whole = 0x1000000;
                else
                    head->posX.whole = -0x100000;
                x = (RandomNumberGenerator() & 0x3F) + 0x40;
                head->posY.part16.integer = x;
                head->scrollWithBg = 1;
                head->spawnIndex = 0;
                head->enemyId = entity->enemyId;
                head->spawnParam1 = entity->spawnParam1 & 1;
                head->drawFunc = (u32 *)Sprite_DrawEntity;
                head->hp = sEnemyData[entity->enemyId].hp;
                head->mp = sEnemyData[entity->enemyId].mp;
                head->updateFunc = entity->updateFunc;
                EnemyMedusaHeadCreate(head);
                head->ext.medusaHead.spawner = entity;
            }
        }
        return;
    case 4:
        Anim_Update(entity);
        return;
    }
    Anim_Update(entity);
    Enemy_CheckHitPlayer(entity);
}

extern s32 MedusaHead_LoadGraphics(struct EwramData_EntityData *entity);
extern void MedusaHead_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void MedusaHead_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

/**
 * @brief 938E8 | Medusa Head create: deleted when MedusaHead_LoadGraphics fails;
 * frozen time gives state 4; otherwise palette 3, box slot 8 (MedusaHead_DamagePlayerOnContact
 * / MedusaHead_OnHit) with box 0x0A0AFBFB; with spawnParam1 bit 1 it is the hidden
 * spawner (state 3, boxes off), else state 0 facing Entity_IsTargetRight with velX
 * 0xC000..0xFFFF towards it, accelY 0xC00 and a random wave phase
 * (unk_504..506).
 */
void EnemyMedusaHeadCreate(struct EwramData_EntityData *entity)
{
    u32 box = 0x0A0AFBFB;
    u32 r;

    if ((u8)MedusaHead_LoadGraphics(entity) == 0)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        return;
    }
    entity->spriteLayer = 3;
    Hit_Enable(entity, 8, MedusaHead_DamagePlayerOnContact, (s32)MedusaHead_OnHit);
    HitBox_SetAttack(entity, &box);
    HitBox_SetBody(entity, &box);
    entity->hit.flags &= ~4;
    entity->hit.flags &= ~2;
    if ((s16)(entity->spawnParam1 & 2))
    {
        entity->state = 3;
        entity->hidden = 1;
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        return;
    }
    entity->state = 0;
    entity->flipX = Entity_IsTargetRight(entity);
    if (entity->flipX)
        entity->velX.whole = (RandomNumberGenerator() & 0x3FFF) + 0xC000;
    else
        entity->velX.whole = -0xC000 - (RandomNumberGenerator() & 0x3FFF);
    entity->accelY.whole = 0xC00;
    r = RandomNumberGenerator() & 1;
    entity->ext.medusaHead.waveUp = r;
    entity->ext.medusaHead.waveFrame = 0;
    r = (RandomNumberGenerator() & 0xF) + 0x2D;
    entity->ext.medusaHead.waveLength = r;
    entity->ext.medusaHead.waveFrame = entity->ext.medusaHead.waveLength / 2;
}

/* Oscillate vertical acceleration and select the rising/falling animation. */
/**
 * @brief Flight state (state 0): weave up and down (velY +- accelY, direction
 * flipped every unk_506 frames) while moving horizontally.
 */
void MedusaHead_UpdateFly(struct EwramData_EntityData *entity)
{
    if (entity->subState != 0) return;
    if ((s8)entity->ext.medusaHead.waveUp != 0) entity->velY.whole -= entity->accelY.whole;
    else entity->velY.whole += entity->accelY.whole;
    if ((s8)++entity->ext.medusaHead.waveFrame >= entity->ext.medusaHead.waveLength)
    {
        entity->ext.medusaHead.waveFrame = 0;
        entity->ext.medusaHead.waveUp ^= 1;
    }
    EntityMove(entity);
    if (entity->velY.whole >= 0)
    {
        if (entity->animScriptIndex == 2)
        {
            if (!(RandomNumberGenerator() & 1)) Anim_StartIndexed(entity, 1, 0, 1);
            else Anim_StartIndexed(entity, 0, 0, 1);
        }
    }
    else if (entity->animScriptIndex != 2) Anim_StartIndexed(entity, 2, 0, 1);
}

/* MedusaHead_OnHit (hit with HP / death), generated with a template script from the
 * template of WingedSkeleton_OnHit and checked byte for byte. */

extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

extern u8 sMedusaHeadGfx[];
extern u8 sMedusaHeadPalette[];
extern u8 sMedusaHeadSpriteSheet[];

/**
 * @brief 93AE0 | Hit: flash (hitFlashTimer = 8), subtract the damage from the HP
 * and show it; at HP <= 0 die (Enemy_DropRewards, song 0x72, animation 3, palette into
 * 0x53E bits 3-6, timer 0x28, state 2). Same template as WingedSkeleton_OnHit.
 */
void MedusaHead_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;

        // (s16)damage, written so that agbcc sign-extends after the HP load
        damage <<= 16;
        damage >>= 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x72);
        Anim_StartIndexed(entity, 3, 0, 0);
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->state = 2;
        entity->subState = 0;
    }
}

/**
 * @brief 93B68 | Load the graphics (sMedusaHeadGfx), palette 0 / 1 of
 * sMedusaHeadPalette by bit 0 of spawnParam1 and animation set sMedusaHeadSpriteSheet;
 * returns 0 when ObjGfx_LoadResource fails.
 */
s32 MedusaHead_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sMedusaHeadGfx, 0);

    if (gfx < 0)
        return 0;

    if (entity->spawnParam1 & 1)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sMedusaHeadPalette, 1);
    else
        entity->basePalette = ObjPalette_FindOrLoadVariant(sMedusaHeadPalette, 0);
    Sprite_Attach(entity, sMedusaHeadSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/* MedusaHead_UpdateIdle and MedusaHead_DamagePlayerOnContact. */
/**
 * @brief 93BCC | Sub-state 0: start the animation (Anim_StartIndexed with
 * 0, 0, 0) and advance the sub-state.
 */
void MedusaHead_UpdateIdle(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->subState++;
    }
}

/**
 * @brief 93BF0 | Player_ApplyContactDamage with (attack, 0x100 when bit 0 of spawnParam1
 * is set else 0, 0, 1) at the entity position.
 */
void MedusaHead_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    if (entity->spawnParam1 & 1)
        contactDamage.attributes = 0x100;
    else
        contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}
