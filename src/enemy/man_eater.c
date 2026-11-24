/*
 * Man Eater: head, segments, chain anchor, projectiles and attached parts (the last part also holds Killer Mantle effects and parts).
 */
#include "man_eater.h"
#include "core.h"
#include "player_core.h"
#include "graphics.h"
#include "gameplay.h"
#include "entity_util.h"
#include "syscalls.h"
#include "sound.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
#include "music.h"
#include "player_level.h"
#include "boss_defeat.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_palette.h"
#include "effects.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "damage_numbers.h"
#include "entity_motion.h"
#include "entity_hit.h"
#include "hit_shared_history.h"


void ManEaterProjectile_DamagePlayerOnContact(struct EwramData_EntityData *);
void ManEaterProjectile_OnHit(struct EwramData_EntityData *);

struct OrbitPoint {
    u16 angle;
    s16 radius;
};
extern const struct OrbitPoint sManEaterHeadOrbitPoints[];

/* Man Eater part hit: propagate flags, subtract HP and notify parent.
 * The local r6 binding preserves the hit-data pointer before address calculation.
 * Matching does not establish the original source spelling. */

/**
 * @brief Resolve damage against the linked head, propagate hit flags and
 * subtract its HP. On defeat mark it dead and decrement the root owner
 * head count. Preserve the existing casts and hit-pointer allocation form.
 */
void ManEaterPart_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    struct EwramData_EntityData *part = &gEwramData->entityData[(u8)entity->ext.manEater.ownerIndex];
    u16 strength = Player_CalculateAttackPower(hit[2]);
    s32 damage;
    if (!(part->enemyFlags & 2))
    {
        if (part->enemyFlags & 1) entity->enemyFlags |= 1;
        else part->enemyFlags |= 1;
        do { damage = Hit_ResolveDamage(entity, attacker, hit, strength); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        part->ext.manEater.headFlags |= 2;
        {
            u16 hp = part->hp;
            damage <<= 16;
            damage >>= 16;
            part->hp = hp - damage;
        }
        Entity_ShowDamage(entity, (s16)damage, 0);
        if ((s16)part->hp <= 0)
        {
            struct EwramData_EntityData *parent = &gEwramData->entityData[(u8)part->ext.manEater.ownerIndex];
            parent->ext.manEaterBody.headCount--;
            part->enemyFlags |= 2;
            part->palette = part->basePalette;
        }
    }
}

/* Man Eater projectile and effect controllers. */

/**
 * @brief Display frame 0x19 for a countdown, enabling drawSkip halfway
 * through, then remove the burst effect.
 */
void ManEaterProjectileBurst_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sManEaterGfx, sManEaterPalette, 0, sManEaterSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        entity->animFrame = 0x19;
        entity->spriteLayer = 0x11;
        entity->timer = 0x10;
        entity->state = 1;
        /* fall through */
    case 1:
        if (entity->timer != 0)
        {
            if (entity->timer == 8) entity->drawSkip = 1;
            entity->timer--;
        }
        else EntityDelete(entity);
        break;
    }
}

/**
 * @brief Aim a destructible projectile toward the target and accelerate it.
 * Keep hit checks during time stop; spawn the burst when HP is cleared
 * and remove on the following state update.
 */
void ManEaterProjectile_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    s32 angle;
    struct EwramData_EntityData *effect;
    if (entity->state != 0 && (s16)entity->hp != 0 && Entity_IsTimeStopped(entity))
    {
        Hit_CheckWithPlayer(&result, entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sManEaterGfx, sManEaterPalette, 0, sManEaterSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        Anim_StartIndexed(entity, 5, 0, 0);
        entity->spriteLayer = 0x11;
        entity->scrollWithBg = 1;
        entity->hp = 1;
        Hit_Enable(entity, 0xA, ManEaterProjectile_DamagePlayerOnContact, ManEaterProjectile_OnHit);
        HitBox_SetBodyRaw(entity, 0x0C0CFAFA);
        HitBox_SetAttackRaw(entity, 0x0C0CFAFA);
        angle = (s16)Entity_GetTargetAngleAbove16(entity);
        entity->velX.whole = (Sin(angle + 0x4000) >> 8) << 6;
        entity->velY.whole = (Sin(angle) >> 8) << 6;
        entity->accelX.whole = entity->velX.whole / 32;
        entity->accelY.whole = entity->velY.whole / 32;
        entity->state = 1;
        /* fall through */
    case 1:
        EntityAccelerateAndMove(entity, 0x10000);
        Hit_CheckWithPlayer(&result, entity);
        if (EntityIsOutsideScreen(entity, 3)) entity->state = 2;
        if ((s16)entity->hp == 0)
        {
            effect = EntityCreateInRange(0x49, 0xCF, ManEaterProjectileBurst_Update);
            if (effect != NULL)
            {
                effect->posX.whole = entity->posX.whole;
                effect->posY.whole = entity->posY.whole;
                effect->scrollWithBg = 1;
            }
            entity->state = 2;
        }
        break;
    case 2:
        EntityDelete(entity);
        return;
    }
    Anim_Update(entity);
}

/**
 * @brief Follow the terminal chain segment, then move toward the target
 * while the linked head attack flag is set. Remove during head breakup.
 */
void ManEaterChainAnchor_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = &gEwramData->entityData[(u8)entity->ext.manEater.ownerIndex];
    struct EwramData_EntityData *target = &gEwramData->entityData[entity->ext.manEater.prevIndex];
    s32 dx;
    s32 dy;
    u16 angle;
    if ((parent->ext.manEater.headFlags & 4) && parent->subState == 1)
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        entity->scrollWithBg = 1;
        entity->state = 1;
        break;
    case 1:
        if (parent->ext.manEater.headFlags & 1)
        {
            entity->ext.manEaterLink.savedState = 1;
            entity->state = 2;
            entity->subState = 0;
            entity->timer = 0x64;
            entity->accelX.whole = parent->posX.whole - entity->posX.whole;
            entity->accelY.whole = parent->posY.whole - entity->posY.whole;
        }
        entity->posX.whole = target->posX.whole;
        entity->posY.whole = target->posY.whole;
        break;
    case 2:
        dx = (s16)(Target_GetPixelX(entity) - entity->posX.part16.integer);
        dy = (s16)(Target_GetPixelY(entity) - (u16)entity->posY.part16.integer);
        angle = ArcTan2(dx, dy);
        {
            s32 offsetX = Sin(angle + 0x4000);
            s32 x = entity->posX.whole;
            x += offsetX;
            entity->posX.whole = x;
        }
        {
            s32 offsetY = Sin(angle);
            s32 y = entity->posY.whole;
            y += offsetY;
            entity->posY.whole = y;
        }
        if (parent->state != 2) entity->state = entity->ext.manEaterLink.savedState;
        break;
    }
}

/* Man Eater body segment: follows the head chain, swings, and breaks apart.
 * (flipX through an s8 byte pointer, one
 * variable for the divisor and the swing amplitude). */

#define INDEX(e) (e->ext.manEater.orbitSlot)
#define SWING(e) (e->ext.manEater.angle)
#define SWING_SPEED(e) (e->ext.manEaterLink.swingSpeed)

void ManEaterSegment_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    struct EwramData_EntityData *head = &gEwramData->entityData[(u8)entity->ext.manEater.ownerIndex];
    struct EwramData_EntityData *prev = &gEwramData->entityData[entity->ext.manEater.prevIndex];
    struct EwramData_EntityData *next = &gEwramData->entityData[entity->ext.manEater.baseIndex];
    s32 amplitude;
    s32 angle;

    if ((head->ext.manEater.headFlags & 4) && head->subState < (INDEX(entity) + 1) * 4)
    {
        struct EwramData_EntityData *debris = Effect_SpawnShared(entity->posX.whole, entity->posY.whole, 13, 8);
        if (debris != NULL)
        {
            { s32 f = RandomNumberGenerator() % 2; s8 *p = (s8 *)debris + 0x58; s32 t = f << 6; *p = (*p & ~0x40) | t; }
            debris->flipY = RandomNumberGenerator() % 2;
        }
        PlaySong(0x70);
        EntityDelete(entity);
        return;
    }
    if (entity->state <= 2 && (head->ext.manEater.headFlags & 1))
    {
        entity->ext.manEaterLink.savedState = entity->state;
        entity->state = 3;
        entity->subState = 0;
    }
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sManEaterGfx, sManEaterPalette, 0, sManEaterSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        Anim_StartIndexed(entity, 4, 0, 1);
        {
            struct AnimScript *anim = entity->animScript;
            entity->animStep = (6 - INDEX(entity)) % anim->stepCount;
        }
        if ((entity->stage = INDEX(entity)) == 5)
        {
            entity->ext.manEaterLink.drops = 1;
            Anim_StartIndexed(entity, 3, 0, 1);
        }
        entity->scrollWithBg = 1;
        entity->ext.manEaterLink.unusedSpeed = 0x60000;
        SWING_SPEED(entity) = 0x60;
        entity->ext.manEaterLink.unusedPhase = INDEX(entity) * 8;
        if (SWING(entity) <= 0x2000)
            entity->state = 1;
        else
        {
            SWING(entity) = 0x4000 - SWING(entity);
            entity->state = 2;
        }
        entity->enemyId = head->enemyId;
        Enemy_ResetFromTemplate(entity);
        if (gEwramData->gameState.bossRushMode)
        {
            entity->attack += 0x14;
            entity->hp += 1000;
        }
        entity->spriteLayer = 0x11;
        Hit_Enable(entity, 8, Enemy_OnContact, ManEaterPart_OnHit);
        HitBox_SetBodyRaw(entity, 0x1010F8F8);
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        break;
    case 1:
        SWING(entity) += SWING_SPEED(entity);
        if (SWING(entity) > 0x2000)
        {
            SWING(entity) = 0x2000;
            entity->state = 2;
        }
        break;
    case 2:
        SWING(entity) -= SWING_SPEED(entity);
        if (SWING(entity) < -0x2000)
        {
            SWING(entity) = -0x2000;
            entity->state = 1;
        }
        break;
    case 3:
        switch (entity->subState)
        {
        case 0:
            entity->accelX.whole = head->posX.whole - entity->posX.whole;
            entity->accelY.whole = head->posY.whole - entity->posY.whole;
            entity->timer = 0;
            entity->subState++;
        case 1:
        {
            s32 x, y, nx;
            entity->posX.whole = x = head->posX.whole - entity->accelX.whole;
            entity->posY.whole = y = head->posY.whole - entity->accelY.whole;
            amplitude = INDEX(entity) * entity->timer / (7 - INDEX(entity)) + 0x60;
            nx = next->posX.whole;
            nx += (x - nx) * 0x60 / amplitude;
            {
                s32 ny = next->posY.whole;
                s32 q = (y - ny) * 0x60 / amplitude;
                entity->posX.whole = nx;
                entity->posY.whole = ny + q;
            }
            if (entity->timer < 0x30)
                entity->timer += 2;
            if (head->state == 3)
                entity->subState++;
            break;
        }
        case 2:
            entity->posX.whole += (head->posX.whole - entity->accelX.whole - entity->posX.whole) / 12;
            entity->posY.whole += (head->posY.whole - entity->accelY.whole - entity->posY.whole) / 12;
            if (head->state != 3)
                entity->state = entity->ext.manEaterLink.savedState;
            break;
        }
        break;
    }
    if (entity->state <= 2)
    {
        if (INDEX(entity) != 0)
            amplitude = (s32)head->ext.manEater.lungeSpeed / 256;
        else
            amplitude = 0;
        angle = (s16)(head->ext.manEater.angle + (prev->ext.manEater.angle + SWING(entity)));
        entity->posX.whole = prev->posX.whole + Sin(angle + 0x4000) / 256 * amplitude;
        entity->posY.whole = prev->posY.whole + Sin(angle) / 256 * amplitude;
        if (*(u8 *)&entity->ext.manEaterLink.drops && (s16)head->hp <= 0xC7
            && entity->animStep <= 2 && entity->animTimer == 0 && (RandomNumberGenerator() & 7))
        {
            struct EwramData_EntityData *drop = Entity_CreateObject(ManEaterProjectile_Update);
            if (drop != NULL)
            {
                drop->posX.whole = entity->posX.whole;
                drop->posY.whole = entity->posY.whole;
                drop->attack = entity->attack - (entity->attack >> 3);
            }
        }
    }
    entity->stage++;
    if (!(entity->stage & 7) && Entity_IsInRect(entity, -0x10, 0, 0x110, 0xB0))
        Hit_CheckWithPlayerSharedHistory(&result, entity, head);
    Anim_Update(entity);
}

/* Man Eater head: spawns the body chain, steers towards the player, orbits.
 *: one function-level u16 (angle) holds both the
 * segment index and the ArcTan2 result, as in the original allocation. */

struct Flip8 {
    u8 slot:5;
    u8 hidden:1;
    u8 flipX:1;
    u8 flipY:1;
};

void ManEaterHead_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = &gEwramData->entityData[(u8)entity->ext.manEater.ownerIndex];
    const struct OrbitPoint *orbit = &sManEaterHeadOrbitPoints[entity->ext.manEater.orbitSlot];
    struct EwramData_EntityData *prev, *link, *base, *fx;
    s32 i;
    s16 a, b;
    u16 angle;
    s32 dx, dy, lim;
    u32 ang;
    s32 x, y;

    if (entity->enemyFlags & 2)
    {
        if (entity->ext.manEater.headFlags & 4)
        {
            if (entity->subState == 0)
            {
                EntityDelete(entity);
                return;
            }
            entity->subState--;
        }
        else
        {
            entity->ext.manEater.headFlags |= 4;
            entity->subState = 0x18;
        }
    }
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sManEaterGfx, sManEaterPalette, 0, sManEaterSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        Anim_StartIndexed(entity, 4, 0, 1);
        prev = entity;
        base = Entity_CreateObject(ManEaterChainAnchor_Update);
        if (base == NULL)
        {
            EntityDelete(entity);
            return;
        }
        entity->enemyId = parent->enemyId;
        Enemy_ResetFromTemplate(entity);
        entity->spriteLayer = 0x12;
        Hit_Enable(entity, 8, Enemy_OnContact, ManEaterPart_OnHit);
        HitBox_SetBodyRaw(entity, 0x1010F8F8);
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        i = 0;
        while (i <= 5 && (link = Entity_CreateObject(ManEaterSegment_Update)) != NULL)
        {
            angle = i + entity->ext.manEater.orbitSlot * 4;
            link->ext.manEater.ownerIndex = ENTITY_INDEX(entity);
            link->ext.manEater.prevIndex = ENTITY_INDEX(prev);
            link->ext.manEater.angle = 0xAAA * angle - 0x2000;
            link->ext.manEater.orbitSlot = i;
            link->ext.manEater.baseIndex = ENTITY_INDEX(base);
            prev = link;
            i++;
        }
        link = base;
        link->ext.manEater.ownerIndex = ENTITY_INDEX(entity);
        link->ext.manEater.prevIndex = ENTITY_INDEX(prev);
        entity->scrollWithBg = 1;
        entity->state = 1;
        break;
    case 1:
        if (entity->ext.manEater.lungeSpeed < 0x60000)
        {
            if (!(entity->timer & 0xF))
            {
                link = Effect_SpawnShared(entity->posX.whole, entity->posY.whole, 0xD, 8);
                if (link)
                {
                    ((struct Flip8 *)(ENTITY_RENDER_FLAGS_ADDRESS(link)))->flipX = RandomNumberGenerator() & 1;
                    ((struct Flip8 *)(ENTITY_RENDER_FLAGS_ADDRESS(link)))->flipY = RandomNumberGenerator() & 1;
                }
            }
            entity->timer++;
            entity->ext.manEater.lungeSpeed += 0x1800;
        }
        else if ((RandomNumberGenerator() & 0x3FF) < 0x10 && Target_IsNearEntity(entity, 0xC8, 0x78))
        {
            entity->state = 2;
            entity->ext.manEater.headFlags |= 1;
            entity->timer = 0x80;
        }
        dx = (s16)(Target_GetPixelX(entity) - entity->posX.part16.integer);
        dy = (s16)(Target_GetPixelY(entity) - entity->posY.part16.integer);
        angle = ArcTan2(dx, dy);
        Angle_TurnTowards((u16 *)&entity->ext.manEater.angle, 0x40, angle);
        if (entity->ext.manEater.angle > entity->ext.manEater.homeAngle + 0x2000)
            entity->ext.manEater.angle = entity->ext.manEater.homeAngle + 0x2000;
        else if (entity->ext.manEater.angle < entity->ext.manEater.homeAngle - 0x2000)
            entity->ext.manEater.angle = entity->ext.manEater.homeAngle - 0x2000;
        break;
    case 2:
        if ((RandomNumberGenerator() & 1) && (entity->ext.manEater.headFlags & 2))
            entity->timer = 0;
        if (entity->timer == 0)
        {
            entity->timer = 0x30;
            entity->state = 3;
            entity->ext.manEater.headFlags &= ~1;
            break;
        }
        entity->timer--;
        break;
    case 3:
        if (entity->timer != 0)
            entity->timer--;
        else
            entity->state = 1;
        break;
    }
    entity->ext.manEater.headFlags &= ~2;
    a = parent->ext.manEaterBody.angle + orbit->angle;
    x = parent->posX.whole + Sin(a + 0x4000) * orbit->radius;
    y = parent->posY.whole + Sin(a) * orbit->radius;
    b = entity->ext.manEater.baseAngle + parent->ext.manEaterBody.angle;
    entity->posX.whole = x + Sin(b + 0x4000) * 10;
    entity->posY.whole = y + Sin(b) * 10;
    Anim_Update(entity);
}

/* Man Eater creation and attached effects. */

/**
 * @brief Follow the root owner using its affine slot; animate the indexed
 * part during head creation, then clear the shared affine reference and
 * delete without freeing the owner slot.
 */
void ManEaterAttachedPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = &gEwramData->entityData[(u8)entity->ext.manEater.ownerIndex];
    const u8 *entry = sUnk_085288A8[entity->ext.manEater.orbitSlot];
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sManEaterGfx, sManEaterPalette, 0, sManEaterSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        entity->affineIndex = parent->affineIndex;
        entity->affine = 1;
        entity->animFrame = entry[0];
        entity->spriteLayer = 0x12;
        entity->state = 1;
        /* fall through */
    case 1:
        if (parent->state == 1 && parent->timer == 0x30 && entity->ext.manEater.orbitSlot == parent->subState)
        {
            Anim_StartIndexed(entity, entry[1], 0, 1);
            entity->state = 2;
            entity->timer = 0x30;
        }
        break;
    case 2:
        Anim_Update(entity);
        if (entity->timer != 0) entity->timer--;
        else entity->state = 3;
        break;
    case 3:
        entity->affineIndex = 0;
        entity->affine = 0;
        EntityDelete(entity);
        return;
    }
    entity->posX.whole = parent->posX.whole;
    entity->posY.whole = parent->posY.whole;
}

/**
 * @brief Damp and move a debris sprite, optionally emit random effects,
 * and request deletion at the end of its countdown.
 */
void ManEaterDebris_Update(struct EwramData_EntityData *entity)
{
    EntityDampVelocityX(entity, 0x50);
    EntityAccelerateAndMove(entity, 0x80000);
    if (entity->timer == 0xA) entity->drawSkip = 1;
    if (entity->subState == 0 && !(entity->timer & 0xF))
        Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0xC0000, 0xC0000, entity->spriteLayer - 1);
    if (--entity->timer == 0) entity->deleteRequested = 1;
}

/* Man Eater creation: affine slot, body segments.
 * (plain literal zeros and entity->flipX as the argument fix the allocation). */

void EnemyManEaterCreate(struct EwramData_EntityData *entity)
{
    u16 affine;
    s32 index;
    struct EwramData_EntityData *child;
    if (entity->spawnParam0 != 0 && gEwramData->gameState.bossRushMode == 0 && (gEwramData->gameState.bossEventFlags & 0x2000))
    {
        EntityDelete(entity);
        return;
    }
    if (!Entity_LoadGraphics(entity, sManEaterGfx, sManEaterPalette, 0, sManEaterSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    affine = ObjAffine_Alloc(1);
    if ((s16)affine < 0)
    {
        EntityDelete(entity);
        return;
    }
    entity->affineIndex = affine;
    entity->affine = 1;
    entity->animFrame = 0;
    entity->flipX = 0;
    ObjAffine_Set(entity->ext.manEaterBody.angle, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x12;
    entity->ext.manEaterBody.headCount = 3;
    for (index = 0; index <= 2; index++)
    {
        child = Entity_CreateObject(ManEaterAttachedPart_Update);
        if (child != NULL)
        {
            child->ext.manEater.ownerIndex = ENTITY_INDEX(entity);
            child->ext.manEater.orbitSlot = index;
            child->scrollWithBg = 1;
        }
    }
    if (gEwramData->gameState.bestiaryPreview) entity->state = 4;
    else if (entity->spawnParam0 != 0)
    {
        Music_SetCurrent(0x10);
        Player_SetSelectedTarget(entity);
    }
}

/* Man Eater update (death burst, chase, crumbling pieces).: the piece's flipX is written through an s8 byte
 * pointer with a u8 masked copy (keeps the mask before the address and the
 * global-allocation priority of entity/child as in the original). */

void EnemyManEaterUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;
    s32 angle;
    s32 x;
    s32 y;
    s32 offset;
    if (entity->state != 3 && (s8)entity->ext.manEaterBody.headCount == 0)
    {
        entity->animFrame = 0x11;
        Enemy_DropRewards(entity);
        entity->state = 3;
        entity->timer = 0x80;
        entity->accelY.whole = 0x400;
        PlaySong(0x6B);
        if (entity->spawnParam0 != 0)
        {
            Player_SetSelectedTarget(NULL);
            BossDefeatEffect_Spawn(entity->posX.whole, entity->posY.whole - 0x200000);
            BossRecoveryPickup_SpawnWide((s16)entity->posX.part16.integer, 0x20, 0xD, 0x60, 0);
        }
    }
    switch (entity->state)
    {
    case 0:
        if (!Target_IsNearEntity(entity, 0xA0, 0x78)) return;
        entity->state = 1;
        entity->timer = 0x30;
        entity->subState = 0;
        break;
    case 1:
        if (entity->timer != 0) entity->timer--;
        else
        {
            child = Entity_CreateObject(ManEaterHead_Update);
            if (child == NULL) return;
            child->ext.manEater.ownerIndex = ENTITY_INDEX(entity);
            {
                s32 value = ((entity->subState + 1) << 16) / 3;
                value -= 0x4000;
                child->ext.manEater.angle = value;
                child->ext.manEater.baseAngle = value;
                child->ext.manEater.homeAngle = value;
            }
            child->ext.manEater.orbitSlot = entity->subState;
            {
                s32 offsetX = Sin(child->ext.manEater.angle + 0x4000) << 4;
                s32 x = entity->posX.whole;
                x += offsetX;
                child->posX.whole = x;
            }
            {
                s32 offsetY = Sin(child->ext.manEater.angle) << 4;
                s32 y = entity->posY.whole;
                y += offsetY;
                child->posY.whole = y;
            }
            entity->timer = 0x30;
            if (++entity->subState > 2) entity->state = 2;
        }
        break;
    case 2:
        offset = 0x200000;
        if (entity->posX.whole < Target_GetX(entity)) offset = -offset;
        x = Target_GetX(entity) + offset;
        y = Target_GetY(entity) - 0x300000;
        Entity_StepTowardPoint(entity, x, y, 8);
        break;
    case 3:
        if (!(entity->timer & 3))
        {
            child = EntityCreateInRange(0x49, 0xCF, ManEaterDebris_Update);
            if (child != NULL)
            {
                {
                    s32 offsetX = RandomSigned(0x200000);
                    s32 x = entity->posX.whole;
                    x += offsetX;
                    child->posX.whole = x;
                }
                {
                    s32 offsetY = RandomSigned(0x200000);
                    s32 y = entity->posY.whole;
                    y += offsetY;
                    child->posY.whole = y;
                }
                child->velX.whole = RandomSigned(0x20000);
                child->velY.whole = RandomSigned(0x8000) - 0x18000;
                child->accelY.whole = 0x2000;
                child->spriteLayer = entity->spriteLayer;
                Entity_LoadGraphics(child, sManEaterGfx, sManEaterPalette, 0, sManEaterSpriteSheet);
                child->animFrame = RandomNumberGenerator() % 3 + 0x12;
                { s32 f = RandomNumberGenerator() & 1; s8 *p = (s8 *)child + 0x58; s32 t = f << 6; u8 m = *p & ~0x40; *p = m | t; }
                child->flipY = RandomNumberGenerator() % 2;
                child->scrollWithBg = 1;
                child->timer = (RandomNumberGenerator() & 0xF) + 0x28;
                child->subState = 1;
            }
        }
        EntityAccelerateAndMove(entity, 0x80000);
        if (entity->timer == 0)
        {
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
        }
        if (--entity->timer == 0x30) entity->drawSkip = 1;
        break;
    }
    entity->ext.manEaterBody.swayPhase += 0x80;
    entity->ext.manEaterBody.angle = (Sin(entity->ext.manEaterBody.swayPhase) / 256) << 4;
    ObjAffine_Set(entity->ext.manEaterBody.angle, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
}

/* Man Eater projectile callbacks and Killer Mantle attached effects. */

/**
 * @brief Apply stored contact damage with effect type 4 and zero flags,
 * then clear projectile HP so its update spawns a burst.
 */
void ManEaterProjectile_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 4;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
    entity->hp = 0;
}

/**
 * @brief Clear projectile HP so its update spawns a burst.
 */
void ManEaterProjectile_OnHit(struct EwramData_EntityData *entity)
{
    entity->hp = 0;
}

/**
 * @brief BB054 | Landing dust of the jump (owner in unk_4F8): sprite 8,
 * layer 0x14, 0x53E bits 1-2 = 1, follows the owner at (-12, 4) until its
 * animation ends.
 */
void KillerMantleLandingEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);

    switch (entity->state)
    {
        case 0:
            if (!Entity_LoadGraphics(entity, sKillerMantleGfx, sKillerMantlePalette, 0, sKillerMantleSpriteSheet))
            {
                EntityDelete(entity);
                return;
            }
            Anim_StartIndexed(entity, 8, 0, 0);
            entity->spriteLayer = 0x14;
            entity->objMode = 1;
            entity->state++;
        case 1:
            Entity_PlaceRelativeToCopy(entity, owner, -12, 4);
            Anim_Update(entity);
            if (entity->animFlags & 4)
                EntityDelete(entity);
            break;
    }
}

/**
 * @brief Load an affine part, show and rotate it while the owner attack
 * flag is set, then shrink and hide it. Pause for time stop or owner
 * hit flash; release the affine slot on owner death. Exact visual unresolved.
 */
void KillerMantlePart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    u16 slot;
    s16 angle;

    if (owner->state == 3)
        entity->state = 4;
    else if (entity->state != 0
             && (gEwramData->gameState.timeStopTimer != 0 || owner->hitFlashTimer != 0))
        return;
    switch (entity->state)
    {
        case 0:
            if (!Entity_LoadGraphics(entity, sKillerMantleGfx, sKillerMantlePalette, 0, sKillerMantleSpriteSheet))
            {
                EntityDelete(entity);
                return;
            }
            slot = ObjAffine_Alloc(1);
            if ((s16)slot < 0)
            {
                EntityDelete(entity);
                return;
            }
            entity->affineIndex = slot;
            entity->affine = 1;
            entity->animFrame = 0x12;
            entity->spriteLayer = owner->spriteLayer + 1;
            entity->hidden = 1;
            entity->state = 1;
        case 1:
            if (!(owner->ext.killerMantle.flags & 1))
                return;
            entity->ext.killerMantlePart.scaleX = 0;
            entity->ext.killerMantlePart.angle = -0x3000; // 0xD000
            entity->hidden = 0;
            entity->state = 2;
        case 2:
            if (!(owner->ext.killerMantle.flags & 1))
            {
                entity->timer = 8;
                entity->state = 3;
                break;
            }
            if (entity->ext.killerMantlePart.scaleX <= 0xFFFF)
                entity->ext.killerMantlePart.scaleX += 0x1000;
            entity->ext.killerMantlePart.angle += 0x300;
            Entity_PlaceRelativeToCopy(entity, owner, 8, 0);
            break;
        case 3:
            if (entity->ext.killerMantlePart.scaleX > 0)
                entity->ext.killerMantlePart.scaleX -= 0x2000;
            entity->ext.killerMantlePart.angle += 0x200;
            Entity_PlaceRelativeToCopy(entity, owner, 8, 0);
            if (entity->timer != 0)
            {
                entity->timer--;
                break;
            }
            entity->state = 1;
            entity->hidden = 1;
            break;
        case 4:
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
    }
    angle = entity->ext.killerMantlePart.angle;
    if (entity->flipX)
        angle = -angle;
    ObjAffine_Set(angle, entity->ext.killerMantlePart.scaleX, 0x10000, entity->flipX, 0, entity->affineIndex);
}
