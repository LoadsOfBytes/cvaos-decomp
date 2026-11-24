/*
 * Head stack enemies (Bone Pillar, Fish Head): segments, projectiles, effects and debris.
 */
#include "core.h"
#include "player_core.h"
#include "entity_util.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "sound.h"
#include "gameplay.h"
#include "structs/entity_render_flags.h"
#include "bg_and_collision.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "obj_palette.h"
#include "data/data_118B48.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, u8 *resource, void *palette, u8 variant, void *sheet);
extern void Entity_PlaceRelativeToCopy(struct EwramData_EntityData *, struct EwramData_EntityData *, s32, s32);
extern s32 Target_IsInRect(s16, s16, s16, s16);

void HeadStackEffect_UpdateFallingPiece(struct EwramData_EntityData *);
void HeadStackProjectile_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void HeadStackProjectile_ClearHealth(struct EwramData_EntityData *);
void HeadStackDebris_Update(struct EwramData_EntityData *entity);
void HeadStackSegment_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

/* Falling debris and the short-lived emitter that spawns it. */
/* Keep the table index as an inline parameter: agbcc then reproduces
 * the original base-plus-0x10 calculation instead of a relocated pointer. */
static inline u8 LoadDebrisGraphics(struct EwramData_EntityData *e, struct HeadStackGraphics *base, s32 idx)
{
    struct HeadStackGraphics *g = base + idx;
    return Entity_LoadGraphics(e, g->graphics, g->palette, 0, g->spriteSheet);
}
/**
 * @brief Load variant-1 frame 27, move with acceleration and count down while the collision probe succeeds; delete when it fails or the timer expires.
 */
void HeadStackEffect_UpdateFallingPiece(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg;

    switch (entity->state)
    {
    case 0:
        if (!LoadDebrisGraphics(entity, sUnk_081193DC, 1))
        {
            EntityDelete(entity);
            return;
        }
        entity->animFrame = 0x1B;
        entity->spriteLayer = 9;
        entity->timer = 0x80;
        entity->velY.whole = -0x4000;
        entity->state++;
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        {
            s32 x = (s16)entity->posX.part16.integer;
            bg = &gEwramData->bgInfo[1];
            if (Collision_IsPointSolid(x + (s16)bg->xPos.part16.integer,
                    entity->posY.part16.integer + (s16)bg->yPos.part16.integer)
                && entity->timer != 0)
            {
                if (entity->timer == 0x40)
                    entity->drawSkip = 1;
                entity->timer--;
            }
            else
                entity->deleteRequested = 1;
        }
        break;
    }
    if (EntityIsOutsideScreen(entity, 1))
        entity->deleteRequested = 1;
}

/**
 * @brief Play variant-1 animation 6, then hide and emit falling pieces during a 32-tick countdown.
 */
void HeadStackEffect_UpdateEmitter(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        {
            if (!LoadDebrisGraphics(entity, sUnk_081193DC, 1))
            { EntityDelete(entity); return; }
            entity->palette = 6;
            Anim_StartIndexed(entity, 6, 0, 0);
            entity->spriteLayer = 9;
            entity->state++;
        }
    case 1:
        if (Anim_Update(entity) == 3)
        {
            entity->hidden = 1;
            entity->timer = 0x20;
            entity->state = 2;
        }
        break;
    case 2:
        if (entity->timer != 0)
        {
            if (!(entity->timer & 0xF))
            {
                struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, HeadStackEffect_UpdateFallingPiece);
                if (effect != NULL)
                {
                    effect->posX.part16.integer = entity->posX.part16.integer + RandomSigned(0x10);
                    effect->posY.part16.integer = entity->posY.part16.integer + RandomSigned(8) - 0x10;
                    effect->scrollWithBg = 1;
                }
            }
            entity->timer--;
        }
        else entity->deleteRequested = 1;
        break;
    }
}

/* Debris particle spawned from asm/this file.s. */
/* Graphics of one debris variant (passed to Entity_LoadGraphics). */

/**
 * @brief Load the variant selected by subState, choose random flips and a 16..31 countdown, then move with acceleration until deletion.
 */
void HeadStackEffect_UpdateParticle(struct EwramData_EntityData *entity)
{
    struct HeadStackGraphics *gfx;
    u32 rnd, bit;

    switch (entity->state)
    {
    case 0:
        gfx = &sUnk_081193DC[entity->subState];
        if (!Entity_LoadGraphics(entity, gfx->graphics, gfx->palette, 0, gfx->spriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        if (entity->subState == 0)
            entity->animFrame = 0x14;
        else
            entity->animFrame = 0x1A;
        entity->spriteLayer = 9;
        entity->objMode = 1;
        /* the mask is kept in a register across both random calls */
        rnd = RandomNumberGenerator();
        bit = 1;
        entity->flipX = rnd & bit;
        entity->flipY = RandomNumberGenerator() & bit;
        entity->timer = (RandomNumberGenerator() & 0xF) + 0x10;
        entity->state++;
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        if (entity->timer != 0)
            entity->timer--;
        else
            entity->deleteRequested = 1;
        break;
    }
}

/**
 * @brief Play the owner variant attack animation, enable its attack box at step 4, and end with the animation or owner death.
 */
void HeadStackEffect_UpdateAttack(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    struct EwramData_EntityData *child;

    switch (entity->state)
    {
    case 0:
    {
        const struct HeadStackGraphics *t = &sUnk_081193DC[owner->ext.headStack.type];
        if (!Entity_LoadGraphics(entity, t->graphics, t->palette, 0, t->spriteSheet))
        {
            EntityDelete(entity);
            return;
        }
    }
        Anim_StartIndexed(entity, 4, 0, 0);
        entity->spriteLayer = 9;
        entity->objMode = 1;
        if (owner->ext.headStack.type != 0)
        {
            child = EntityCreateInRange(0x49, 0xCF, HeadStackEffect_UpdateEmitter);
            if (child != NULL)
            {
                child->posX.part16.integer = entity->posX.part16.integer;
                child->posY.part16.integer = entity->posY.part16.integer + 0x10;
                child->flipX = entity->flipX;
                child->scrollWithBg = 1;
            }
        }
        Hit_Enable(entity, 8, HeadStackProjectile_DamagePlayerOnContact, 0);
        HitBox_SetAttackRaw(entity, 0x1032F8CE);
        entity->hit.flags |= 2;
        entity->hit.flags |= 4;
        PlaySong(0x6E);
        entity->timer = 8;
        entity->state++;
    case 1:
        if (entity->animTimer == 0)
            if (entity->animStep == 4)
                entity->hit.flags &= 0xFD;
        break;
    }
    Hit_CheckWithPlayer((struct HitResult *)collision, entity);
    if (Anim_Update(entity) == 3 || owner->state == 2)
        entity->deleteRequested = 1;
}

/* Enemy projectile: emits trailing particles every eight frames. */
/**
 * @brief Initialize a moving attack projectile and emit trailing variant particles; delete when health reaches zero or it leaves the screen.
 */
void HeadStackProjectile_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    s32 vx;
    u32 timer;
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    struct EwramData_EntityData *child;
    switch (entity->state)
    {
    case 0:
    {
        const struct HeadStackGraphics *t = &sUnk_081193DC[owner->ext.headStack.type];
        if (!Entity_LoadGraphics(entity, t->graphics, t->palette, 0, t->spriteSheet))
        {
            EntityDelete(entity);
            return;
        }
    }
        Anim_StartIndexed(entity, 5, 0, 1);
        entity->spriteLayer = 9;
        Entity_PlaceRelativeToCopy(entity, owner, -8, -12);
        vx = -0x20000;
        entity->velX.whole = vx;
        if (entity->flipX) entity->velX.whole = -vx;
        PlaySong(0x7A);
        entity->hp = 1;
        Hit_Enable(entity, 10, HeadStackProjectile_DamagePlayerOnContact, HeadStackProjectile_ClearHealth);
        HitBox_SetAttackRaw(entity, 0x0808FCFC);
        HitBox_SetBodyRaw(entity, 0x0808FCFC);
        entity->state++;
    case 1:
        timer = entity->timer + 1;
        entity->timer = timer;
        if ((timer & 7) == 0)
        {
            child = EntityCreateInRange(0x49, 0xCF, HeadStackEffect_UpdateParticle);
            if (child != NULL)
            {
                child->posX.part16.integer = entity->posX.part16.integer + RandomSigned(4);
                child->posY.part16.integer = entity->posY.part16.integer + RandomSigned(8);
                child->velX.whole = -(entity->velX.whole + RandomSigned(0x8000)) / 4;
                child->velY.whole = -0x1000;
                child->accelX.whole = -0x100;
                child->scrollWithBg = 1;
                child->subState = entity->ext.headStack.type;
            }
        }
        EntityAccelerateAndMove(entity, 0x80000);
        break;
    }
    Hit_CheckWithPlayer((struct HitResult *)collision, entity);
    Anim_Update(entity);
    if ((s16)entity->hp == 0)
    {
        Effect_SpawnAt(entity->posX.whole, entity->posY.whole, entity->spriteLayer);
        entity->deleteRequested = 1;
    }
    if (EntityIsOutsideScreen(entity, 1)) entity->deleteRequested = 1;
}

#define FRAME_PAIR(e) (*(u16 *)&(e)->animStep)

/* Attack routine of a spawner enemy: spawns HeadStackEffect_UpdateAttack / HeadStackEffect_UpdateParticle /
 * HeadStackProjectile_Update on animation frames, then turns towards the player. */
/**
 * @brief Emit attacks selected by segment-index parity at animation events; wait and turn toward the target.
 */
void HeadStackSegment_UpdateAttack(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *spawned;
    s32 dx; /* also reused as the facing flag in state 4 */
    s32 offset;
    s32 i;
    u8 timer;

    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->subState = 1;
        return;
    case 1:
        if (!(entity->animFlags & 4))
            break;
        Anim_StartIndexed(entity, 3, 0, 0);
        if (!(entity->ext.headStack.level & 1))
        {
            entity->subState = 2;
            return;
        }
        entity->subState = 3;
        return;
    case 2:
        if (FRAME_PAIR(entity) == 2)
        {
            spawned = EntityCreateInRange(0x49, 0xCF, HeadStackEffect_UpdateAttack);
            if (spawned != NULL)
            {
                u16 off = 0xFFFC;
                spawned->flipX = entity->flipX;
                if (entity->flipX)
                    off = 4;
                {
                    u16 *x = &entity->posX.part16.integer;
                    s32 value = (s16)off;
                    value += *x;
                    x = &spawned->posX.part16.integer;
                    *x = value;
                }
                spawned->posY.part16.integer = entity->posY.part16.integer - 10;
                spawned->scrollWithBg = 1;
                spawned->attack = entity->attack;
                ENTITY_OWNER(spawned) = entity;
            }
        }
        if (FRAME_PAIR(entity) == 0x2802)
        {
            dx = -12;
            if (entity->flipX)
                dx = 12;
            offset = dx * 2;
            for (i = 2; i >= 0; i--)
            {
                spawned = EntityCreateInRange(0x49, 0xCF, HeadStackEffect_UpdateParticle);
                if (spawned != NULL)
                {
                    spawned->posX.part16.integer = entity->posX.part16.integer + offset + RandomSigned(0x10);
                    spawned->posY.part16.integer = ({ s32 t = RandomSigned(0x10) - 0x10; t; }) /* keep order */ + entity->posY.part16.integer;
                    spawned->velY.whole = RandomSigned(0x4000) - 0x1000;
                    spawned->accelY.whole = -0x800;
                    spawned->scrollWithBg = 1;
                    spawned->subState = entity->ext.headStack.type;
                }
                offset += dx;
            }
        }
        entity->timer++;
        if (entity->animFlags & 4)
        {
            entity->timer = 0x20;
            entity->subState = 4;
        }
        break;
    case 3:
        if (FRAME_PAIR(entity) == 2)
        {
            spawned = EntityCreateInRange(0x49, 0xCF, HeadStackProjectile_Update);
            if (spawned != NULL)
            {
                spawned->scrollWithBg = 1;
                spawned->attack = entity->attack;
                ENTITY_OWNER(spawned) = entity;
            }
        }
        if (entity->animFlags & 4)
        {
            entity->timer = 0x20;
            entity->subState = 4;
        }
        break;
    case 4:
        if (entity->timer != 0)
            goto count;
        {
            s32 t = Target_GetX(entity);
            s32 px = entity->posX.whole;
            dx = 1;
            if (px > t)
                dx = 0;
        }
        if (entity->flipX != dx)
        {
            Anim_StartIndexed(entity, 2, 0, 0);
            entity->subState = 5;
        }
        else
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->subState = 6;
            entity->timer = 0x80;
        }
        break;
    case 5:
        if (entity->animFlags & 4)
        {
            entity->flipX ^= 1;
            entity->state = 0;
            entity->stage = 0;
            entity->subState = 0;
            Anim_StartIndexed(entity, 0, 0, 1);
        }
        break;
    case 6:
        if (entity->timer != 0)
        {
        count:
            entity->timer--;
            break;
        }
        entity->state = 0;
        entity->stage = 0;
        entity->subState = 0;
        break;
    }
}

/**
 * @brief Spawn debris frames from the variant table, then hide for eight countdown ticks; return 1 when finished.
 */
s32 HeadStackSegment_UpdateDeath(struct EwramData_EntityData *entity)
{
    const struct HeadStackGraphics *info = &sUnk_081193DC[entity->ext.headStack.type];
    struct EwramData_EntityData *piece;
    s32 i;

    switch (entity->subState)
    {
    case 0:
        PlaySong(0x6B);
        for (i = info->lastDebrisFrame; i >= info->firstDebrisFrame; i--)
        {
            piece = EntityCreateInRange(0x49, 0xCF, HeadStackDebris_Update);
            if (piece != NULL)
            {
                const struct HeadStackGraphics *t = &sUnk_081193DC[entity->ext.headStack.type];
                Entity_LoadGraphics(piece, t->graphics, t->palette, 0, t->spriteSheet);
                piece->animFrame = i;
                piece->posX.part16.integer = entity->posX.part16.integer;
                piece->posY.part16.integer = entity->posY.part16.integer;
                piece->velX.whole = RandomSigned(0x10) << 13;
                piece->velY.whole = (RandomSigned(0x10) << 12) - 0x30000;
                piece->accelY.whole = 0x2000;
                piece->spriteLayer = entity->spriteLayer;
                piece->flipX = entity->flipX;
                piece->scrollWithBg = 1;
            }
        }
        entity->timer = 8;
        entity->subState = 1;
        break;
    case 1:
        entity->hidden = 1;
        if (entity->timer == 0)
            return 1;
        entity->timer--;
        break;
    }
    return 0;
}

/* Column member settling. */
/* old + (s16)(target - old): move a u16 coordinate onto target. */
static inline s32 Approach(s32 target, u16 old) { return (s16)(target - old) + old; }

/* posY of entity slot idx; the offset is computed before gEwramData is read. */
static inline s32 SlotY(u32 idx)
{
    return ((struct EwramData_EntityData *)(({ u32 o_ = idx * sizeof(struct EwramData_EntityData); o_; })
                                            + (u32)gEwramData + 0x4E4))->posY.part16.integer;
}

/**
 * @brief Fall onto a position 22 pixels above the highest occupied lower segment, or above the controller.
 */
void HeadStackSegment_Settle(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    u8 *slots = owner->ext.headStack.slots;
    s32 i;
    s32 y;
    u16 old;

    for (i = entity->ext.headStack.level; i > 0; i--)
    {
        if (slots[i - 1] != 0)
        {
            y = SlotY(slots[i - 1]);
            goto found;
        }
    }
    y = owner->posY.part16.integer;
found:
    y -= 22;
    entity->posY.whole += entity->velY.whole;
    old = entity->posY.part16.integer;
    {
        s32 cur = entity->posY.part16.integer;

        /* in-place shifts (not (s16)y) keep the truncation in y's register */
        y <<= 16;
        y >>= 16;
        if (cur >= y)
        {
            entity->posY.part16.integer = Approach(y, old);
            entity->velY.whole = 0x1000;
        }
    }
    entity->velY.whole += 0x1000;
}

/* Update of a stacked column member. */

/**
 * @brief Settle a stack segment and dispatch proximity wait, attack, death or animation-only state.
 */
void HeadStackSegment_Update(struct EwramData_EntityData *entity)
{
    u32 buf[4];
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);

    if (entity->state != 2)
    {
        if (owner->ext.headStack.onScreen == 0)
            return;
        HeadStackSegment_Settle(entity);
    }
    if (Enemy_IsStunned(entity) != 0)
    {
        if (entity->state != 2)
            Hit_CheckWithPlayer((struct HitResult *)buf, entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        if (entity->subState == 0
            && Target_IsInRect(entity->posX.part16.integer - 0x50, entity->posY.part16.integer - 0x3C, 0xA0, 0x78))
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 1:
        HeadStackSegment_UpdateAttack(entity);
        break;
    case 2:
        if (HeadStackSegment_UpdateDeath(entity) != 0)
        {
            EntityDelete(entity);
            return;
        }
        break;
    case 3:
        Anim_Update(entity);
        return;
    }
    if (entity->state != 2)
        Hit_CheckWithPlayer((struct HitResult *)buf, entity);
    Anim_Update(entity);
}

/* HeadStack_Create: create a stack of child segments from table sUnk_081193DC; matched. */

struct Flags58 { u8 rest:6; u8 flipX:1; u8 flipY:1; } __attribute__((packed));

static inline u8 InitFromTable(struct EwramData_EntityData *entity, s32 type)
{
    struct HeadStackGraphics *info = &sUnk_081193DC[type];
    return Entity_LoadGraphics(entity, info->graphics, info->palette, 0, info->spriteSheet);
}

/**
 * @brief Create the Bone Pillar (type 0) or Fish Head (type 1) controller and its stack of 1..8 segments.
 */
void HeadStack_Create(struct EwramData_EntityData *entity, s32 type)
{
    s32 failed = 0;
    s32 savedY;
    u8 *slot;
    u8 *slots;
    s32 i;
    struct EwramData_EntityData *child;

    if (!InitFromTable(entity, type))
        goto delete;
    entity->drawFunc = NULL;
    if (!gEwramData->gameState.bestiaryPreview)
    {
        Entity_SnapToGround(entity);
        entity->posY.part16.integer++;
    }
    else
    {
        entity->spawnParam0 = 2;
    }
    if (entity->spawnParam0 <= 0)
        entity->spawnParam0 = 1;
    else if (entity->spawnParam0 > 8)
        entity->spawnParam0 = 8;
    savedY = entity->posY.whole;
    slot = entity->ext.headStack.slots;
    i = 1;
    slots = slot;
    goto test;
body:

        *slot = ENTITY_INDEX(child);
        child->ext.headStack.level = i - 1;
        ENTITY_OWNER(child) = entity;
        child->ext.headStack.type = type;
        {
            register s32 off asm("r3") = type << 4; /* FAKEMATCH: pinned register */
            struct HeadStackGraphics *info = (void *)(off + (u32)sUnk_081193DC);
            Entity_LoadGraphics(child, info->graphics, info->palette, 0, info->spriteSheet);
        }
        child->enemyId = entity->enemyId;
        Enemy_ResetFromTemplate(child);
        Anim_StartIndexed(child, 0, 0, 1);
        Anim_Update(child);
        {
            struct Flags58 *ef = (void *)(ENTITY_RENDER_FLAGS_ADDRESS(entity));
            u8 v = ef->flipX;
            register struct Flags58 *cf asm("r4") = (void *)(ENTITY_RENDER_FLAGS_ADDRESS(child)); /* FAKEMATCH: pinned register */
            cf->flipX = v;
            ef->flipX ^= 1;
        }
        child->spriteLayer = 10;
        child->posX.whole = entity->posX.whole;
        child->posY.whole = entity->posY.whole;
        entity->posY.whole -= 0x160000;
        child->scrollWithBg = 1;
        if (gEwramData->gameState.bestiaryPreview)
        {
            child->state = 3;
        }
        else
        {
            child->state = 0;
            Hit_Enable(child, 8, Enemy_OnContact, HeadStackSegment_OnHit);
            HitBox_SetAttackRaw(child, 0x141CECF2);
            HitBox_SetBodyRaw(child, 0x141CECF2);
        }
        i++;
        slot++;
test:
    if (i <= entity->spawnParam0)
    {
        child = Entity_CreateObject(HeadStackSegment_Update);
        if (child != NULL)
            goto body;
        entity->spawnParam0 = i;
        failed = 1;
    }

    if (failed)
    {
        slot = slots;
        for (i = entity->spawnParam0; i > 0; slot++, i--)
        {
            child = &gEwramData->entityData[*slot];
            EntityDelete(child);
        }
    delete:
        EntityDelete(entity);
        return;
    }
    entity->posY.whole = savedY + 0x160000;
    entity->scrollWithBg = 1;
}

/* Stack keeper. */
/**
 * @brief Set the controller visibility flag, discard dying segment slots and select the highest live segment; delete an empty stack.
 */
void HeadStack_Update(struct EwramData_EntityData *entity)
{
    s32 found = 0;
    u8 *slot;
    s32 i;

    if (!Entity_IsInRect(entity, -32, -32, 0x130, (u16)(entity->spawnParam0 * 32 + 0xE0)))
    {
        entity->ext.headStack.onScreen = 0;
        return;
    }
    entity->ext.headStack.onScreen = 1;
    slot = (entity->spawnParam0 + entity->ext.headStack.slots) - 1;
    for (i = entity->spawnParam0; i > 0; slot--, i--)
    {
        if (*slot != 0)
        {
            struct EwramData_EntityData *e = &gEwramData->entityData[*slot];

            if (e->state == 2)
                *slot = 0;
            else if (!found)
            {
                Player_QueueEntity(e, 0);
                found = 1;
            }
        }
    }
    if (!found)
        EntityDelete(entity);
}

/* Bone Pillar / Fish Head helpers 0x080B3E70..0x080B3F4B. */

/**
 * @brief Apply damage; on death drop rewards, restore the palette and enter state 2.
 */
void HeadStackSegment_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/**
 * @brief B3EB0 | Player_ApplyContactDamage with (attack, 2, 0, 1) at the entity position.
 */
void HeadStackProjectile_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 2;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief Clear projectile health; used as a projectile hit callback.
 */
void HeadStackProjectile_ClearHealth(struct EwramData_EntityData *entity)
{
    entity->hp = 0;
}

/**
 * @brief Damp horizontal velocity, accelerate and move downward, and request deletion outside the screen.
 */
void HeadStackDebris_Update(struct EwramData_EntityData *entity)
{
    EntityDampVelocityX(entity, 0x28);
    EntityAccelerateAndMove(entity, 0x80000);
    if (EntityIsOutsideScreen(entity, 4))
        entity->deleteRequested = 1;
}

/**
 * @brief B3F1C | Bone Pillar create: HeadStack_Create variant 0.
 */
void EnemyBonePillarCreate(struct EwramData_EntityData *entity)
{
    HeadStack_Create(entity, 0);
}

/**
 * @brief B3F28 | Bone Pillar update.
 */
void EnemyBonePillarUpdate(struct EwramData_EntityData *entity)
{
    HeadStack_Update(entity);
}

/**
 * @brief B3F34 | Fish Head create: HeadStack_Create variant 1.
 */
void EnemyFishHeadCreate(struct EwramData_EntityData *entity)
{
    HeadStack_Create(entity, 1);
}

/**
 * @brief B3F40 | Fish Head update: same as Bone Pillar.
 */
void EnemyFishHeadUpdate(struct EwramData_EntityData *entity)
{
    HeadStack_Update(entity);
}
