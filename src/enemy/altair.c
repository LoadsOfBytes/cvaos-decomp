/*
 * Altair: body, carried enemy and feathers (more helpers are in the next, separately compiled file).
 */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "structs/entity_template.h"
#include "structs/ewram.h"

/* Altair creation, update, graphics setup and hit response. */
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, void *);
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Altair_UpdateFlight(struct EwramData_EntityData *);
extern void Altair_UpdateDeath(struct EwramData_EntityData *);
extern void Altair_UpdateSpawner(struct EwramData_EntityData *);
s32 Altair_CreateCarriedEnemy(struct EwramData_EntityData *entity, u8 type, u16 spawnParam0, u16 spawnParam1);
extern void Altair_UpdateBestiaryPreview(struct EwramData_EntityData *);
extern void Altair_DamagePlayerOnContact(struct EwramData_EntityData *);
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s16, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern u8 sAltairPalette[], sAltairSpriteSheet[], sAltairGfx[];
u8 Altair_LoadGraphics(struct EwramData_EntityData *);
void Altair_OnHit(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *);

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
u8 Altair_LoadGraphics(struct EwramData_EntityData *);
void Altair_OnHit(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *);

extern struct EwramData_EntityData *Altair_CreateFeather(struct EwramData_EntityData *);
extern void AltairFeather_SetPhase(struct EwramData_EntityData *, s32);
extern void Enemy_UpdateFragmentDeathWithSpread(struct EwramData_EntityData *, s32, s32, s32);

void EnemyAltairUpdate(struct EwramData_EntityData *entity)
{
    u32 box[4];
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
    if (Entity_IsTimeStopped(entity) && entity->state != 2) goto hit;
    if (entity->hitFlashTimer != 0 && entity->state != 3 && entity->state != 2)
    {
hit:
        Hit_CheckWithPlayer(box, entity);
        return;
    }
    switch (entity->state)
    {
    case 1: Altair_UpdateFlight(entity); break;
    case 2: Altair_UpdateDeath(entity); break;
    }
    Entity_AnimateWithBoxes(entity);
    if (entity->state != 2) Hit_CheckWithPlayer(box, entity);
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
u8 Altair_LoadGraphics(struct EwramData_EntityData *);
void Altair_OnHit(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *);

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
    child = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
    if (child != NULL)
    {
        child->posY.whole = entity->posY.whole;
        child->scrollWithBg = 1;
        child->spawnIndex = random;
        child->enemyId = entity->enemyId;
        child->spawnParam0 = ((u16)entity->ext.altair.spawnCount & 1) + 2;
        child->spawnParam1 = entity->spawnParam1;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
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
    struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
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
        child->drawFunc = (u32 *)Sprite_DrawEntity;
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

extern s8 ObjPalette_FindOrLoad(void *);
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
    entity->basePalette = ObjPalette_FindOrLoad(palette);
    if (entity->basePalette == 0xFF) goto fail;
    Sprite_Attach(entity, sprite, graphics, entity->basePalette);
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
fail:
    EntityDelete(saved);
    return 0;
}
