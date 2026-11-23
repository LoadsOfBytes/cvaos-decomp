/*
 * Tsuchinoko: attack, death, hits, spit shots, droplets, graphics and update (after the jump state, own CFLAGS).
 */
#include "tsuchinoko.h"
#include "core.h"
#include "sound.h"
#include "graphics.h"
#include "player_core.h"
#include "structs/ewram.h"
#include "gameplay.h"
#include "entity_common.h"
#include "erinys.h"
#include "hitbox.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"

extern void Tsuchinoko_SpitShot(struct EwramData_EntityData *);

/* Attack on animation event 3, then resume jumping when it finishes. */
void Tsuchinoko_UpdateAttack(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->flipX = Entity_IsTargetRight(entity);
        entity->subState++;
        break;
    case 1:
        if (*(u16 *)&entity->animStep == 3)
        {
            Tsuchinoko_SpitShot(entity);
            PlaySong(0x85);
        }
        if (entity->animDone)
        {
            entity->state = 0;
            entity->subState = 1;
            Anim_StartIndexed(entity, 1, 0, 0);
        }
        break;
    }
}
/* Fall and blink for 60 frames; emit effects at frames 0 and 8. */
void Tsuchinoko_UpdateDeath(struct EwramData_EntityData *entity)
{
    s32 x, y;
    switch (entity->subState)
    {
    case 0:
        entity->velX.whole = 0;
        entity->velY.whole = 0;
        entity->accelY.whole = 0x1000;
        {
            x = entity->posX.whole + (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
            y = entity->posY.whole + (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
            {
                s32 vx = RandomNumberGenerator();
                s32 vy;
                u32 velMask;
                s32 velOffset;
                velMask = 0x1FFFF;
                vx &= velMask;
                velOffset = -0x10000;
                vx += velOffset;
                vy = RandomNumberGenerator();
                vy &= velMask;
                vy += velOffset;
                Effect_SpawnParticleAnim13(x, y, vx, vy, entity->spriteLayer - 1, 2);
            }
        }
        entity->timer = 60;
        entity->subState++;
        /* fall through */
    case 1:
        if (entity->timer == 52)
        {
            x = entity->posX.whole + (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
            y = entity->posY.whole + (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
            {
                s32 vx = RandomNumberGenerator();
                s32 vy;
                u32 velMask;
                s32 velOffset;
                velMask = 0x1FFFF;
                vx &= velMask;
                velOffset = -0x10000;
                vx += velOffset;
                vy = RandomNumberGenerator();
                vy &= velMask;
                vy += velOffset;
                Effect_SpawnParticleAnim13(x, y, vx, vy, entity->spriteLayer - 1, 2);
            }
        }
        EntityAccelerateAndMove(entity, 0x8000);
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0)
            entity->deleteRequested = 1;
        break;
    }
}

extern s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s16, s32);

void Tsuchinoko_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
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
        Enemy_DropRewards(entity);
        PlaySong(0x70);
        Anim_Start(entity, &sTsuchinokoHitAnimScript, 0, 0);
        entity->palette = entity->basePalette;
        entity->timer = 0x3C;
        entity->state = 2;
        entity->subState = 0;
    }
}

/* Tsuchinoko: create an arcing projectile sharing the parent's graphics. */
extern void TsuchinokoShot_Update(struct EwramData_EntityData *);
extern s32 TsuchinokoShot_LoadGraphics(struct EwramData_EntityData *, struct EwramData_EntityData *);
extern void TsuchinokoShot_EnableHitBoxes(struct EwramData_EntityData *);
/**
 * @brief Spit a falling shot from parent (TsuchinokoShot_Update, attribute 0x40
 * on contact).
 */
void Tsuchinoko_SpitShot(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, (u32 *)TsuchinokoShot_Update);
    if (child != NULL)
    {
        if ((u8)TsuchinokoShot_LoadGraphics(parent, child) == 0)
            EntityDelete(child);
        child->scrollWithBg = 1;
        Anim_StartIndexed(child, 4, 0, 1);
        Anim_Update(child);
        TsuchinokoShot_EnableHitBoxes(child);
        child->spriteLayer = parent->spriteLayer + 1;
        ENTITY_CREATOR(child) = parent;
        child->flipX = parent->flipX;
        if (parent->flipX)
            child->posX.part16.integer = parent->posX.part16.integer;
        else
            child->posX.part16.integer = parent->posX.part16.integer;
        child->posY.part16.integer = parent->posY.part16.integer - 26;
        if (parent->flipX)
            child->velX.whole = 0x30000;
        else
            child->velX.whole = -0x30000;
        child->velY.whole = -0x10000;
        child->accelX.whole = 0;
        child->accelY.whole = 0x1800;
        child->spawnIndex = parent->spawnIndex;
        child->basePalette = parent->basePalette;
        child->attack = parent->attack;
    }
}

/* TsuchinokoShot_EnableHitBoxes (hit boxes, same code as MermanShot_EnableHitBoxes). */
extern void TsuchinokoShot_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void TsuchinokoShot_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

/**
 * @brief A785C | Activate box slot 0xA with TsuchinokoShot_DamagePlayerOnContact / TsuchinokoShot_OnHit and set both
 * boxes to the bytes of 0x0808FCFC (x -4, y -4, w 8, h 8 - not a ROM
 * address); clear flags bits 1 and 2.
 */
void TsuchinokoShot_EnableHitBoxes(struct EwramData_EntityData *entity)
{
    u32 box = 0x0808FCFC;

    Hit_Enable(entity, 0xA, TsuchinokoShot_DamagePlayerOnContact, (s32)TsuchinokoShot_OnHit);
    HitBox_SetBody(entity, &box);
    HitBox_SetAttack(entity, &box);
    entity->hit.flags &= ~2;
    entity->hit.flags &= ~4;
}

extern void TsuchinokoDroplet_Update(struct EwramData_EntityData *);
static inline u8 LoadTsuchinokoGraphics(struct EwramData_EntityData *parent, struct EwramData_EntityData *child)
{
    s32 tiles = ObjGfx_Find(sTsuchinokoGfx);
    if (tiles < 0)
        return 0;
    Sprite_Attach(child, sTsuchinokoSpriteSheet, tiles, parent->basePalette);
    child->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/* Create a randomly flipped falling effect with no contact callback. */
void Tsuchinoko_SpawnDroplet(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, (u32 *)TsuchinokoDroplet_Update);
    if (child != NULL)
    {
        s32 zero;
        u8 *frame;
        if (!LoadTsuchinokoGraphics(parent, child))
            EntityDelete(child);
        child->scrollWithBg = 1;
        frame = &child->animFrame;
        zero = 0;
        *frame = 18;
        {
            u8 flags = child->hit.flags;
            flags |= 2;
            flags |= 4;
            child->hit.flags = flags;
        }
        child->spriteLayer = parent->spriteLayer - 1;
        ENTITY_CREATOR(child) = parent;
        child->flipX = RandomNumberGenerator() % 2;
        child->flipY = RandomNumberGenerator() % 2;
        child->posX.part16.integer = parent->posX.part16.integer;
        child->posY.part16.integer = parent->posY.part16.integer;
        child->velX.whole = (RandomNumberGenerator() % 0x20000) - 0x10000;
        child->velY.whole = -0x10000 - (RandomNumberGenerator() % 0x10000);
        child->accelX.whole = zero;
        child->accelY.whole = 0x1800;
        child->spawnIndex = parent->spawnIndex;
        child->basePalette = parent->basePalette;
        child->attack = parent->attack;
    }
}

/* Tsuchinoko helpers 0x080A79D8..0x080A7B53. */
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);

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
 * @brief A79D8 | Falling projectile with a hit box (Hit_CheckWithPlayer); deleted
 * outside the screen.
 */
void TsuchinokoShot_Update(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (entity->state == 0)
    {
        EntityAccelerateAndMove(entity, 0x40000);
        if (EntityIsOutsideScreen(entity, 1))
        {
            EntityDelete(entity);
            return;
        }
    }
    Hit_CheckWithPlayer(collision, entity);
    Anim_Update(entity);
}

/**
 * @brief A7A18 | Give child Tsuchinoko's graphics (palette row of
 * entity); 0 when no graphics slot is free.
 */
s32 TsuchinokoShot_LoadGraphics(struct EwramData_EntityData *entity, struct EwramData_EntityData *child)
{
    s32 gfx = ObjGfx_Find(sTsuchinokoGfx);

    if (gfx < 0)
        return 0;
    Sprite_Attach(child, sTsuchinokoSpriteSheet, gfx, entity->basePalette);
    child->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief A7A54 | Contact: Player_ApplyContactDamage(attack, 0x40, 0, 0), then end
 * (0x53D bit 3).
 */
void TsuchinokoShot_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0x40;
    contactDamage.unused = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
    entity->deleteRequested = 1;
}

/**
 * @brief A7A90 | Hit: hit effect and end (0x53D bit 3).
 */
void TsuchinokoShot_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    Player_CalculateAttackPower(hit[2]);
    entity->deleteRequested = 1;
}

/**
 * @brief A7AAC | Falling projectile without hit box; deleted outside the
 * screen.
 */
void TsuchinokoDroplet_Update(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (entity->state == 0)
    {
        EntityAccelerateAndMove(entity, 0x40000);
        if (EntityIsOutsideScreen(entity, 1))
            EntityDelete(entity);
    }
}

/**
 * @brief A7ADC | Load the graphics of Tsuchinoko; 0 when no slot is free.
 */
u8 Tsuchinoko_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sTsuchinokoGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sTsuchinokoPalette, 0);
    Sprite_Attach(entity, sTsuchinokoSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief A7B28 | Player_ApplyContactDamage with (attack, 0, 0, 0) at the entity position
 * (same as Chaos2Part_DamagePlayerOnContact).
 */
void Tsuchinoko_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);

void EnemyTsuchinokoUpdate(struct EwramData_EntityData *entity)
{
    u32 box[4];
    if (entity->state != 2) Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity) && entity->state != 2)
    {
        Hit_CheckWithPlayer(box, entity);
        return;
    }
    switch (entity->state)
    {
    case 0: Tsuchinoko_UpdateJump(entity); break;
    case 1: Tsuchinoko_UpdateAttack(entity); break;
    case 2: Tsuchinoko_UpdateDeath(entity); break;
    case 3: Anim_Update(entity); return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}
