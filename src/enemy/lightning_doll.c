/*
 * Lightning Doll: create, graphics and states.
 * Built without -f2003-patch (EnemyLightningDollCreate needs it off; the rest compiles the same either way).
 */
#include "core.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "sound.h"
#include "player_core.h"
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
#include "player_soma.h"
#include "entity_hit.h"
#include "bg_and_collision.h"
#include "music.h"

s32 LightningDoll_LoadGraphics(struct EwramData_EntityData *entity);
void LightningDoll_ResetActive(struct EwramData_EntityData *entity);
void LightningDoll_OnContact(struct EwramData_EntityData *entity);
void LightningDoll_OnHit(struct EwramData_EntityData *entity, s32 hit, u16 *data);
void LightningDoll_UpdateState2Animation(struct EwramData_EntityData *entity);
void LightningDoll_UpdateActive(struct EwramData_EntityData *entity);
void LightningDoll_UpdateDeath(struct EwramData_EntityData *entity);
void LightningDoll_ResolveMovementBounds(struct EwramData_EntityData *entity);
void LightningDoll_SpawnDeathParts(struct EwramData_EntityData *entity);
void LightningDollPlayerEffect_Update(struct EwramData_EntityData *entity);

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   EnemyLightningDollCreate = EnemyKillerDollCreate */

/**
 * @brief C89B0 | Same as EnemyKillerDollCreate.
 */
void EnemyLightningDollCreate(struct EwramData_EntityData *entity)
{
    if (!LightningDoll_LoadGraphics(entity))
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
    Entity_EnableBoxes(entity, LightningDoll_OnContact, LightningDoll_OnHit);
    LightningDoll_ResetActive(entity);
}

/* Graphics loader 0x080C8A18. It only matches without -f2003-patch (see
 * NO_2003_PATCH_SRCS in ../Makefile). */

/**
 * @brief Find or load graphics and palette, attach the sprite and start animation 0; return 0 on failure, 1 on success.
 */
s32 LightningDoll_LoadGraphics(struct EwramData_EntityData *entity)
{
    u8 *graphics = sKillerDollLightningDollGfx;
    s32 gfx = ObjGfx_Find(graphics);
    s8 paletteIndex;

    if (gfx < 0)
    {
        gfx = ObjGfx_LoadResource(graphics, 0);
        if (gfx < 0)
            return 0;
    }
    paletteIndex = ObjPalette_FindOrLoadVariant(sKillerDollLightningDollPalette, 0);
    if (paletteIndex == -1)
        return 0;
    entity->basePalette = paletteIndex;
    if (!Sprite_Attach(entity, sLightningDollSpriteSheet, gfx, entity->basePalette))
        return 0;
    if (!Anim_StartIndexed(entity, 0, 0, 1))
        return 0;
    Anim_Update(entity);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/* Lightning Doll helpers reconstructed from the original assembly. */

void EnemyLightningDollUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    if (entity->state == 2)
    {
        LightningDoll_UpdateState2Animation(entity);
        return;
    }
    Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity) && entity->state != 0xFF)
    {
        Sound_StopTracked(0x75);
        Hit_CheckWithPlayer(&result, entity);
        return;
    }
    switch (entity->state)
    {
        case 1:
            LightningDoll_UpdateActive(entity);
            break;
        case 0xFF:
            LightningDoll_UpdateDeath(entity);
            break;
    }
    if (entity->state != 0xFF)
    {
        u8 frame = entity->animFrame;
        if (Anim_Update(entity) && frame != entity->animFrame)
        {
            if (entity->animScriptIndex == 4)
                HitBox_SetAttackRaw(entity, 0x1C1EE0F2);
            else
                Entity_UpdateBoxes(entity);
        }
        Hit_CheckWithPlayer(&result, entity);
    }
}

/* Lightning Doll movement and electrical attack states. */

#define TIMER_X(e) (e->ext.lightningDoll.attackTimer)
#define TIMER_Y(e) (e->ext.lightningDoll.moveTimer)
#define TIMER_Z(e) (e->ext.lightningDoll.restTimer)
#define ANIM(e, n, loop) do { if ((e)->animScriptIndex != (n)) Anim_StartIndexed((e), (n), 0, (loop)); } while (0)
#define ABS(x) ((x) < 0 ? -(x) : (x))

/**
 * @brief Dispatch awakening, randomized flight, electrical attack and floor-wait substates; integrate velocity and select behavior from countdowns and target checks.
 */
void LightningDoll_UpdateActive(struct EwramData_EntityData *entity)
{
    if ((s16)TIMER_X(entity) != 0)
        TIMER_X(entity)--;
    if (TIMER_Z(entity) != 0)
        TIMER_Z(entity)--;
    switch (entity->subState)
    {
        case 0:
            ANIM(entity, 0, 1);
            if (Target_IsNearEntity(entity, 240, 128) &&
                (Target_GetX(entity) != entity->posX.whole || Target_GetY(entity) != entity->posY.whole))
            {
                ANIM(entity, 1, 0);
                entity->subState = 1;
                entity->stage = 0;
            }
            break;
        case 1:
            if (entity->animDone)
            {
                entity->timer = 0;
                entity->subState = 2;
                entity->stage = 0;
                entity->spriteLayer = 3;
            }
            break;
        case 2:
            switch (entity->stage)
            {
                case 0:
                    Entity_FaceTarget(entity);
                    entity->accelX.whole = 0;
                    entity->velX.whole = 0;
                    entity->accelY.whole = 0;
                    entity->velY.whole = 0;
                    switch (RandomNumberGenerator() % 6)
                    {
                        case 0:
                        case 1:
                        case 2:
                            entity->velX.whole = 0x8000;
                            entity->velY.whole = 0x8000;
                            if ((s16)Entity_GetTargetDistanceX(entity) > 0)
                                entity->velX.whole = -entity->velX.whole;
                            if ((s16)Entity_GetTargetDistanceY(entity) > 0)
                                entity->velY.whole = -entity->velY.whole;
                            TIMER_Y(entity) = RandomNumberGenerator() % 0x100 + 0x80;
                            break;
                        case 3:
                            entity->flipX = 1 - entity->flipX;
                            entity->velX.whole = 0x4000;
                            if ((s16)Entity_GetTargetDistanceX(entity) < 0)
                                entity->velX.whole = -entity->velX.whole;
                            TIMER_Y(entity) = RandomNumberGenerator() % 0x100 + 0x80;
                            break;
                        case 4:
                        case 5:
                        {
                            u16 angle = Entity_GetTargetAngleAbove16(entity);
                            entity->velX.whole = FixedMultiply(Sin(angle + 0x14000), 0x10000);
                            entity->velY.whole = FixedMultiply(Sin(angle + 0x10000), 0x10000);
                            TIMER_Y(entity) = ABS((Target_GetX(entity) - entity->posX.whole) / entity->velX.whole);
                            break;
                        }
                    }
                    ANIM(entity, 2, 1);
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
                    if (TIMER_Y(entity)-- <= 0)
                        entity->stage = 0;
                    LightningDoll_ResolveMovementBounds(entity);
                    break;
            }
            break;
        case 3:
            switch (entity->stage)
            {
                case 0:
                    ANIM(entity, 3, 0);
                    entity->accelX.whole = 0;
                    entity->velX.whole = 0;
                    entity->accelY.whole = 0;
                    entity->velY.whole = 0;
                    entity->timer = RandomNumberGenerator() % 0x100 + 0x80;
                    entity->stage++;
                    Sound_PlayTracked(0x75);
                case 1:
                    if (entity->animDone)
                    {
                        ANIM(entity, 4, 1);
                        entity->stage++;
                    }
                    break;
                case 2:
                    if ((entity->timer & 15) == 0 && entity->timer > 32)
                        Sound_PlayTracked(0x75);
                    if ((s8)entity->timer-- == 0)
                        entity->stage++;
                    break;
                case 3:
                    Sound_StopTracked(0x75);
                    entity->subState = 2;
                    entity->stage = 0;
                    TIMER_X(entity) = RandomNumberGenerator() % 0x20 + 0x10;
                    TIMER_Z(entity) += 0x20;
                    break;
            }
            break;
        case 4:
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
                    if (Collision_GetTileOnScreenWide((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer) == 0)
                        entity->posY.part16.integer++;
                    else
                    {
                        ANIM(entity, 5, 0);
                        entity->timer = RandomNumberGenerator() % 0x40 + 0x40;
                        entity->stage++;
                    }
                    break;
                case 2:
                    if (--entity->timer == 0xFF)
                    {
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
    if (entity->subState == 2)
    {
        if (TIMER_Z(entity) == 0)
        {
            entity->subState = 4;
            entity->stage = 0;
        }
        if ((s16)TIMER_X(entity) == 0)
        {
            if ((entity->flipX && (s16)Entity_GetTargetDistanceX(entity) < 0) ||
                (!entity->flipX && (s16)Entity_GetTargetDistanceX(entity) > 0))
            {
                u32 chance;
                if (Entity_IsTargetWithinRange(entity, 0x140000))
                    chance = 0;
                else
                    chance = RandomNumberGenerator() & 0x3FF;
                if (chance == 0)
                {
                    entity->subState = 3;
                    entity->stage = chance;
                }
            }
        }
    }
}

/* Lightning Doll helpers reconstructed from the original assembly. */

/**
 * @brief Resolve damage and flash; on lethal damage stop sound 0x75, spawn counted body parts, drop rewards and wait for all parts to finish.
 */
void LightningDoll_OnHit(struct EwramData_EntityData *entity, s32 hit, u16 *data)
{
    s32 damage = Hit_ResolveDamage(entity, (struct EwramData_EntityData *)hit, data, Player_CalculateAttackPower(data[2]));
    u16 hp = entity->hp;
    damage = (damage << 16) >> 16;
    entity->hp = hp - damage;
    Entity_ShowDamage(entity, (s16)damage, 0);
    entity->hitFlashTimer = 8;
    if ((s16)entity->hp <= 0)
    {
        entity->subState = 0;
        Sound_StopTracked(0x75);
        PlaySong(0x6E);
        LightningDoll_SpawnDeathParts(entity);
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->state = 0xFF;
        entity->subState++;
        if (entity->ext.doll.deathPartCount == 0)
            entity->deleteRequested = 1;
    }
}

/* LightningDollPartEffect_Update. */

/**
 * @brief Attach cached effect graphics and follow the body part with its table-derived vertical offset while advancing animation.
 */
void LightningDollPartEffect_Update(struct EwramData_EntityData *entity)
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
            entity->posY.part16.integer += sLightningDollPartOffsets[(u8)doll->ext.dollPart.index][1];
            break;
    }
    Anim_Update(entity);
}

/* Lightning Doll: electrical effect at the player's position. */

#undef ANIM
#undef ANIM
#define ANIM(e,n,loop) do { if ((e)->animScriptIndex != (n)) Anim_StartIndexed((e),(n),0,(loop)); } while (0)

static inline s32 RandomFlip(s32 mask) { return RandomNumberGenerator() & mask; }
/**
 * @brief Create an electrical effect at the player position minus 16 pixels vertically, with random horizontal flip.
 */
void LightningDoll_SpawnPlayerEffect(void)
{
    s32 i;
    for (i = 0; i < 1; i++)
    {
        struct EwramData_EntityData *effect;
        effect = EntityCreateInRange(0x49, 0xCF, LightningDollPlayerEffect_Update);
        if (effect != NULL)
        {
            effect->ext.lightningDoll.unusedEffectIndex = i;
            effect->posX.whole = gEwramData->playerState.player->posX.whole;
            effect->posY.whole = gEwramData->playerState.player->posY.whole - 0x100000;
            effect->flipX = RandomFlip(1);
            effect->spriteLayer = gEwramData->playerState.player->spriteLayer - 1;
        }
    }
}

/**
 * @brief Attach palette-6 effect graphics and animation 16, alternate facing and follow player X with a timer-dependent Y offset until animation completion.
 */
void LightningDollPlayerEffect_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
        {
            s32 graphics = ObjGfx_Find(sCommonObjectGfx);
            Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, 6);
            Anim_StartIndexed(entity, 16, 0, 0);
            Anim_Update(entity);
            entity->scrollWithBg = 1;
            entity->drawFunc = Sprite_DrawEntity;
            entity->flipX = RandomNumberGenerator() % 2;
            entity->timer = 0;
            entity->state++;
            break;
        }
        case 1:
            if (entity->timer % 2 == 0)
                entity->flipX = 1 - entity->flipX;
            entity->posX.whole = gEwramData->playerState.player->posX.whole;
            entity->posY.part16.integer = Player_GetOverheadY() + 8;
            entity->posY.whole += entity->timer << 15;
            if (entity->animDone)
                entity->deleteRequested = 1;
            entity->timer++;
            Anim_Update(entity);
            break;
    }
}

/* Lightning Doll body parts after death. */

static inline u16 Hidden(struct EwramData_EntityData *e) { return 1 - e->hidden; }
/**
 * @brief Initialize a table-configured body part and attached effect, fall and optionally bounce; decrement the parent count after the attached animation completes.
 */
void LightningDollPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    struct EwramData_EntityData *child = &gEwramData->entityData[entity->ext.dollPart.effectIndex];
    s32 x, y;

    switch (entity->state)
    {
        case 0:
        {
            s32 graphics;
            s32 sprite, vx, vy, timer;
            void *spriteData;
            struct EwramData_EntityData *effect;
            entity->scrollWithBg = 1;
            spriteData = sLightningDollSpriteSheet;
            graphics = Sprite_GetTileSlot(parent);
            Sprite_Attach(entity, spriteData, graphics, ObjPalette_FindOrLoad(sKillerDollLightningDollPalette));
            entity->drawFunc = Sprite_DrawEntity;
            entity->posX.whole = parent->posX.whole;
            entity->posY.whole = parent->posY.whole;
            x = sLightningDollPartOffsets[(u8)entity->ext.dollPart.index][0];
            sprite = sLightningDollPartOffsets[(u8)entity->ext.dollPart.index][2];
            vx = sLightningDollPartOffsets[(u8)entity->ext.dollPart.index][3] << 12;
            vy = sLightningDollPartOffsets[(u8)entity->ext.dollPart.index][4] << 12;
            timer = sLightningDollPartOffsets[(u8)entity->ext.dollPart.index][5];
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
            effect = EntityCreateInRange(0x49, 0xCF, LightningDollPartEffect_Update);
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
            zero = entity->timer & 15;
            if (zero == 0 && entity->timer < 0x40 && entity->timer > 0x20)
            {
                x = sLightningDollPartOffsets[(u8)entity->ext.dollPart.index][0];
                y = sLightningDollPartOffsets[(u8)entity->ext.dollPart.index][1];
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
                    correction = Collision_GetPushUpWide(ToRoomX((s16)entity->posX.part16.integer), ToRoomY((s16)entity->posY.part16.integer) - 15);
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
                    correction = Collision_GetPushDownWide(ToRoomX((s16)entity->posX.part16.integer), ToRoomY((s16)entity->posY.part16.integer) - 32);
                    if (correction != 0)
                        entity->posY.part16.integer += correction;
                }
                if (entity->velX.whole > 0)
                {
                    correction = Collision_GetPushLeftWide(ToRoomX((s16)entity->posX.part16.integer) + 8, ToRoomY((s16)entity->posY.part16.integer) - 16);
                    if (correction != 0)
                    {
                        entity->posX.part16.integer += correction;
                        entity->velX.whole = 0;
                    }
                }
                else if (entity->velX.whole != 0)
                {
                    correction = Collision_GetPushRightWide(ToRoomX((s16)entity->posX.part16.integer) - 8, ToRoomY((s16)entity->posY.part16.integer) - 16);
                    if (correction != 0)
                    {
                        entity->posX.part16.integer += correction;
                        entity->velX.whole = 0;
                    }
                }
            }
            if ((entity->timer & 1) && entity->timer < 0x40)
                entity->hidden = Hidden(entity);
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

/* Lightning Doll: six-state animation sequence for display mode. */
/**
 * @brief Cycle through six animation phases with completion and timer checks. The gameplay purpose of state 2 remains unresolved.
 */
void LightningDoll_UpdateState2Animation(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            ANIM(entity, 0, 0);
            if (entity->animDone || entity->timer++ > 16)
            {
                entity->subState++;
                entity->timer = 0;
            }
            break;
        case 1:
            ANIM(entity, 1, 0);
            if (entity->animDone)
                entity->subState++;
            break;
        case 2:
            ANIM(entity, 2, 1);
            if (entity->timer++ > 64)
            {
                entity->timer = 0;
                entity->subState++;
            }
            break;
        case 3:
            ANIM(entity, 3, 0);
            if (entity->animDone)
                entity->subState++;
            break;
        case 4:
            ANIM(entity, 4, 1);
            if (entity->timer++ > 128)
            {
                entity->timer = 0;
                entity->subState++;
            }
            break;
        case 5:
            ANIM(entity, 5, 0);
            if (entity->timer++ > 48)
            {
                entity->timer = 0;
                entity->subState = 0;
            }
            break;
    }
    Anim_Update(entity);
}

/* Lightning Doll: reflect motion at room boundaries and tile collisions. */

static inline s32 AtLimit(s32 a,s32 b) { return a >= b; }
static inline s32 RoomLimit(s32 size,s32 base) { return size == 1 ? base : size << 5; }
/**
 * @brief Reverse velocity at room boundaries and directional tile collisions, apply coordinate corrections and update horizontal facing.
 */
void LightningDoll_ResolveMovementBounds(struct EwramData_EntityData *entity)
{
    s32 size;
    s32 tile;
    s32 limit;
    s32 outside;
    s32 outsideY;
    s32 correction;
    s16 *position;
    struct EwramData **state;

    GetEntityRoomYPositionIntegerWide(entity);
    state = &gEwramData;
    size = ((u8 *)(*state)->bgInfo[1].pBgMetadata)[0];
    tile = GetEntityRoomXPositionIntegerWide(entity) / 8;
    limit = RoomLimit(size,30);
    if (tile < 0 || AtLimit(tile,limit))
    {
        entity->velX.whole = -entity->velX.whole;
        entity->posX.whole += entity->velX.whole * 2;
        entity->flipX = 1 - entity->flipX;
    }
    else
    {
        if (entity->velX.whole >= 0)
        {
            position = (s16 *)&entity->posX.part16.integer;
            correction = Collision_GetPushLeftWide(ToRoomX(*position) + 8,
                                     ToRoomY((s16)entity->posY.part16.integer) - 16);
        }
        else
        {
            position = (s16 *)&entity->posX.part16.integer;
            correction = Collision_GetPushRightWide(ToRoomX(*position) - 8,
                                     ToRoomY((s16)entity->posY.part16.integer) - 16);
        }
        if (correction != 0)
        {
            *position += correction;
            entity->velX.whole = -entity->velX.whole;
            entity->flipX = 1 - entity->flipX;
        }
    }

    GetEntityRoomXPositionIntegerWide(entity);
    state = &gEwramData;
    size = ((u8 *)(*state)->bgInfo[1].pBgMetadata)[1];
    tile = GetEntityRoomYPositionIntegerWide(entity) / 8;
    limit = RoomLimit(size,26);
    if (tile < 0 || AtLimit(tile,limit))
    {
        entity->velY.whole = -entity->velY.whole;
        entity->posY.whole += entity->velY.whole * 2;
    }
    else
    {
        if (entity->velY.whole >= 0)
        {
            s32 x = ToRoomX((s16)entity->posX.part16.integer);
            position = (s16 *)&entity->posY.part16.integer;
            correction = Collision_GetPushUpWide(x,
                                     ToRoomY(*position) + 1);
        }
        else
        {
            s32 x = ToRoomX((s16)entity->posX.part16.integer);
            position = (s16 *)&entity->posY.part16.integer;
            correction = Collision_GetPushDownWide(x,
                                     ToRoomY(*position) - 32);
        }
        if (correction != 0)
        {
            *position += correction;
            entity->velY.whole = -entity->velY.whole;
        }
    }
}

/* Lightning Doll helpers reconstructed from the original assembly. */

/**
 * @brief Clear velocity and acceleration, snap to ground and enter state 1 with randomized movement and floor-wait countdowns.
 */
void LightningDoll_ResetActive(struct EwramData_EntityData *entity)
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
    entity->ext.lightningDoll.attackTimer = RandomNumberGenerator() % 0x20 + 0x10;
    entity->ext.lightningDoll.restTimer = RandomNumberGenerator() % 0x400 + 0x200;
    entity->ext.lightningDoll.moveTimer = zero;
}

/**
 * @brief Stop sound 0x75, spawn body parts and drop rewards once; request deletion when the live-part count reaches zero.
 */
void LightningDoll_UpdateDeath(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            Sound_StopTracked(0x75);
            PlaySong(0x6E);
            LightningDoll_SpawnDeathParts(entity);
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

/**
 * @brief Submit the attack contact record; during animation 3 or 4 also create the electrical effect at the player.
 */
void LightningDoll_OnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage effect;
    effect.attack = entity->attack;
    effect.attributes = 0;
    effect.unused = 0;
    effect.reaction = 0;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
    if ((u8)(entity->animScriptIndex - 3) <= 1)
        LightningDoll_SpawnPlayerEffect();
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   LightningDoll_SpawnDeathParts = KillerDoll_SpawnDeathParts */

/**
 * @brief Hide the parent and attempt to create five indexed body parts; store the parent slot and increment its live-part count for each success.
 */
void LightningDoll_SpawnDeathParts(struct EwramData_EntityData *entity)
{
    s32 i;

    entity->hidden = 1;
    for (i = 0; i < 5; i++)
    {
        struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, LightningDollPart_Update);

        if (child != NULL)
        {
            ENTITY_PARENT_INDEX(child) = ENTITY_INDEX(entity);
            child->ext.dollPart.index = i;
            entity->ext.doll.deathPartCount++;
            child->spriteLayer = entity->spriteLayer;
        }
    }
}
