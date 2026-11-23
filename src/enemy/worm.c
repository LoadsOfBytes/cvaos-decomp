/*
 * Giant Worm and Poison Worm: body, segments and ground following.
 */
#include "core.h"
#include "sound.h"
#include "player_core.h"
#include "graphics.h"
#include "gameplay.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
#include "bg_and_collision.h"
#include "syscalls.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"

extern s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s16, s32);

/**
 * @brief Hit on the head or a segment: resolve the damage and apply it to the
 * worm's root entity (flash, HP, death).
 */
void Worm_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    s32 damage = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(entity, (s32)attacker, hit, (s16)damage);
    parent->hitFlashTimer = 8;
    {
        u16 hp = parent->hp;
        damage <<= 16;
        damage >>= 16;
        parent->hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)parent->hp <= 0)
    {
        Enemy_DropRewards(parent);
        PlaySong(0x70);
        parent->palette = parent->basePalette;
        parent->state = 3;
        parent->timer = 0x40;
        parent->velX.whole = -0x8000;
        if (gEwramData->playerState.player->posX.whole > parent->posX.whole) parent->velX.whole = -parent->velX.whole;
        parent->velY.whole = -0x30000;
    }
}

/* Giant Worm / Poison Worm: create the root and four linked affine segments. */
extern void Worm_SettleOnGround(struct EwramData_EntityData *);
extern void WormSegment_Update(struct EwramData_EntityData *);
extern void Worm_Draw(struct EwramData_EntityData *);
extern s32 ObjPalette_FindOrLoadVariant(void *, s32);
extern void Worm_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void PoisonWorm_DamagePlayerOnContact(struct EwramData_EntityData *);
/**
 * @brief Shared create of Giant Worm (variant 0) and Poison Worm (variant 1):
 * graphics, affine head, body segments (WormSegment_Update), hit boxes
 * (Poison Worm contact with attribute 0x40) and Worm_Draw.
 */
void Worm_Init(struct EwramData_EntityData *root, s32 variant)
{
    u16 matrix;
    struct EwramData_EntityData *previous; register struct EwramData_EntityData *part asm("r5"); /* FAKEMATCH: loop pointers */
    s32 spacing, tiles;
    root->scrollWithBg = 1;
    matrix = ObjAffine_Alloc(1);
    if ((s16)matrix < 0)
    {
        EntityDelete(root);
        return;
    }
    root->affineIndex = matrix;
    root->affine = 1;
    Worm_SettleOnGround(root);
    previous = root;
    root->ext.worm.root = root;
    spacing = 0xA0000;
    if (root->flipX)
        spacing = -spacing;
    {
    s32 i = 0;
    for (; i <= 3;)
    {
        part = Entity_CreateObject(WormSegment_Update);
        if (part == NULL)
            goto fail;
        matrix = ObjAffine_Alloc(1);
        if ((s16)matrix < 0)
            goto fail;
        {
            u8 slot = *(u8 *)&part->ext;
            *(u8 *)&previous->ext.raw.unk_504 = slot;
        }
        part->affineIndex = matrix;
        part->affine = 1;
        i++;
        part->ext.worm.wavePhase = i << 12;
        part->ext.worm.root = root;
        part->ext.worm.previous = previous;
        part->ext.worm.segmentIndex = i;
        part->posX.whole = previous->posX.whole + spacing;
        part->posY.whole = previous->posY.whole;
        part->scrollWithBg = 1;
        Worm_SettleOnGround(part);
        { u8 t_ = root->enemyId; register u8 *d_ asm("r1") = &part->enemyId; *d_ = t_; } /* FAKEMATCH: address in r1 */
        Enemy_ResetFromTemplate(part);
        previous = part;
    }
    }
    Enemy_ResetFromTemplate(root);
    root->drawFunc = (u32 *)Worm_Draw;
    tiles = ObjGfx_LoadResource(sWormGfx, 0);
    root->basePalette = ObjPalette_FindOrLoadVariant(sWormPalette, (u8)variant);
    Sprite_Attach(root, sWormSpriteSheet, tiles, root->basePalette);
    if (variant == 0)
        Anim_StartIndexed(root, 3, 0, 1);
    else
        Anim_StartIndexed(root, 0, 0, 1);
    Anim_Update(root);
    ObjAffine_Set(0, 0x10000, 0x10000, root->flipX, 0, root->affineIndex);
    root->ext.worm.variant = variant;
    root->spriteLayer = 17;
    if (gEwramData->gameState.bestiaryPreview)
    {
        root->state = 6;
        return;
    }
    if (root->ext.worm.variant == 0)
        Hit_Enable(root, 8, Worm_DamagePlayerOnContact, (s32)Worm_OnHit);
    else
    {
        if (0)
        {
        fail:
            root->ext.worm.aggressive = 1;
            root->state = 4;
            return;
        }
        Hit_Enable(root, 8, PoisonWorm_DamagePlayerOnContact, (s32)Worm_OnHit);
    }
    HitBox_SetBodyRaw(root, 0x1010F8F8);
    HitBox_SetAttackRaw(root, 0x1010F8F8);
}

/* Worm_FollowGround: keep the entity on the floor (Collision_GetPushUp 10 px below) and
 * test the wall ahead (Collision_GetPushRight left / Collision_GetPushLeft right, by flipX and
 * unk_502/unk_503 or the controller byte 0x1F); flag 4 in controller 0x1D
 * when airborne or blocked. */

/**
 * @brief Push the entity up out of the floor probed 10 px below its position.
 */
void Worm_FollowGround(struct EwramData_EntityData *entity)
{
    u8 *ctl = (u8 *)entity->ext.worm.root;
    u16 x = GetEntityRoomXPositionInteger(entity);
    u16 y = GetEntityRoomYPositionInteger(entity);
    s16 d = Collision_GetPushUp((s16)x, (s16)y + 10);
    u16 hit;

    if (d != 0)
        entity->posY.part16.integer += d + 2;
    else if (ctl[0x1D] == 0)
        ctl[0x1D] = 4;
    hit = 0;
    if (entity->ext.worm.segmentIndex == 0)
    {
        if (entity->ext.worm.crawlDir == 0 && !entity->flipX)
            hit = Collision_GetPushRight((s16)x - 0x18, (s16)y - 8);
        if (entity->ext.worm.crawlDir != 0 && entity->flipX)
            hit = Collision_GetPushLeft((s16)x + 0x18, (s16)y - 8);
    }
    else if (entity->ext.worm.segmentIndex == 4)
    {
        if (ctl[0x1F] != 0 && !entity->flipX)
            hit = Collision_GetPushLeft((s16)x + 0x18, (s16)y - 8);
        if (ctl[0x1F] == 0 && entity->flipX)
            hit = Collision_GetPushRight((s16)x - 0x18, (s16)y - 8);
    }
    if (hit != 0 && ctl[0x1D] == 0)
        ctl[0x1D] = 4;
}

/* Probe up to four vertical offsets and adjust entity Y on the first hit.
 * Keep the original u16 X result and its later separate signed conversion.
 * The correction local and short register lifetimes preserve agbcc's
 * arithmetic order; a match does not prove original source spelling. */
void Worm_SettleOnGround(struct EwramData_EntityData *entity)
{
    s32 offset = 0xFFF8;
    if (!gEwramData->gameState.bestiaryPreview)
    {
        s32 i = 0;
        s16 *posY = (s16 *)&entity->posY.part16.integer;
        for (; i <= 3; i++)
        {
            u16 x = GetEntityRoomXPositionInteger(entity);
            u16 y = GetEntityRoomYPositionInteger(entity);
            s32 gap;
            s32 probeOffset;
            s32 signedX = (s16)x;
            { s32 signedY = (s16)y;
            offset = (s16)offset;
            probeOffset = offset;
            probeOffset += 10;
            signedY += probeOffset;
            gap = Collision_GetPushUp(signedX, signedY); }
            if (gap != 0)
            {
                register s32 correction asm("r0") = offset + 2; /* FAKEMATCH: pinned register */
                correction = gap + correction;
                *posY = correction + *posY;
                break;
            }
            offset = (u16)(offset + 16);
        }
    }
}

/* Start a half-pixel step, decelerate through zero, wait, then repeat. */
void Worm_StepCrawl(struct EwramData_EntityData *entity, s32 direction, s32 distance, s32 blocked)
{
    switch (entity->subState)
    {
    case 0:
        if (blocked == 0 || distance > 7)
        {
            if (direction != 0)
            {
                entity->velX.whole = 0x8000;
                entity->accelX.whole = -0x300;
            }
            else
            {
                entity->velX.whole = -0x8000;
                entity->accelX.whole = 0x300;
            }
            entity->subState = 1;
            entity->timer = 0;
        }
        break;
    case 1:
        entity->timer++;
        entity->velX.whole += entity->accelX.whole;
        if (entity->accelX.whole > 0)
        {
            if (entity->velX.whole >= 0)
                entity->subState = 2;
        }
        else if (entity->velX.whole <= 0)
            entity->subState = 2;
        break;
    case 2:
        if (entity->timer++ > 16)
        {
            entity->subState = 0;
            entity->timer = 0;
        }
        break;
    }
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
}

/* Thrown hand weapon of an enemy: follows the owner, flies off in an arc once released. */
extern void Hit_CheckWithPlayerSharedHistory(void *, struct EwramData_EntityData *, struct EwramData_EntityData *);

#define ABS(x) ((x) < 0 ? -(x) : (x))

void WormSegment_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target = entity->ext.worm.previous;
    struct EwramData_EntityData *owner = entity->ext.worm.root;
    u8 buf[0x10];
    s32 angle = 0;
    s32 amp = 0;

    entity->palette = owner->palette;
    if (entity->state != 0 && owner->state != 3)
    {
        if (owner->hitFlashTimer != 0 || Entity_IsTimeStopped(owner) != 0)
        {
            if (owner->state == 3)
                return;
            Player_QueueEntity(entity, 0);
            Hit_CheckWithPlayerSharedHistory(buf, entity, owner);
            return;
        }
        Player_QueueEntity(entity, 1);
    }
    switch (entity->state)
    {
    case 0:
        if (owner->ext.worm.aggressive != 0)
            goto delete;
        {
            s32 tiles = ObjGfx_LoadResource(sWormGfx, 0);
            Sprite_Attach(entity, sWormSpriteSheet, tiles, owner->basePalette);
        }
        entity->basePalette = owner->basePalette;
        entity->flipX = owner->flipX;
        if (entity->ext.worm.segmentIndex != 4)
            Anim_StartIndexed(entity, 1, 0, 1);
        else
            Anim_StartIndexed(entity, 2, 0, 1);
        Anim_Update(entity);
        entity->spriteLayer = 0x11;
        if (*((u8 *)gEwramData + 0x8E) & 0x40)
        {
            entity->state = 4;
            break;
        }
        if (owner->ext.worm.variant == 0)
            Hit_Enable(entity, 8, Worm_DamagePlayerOnContact, (s32)Worm_OnHit);
        else
            Hit_Enable(entity, 8, PoisonWorm_DamagePlayerOnContact, (s32)Worm_OnHit);
        HitBox_SetBodyRaw(entity, 0x1010F8F8);
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->state++;
    case 1:
        Worm_StepCrawl(entity, owner->ext.worm.crawlDir, target->timer, 1);
        Worm_FollowGround(entity);
        if (!entity->flipX)
            angle = 0x8000;
        if (owner->ext.worm.aggressive != 0)
        {
            entity->state = 2;
            entity->timer = 0x38;
            entity->accelY.whole = angle;
            entity->velX.whole = (entity->ext.worm.segmentIndex << 15) - 0x6000;
            if (gEwramData->playerState.player->posX.whole > entity->posX.whole)
                entity->velX.whole = -entity->velX.whole;
            entity->velY.whole = -0x30000;
        }
        break;
    case 2:
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += 0x2000;
        if (entity->flipX)
            entity->accelY.whole -= 0x100;
        else
            entity->accelY.whole += 0x100;
        angle = entity->accelY.whole;
        if ((entity->timer & 7) == 0)
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0x100000, 0x100000, entity->spriteLayer - 2);
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
    case 3:
    delete:
        if (ENTITY_RENDER_FLAGS16(entity) & 0x101F)
            ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
        return;
    }
    if (entity->state == 4)
    {
        if (!entity->flipX)
            angle = 0x8000;
        if (owner->ext.worm.variant != 0)
        {
            amp = ABS(Sin((u16)entity->ext.worm.wavePhase) * 0x6000 / 0x10000);
            entity->ext.worm.wavePhase += 0x200;
        }
        ObjAffine_Set((u16)EntityAngleTo(entity, target) + angle, 0x10000, amp + 0x10000, entity->flipX, 0, entity->affineIndex);
        Anim_Update(entity);
    }
    else
    {
        if (owner->ext.worm.variant != 0)
        {
            amp = ABS(Sin((u16)entity->ext.worm.wavePhase) * 0x6000 / 0x10000);
            entity->ext.worm.wavePhase += 0x200;
        }
        ObjAffine_Set((u16)EntityAngleTo(entity, target) + angle, 0x10000, amp + 0x10000, entity->flipX, 0, entity->affineIndex);
        if (owner->state != 3)
            Hit_CheckWithPlayerSharedHistory(buf, entity, owner);
        Anim_Update(entity);
    }
}

/* Worm_GetSlopeAngle: matched (s16 coordinates: stored zero-extended, sign-extended
   at use; loop 1 argument y1 + (i + 10) kept unassociated). */
/**
 * @brief Return the angle of the floor under the entity from probes 8 px to each
 * side.
 */
s32 Worm_GetSlopeAngle(struct EwramData_EntityData *e)
{
    s16 x = GetEntityRoomXPositionInteger(e);
    s16 y1 = GetEntityRoomYPositionInteger(e);
    s16 x2 = x + 8;
    s16 y2 = y1;
    s32 i;
    s16 x1 = x - 8;
    s16 h;
    for (i = 0; i < 32; i += 8)
    {
        h = Collision_GetPushUp(x1, y1 + ({ s32 t_ = i + 10; t_; }));
        if (h != 0)
        {
            y1 = y1 + h + i;
            break;
        }
    }
    for (i = 0; i < 32; i += 8)
    {
        h = Collision_GetPushUp(x2, y2 + i + 10);
        if (h != 0)
        {
            y2 = y2 + h + i;
            break;
        }
    }
    return (u16)ArcTan2(x1 - x2, y1 - y2) + 0x8000;
}

/* Enemy update: counted hops with random/target-based facing, knockback fall and despawn states. */
extern s32 Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern s32 Target_IsInRect(s16, s16, s16, s16);

static inline struct EwramData_EntityData *CurV(struct EwramData *ew) { return *(struct EwramData_EntityData * volatile *)&ew->playerState.enemyTarget; }
static inline struct EwramData_EntityData *Cur(struct EwramData *ew) { return ew->playerState.enemyTarget; }
void Worm_Update(struct EwramData_EntityData *entity)
{
    u8 buffer[16];
    s32 zero = 0;
    s32 angle = 0;
    u32 n;
    struct EwramData *ew;

    if (entity->state)
    {
        Player_TrackEntity(entity);
        if (Enemy_IsStunned(entity))
        {
            if (entity->state == 3)
                return;
            Player_QueueEntity(entity, 0);
            Hit_CheckWithPlayer(buffer, entity);
            return;
        }
        if (entity->state != 3)
            Player_QueueEntity(entity, 1);
    }
    switch (entity->state)
    {
    case 0:
        entity->state = 0;
    case 1:
        { register u32 raw_ asm("r0") = *(vu8 *)&entity->ext.worm.turnTimer; u8 t_; n = raw_; t_ = raw_; /* FAKEMATCH: pinned register */
        if (n)
        {
            entity->state = 2;
            if (n == 4)
            {
                if (entity->ext.worm.crawlDir)
                    entity->ext.worm.crawlDir = 0;
                else
                    entity->ext.worm.crawlDir = 1;
            }
            entity->ext.worm.turnTimer=t_-1;
            break;
        } }
        if (!Target_IsInRect(entity->posX.part16.integer - 0xA0, entity->posY.part16.integer - 0x3C, 0x140, 0x78))
            entity->ext.worm.crawlDir = RandomNumberGenerator() & 1;
        else if (!(ew = gEwramData)->playerState.enemyTarget)
            entity->ext.worm.crawlDir = RandomNumberGenerator() & 1;
        else if (CurV(ew)->posX.whole < entity->posX.whole)
            entity->ext.worm.crawlDir = n;
        else
            entity->ext.worm.crawlDir = 1;
        entity->state = 2;
        break;
    case 2:
        Worm_StepCrawl(entity, entity->ext.worm.crawlDir, 0, 0);
        Worm_FollowGround(entity);
        if (!entity->subState)
            entity->state = 1;
        break;
    case 3:
        entity->ext.worm.aggressive = 1;
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += 0x2000;
        if (entity->flipX)
            entity->accelY.whole -= 0x100;
        else
            entity->accelY.whole += 0x100;
        angle = entity->accelY.whole;
        if (!(entity->timer & 7))
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0x100000, 0x10000, entity->spriteLayer - 2);
        if (--entity->timer == 0)
        {
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
        }
        break;
    case 4:
        entity->timer = 2;
        entity->state = 5;
    case 5:
        if (--entity->timer == 0)
        {
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
        }
        return;
    case 6:
        ObjAffine_Set(angle, 0x10000, zero + 0x10000, entity->flipX, 0, entity->affineIndex);
        Anim_Update(entity);
        return;
    }
    if (entity->state != 3)
    {
        Hit_CheckWithPlayer(buffer, entity);
        angle = Worm_GetSlopeAngle(entity);
        entity->accelY.whole = angle;
    }
    ObjAffine_Set(angle, 0x10000, zero + 0x10000, entity->flipX, 0, entity->affineIndex);
    Anim_Update(entity);
}

/* Giant Worm / Poison Worm helpers 0x080AE334..0x080AE3FB. */
static inline void Entity_DamagePlayerOnContact(struct EwramData_EntityData *entity, u16 attributes, u16 unused, u16 reaction)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = attributes;
    contactDamage.unused = unused;
    contactDamage.reaction = reaction;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief AE334 | Player_ApplyContactDamage with (attack, 0, 0, 0) at the entity position
 * (same as Chaos2Part_DamagePlayerOnContact).
 */
void Worm_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief AE360 | Giant Worm create: Worm_Init variant 0.
 */
void EnemyGiantWormCreate(struct EwramData_EntityData *entity)
{
    Worm_Init(entity, 0);
}

/**
 * @brief AE36C | Giant Worm update.
 */
void EnemyGiantWormUpdate(struct EwramData_EntityData *entity)
{
    Worm_Update(entity);
}

/**
 * @brief AE378 | Poison Worm create: Worm_Init variant 1.
 */
void EnemyPoisonWormCreate(struct EwramData_EntityData *entity)
{
    Worm_Init(entity, 1);
}

/**
 * @brief AE384 | Poison Worm update: same as Giant Worm.
 */
void EnemyPoisonWormUpdate(struct EwramData_EntityData *entity)
{
    Worm_Update(entity);
}

/**
 * @brief AE390 | Draw up to 5 body segments, following the chain of slots
 * in unk_504 until an empty entity.
 */
void Worm_Draw(struct EwramData_EntityData *entity)
{
    s32 i = 0;
    struct EwramData **ewramRef = &gEwramData;

    do
    {
        if (entity->updateFunc == NULL)
            break;
        Sprite_Draw(entity);
        entity = &(*ewramRef)->entityData[entity->ext.worm.nextIndex];
    } while (++i <= 4);
}

/**
 * @brief AE3CC | Player_ApplyContactDamage with (attack, 0x40, 0, 0) at the entity
 * position.
 */
void PoisonWorm_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0x40;
    contactDamage.unused = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}
