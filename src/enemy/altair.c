/*
 * Altair: movement, attacks, hits, bestiary preview and contact damage.
 */
#include "core.h"
#include "player_core.h"
#include "graphics.h"
#include "sound.h"
#include "structs/entity_template.h"
#include "structs/ewram.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "fleaman.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "hitbox.h"
#include "damage_numbers.h"
#include "entity_hit.h"

void Altair_UpdateFlight(struct EwramData_EntityData *);
void Altair_UpdateDeath(struct EwramData_EntityData *);
void Altair_UpdateSpawner(struct EwramData_EntityData *);
s32 Altair_CreateCarriedEnemy(struct EwramData_EntityData *entity, u8 type, u16 spawnParam0, u16 spawnParam1);
void Altair_UpdateBestiaryPreview(struct EwramData_EntityData *);
void Altair_DamagePlayerOnContact(struct EwramData_EntityData *);
u8 Altair_LoadGraphics(struct EwramData_EntityData *);
void Altair_OnHit(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *);
u8 Altair_LoadGraphics(struct EwramData_EntityData *);
void Altair_OnHit(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *);
struct EwramData_EntityData *Altair_CreateFeather(struct EwramData_EntityData *);
void AltairFeather_SetPhase(struct EwramData_EntityData *entity, u8 phase);
u8 Altair_LoadGraphics(struct EwramData_EntityData *);
void Altair_OnHit(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *);
s32 AltairFeather_SetupGraphics(struct EwramData_EntityData *entity);
u8 AltairFeather_LoadGraphics(struct EwramData_EntityData *entity);
u8 Fleaman_LoadGraphics(struct EwramData_EntityData *entity);

/* from entity_util.h (not included: see the module notes) */

/* Altair creation, update, graphics setup and hit response. */

void EnemyAltairCreate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent;
    if (!Altair_LoadGraphics(entity)) return;
    entity->spriteLayer = 3;
    entity->state = 1;
    entity->stage = 0;
    entity->subState = 0;
    Enemy_ResetFromTemplate(entity);
    entity->ext.altair.carried = 0;
    entity->ext.altair.dropReady = 0;
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        entity->stage = 0;
        entity->subState = 0;
        return;
    }
    if (entity->spawnParam0 == 0)
    {
        entity->state = 5;
        entity->hidden = 1;
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        return;
    }
    Entity_EnableBoxes(entity, Altair_DamagePlayerOnContact, Altair_OnHit);
    if (entity->spawnParam0 == 2)
    {
        s32 x = -(u16)entity->velX.part16.integer;
        x -= 0x10;
        entity->posX.part16.integer = x;
    }
    else if (entity->spawnParam0 == 3)
        entity->posX.part16.integer = 0x100 + (u16)entity->velX.part16.integer;
    {
        u32 random = RandomNumberGenerator();
        u16 *position = (u16 *)&entity->posY.part16.integer;
        s32 offset = 0x1F;
        offset &= random;
        *position = offset + 0xFFF0 + *position;
    }
    if ((s16)Entity_GetTargetDistanceX(entity) > 0) entity->flipX = 0;
    else entity->flipX = 1;
    parent = entity->ext.altair.spawner;
    if (parent->ext.altairDrop.count <= 1 && parent != NULL)
    {
        if (entity->spawnParam1 == 1)
        {
            if (Altair_CreateCarriedEnemy(entity, 0x18, 0xFF, 0xFF)) return;
        }
        else if (entity->spawnParam1 == 2)
        {
            if (Altair_CreateCarriedEnemy(entity, 0x4A, 0xFF, 0xFF)) return;
        }
        else return;
    }
    entity->deleteRequested = 1;
}

/* Altair creation, update, graphics setup and hit response. */

void EnemyAltairUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    struct EwramData_EntityData *child = entity->ext.altair.carried;
    struct EwramData_EntityData *parent;
    if (entity->state == 4)
    {
        Altair_UpdateBestiaryPreview(entity);
        return;
    }
    if (entity->state == 5)
    {
        Altair_UpdateSpawner(entity);
        return;
    }
    if (child->updateFunc == NULL) entity->ext.altair.carried = 0;
    if (entity->state != 2) Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity) && entity->state != 2)
    {
        Hit_CheckWithPlayer(&result, entity);
        return;
    }
    if (entity->hitFlashTimer != 0 && entity->state != 3 && entity->state != 2)
    {
        Hit_CheckWithPlayer(&result, entity);
        return;
    }
    switch (entity->state)
    {
    case 1: Altair_UpdateFlight(entity); break;
    case 2: Altair_UpdateDeath(entity); break;
    }
    Entity_AnimateWithBoxes(entity);
    if (entity->state != 2) Hit_CheckWithPlayer(&result, entity);
    if (entity->deleteRequested)
    {
        parent = entity->ext.altair.spawner;
        if (parent != NULL && parent->updateFunc != NULL && parent->enemyId == 0x2C && parent->ext.altair.spawnCount != 0)
            parent->ext.altair.spawnCount--;
    }
}

/* Altair flight, carried-object release and screen exit. */
/**
 * @brief Flight state (1): carry and release the linked child through
 * animation events, then fly offscreen. Request child termination when
 * leaving before release; preserve the existing coordinate and lifetime tests.
 */
void Altair_UpdateFlight(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = entity->ext.altair.carried;
    s32 distance;
    switch (entity->subState)
    {
    case 0:
        if (child != NULL)
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->ext.altair.flightPhase = 0;
        }
        else
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->ext.altair.flightPhase = 1;
        }
        entity->velX.whole = 0xA000;
        entity->velY.whole = 0;
        if (!entity->flipX) entity->velX.whole = -entity->velX.whole;
        entity->timer = 0x30;
        entity->subState++;
        /* fall through */
    case 1:
        entity->posX.whole += entity->velX.whole;
        if (child == NULL) break;
        distance = (s16)Entity_GetTargetDistanceX(entity);
        child->posX.whole = entity->posX.whole;
        child->posY.whole = entity->posY.whole + 0x130000;
        child->flipX = entity->flipX;
        if (!entity->flipX)
        {
            if (distance > 0x50)
            {
                if (!(RandomNumberGenerator() & 0x1F)) entity->ext.altair.dropReady = 1;
            }
            else if (distance < -4) entity->ext.altair.dropReady = 1;
        }
        else
        {
            if (distance < -0x50)
            {
                if (!(RandomNumberGenerator() & 0x1F)) entity->ext.altair.dropReady = 1;
            }
            else if (distance > 4) entity->ext.altair.dropReady = 1;
        }
        if ((u8)entity->ext.altair.dropReady == 1 && (u16)(entity->posX.part16.integer - 0x21) <= 0xAE)
        {
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 2:
        Anim_StartIndexed(entity, 2, 0, 1);
        entity->ext.altair.flightPhase = 2;
        entity->subState++;
        entity->timer = 0;
        /* fall through */
    case 3:
        {
            s32 x = entity->posX.whole;
            x += entity->velX.whole;
            entity->posX.whole = x;
            if (child != NULL)
            {
                child->posX.whole = x;
                child->posY.whole = entity->posY.whole + 0x110000;
                child->flipX = entity->flipX;
            }
        }
        if ((u8)entity->ext.altair.flightPhase == 2 && entity->animFrame == 7 && entity->animTimer == 0)
        {
            if (child != NULL) child->subState++;
            entity->ext.altair.carrying = 0;
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 4:
        Anim_StartIndexed(entity, 1, 0, 1);
        entity->ext.altair.flightPhase = 1;
        entity->subState++;
        entity->timer = 0;
        /* fall through */
    case 5:
        entity->posX.whole += entity->velX.whole;
        break;
    }
    {
        s16 *position = (s16 *)&entity->posX.part16.integer;
        s32 x = *position;
        s32 padding;
        position = &entity->velX.part16.integer;
        padding = *position;
        if (x < -padding - 0x80 || x > padding + 0x170)
        {
            struct EwramData_EntityData *attached;
            entity->deleteRequested = 1;
            attached = entity->ext.altair.carried;
            if (attached != NULL && entity->subState <= 2)
            {
                attached->state = 0xFF;
                attached->timer = 0;
            }
        }
    }
}

/* Altair creation, update, graphics setup and hit response. */

/**
 * @brief Death state (2): terminate an attached child, create five feathers,
 * then move and update fragment death. Preserve the existing feather
 * allocation assumptions and phase selection calls.
 */
void Altair_UpdateDeath(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *attached = entity->ext.altair.carried;
    struct EwramData_EntityData *child;
    u32 randomMask;
    s32 count;
    switch (entity->subState)
    {
    case 0:
        if (attached != NULL && entity->ext.altair.carrying != 0 && attached->state != 0xFF)
        {
            attached->state = 0xFF;
            attached->subState = 0;
        }
        randomMask = 1;
        for (count = 4; count >= 0; count--)
        {
            child = Altair_CreateFeather(entity);
            if (child != NULL && (RandomNumberGenerator() & randomMask)) AltairFeather_SetPhase(child, 0);
            else AltairFeather_SetPhase(child, 1);
        }
        PlaySong(0x72);
        Anim_StartIndexed(entity, 3, 0, 1);
        entity->ext.altair.flightPhase = 3;
        entity->velY.whole = 0xC000;
        entity->timer = 0x3F;
        entity->subState++;
        /* fall through */
    case 1:
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        Enemy_UpdateFragmentDeathWithSpread(entity, 0xF, 0x100000, 0xC0000);
        break;
    }
}

/**
 * @brief Hidden spawner state (5): randomly create a flight instance while
 * the active count allows it, copy enemy parameters and link it back
 * to this spawner. Flight instances decrement the count on requested deletion.
 */
void Altair_UpdateSpawner(struct EwramData_EntityData *entity)
{
    u32 random = RandomNumberGenerator() & 0x3F;
    struct EwramData_EntityData *child;
    if (random != 0 || entity->ext.altair.spawnCount > 1) return;
    child = EntityCreateInRange(0x1B, 0x48, EntityUpdateNothing);
    if (child != NULL)
    {
        child->posY.whole = entity->posY.whole;
        child->scrollWithBg = 1;
        child->spawnIndex = random;
        child->enemyId = entity->enemyId;
        child->spawnParam0 = ((u16)entity->ext.altair.spawnCount & 1) + 2;
        child->spawnParam1 = entity->spawnParam1;
        child->drawFunc = Sprite_DrawEntity;
        child->hp = sEnemyData[entity->enemyId].hp;
        child->mp = sEnemyData[entity->enemyId].mp;
        child->updateFunc = entity->updateFunc;
        child->ext.altair.spawner = entity;
        EnemyAltairCreate(child);
        entity->ext.altair.spawnCount++;
    }
}

/**
 * @brief Resolve damage, apply hit flash and subtract HP; on defeat drop
 * rewards, restore the palette, reset movement and enter death state 2.
 */
void Altair_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 damage = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(entity, attacker, hit, damage);
    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;
        damage <<= 16;
        damage >>= 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, (s16)damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->timer = 0x32;
        entity->state = 2;
        entity->subState = 0;
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
        entity->velY.whole = 0;
        entity->accelY.whole = 0x2000;
    }
}

/* Altair child creation and feather motion. */
/**
 * @brief Create the selected enemy type below Altair, set its parameters
 * and call its template create callback. Return nonzero if it remains
 * active; update linked spawner counters using the existing rules.
 */
s32 Altair_CreateCarriedEnemy(struct EwramData_EntityData *entity, u8 type, u16 spawnParam0, u16 spawnParam1)
{
    u8 result = 0;
    struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, EntityUpdateNothing);
    entity->ext.altair.carried = child;
    if (child != NULL)
    {
        struct EntityTemplate *entry;
        struct EwramData_EntityData *parent;
        s32 x;
        entity->ext.altair.carrying = 1;
        x = entity->posX.whole;
        child->posX.whole = x;
        child->posY.whole = entity->posY.whole + 0x130000;
        child->flipX = x < gEwramData->playerState.player->posX.whole;
        child->scrollWithBg = 1;
        child->spawnParam0 = spawnParam0;
        child->spawnParam1 = spawnParam1;
        child->spawnIndex = entity->spawnIndex;
        child->enemyId = type;
        child->drawFunc = Sprite_DrawEntity;
        entry = (struct EntityTemplate *)(type * sizeof(*entry) + (u32)sEnemyData);
        child->hp = entry->hp;
        child->updateFunc = entry->updateFunc;
        entry->createFunc(child);
        result = child->updateFunc != NULL;
        parent = entity->ext.altair.spawner;
        if (parent != NULL)
        {
            parent->ext.altairDrop.count++;
            child->ext.altairDrop.count++;
            child->ext.altairDrop.spawnerIndex = (u16)ENTITY_INDEX_PAIR(entity->ext.altair.spawner);
        }
    }
    return result;
}

/* Altair feather motion. Local register bindings preserve phase accumulation
 * and threshold selection in agbcc; they do not prove original source spelling. */

/**
 * @brief Initialize a feather near the owner, drift downward with sine
 * wobble, then request deletion at the phase threshold. Preserve the
 * pinned-register phase and threshold calculation.
 */
void AltairFeather_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.altairFeather.owner;
    s32 phase;
    register u32 limit asm("r0"); /* FAKEMATCH: pinned register */
    switch (entity->state)
    {
    case 0:
    case 1:
        entity->spriteLayer = 2;
        {
            u32 random = RandomNumberGenerator();
            u16 *position = &parent->posX.part16.integer;
            s32 mask = 0x1F;
            mask &= random;
            entity->posX.part16.integer = mask + 0xFFF0 + *position;
        }
        {
            u32 random = RandomNumberGenerator();
            u16 *position = (u16 *)&parent->posY.part16.integer;
            s32 mask = 0xF;
            mask &= random;
            {
                s32 offset = mask + 0xFFF8;
                u16 value = *position;
                entity->posY.part16.integer = offset + value;
            }
        }
        entity->velY.whole = 0x1000;
        entity->state++;
        entity->timer = 0;
        {
            u32 random = RandomNumberGenerator();
            s32 mask = 0x3FFF;
            mask &= random;
            entity->ext.altairFeather.unusedSpin = mask - 0x2000;
        }
        entity->ext.altairFeather.unusedSpeed = 0xD00;
        {
            u32 random = RandomNumberGenerator();
            s32 mask = 0x7F;
            mask &= random;
            entity->ext.altairFeather.swaySpeed = mask + 0x40;
        }
        entity->scrollWithBg = 1;
        break;
    case 2:
        entity->posX.whole += Sin((u16)entity->ext.altairFeather.swayPhase) / 4;
        entity->posY.whole += entity->velY.whole;
        {
            u32 value = (u16)entity->ext.altairFeather.swaySpeed;
            {
                u32 increment = (u16)entity->ext.altairFeather.swayPhase;
                value += increment;
            }
            entity->ext.altairFeather.swayPhase = value;
            phase = value << 16;
        }
        if (phase < 0)
            limit = 0x9FFF0000;
        else
            limit = 0x1FFF0000;
        if ((u32)phase > limit)
            entity->state++;
        break;
    case 3:
        entity->deleteRequested = 1;
        break;
    }
}

/**
 * @brief Find or load Altair graphics and palette, attach the sprite and
 * start animation 0. Return 1 on success; delete and return 0 on the
 * existing graphics/palette failure paths.
 */
u8 Altair_LoadGraphics(struct EwramData_EntityData *entity)
{
    u8 *palette = sAltairPalette;
    u8 *sprite = sAltairSpriteSheet;
    u8 *graphicsData = sAltairGfx;
    s32 graphics = ObjGfx_Find(graphicsData);
    struct EwramData_EntityData *saved = entity;
    if (graphics < 0)
    {
        graphics = ObjGfx_LoadResource(graphicsData, 0);
        if (graphics < 0) goto fail;
    }
    entity->basePalette = (s8)ObjPalette_FindOrLoad(palette);
    if (entity->basePalette == 0xFF)
    {
        EntityDelete(saved);
        return 0;
    }
    Sprite_Attach(entity, sprite, graphics, entity->basePalette);
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
fail:
    EntityDelete(saved);
    return 0;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Altair_UpdateBestiaryPreview = Imp_UpdateState4Animation
 *   Altair_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */

/**
 * @brief Bestiary state (4): start animation 0 once, then update it.
 */
void Altair_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->subState++;
        case 1:
            Anim_Update(entity);
            break;
    }
}

/**
 * @brief Apply the entity damage with zero effect type and flags at its position.
 */
void Altair_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/* Fleaman helpers, create and graphics loader 0x080C1D38..0x080C1F1B. The loader
 * Fleaman_LoadGraphics only matches without -f2003-patch. */

/**
 * @brief Allocate a feather, attach its graphics and store the owner pointer.
 * Return the feather or NULL; retain the existing failure cleanup calls.
 */
struct EwramData_EntityData *Altair_CreateFeather(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, AltairFeather_Update);

    if (child != NULL)
    {
        if (!AltairFeather_SetupGraphics(child))
        {
            EntityDelete(child);
            return NULL;
        }
        ENTITY_CREATOR(child) = entity;
    }
    return child;
}

/**
 * @brief Return whether the feather graphics loader succeeds.
 */
s32 AltairFeather_SetupGraphics(struct EwramData_EntityData *entity)
{
    return AltairFeather_LoadGraphics(entity) != 0;
}

/**
 * @brief Attach frame 9 using already-loaded Altair graphics and a palette.
 * Return 1 on success; delete the entity and return 0 on failure.
 */
u8 AltairFeather_LoadGraphics(struct EwramData_EntityData *entity)
{
    void *palette = sAltairPalette;
    void *sprite = sAltairSpriteSheet;
    s32 tileSlot = ObjGfx_Find(sAltairGfx);

    if (tileSlot >= 0)
    {
        entity->basePalette = ObjPalette_FindOrLoad(palette);
        if (entity->basePalette != 0xFF
            && Sprite_Attach(entity, sprite, tileSlot, entity->basePalette))
        {
            entity->animFrame = 9;
            entity->drawFunc = Sprite_DrawEntity;
            return 1;
        }
    }
    EntityDelete(entity);
    return 0;
}

/**
 * @brief Set the feather phase to zero or 0x8000 according to the supplied flag.
 */
void AltairFeather_SetPhase(struct EwramData_EntityData *entity, u8 phase)
{
    if (phase == 0)
        entity->ext.altairFeather.swayPhase = phase;
    else
        entity->ext.altairFeather.swayPhase = 0x8000;
}

/**
 * @brief C1E0C | Fleaman create: load the graphics, layer 0x14, state 1,
 * hit boxes; state 3 when bestiaryPreview is set. unk_4F8 = 1 when spawnParam0 and
 * spawnParam1 both have all low bits set (then no Entity_SnapToGround), collision
 * callbacks Fleaman_DamagePlayerOnContact / Fleaman_OnHit.
 */
void EnemyFleamanCreate(struct EwramData_EntityData *entity)
{
    if (!Fleaman_LoadGraphics(entity))
        return;
    entity->spriteLayer = 0x14;
    entity->state = 1;
    entity->stage = 0;
    entity->subState = 0;
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        entity->stage = 0;
        entity->subState = 0;
        return;
    }
    if ((entity->spawnParam0 & entity->spawnParam1) == 0xFF)
        entity->ext.fleaman.airborne = 1;
    else
        entity->ext.fleaman.airborne = 0;
    Entity_EnableBoxes(entity, Fleaman_DamagePlayerOnContact, Fleaman_OnHit);
    if ((u8)entity->ext.fleaman.airborne != 1)
        Entity_SnapToGround(entity);
}

/**
 * @brief C1E88 | Load the Fleaman graphics (reusing an already loaded
 * copy), palette, sprite and hit boxes; deletes the entity and returns 0
 * when any step fails.
 */
u8 Fleaman_LoadGraphics(struct EwramData_EntityData *entity)
{
    void *palette = sFleamanRipperPalette;
    void *sprite = sFleamanRipperSpriteSheet;
    u8 *graphics = sFleamanRipperGfx;
    s32 gfx = ObjGfx_Find(graphics);
    s32 paletteIndex;

    if (gfx < 0)
    {
        gfx = ObjGfx_LoadResource(graphics, 0);
        if (gfx < 0)
        {
            EntityDelete(entity);
            return 0;
        }
    }
    paletteIndex = (s8)ObjPalette_FindOrLoad(palette);
    if (paletteIndex == -1)
    {
        EntityDelete(entity);
        return 0;
    }
    entity->basePalette = paletteIndex;
    if (!Sprite_Attach(entity, sprite, gfx, entity->basePalette))
    {
        EntityDelete(entity);
        return 0;
    }
    if (!Anim_StartIndexed(entity, 0, 0, 1))
    {
        entity->ext.fleaman.anim = 0;
        EntityDelete(entity);
        return 0;
    }
    Anim_Update(entity);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
    EntityDelete(entity);
    return 0;
}
