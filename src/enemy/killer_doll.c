/*
 * Killer Doll: create, graphics and states.
 * Built without -f2003-patch (EnemyKillerDollCreate needs it off; the rest compiles the same either way).
 */
#include "core.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "player_core.h"
#include "sound.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "entity_inline.h"
#include "hitbox.h"
#include "damage_numbers.h"
#include "entity_hit.h"

extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 hit, u16 *data, s32 kind);
extern void Entity_FaceTarget(struct EwramData_EntityData *entity);
extern s32 Collision_GetPushUp(s32 x, s32 y);
extern s32 Collision_GetPushDown(s32 x, s32 y);
extern s32 Collision_GetPushLeft(s32 x, s32 y);
extern s32 Collision_GetPushRight(s32 x, s32 y);

/* This caller tests the full return register (no u8 truncation). */
extern s32 Collision_GetTileOnScreen(s32 x, s32 y);
/* These callers use full return registers, without 16-bit truncation. */
extern s32 GetEntityRoomXPositionInteger(struct EwramData_EntityData *entity);
extern s32 GetEntityRoomYPositionInteger(struct EwramData_EntityData *entity);

s32 KillerDoll_LoadGraphics(struct EwramData_EntityData *entity);
void KillerDoll_OnContact(struct EwramData_EntityData *entity);
void KillerDoll_OnHit(struct EwramData_EntityData *entity, s32 hit, u16 *data);
void KillerDoll_ResetActive(struct EwramData_EntityData *entity);
void KillerDoll_SpawnDeathParts(struct EwramData_EntityData *entity);
void KillerDoll_UpdateState2Animation(struct EwramData_EntityData *entity);
void KillerDoll_UpdateActive(struct EwramData_EntityData *entity);
void KillerDoll_UpdateDeath(struct EwramData_EntityData *entity);
void KillerDoll_ResolveMovementBounds(struct EwramData_EntityData *entity);

/* EnemyKillerDollCreate. */

/**
 * @brief C7B9C | Killer Doll create: load the graphics (delete the enemy
 * when that fails), layer 9, hit boxes and KillerDoll_ResetActive; state 2 instead
 * when bestiaryPreview is set.
 */
void EnemyKillerDollCreate(struct EwramData_EntityData *entity)
{
    if (!KillerDoll_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    entity->state = 0;
    entity->stage = 0;
    entity->subState = 0;
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 9;
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
        return;
    }
    Entity_EnableBoxes(entity, KillerDoll_OnContact, KillerDoll_OnHit);
    KillerDoll_ResetActive(entity);
}

/* Graphics loader 0x080C7C04. It only matches without -f2003-patch (see
 * NO_2003_PATCH_SRCS in ../Makefile). */

/**
 * @brief Find or load graphics and palette, attach the sprite and start animation 0; return 0 on failure, 1 on success.
 */
s32 KillerDoll_LoadGraphics(struct EwramData_EntityData *entity)
{
    void *palette = sKillerDollLightningDollPalette;
    void *sprite = sKillerDollSpriteSheet;
    u8 *graphics = sKillerDollLightningDollGfx;
    s32 gfx = ObjGfx_Find(graphics);
    s8 paletteIndex;

    if (gfx < 0)
    {
        gfx = ObjGfx_LoadResource(graphics, 0);
        if (gfx < 0)
            return 0;
    }
    paletteIndex = ObjPalette_FindOrLoadVariant(palette, 0);
    if (paletteIndex == -1)
        return 0;
    entity->basePalette = paletteIndex;
    if (!Sprite_Attach(entity, sprite, gfx, entity->basePalette))
        return 0;
    if (!Anim_StartIndexed(entity, 0, 0, 1))
        return 0;
    Anim_Update(entity);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/* Killer Doll: stop movement, resolve floor placement and initialize three
 * randomized timers. */

/**
 * @brief Clear velocity and acceleration, snap to ground and enter state 1 with three randomized behavior countdowns.
 */
void KillerDoll_ResetActive(struct EwramData_EntityData *entity)
{
    s32 zero = 0;

    entity->accelX.whole = zero;
    entity->velX.whole = zero;
    entity->accelY.whole = zero;
    entity->velY.whole = zero;
    Entity_SnapToGround(entity);
    entity->state = 1;
    entity->stage = zero;
    entity->subState = zero;
    entity->ext.killerDoll.lungeTimer = (RandomNumberGenerator() % 0x800) + 0x80;
    entity->ext.killerDoll.turnTimer = (RandomNumberGenerator() % 0x100) + 0x20;
    entity->ext.killerDoll.restTimer = (RandomNumberGenerator() % 0x200) + 0x100;
}

/* Killer Doll movement and attack state machine, reconstructed from the
 * original assembly. */

#define TIMER_X(e) (e->ext.killerDoll.lungeTimer)
#define TIMER_Y(e) (e->ext.killerDoll.turnTimer)
#define TIMER_Z(e) (e->ext.killerDoll.restTimer)
#define ANIM(e, n, loop) do { if ((e)->animScriptIndex != (n)) Anim_StartIndexed((e), (n), 0, (loop)); } while (0)
#define ABS(x) ((x) < 0 ? -(x) : (x))

/**
 * @brief Dispatch awakening, floating movement, turning, charge and floor-wait substates; integrate velocity and use three countdowns to select behavior.
 */
void KillerDoll_UpdateActive(struct EwramData_EntityData *entity)
{
    Target_GetX(entity);
    Target_GetY(entity);
    if ((s16)TIMER_X(entity) != 0)
        TIMER_X(entity)--;
    if (TIMER_Y(entity) != 0)
        TIMER_Y(entity)--;
    if (TIMER_Z(entity) != 0)
        TIMER_Z(entity)--;

    switch (entity->subState)
    {
        case 0:
            ANIM(entity, 0, 1);
            if (Entity_IsTargetWithinRange(entity, 0x3C0000) &&
                (Target_GetX(entity) != entity->posX.whole ||
                 Target_GetY(entity) != entity->posY.whole))
            {
                ANIM(entity, 1, 0);
                entity->timer = 0;
                entity->subState++;
            }
            break;
        case 1:
            if (entity->animDone)
            {
                ANIM(entity, 2, 0);
                entity->velY.whole = -0x4000;
                entity->timer = 0;
                entity->subState++;
            }
            break;
        case 2:
            if (entity->timer++ > 0x20)
            {
                ANIM(entity, 4, 1);
                entity->velY.whole = 0;
                entity->timer = 0;
                entity->subState = 4;
                entity->spriteLayer = 3;
            }
            break;
        case 3:
            switch (entity->stage)
            {
                case 0:
                    entity->accelX.whole = 0;
                    entity->velX.whole = 0;
                    entity->accelY.whole = 0;
                    entity->velY.whole = 0;
                    if ((RandomNumberGenerator() & 3) == 0)
                    {
                        Entity_FaceTarget(entity);
                        entity->flipX = 1 - entity->flipX;
                        entity->velX.whole = 0x4000;
                        if ((s16)Entity_GetTargetDistanceX(entity) < 0)
                            entity->velX.whole = -entity->velX.whole;
                    }
                    else
                    {
                        entity->velX.whole = 0x8800;
                        entity->velY.whole = 0x8800;
                        if ((s16)Entity_GetTargetDistanceX(entity) > 0)
                            entity->velX.whole = -entity->velX.whole;
                        if ((s16)Entity_GetTargetDistanceY(entity) > 0)
                            entity->velY.whole = -entity->velY.whole;
                    }
                    ANIM(entity, 4, 1);
                    entity->stage++;
                    break;
                case 1:
                    if (!entity->flipX)
                    {
                        if (entity->velX.whole > 0)
                            entity->flipX = 1;
                    }
                    if (entity->flipX)
                    {
                        if (entity->velX.whole < 0)
                            entity->flipX = 0;
                    }
                    KillerDoll_ResolveMovementBounds(entity);
                    break;
            }
            break;
        case 4:
            switch (entity->stage)
            {
                case 0:
                    entity->accelX.whole = 0;
                    entity->velX.whole = 0;
                    entity->accelY.whole = 0;
                    entity->velY.whole = 0;
                    ANIM(entity, 3, 0);
                    if (entity->animDone)
                    {
                        entity->subState = 3;
                        entity->stage = 0;
                        TIMER_X(entity) += 0x20;
                        TIMER_Z(entity) += 0x20;
                        TIMER_Y(entity) = RandomNumberGenerator() % 0x100 + 0x20;
                        Entity_FaceTarget(entity);
                    }
                    break;
            }
            break;
        case 5:
            switch (entity->stage)
            {
                case 0:
                {
                    u16 angle;
                    ANIM(entity, 5, 0);
                    angle = Entity_GetTargetAngleAbove16(entity);
                    entity->velX.whole = FixedMultiply(Sin(angle + 0x14000), 0x10000);
                    entity->velY.whole = FixedMultiply(Sin(angle + 0x10000), 0x10000);
                    entity->timer = ABS((Target_GetX(entity) - entity->posX.whole) / entity->velX.whole);
                    entity->stage++;
                }
                case 1:
                {
                    if ((s8)entity->timer-- == 0)
                        entity->stage++;
                    break;
                }
                case 2:
                    if (!entity->flipX || (s16)Entity_GetTargetDistanceX(entity) >= 0)
                    {
                        entity->subState = 4;
                        entity->stage = 0;
                    }
                    else
                    {
                        entity->subState = 3;
                        entity->stage = 0;
                    }
                    TIMER_X(entity) = RandomNumberGenerator() % 0x800 + 0x80;
                    TIMER_Y(entity) += 0x20;
                    TIMER_Z(entity) += 0x20;
                    break;
            }
            break;
        case 6:
            switch (entity->stage)
            {
                case 0:
                    entity->spriteLayer = 9;
                    entity->accelX.whole = 0;
                    entity->velX.whole = 0;
                    entity->accelY.whole = 0;
                    entity->velY.whole = 0;
                    entity->stage = 0;
                case 1:
                    if (Collision_GetTileOnScreen((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer) == 0)
                        entity->posY.part16.integer++;
                    else
                    {
                        ANIM(entity, 6, 0);
                        entity->timer = RandomNumberGenerator() % 0x40 + 0x40;
                        entity->stage++;
                    }
                    break;
                case 2:
                    if (--entity->timer == 0xFF)
                    {
                        TIMER_X(entity) += 0x20;
                        TIMER_Y(entity) += 0x20;
                        TIMER_Z(entity) = RandomNumberGenerator() % 0x400 + 0x200;
                        ANIM(entity, 1, 0);
                        entity->subState = 1;
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
    if (entity->subState == 3)
    {
        if (TIMER_Z(entity) == 0)
        {
            entity->subState = 6;
            entity->stage = 0;
        }
        if (TIMER_Y(entity) == 0)
        {
            entity->subState = 4;
            entity->stage = 0;
        }
        if ((s16)TIMER_X(entity) == 0)
        {
            if ((entity->flipX && (s16)Entity_GetTargetDistanceX(entity) < 0) ||
                (!entity->flipX && (s16)Entity_GetTargetDistanceX(entity) > 0))
            {
                if (Entity_IsTargetWithinRange(entity, 0x280000))
                {
                    entity->subState = 5;
                    entity->stage = 0;
                }
            }
        }
    }
}

/* Killer Doll damage and transition to death, reconstructed from the
 * original assembly. */

/**
 * @brief Resolve damage and flash; on lethal damage spawn counted body parts, drop rewards and wait for the part count to reach zero.
 */
void KillerDoll_OnHit(struct EwramData_EntityData *entity, s32 hit, u16 *data)
{
    s32 damage = Hit_ResolveDamage(entity, hit, data, (s16)Player_CalculateAttackPower(data[2]));
    u16 hp = entity->hp;

    damage = (damage << 16) >> 16;
    entity->hp = hp - damage;
    Entity_ShowDamage(entity, (s16)damage, 0);
    entity->hitFlashTimer = 8;
    if ((s16)entity->hp <= 0)
    {
        entity->subState = 0;
        PlaySong(0x6E);
        KillerDoll_SpawnDeathParts(entity);
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->state = 0xFF;
        entity->subState++;
        if (entity->ext.doll.deathPartCount == 0)
            entity->deleteRequested = 1;
    }
}

/* KillerDollPartEffect_Update. */

/**
 * @brief Attach cached effect graphics and follow the body part with its table-derived vertical offset while advancing animation.
 */
void KillerDollPartEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *doll = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];

    switch (entity->state)
    {
        case 0:
        {
            s32 gfx = ObjGfx_Find(sCommonObjectGfx);

            Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, 7);
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->scrollWithBg = 1;
            entity->drawFunc = Sprite_DrawEntity;
            entity->state++;
            break;
        }
        case 1:
            entity->posX.whole = doll->posX.whole;
            entity->posY.whole = doll->posY.whole - 0x100000;
            entity->posY.part16.integer += sKillerDollPartOffsets[(u8)doll->ext.dollPart.index][1];
            break;
    }
    Anim_Update(entity);
}

/* Killer Doll body parts after death. */

static inline s32 Hidden(struct EwramData_EntityData *e) { return 1 - e->hidden; }
/**
 * @brief Initialize a table-configured body part and attached effect, fall and optionally bounce; decrement the parent count after the attached animation completes.
 */
void KillerDollPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    struct EwramData_EntityData *child = &gEwramData->entityData[entity->ext.dollPart.effectIndex];
    s32 x, y;
    s32 state = entity->state;

    switch (state)
    {
        case 0:
        {
            s32 graphics;
            s32 sprite, vx, vy, timer;
            void *spriteData;
            struct EwramData_EntityData *effect;
            entity->scrollWithBg = 1;
            spriteData = sKillerDollSpriteSheet;
            graphics = Sprite_GetTileSlot(parent);
            Sprite_Attach(entity, spriteData, graphics, ObjPalette_FindOrLoad(sKillerDollLightningDollPalette));
            entity->drawFunc = Sprite_DrawEntity;
            entity->posX.whole = parent->posX.whole;
            entity->posY.whole = parent->posY.whole;
            x = sKillerDollPartOffsets[(u8)entity->ext.dollPart.index][0];
            sprite = sKillerDollPartOffsets[(u8)entity->ext.dollPart.index][2];
            vx = sKillerDollPartOffsets[(u8)entity->ext.dollPart.index][3] << 12;
            vy = sKillerDollPartOffsets[(u8)entity->ext.dollPart.index][4] << 12;
            timer = sKillerDollPartOffsets[(u8)entity->ext.dollPart.index][5];
            entity->velX.whole = vx;
            entity->velY.whole = -vy;
            entity->accelY.whole = 0x1800;
            entity->timer = timer;
            entity->animFrame = sprite;
            entity->flipX = parent->flipX;
            if ((s8)Entity_GetTargetDirection(parent) > 0)
            {
                entity->velX.whole = -ABS(entity->velX.whole);
                x = -ABS(x);
            }
            effect = EntityCreateInRange(0x49, 0xCF, KillerDollPartEffect_Update);
            if (effect != NULL)
            {
                effect->posX.whole = entity->posX.whole;
                effect->posY.whole = entity->posY.whole - 0x100000;
                effect->flipX = RandomNumberGenerator() % 2;
                effect->spriteLayer = entity->spriteLayer + 1;
                ENTITY_PARENT_INDEX(effect) = ENTITY_INDEX(entity);
                entity->ext.dollPart.effectIndex = ENTITY_INDEX(effect);
            }
            entity->state++;
            break;
        }
        case 1:
        {
            s32 correction;
            s32 zero;
            s32 done;
            state &= entity->timer;
            if (state && entity->timer < 0x40)
                entity->hidden = Hidden(entity);
            zero = entity->timer & 15;
            if (zero == 0 && entity->timer < 0x40 && entity->timer > 0x20)
            {
                x = sKillerDollPartOffsets[(u8)entity->ext.dollPart.index][0];
                y = sKillerDollPartOffsets[(u8)entity->ext.dollPart.index][1];
                Effect_SpawnRandom(entity->posX.whole + x, entity->posY.whole + y, 0, 0, entity->spriteLayer - 1);
                Effect_SpawnRisingParticleInBox(entity->posX.whole + x, entity->posY.whole + y, 0, 0, entity->spriteLayer - 2, zero);
            }
            entity->posX.whole += entity->velX.whole;
            entity->posY.whole += entity->velY.whole;
            entity->velY.whole += entity->accelY.whole;
            if ((u8)entity->ext.dollPart.index == 1)
            {
                if (entity->velY.whole >= 0)
                {
                    correction = Collision_GetPushUp(ToRoomX((s16)entity->posX.part16.integer), ToRoomY((s16)entity->posY.part16.integer) - 15);
                    if (correction != 0)
                    {
                        PlaySong(0x153);
                        entity->posY.part16.integer += correction;
                        entity->velY.whole = -entity->velY.whole / 2;
                        if (ABS(entity->velY.whole) < 0x6000)
                        {
                            entity->accelX.whole = 0;
                            entity->velX.whole = 0;
                            entity->accelY.whole = 0;
                            entity->velY.whole = 0;
                        }
                    }
                }
                else
                {
                    correction = Collision_GetPushDown(ToRoomX((s16)entity->posX.part16.integer), ToRoomY((s16)entity->posY.part16.integer) - 32);
                    if (correction != 0)
                        entity->posY.part16.integer += correction;
                }
                if (entity->velX.whole > 0)
                    correction = Collision_GetPushLeft(ToRoomX((s16)entity->posX.part16.integer) + 8, ToRoomY((s16)entity->posY.part16.integer) - 16);
                else
                {
                    if (entity->velX.whole == 0)
                        goto timer;
                    correction = Collision_GetPushRight(ToRoomX((s16)entity->posX.part16.integer) - 8, ToRoomY((s16)entity->posY.part16.integer) - 16);
                }
                if (correction != 0)
                {
                    entity->posX.part16.integer += correction;
                    entity->velX.whole = 0;
                }
            }
timer:
            done = 0;
            if (--entity->timer == 0)
                done = 1;
            if ((s16)entity->posY.part16.integer > 200)
                done = 2;
            if (done != 0)
            {
                entity->velY.whole = 0;
                entity->velX.whole = 0;
                entity->state++;
                entity->hidden = 1;
            }
            break;
        }
        case 2:
            if (child->animDone)
            {
                parent->ext.doll.deathPartCount--;
                entity->deleteRequested = 1;
                child->deleteRequested = 1;
            }
            break;
    }
}

/**
 * @brief Reverse velocity at room boundaries; apply directional tile corrections and stop the affected velocity component on collision.
 */
void KillerDoll_ResolveMovementBounds(struct EwramData_EntityData *entity)
{
    s32 size;
    s32 tile;
    s32 limit;
    s32 outside;
    s32 outsideY;
    s32 correction;
    struct EwramData **state;

    GetEntityRoomYPositionInteger(entity);
    state = &gEwramData;
    size = ((u8 *)(*state)->bgInfo[1].pBgMetadata)[0];
    tile = GetEntityRoomXPositionInteger(entity) / 8;
    limit = 30;
    if (size != 1)
        limit = size << 5;
    if (tile < 0 || (outside = tile >= limit))
    {
        entity->velX.whole = -entity->velX.whole;
        entity->posX.whole += entity->velX.whole * 2;
        entity->flipX = 1 - entity->flipX;
    }
    else
    {
        if (entity->velX.whole >= 0)
            correction = Collision_GetPushLeft(ToRoomX((s16)entity->posX.part16.integer) + 8,
                                     ToRoomY((s16)entity->posY.part16.integer) - 16);
        else
            correction = Collision_GetPushRight(ToRoomX((s16)entity->posX.part16.integer) - 8,
                                     ToRoomY((s16)entity->posY.part16.integer) - 16);
        if (correction != 0)
        {
            entity->posX.part16.integer += correction;
            entity->velX.whole = outside;
            entity->flipX = 1 - entity->flipX;
        }
    }

    GetEntityRoomXPositionInteger(entity);
    state = &gEwramData;
    size = ((u8 *)(*state)->bgInfo[1].pBgMetadata)[1];
    tile = GetEntityRoomYPositionInteger(entity) / 8;
    limit = 26;
    if (size != 1)
        limit = size << 5;
    if (tile < 0 || (outsideY = tile >= limit))
    {
        entity->velY.whole = -entity->velY.whole;
        entity->posY.whole += entity->velY.whole * 2;
    }
    else
    {
        if (entity->velY.whole >= 0)
            correction = Collision_GetPushUp(ToRoomX((s16)entity->posX.part16.integer),
                                     ToRoomY((s16)entity->posY.part16.integer) + 1);
        else
            correction = Collision_GetPushDown(ToRoomX((s16)entity->posX.part16.integer),
                                     ToRoomY((s16)entity->posY.part16.integer) - 32);
        if (correction != 0)
        {
            entity->posY.part16.integer += correction;
            entity->velY.whole = outsideY;
        }
    }
}

/* Killer Doll update and death waiting for the body parts to disappear.
 * Reconstructed from the original assembly. */

void EnemyKillerDollUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;

    if (entity->state == 2)
    {
        KillerDoll_UpdateState2Animation(entity);
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
        case 1:
            KillerDoll_UpdateActive(entity);
            break;
        case 0xFF:
            KillerDoll_UpdateDeath(entity);
            break;
    }
    if (entity->state != 0xFF)
    {
        Entity_AnimateWithBoxes(entity);
        Hit_CheckWithPlayer(&result, entity);
    }
}

/**
 * @brief Spawn body parts and drop rewards once, restore the palette and request deletion when the live-part count reaches zero.
 */
void KillerDoll_UpdateDeath(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            PlaySong(0x6E);
            KillerDoll_SpawnDeathParts(entity);
            Enemy_DropRewards(entity);
            entity->palette = entity->basePalette;
            entity->state = 0xFF;
            entity->subState++;
        case 1:
            if (entity->ext.doll.deathPartCount == 0)
                entity->deleteRequested = 1;
            break;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   KillerDoll_UpdateState2Animation = Ukoback_UpdateState1Animation
 *   KillerDoll_OnContact = Enemy_DamagePlayerOnContact */

/**
 * @brief Start animation 0 once and then advance it. The gameplay purpose of state 2 remains unresolved.
 */
void KillerDoll_UpdateState2Animation(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->subState++;
    }
    Anim_Update(entity);
}

/**
 * @brief Submit the attack contact record with mode fields 0, 0, 0.
 */
void KillerDoll_OnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/* KillerDoll_SpawnDeathParts. */
/**
 * @brief Hide the parent and attempt to create five indexed body parts; store the parent slot and increment its live-part count for each success.
 */
void KillerDoll_SpawnDeathParts(struct EwramData_EntityData *entity)
{
    s32 i;

    entity->hidden = 1;
    for (i = 0; i < 5; i++)
    {
        struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, KillerDollPart_Update);

        if (child != NULL)
        {
            ENTITY_PARENT_INDEX(child) = ENTITY_INDEX(entity);
            child->ext.dollPart.index = i;
            entity->ext.doll.deathPartCount++;
            child->spriteLayer = entity->spriteLayer;
        }
    }
}
