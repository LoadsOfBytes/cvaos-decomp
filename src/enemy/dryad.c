/*
 * Dryad: body and seeds.
 */
#include "core.h"
#include "sound.h"
#include "player_core.h"
#include "graphics.h"
#include "entity_util.h"
#include "macros.h"
#include "gameplay.h"
#include "structs/ewram.h"

extern s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s16, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);

void Dryad_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 damage = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(entity, (s32)attacker, hit, (s16)damage);
    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;
        damage <<= 16;
        damage >>= 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)entity->hp <= 0)
    {
        if (entity->state == 1 && entity->subState != 0) Anim_StartIndexed(entity, 5, 0, 0);
        else Anim_StartIndexed(entity, 4, 0, 0);
        Enemy_DropRewards(entity);
        if (entity->spawnParam0 != 0) gEwramData->gameState.bossEventFlags |= 1;
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->state = 3;
        entity->subState = 0;
        entity->stage = 0;
    }
}

/* Dryad activation and attack selection. */
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_IsInRect(s32, s32, s32, s32);
extern void Dryad_ThrowSeed(struct EwramData_EntityData *);

void Dryad_UpdateIdle(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        if (Target_IsInRect((s16)(entity->posX.part16.integer - 0x6E),
                        (s16)(entity->posY.part16.integer - 0x28), 0xDC, 0x50))
            entity->subState = 1;
        break;
    case 1:
        entity->flipX = Entity_IsTargetRight(entity);
        if (!(RandomNumberGenerator() & 3))
        {
            entity->state = 1;
            entity->subState = 0;
            entity->stage = 0;
            Anim_StartIndexed(entity, 1, 0, 0);
        }
        break;
    }
}

/* Dryad activation and attack selection. */

void Dryad_UpdateAttack(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        if (entity->animFlags & 4)
        {
            Anim_StartIndexed(entity, 2, 0, 1);
            entity->subState = 1;
        }
        break;
    case 1:
        if (!Entity_IsInRect(entity, -0x80, -0x80, 0x1F0, 0x1A0))
        {
            u8 *base = (u8 *)gEwramData;
            u32 event = entity->spawnIndex;
            s32 index = event >> 5;
            s32 *word;
            index <<= 2;
            base += OFFSET_OF(struct EwramData, gameState.roomEntitySpawn.spawnedFlags);
            word = (s32 *)(base + index);
            {
                s32 bit = 31;
                bit &= event;
                *word &= ~(1 << bit);
            }
            entity->deleteRequested = 1;
        }
        else
        {
            entity->flipX = Entity_IsTargetRight(entity);
            if (!(RandomNumberGenerator() & 7))
            {
                Anim_StartIndexed(entity, 3, 0, 0);
                entity->timer = 0x12;
                entity->subState = 2;
            }
        }
        break;
    case 2:
        if (entity->timer != 0)
        {
            if (--entity->timer <= 2)
            {
                PlaySong(0x89);
                Dryad_ThrowSeed(entity);
            }
        }
        if (entity->animFlags & 4) entity->subState = 1;
        break;
    }
}

/* Dryad death: palette flashes, smoke, shrinking affine sprite. */
extern s32 Entity_GetVisibilityPattern(s32);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);

/**
 * @brief Death state (state 3): effects with sound 0x6E every 8 frames for 64
 * frames, then free the affine slot and request deletion.
 */
void Dryad_UpdateDeath(struct EwramData_EntityData *entity)
{
    u16 slot;
    switch (entity->subState)
    {
    case 0:
        entity->timer = 0x40;
        entity->subState++;
        slot = ObjAffine_Alloc(1);
        if ((s16)slot >= 0)
        {
            entity->affineIndex = slot;
            entity->affine = 1;
        }
        break;
    case 1:
        if (entity->timer == 0x10) entity->drawSkip = 1;
        if (!(entity->timer & 7))
        {
            PlaySong(0x6E);
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x100000, 0x100000, 0x100000, entity->spriteLayer - 1);
        }
        if (--entity->timer == 0)
        {
            entity->deleteRequested = 1;
            entity->hidden = 1;
            if (entity->affine)
            {
                ObjAffine_Free(entity->affineIndex);
                entity->affine = 0;
            }
            return;
        }
        break;
    }
    if (Entity_GetVisibilityPattern(entity->timer)) entity->palette = 5;
    else entity->palette = entity->basePalette;
    if (entity->affine)
        ObjAffine_Set(0, entity->timer << 10, 0x10000, entity->flipX, 0, entity->affineIndex);
}

void DryadSeed_OnContact(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    if ((hit[0] & 1) && entity->state <= 1)
    {
        entity->state = 2;
        entity->timer = 0xB4;
        entity->accelX.part16.integer = entity->posX.part16.integer - attacker->posX.part16.integer;
        entity->accelY.part16.integer = entity->posY.part16.integer - attacker->posY.part16.integer;
        entity->ext.dryadSeed.unusedAttacker = attacker;
        entity->hit.flags |= 4;
    }
}

/* Dryad projectile: ballistic launch, animation sequence, then a timed
 * player-following effect with contact damage every sixteen frames. */
extern s32 EntityIsOutsideScreen(struct EwramData_EntityData *, s32);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void DryadSeed_OnHit(struct EwramData_EntityData *);
extern s32 Player_GetOverheadY(void);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern u8 sDryadGfx[], sDryadPalette[], sDryadSpriteSheet[], sUnk_08528604[];
static inline u8 AdvanceDryadEffect(struct EwramData_EntityData *entity)
{
    if (sUnk_08528604[entity->subState] == 0xFF)
        return 1;
    if (entity->subState == 0 || (entity->animFlags & 4))
    {
        u8 index = entity->subState++;
        Anim_StartIndexed(entity, sUnk_08528604[index], 0, 0);
    }
    return 0;
}
static inline s16 PlayerEffectX(void)
{
    return gEwramData->playerState.player->posX.part16.integer;
}
void DryadSeed_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct PlayerContactDamage contact;
    if (EntityIsOutsideScreen(entity, 1))
        entity->deleteRequested = 1;
    switch (entity->state)
    {
    case 0:
        Entity_LoadGraphics(entity, sDryadGfx, sDryadPalette, 0, sDryadSpriteSheet);
        Hit_Enable(entity, 8, DryadSeed_OnContact, (s32)DryadSeed_OnHit);
        HitBox_SetBodyRaw(entity, 0x0404FEFE);
        HitBox_SetAttackRaw(entity, 0x0404FEFE);
        entity->spriteLayer = 17;
        Anim_StartIndexed(entity, 8, 0, 1);
        entity->scrollWithBg = 1;
        entity->state++;
        /* fall through */
    case 1:
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += 0x1000;
        break;
    case 2:
        if (AdvanceDryadEffect(entity))
        {
            entity->state = 3;
            entity->timer = 120;
        }
        /* fall through */
    case 3:
        entity->posX.part16.integer = PlayerEffectX();
        entity->posY.part16.integer = Player_GetOverheadY();
        if (entity->state == 3)
        {
            if ((entity->timer & 15) == 0)
            {
                contact.attributes = 0;
                contact.unused = 0;
                contact.attack = entity->attack;
                contact.reaction = 2;
                contact.sourceX = entity->posX.whole;
                contact.sourceY = entity->posY.whole;
                Player_ApplyContactDamage(&contact);
            }
            if (--entity->timer == 0)
                entity->deleteRequested = 1;
        }
        break;
    }
    Anim_Update(entity);
    Hit_CheckWithPlayer(collision, entity);
}

/* Dryad projectile creation: random launch spread and target-distance correction. */
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);

/* Keep the signed offsets separate from the wrapping 16-bit positions. */
static inline s16 NegateDryadOffset(s16 value)
{
    return -value;
}

static inline s32 AddDryadOffset(u16 position, s16 offset)
{
    return offset + position;
}

static inline s32 AdjustDryadY(s16 offset)
{
    return offset - 16;
}

static inline s32 AddDryadY(u16 position, u16 offset)
{
    return position + AdjustDryadY(offset);
}

/**
 * @brief Throw a seed from parent toward the target with a random spread and
 * three quarters of its attack (DryadSeed_Update).
 */
void Dryad_ThrowSeed(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *shot = EntityCreateInRange(0x1B, 0x48, (u32 *)DryadSeed_Update);
    if (shot != NULL)
    {
        u16 x, y;
        s32 vx, velocity;
        ENTITY_OWNER(shot) = parent;
        shot->flipX = parent->flipX;
        /* Signed spread -10..5, stored modulo 65536 before mirroring. */
        x = (RandomNumberGenerator() & 15) + 0xFFF6;
        y = (RandomNumberGenerator() & 15) + 0xFFF6;
        if (shot->flipX)
            x = NegateDryadOffset(x);
        shot->posX.part16.integer = AddDryadOffset(parent->posX.part16.integer, x);
        shot->posY.part16.integer = AddDryadY(parent->posY.part16.integer, y);
        vx = -0x800 - ((RandomNumberGenerator() % 16) << 10);
        if (Target_GetX(parent) - shot->posX.whole < 0)
            velocity = vx - ((shot->posX.whole - Target_GetX(parent)) >> 6);
        else
            velocity = vx - ((Target_GetX(parent) - shot->posX.whole) >> 6);
        shot->velX.whole = velocity;
        if (shot->flipX)
            shot->velX.whole = -velocity;
        shot->velY.whole = ((RandomNumberGenerator() % 16) << 8) - 0x20000;
        shot->velY.whole += (Target_GetY(parent) - parent->posY.whole) / 64;
        /* Parent contact damage minus its truncated quarter. */
        shot->attack = parent->attack - (parent->attack >> 2);
    }
}

extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Dryad_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern s32 Hit_EnableContact(struct EwramData_EntityData *, s32, void *, s32);
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern u8 sDryadGfx[], sDryadPalette[], sDryadSpriteSheet[];

void EnemyDryadCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sDryadGfx, sDryadPalette, 0, sDryadSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview) entity->state = 4;
    else
    {
        Enemy_ResetFromTemplate(entity);
        Hit_EnableContact(entity, 8, Dryad_DamagePlayerOnContact, (s32)Dryad_OnHit);
        Entity_UpdateBoxes(entity);
        Entity_SnapToGround(entity);
        entity->posY.part16.integer++;
        entity->scrollWithBg = 1;
        entity->state = 0;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Dryad_DamagePlayerOnContact = Minotaur_DamagePlayerOnContact
 *   DryadSeed_OnHit = EventScene21_Init */

/**
 * @brief AEB50 | Same as Minotaur_DamagePlayerOnContact.
 */
void Dryad_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief AEB80 | Same as EventScene21_Init.
 */
void DryadSeed_OnHit(struct EwramData_EntityData *entity)
{
    entity->deleteRequested = 1;
}

void EnemyDryadUpdate(struct EwramData_EntityData *entity)
{
    if (Enemy_PreUpdate(entity)) return;
    switch (entity->state)
    {
    case 0: Dryad_UpdateIdle(entity); break;
    case 1: Dryad_UpdateAttack(entity); break;
    case 2:
        if (entity->animFlags & 4)
        {
            entity->state = 0;
            entity->subState = 0;
            entity->stage = 0;
        }
        break;
    case 3: Dryad_UpdateDeath(entity); break;
    case 4:
        Anim_Update(entity);
        return;
    }
    Enemy_CheckHitPlayer(entity);
    Entity_AnimateWithBoxes(entity);
}
