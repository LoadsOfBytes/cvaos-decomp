/*
 * Room objects 0x22, 0x29 and 0x2A: falling parts, effects and movement.
 */
#include "objects_22_2a.h"
#include "core.h"
#include "graphics.h"
#include "gameplay.h"
#include "structs/ewram.h"
#include "sound.h"
#include "player_core.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "data/data_0E5364.h"
#include "obj_palette.h"
#include "entity_hit.h"

extern void *memcpy(void *dst, const void *src, unsigned long size);
extern s32 Entity_MoveWithBoxCollision(struct EwramData_EntityData *, s32, s32, s32, s32, s32, s32);
extern u8 sWarpLightPaletteSteps[];

u8 Object22_LoadGraphics(struct EwramData_EntityData *entity);
void Object22_SpawnEffect(struct EwramData_EntityData *, s32, s32, u8);
void Object22_SpawnFallingPart(struct EwramData_EntityData *);
void Object22FallingPart_Update(struct EwramData_EntityData *);
void Object22FallingPart_OnPlayerContact(struct EwramData_EntityData *);
void Object22Effect_Update(struct EwramData_EntityData *entity);
u8 Object29_LoadGraphics(struct EwramData_EntityData *);
void Object29_Draw(struct EwramData_EntityData *entity);
u8 Object2A_LoadGraphics(struct EwramData_EntityData *);
void Object2A_Draw(struct EwramData_EntityData *entity);

/**
 * @brief 56828 | Object 0x22 init: deleted unless Object22_LoadGraphics accepts it;
 * box 0x0C20FFF0 in slot 8 (HitBox_SetBody / HitBox_SetAttack, disabled),
 * animation 3, layer 0x12, attack = equipmentDefenseBonus / 2 + equipmentDefenseBonus / 4 +
 * maxHP / 16, fixed screen position (0x40, 0x9D) and gravity 0x2800.
 */
void Object22Create(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    u8 state = 0; /* the original keeps this zero in its own register */
    u32 box = 0x0C20FFF0;

    if (!Object22_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Hit_Enable(entity, 8, 0, 0);
    HitBox_SetBody(entity, &box);
    HitBox_SetAttack(entity, &box);
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    Anim_StartIndexed(entity, 3, 0, 1);
    Anim_Update(entity);
    {
        s32 strength = (gEwramData->playerState.equipmentDefenseBonus >> 1) + (gEwramData->playerState.equipmentDefenseBonus >> 2);
        strength += gEwramData->playerStatus.maxHP >> 4;
        entity->attack = strength;
    }
    entity->spriteLayer = 0x12;
    entity->state = state;
    entity->posX.part16.integer = 0x40 - bg->xPos.part16.integer;
    entity->posY.part16.integer = 0x9D - bg->yPos.part16.integer;
    entity->accelY.whole = 0x2800;
}

/* Object22_UpdateState. */

/**
 * @brief Main behaviour of object 0x22, called by Object22Update.
 */
void Object22_UpdateState(struct EwramData_EntityData *e)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    u8 durations[5];
    u8 *pat;

    memcpy(durations, sWarpLightPaletteSteps + 0x30, 5);
    switch (e->subState)
    {
    case 0:
        if (gEwramData->playerState.standingPlatform == e)
            e->posY.part16.integer = 0x9E - bg->yPos.part16.integer;
        else
            e->posY.part16.integer = 0x9D - bg->yPos.part16.integer;
        if (gEwramData->playerState.standingPlatform == e)
        {
            if (e->timer++ > 0x3B)
            {
                e->subState++;
                e->ext.object22.level = 1;
                e->timer = 0;
                e->velX.whole = 0x10000;
            }
        }
        else
            e->timer = 0;
        break;
    case 1:
        EntityMoveX(e);
        if (gEwramData->playerState.standingPlatform != e)
        {
            e->posY.part16.integer = 0x9D - bg->yPos.part16.integer;
            if (++e->ext.object22.emptyTimer > 0x3B)
            {
                e->stage = 0;
                e->subState = 4;
                e->timer = 0x14;
                break;
            }
        }
        else
        {
            e->posY.part16.integer = 0x9E - bg->yPos.part16.integer;
            e->ext.object22.emptyTimer = 0;
        }
        if ((s16)e->posX.part16.integer + (s16)bg->xPos.part16.integer >= sObject22LevelThresholds[e->ext.object22.level])
        {
            e->posX.part16.integer = sObject22LevelThresholds[e->ext.object22.level] - bg->xPos.part16.integer;
            e->velX.whole = 0;
            e->subState++;
            e->stage = 0;
            e->timer = 0;
        }
        break;
    case 2:
        { u8 **pt = sObject22LevelPatterns; s32 i = e->ext.object22.level - 1; pat = pt[i]; }
        if (gEwramData->playerState.standingPlatform != e)
        {
            e->posY.part16.integer = 0x9D - bg->yPos.part16.integer;
            if (++e->ext.object22.emptyTimer > 0x3B)
            {
                e->stage = 0;
                e->subState = 4;
                e->timer = 0x14;
                break;
            }
        }
        else
        {
            e->posY.part16.integer = 0x9E - bg->yPos.part16.integer;
            e->ext.object22.emptyTimer = 0;
        }
        switch (e->stage)
        {
        case 0:
            if (e->timer++ > 0x1D)
            {
                e->stage++;
                e->timer = 0;
            }
            break;
        case 1:
        {
            s32 dx, dy;
            u8 key = pat[e->ext.object22.step + 1];
            if (key == 0x40)
            {
                dx = 0;
                dy = 0x6B0000;
            }
            else if (key == 0x10)
            {
                dx = 0x60000;
                dy = 0x720000;
            }
            else if (key == 0x80)
            {
                dx = 0;
                dy = 0x790000;
            }
            else if (key == 0x20)
            {
                dx = -0x60000;
                dy = 0x720000;
            }
            { const s16 *tbl = sObject22LevelThresholds; dx += tbl[e->ext.object22.level] << 16; }
            dx -= bg->xPos.whole;
            dy -= bg->yPos.whole;
            Object22_SpawnEffect(e, dx, dy, durations[e->ext.object22.level - 1]);
            e->stage++;
            e->timer = 0;
            e->ext.object22.hit = 0;
            PlaySong(0x15B);
            break;
        }
        case 2:
            if (++e->timer <= durations[e->ext.object22.level - 1] && e->ext.object22.hit == 0)
            {
                u16 in = gEwramData->inputData.playerNewInput;
                if (pat[e->ext.object22.step + 1] & in)
                {
                    e->ext.object22.hit = 1;
                    PlaySong(0x15A);
                }
                else if (in & 0xF0)
                {
                    e->ext.object22.hit = 2;
                    e->timer = durations[e->ext.object22.level - 1];
                    PlaySong(0x15A);
                }
            }
            if (e->timer >= durations[e->ext.object22.level - 1] + 0x1E)
            {
                if (e->ext.object22.hit == 1)
                {
                    e->timer = 0;
                    e->ext.object22.step++;
                    if (e->ext.object22.step >= pat[0])
                    {
                        e->ext.object22.step = 0;
                        e->stage = 0;
                        e->subState++;
                    }
                    else
                        e->stage = 1;
                }
                else
                {
                    e->stage = 0;
                    e->subState = 4;
                    e->timer = 0x14;
                }
            }
            break;
        }
        break;
    case 3:
        if (gEwramData->playerState.standingPlatform == e)
            e->posY.part16.integer = 0x9E - bg->yPos.part16.integer;
        else
            e->posY.part16.integer = 0x9D - bg->yPos.part16.integer;
        if (e->timer++ << 24)
        {
            if (++e->ext.object22.level > 5)
            {
                e->subState = 10;
                e->timer = 0;
            }
            else
            {
                e->timer = 0;
                e->velX.whole = 0x10000;
                e->subState = 1;
            }
        }
        break;
    case 4:
        if (e->stage == 0)
        {
            e->stage++;
            e->velX.whole = 0;
            e->accelX.whole = 0;
            Object22_SpawnFallingPart(e);
            PlaySong(0x15C);
        }
        if ((s8)--e->timer <= 0)
        {
            e->ext.object22.flags |= 1;
            e->timer = 0x28;
            e->subState++;
        }
        break;
    case 5:
        Entity_MoveWithBoxCollision(e, 0x10000, -0x10, 0, 0x20, 0xC, 0x60000);
        e->hidden = Entity_GetVisibilityPattern(e->timer);
        if ((s8)--e->timer <= 0)
            e->deleteRequested = 1;
        break;
    case 10:
        gEwramData->gameState.gameFlagsTail[1] |= 0x8000;
        e->subState++;
        break;
    case 11:
        break;
    }
}

/* Spawn the object 0x22's attached falling part. */
void Object22_SpawnFallingPart(struct EwramData_EntityData *owner)
{
    struct SpriteBox box;
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, Object22FallingPart_Update);
    s32 graphics;
    if (child != NULL)
    {
        child->posX.whole = owner->posX.whole;
        child->posY.whole = 0;
        child->velY.whole = 0x60000;
        child->spriteLayer = owner->spriteLayer + 1;
        graphics = ObjGfx_Find(sObject22Gfx);
        child->basePalette = ObjPalette_FindOrLoadVariant(sUnk_08209FD8, 1);
        Sprite_Attach(child, sObject22SpriteSheet, graphics, child->basePalette);
        Anim_StartIndexed(child, 4, 0, 0);
        Anim_Update(child);
        child->attack = owner->attack;
        box.x = -3;
        box.y = -15;
        box.width = 6;
        box.height = 15;
        HitBox_SetAttack(child, &box);
        child->hit.flags |= 4;
        child->hit.flags &= ~2;
        Hit_Enable(child, 8, Object22FallingPart_OnPlayerContact, 0);
        child->enemyId = owner->enemyId;
        child->ext.object22Part.owner = owner;
        child->scrollWithBg = 1;
        child->drawFunc = Sprite_DrawEntity;
    }
}

/* Follow the owner's Y after falling onto it; remove if the owner is lost. */
static inline void CopyBit(struct EwramData_EntityData *a, u32 b) { a->hidden = b; }
void Object22FallingPart_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    struct EwramData_EntityData *owner = entity->ext.object22Part.owner;
    if (owner == NULL || owner->updateFunc == NULL || owner->enemyId != entity->enemyId)
        goto remove;
    if (entity->state == 0)
    {
        CopyBit(entity, owner->hidden);
        entity->posY.whole += entity->velY.whole;
        if (entity->posY.whole >= owner->posY.whole)
        {
            entity->posY.whole = owner->posY.whole;
            entity->state++;
        }
        Hit_CheckWithPlayer(&result, entity);
    }
    else if (entity->state == 1)
    {
        CopyBit(entity, owner->hidden);
        entity->posX.whole = owner->posX.whole;
        entity->posY.whole = owner->posY.whole;
    }
    else
    {
    remove:
        EntityDelete(entity);
    }
}

/* Object 0x22 helpers 0x08056F14..0x08057003. */

/**
 * @brief 56F14 | Object 0x22 update.
 */
void Object22Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;

    if (!(entity->ext.object22.flags & 1))
        Player_QueueEntity(entity, 1);
    if (entity->state == 0)
        Object22_UpdateState(entity);
    Anim_Update(entity);
    Hit_CheckWithPlayer(&result, entity);
}

/**
 * @brief 56F50 | Spawn a short-lived effect (animation 2 of sObject22SpriteSheet)
 * at (dx, dy) lasting param_3 frames.
 */
void Object22_SpawnEffect(struct EwramData_EntityData *entity, s32 dx, s32 dy, u8 duration)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, Object22Effect_Update);
    s32 gfx;

    if (child != NULL)
    {
        child->posX.whole = dx;
        child->posY.whole = dy;
        child->spriteLayer = entity->spriteLayer;
        gfx = ObjGfx_Find(sObject22Gfx);
        child->basePalette = ObjPalette_FindOrLoadVariant(sUnk_08209FD8, 1);
        Sprite_Attach(child, sObject22SpriteSheet, gfx, child->basePalette);
        Anim_StartIndexed(child, 2, 0, 0);
        Anim_Update(child);
        child->timer = duration;
        child->scrollWithBg = 1;
        child->drawFunc = Sprite_DrawEntity;
    }
}

/**
 * @brief 56FE8 | Effect update: delete when the timer timer runs out.
 */
void Object22Effect_Update(struct EwramData_EntityData *entity)
{
    if ((s8)--entity->timer <= 0)
        EntityDelete(entity);
}

/* Object22FallingPart_OnPlayerContact: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */

/**
 * @brief 57004 | Like GreatArmorShockwave_DamagePlayerOnContact, but unk_2 = 1.
 */
void Object22FallingPart_OnPlayerContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 1;
    contactDamage.unused = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Object22_LoadGraphics = Object26_LoadGraphics */

/**
 * @brief 57034 | Same as Object26_LoadGraphics.
 */
u8 Object22_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject22Gfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sUnk_08209FD8, 2);
    Sprite_Attach(entity, sObject22SpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

void Object29Create(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    struct FourBoxes boxes = sObject29Boxes;
    if (!Object29_LoadGraphics(entity)) { EntityDelete(entity); return; }
    Hit_Enable(entity, 8, NULL, 0);
    HitBox_SetBody(entity, &boxes.box[entity->spawnParam1]);
    HitBox_SetAttack(entity, &boxes.box[entity->spawnParam1]);
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    Anim_StartIndexed(entity, sObject29Variants[entity->spawnParam1].animation, 0, 1);
    Anim_Update(entity);
    entity->spriteLayer = 18;
    entity->state = 0;
    entity->ext.layerObject.shuttleStart = entity->posY.whole + camera->yPos.whole;
    entity->ext.layerObject.shuttleEnd = entity->spawnParam0 << 16;
    entity->ext.layerObject.x = entity->posX.whole + camera->xPos.whole;
    entity->ext.layerObject.y = entity->posY.whole + camera->yPos.whole;
    entity->ext.layerObject.drawX = entity->ext.layerObject.x - camera->xPos.whole;
    entity->ext.layerObject.drawY = entity->ext.layerObject.y - camera->yPos.whole;
    entity->scrollWithBg = 0;
    if ((s32)entity->ext.layerObject.shuttleStart > entity->ext.layerObject.shuttleEnd) entity->ext.layerObject.shuttleDir = 1;
    else entity->ext.layerObject.shuttleDir = 0;
}

/* Vertical platform: accelerate between two room-space endpoints,
 * slow near each endpoint and wait for the per-variant delay. */
#define DIRECTION(e) (e->ext.layerObject.shuttleDir)
void Object29_UpdateMovement(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    register s32 y asm("r1"); /* FAKEMATCH: y in r1 */
    s32 v, k;
    
    entity->ext.layerObject.drawX = entity->ext.layerObject.x - camera->xPos.whole;

    y = entity->ext.layerObject.y;
    entity->ext.layerObject.drawY = y - camera->yPos.whole;
    switch (entity->subState)
    {
    case 0:
    {
        s32 target = entity->ext.layerObject.shuttleEnd;
        register s32 current asm("r2") = y; /* FAKEMATCH: current in r2 */
        s32 distance = target - current;
        
        if (distance < 0) distance = -distance;
        asm("" : "+r"(target)); /* FAKEMATCH: two more references raise target priority */
        if (DIRECTION(entity) == 0)
        {
            if (current >= target)
            {
                entity->subState = 1;
                entity->velY.whole = 0;
                goto wait;
            }
            if (distance > 0xFFFFF)
            {
                if (entity->velY.whole > 0xFFFF)
                    goto move;
                entity->velY.whole += 0x800;
            }
            else { v = (distance / 0x10000) << 12; k = 0x1000; goto add; }
        }
        else
        {
            if (current <= target)
            {
                entity->subState = 1;
                entity->velY.whole = 0;
                goto wait;
            }
            if (distance > 0xFFFFF)
            {
                if (entity->velY.whole <= -0x10000)
                    goto move;
                entity->velY.whole -= 0x800;
            }
            else { { s32 q_ = distance; asm("" : "+r"(q_)); /* FAKEMATCH: keep the test on distance */ if (distance < 0) q_ = distance + 0xFFFF; v = -((q_ >> 16) << 16) / 16; } k = -0x1000; add: entity->velY.whole = v + k; }
        }
        goto move;
    }
    case 1:
        if ((s8)--entity->timer <= 0)
        {
            entity->subState++;
            DIRECTION(entity) ^= 1;
        }
        break;
    case 2:
    {
        s32 target = entity->ext.layerObject.shuttleStart;
        register s32 current asm("r2") = y; /* FAKEMATCH: current in r2 */
        s32 distance = target - current;
        if (distance < 0) distance = -distance;
        if (DIRECTION(entity) == 0)
        {
            if (current >= target)
            {
                entity->subState = 3;
                entity->velY.whole = 0;
                asm(""); /* FAKEMATCH: keep this copy apart from the case 0 block (cross-jump) */
                goto wait;
            }
            if (distance > 0xFFFFF)
            {
                if (entity->velY.whole > 0xFFFF)
                    goto move;
                entity->velY.whole += 0x800;
            }
            else entity->velY.whole = ((distance / 0x10000) << 12) + 0x1000;
        }
        else
        {
            if (current <= target)
            {
                { u8 three = 3; entity->subState = three; entity->velY.whole = 0; }
            wait:
                { register struct ObjectVariant *tbl asm("r1") = sObject29Variants; /* FAKEMATCH: table base in r1 */ entity->timer = tbl[entity->spawnParam1].delay; }
                goto move;
            }
            if (distance > 0xFFFFF)
            {
                if (entity->velY.whole <= -0x10000)
                    goto move;
                entity->velY.whole -= 0x800;
            }
            else entity->velY.whole = -((distance / 0x10000) << 16) / 16 - 0x1000;
        }
    move:
        entity->ext.layerObject.y += entity->velY.whole;
        break;
    }
    case 3:
        if ((s8)--entity->timer <= 0)
        {
            entity->subState = 0;
            DIRECTION(entity) ^= 1;
        }
        break;
    }
    entity->posX.whole = entity->ext.layerObject.x - camera->xPos.whole;
    entity->posY.whole = entity->ext.layerObject.y - camera->yPos.whole;
}

/* Object29Update. */
/**
 * @brief 57380 | Object 0x29 update: Player_QueueEntity(entity, 1), state 0
 * Object29_UpdateMovement, animate and Hit_CheckWithPlayer.
 */
void Object29Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;

    Player_QueueEntity(entity, 1);
    if (entity->state == 0)
        Object29_UpdateMovement(entity);
    Anim_Update(entity);
    Hit_CheckWithPlayer(&result, entity);
}

/* Object29_LoadGraphics (object 0x29 graphics). */

/* Graphics per variant (spawnParam1): gfx, palette, animation; unk_D is the
 * palette argument of ObjPalette_FindOrLoadVariant. */

/**
 * @brief 573B0 | Load the graphics of variant spawnParam1 from sObject29Variants
 * and draw with Object29_Draw; 0 when no slot is free.
 */
u8 Object29_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject29Variants[entity->spawnParam1].graphics, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sObject29Variants[entity->spawnParam1].palette,
                                                      sObject29Variants[entity->spawnParam1].paletteIndex);
    Sprite_Attach(entity, sObject29Variants[entity->spawnParam1].sprites, gfx, entity->basePalette);
    entity->drawFunc = Object29_Draw;
    return 1;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Object29_Draw = Object34Part_Draw */

/**
 * @brief 57418 | Same as Object34Part_Draw.
 */
void Object29_Draw(struct EwramData_EntityData *entity)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;

    entity->posX.whole = entity->ext.layerObject.drawX;
    entity->posY.whole = entity->ext.layerObject.drawY;
    Sprite_Draw(entity);
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}

void Object2ACreate(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    struct FourBoxes boxes = sObject2ABoxes;
    if (!Object2A_LoadGraphics(entity)) { EntityDelete(entity); return; }
    Hit_Enable(entity, 8, NULL, 0);
    HitBox_SetBody(entity, &boxes.box[entity->spawnParam1]);
    HitBox_SetAttack(entity, &boxes.box[entity->spawnParam1]);
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    Anim_StartIndexed(entity, sObject2AVariants[entity->spawnParam1].animation, 0, 1);
    Anim_Update(entity);
    entity->spriteLayer = 18;
    entity->state = 0;
    entity->ext.layerObject.shuttleStart = entity->posX.whole + camera->xPos.whole;
    entity->ext.layerObject.shuttleEnd = entity->spawnParam0 << 16;
    entity->ext.layerObject.x = entity->posX.whole + camera->xPos.whole;
    entity->ext.layerObject.y = entity->posY.whole + camera->yPos.whole;
    entity->ext.layerObject.drawX = entity->ext.layerObject.x - camera->xPos.whole;
    entity->ext.layerObject.drawY = entity->ext.layerObject.y - camera->yPos.whole;
    entity->scrollWithBg = 0;
    if ((s32)entity->ext.layerObject.shuttleStart > entity->ext.layerObject.shuttleEnd) entity->ext.layerObject.shuttleDir = 1;
    else entity->ext.layerObject.shuttleDir = 0;
}

/* Object2A_UpdateMovement: horizontal moving platform; matched. */

void Object2A_UpdateMovement(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    s32 target, x, dist, vel, vel2;

    switch (entity->subState)
    {
    case 0:
        target = entity->ext.layerObject.shuttleEnd;
        x = entity->ext.layerObject.x;
        dist = target - x;
        if (dist < 0)
            dist = -dist;
        vel = entity->velX.whole;
        entity->ext.layerObject.x = x + vel;
        if (!entity->ext.layerObject.shuttleDir)
        {
            if (x >= target)
            {
                entity->subState++;
                entity->velX.whole = 0;
                entity->timer = sObject2AVariants[entity->spawnParam1].delay;
                break;
            }
            if (dist > 0xFFFFF)
            {
                if (vel > 0xFFFF)
                    break;
                entity->velX.whole = vel + 0x800;
                break;
            }
            entity->velX.whole = (dist / 0x10000) * 0x1000 + 0x1000;
        }
        else
        {
            if (x <= target)
            {
                entity->subState++;
                entity->velX.whole = 0;
                entity->timer = sObject2AVariants[entity->spawnParam1].delay;
                break;
            }
            if (dist > 0xFFFFF)
            {
                if (vel <= -0x10000)
                    break;
                entity->velX.whole = vel - 0x800;
                break;
            }
            entity->velX.whole = -((dist / 0x10000) << 16) / 16 - 0x1000;
        }
        break;
    case 1:
        if ((s8)--entity->timer > 0)
            break;
        entity->subState++;
        entity->ext.layerObject.shuttleDir ^= 1;
        break;
    case 2:
        target = entity->ext.layerObject.shuttleStart;
        x = entity->ext.layerObject.x;
        dist = target - x;
        if (dist < 0)
            dist = -dist;
        vel2 = entity->velX.whole;
        entity->ext.layerObject.x = x + vel2;
        if (!entity->ext.layerObject.shuttleDir)
        {
            if (x >= target)
            {
                entity->subState++;
                entity->velX.whole = 0;
                entity->timer = sObject2AVariants[entity->spawnParam1].delay;
                break;
            }
            if (dist > 0xFFFFF)
            {
                if (vel2 > 0xFFFF)
                    break;
                entity->velX.whole = vel2 + 0x800;
                break;
            }
            entity->velX.whole = (dist / 0x10000) * 0x1000 + 0x1000;
        }
        else
        {
            if (x <= target)
            {
                entity->subState++;
                entity->velX.whole = 0;
                entity->timer = sObject2AVariants[entity->spawnParam1].delay;
                break;
            }
            if (dist > 0xFFFFF)
            {
                if (vel2 <= -0x10000)
                    break;
                entity->velX.whole = vel2 - 0x800;
                break;
            }
            entity->velX.whole = -((dist / 0x10000) << 16) / 16 - 0x1000;
        }
        break;
    case 3:
        if ((s8)--entity->timer > 0)
            break;
        entity->subState = 0;
        entity->ext.layerObject.shuttleDir ^= 1;
        break;
    }
    entity->posX.whole = entity->ext.layerObject.x - camera->xPos.whole;
    entity->posY.whole = entity->ext.layerObject.y - camera->yPos.whole;
    entity->ext.layerObject.drawX = entity->ext.layerObject.x - camera->xPos.whole;
    entity->ext.layerObject.drawY = entity->ext.layerObject.y - camera->yPos.whole;
    if (gEwramData->playerState.standingPlatform == entity)
        Player_SetXFraction((s16)entity->posX.part16.fraction);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Object2AUpdate = Object29Update */

/**
 * @brief 57744 | Same as Object29Update.
 */
void Object2AUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;

    Player_QueueEntity(entity, 1);
    if (entity->state == 0)
        Object2A_UpdateMovement(entity);
    Anim_Update(entity);
    Hit_CheckWithPlayer(&result, entity);
}

/* Object2A_LoadGraphics (object 0x2A graphics). */

/* Graphics per variant (spawnParam1): gfx, palette, animation; unk_D is the
 * palette argument of ObjPalette_FindOrLoadVariant. */

/**
 * @brief 57774 | Load the graphics of variant spawnParam1 from sObject2AVariants
 * and draw with Object2A_Draw; 0 when no slot is free.
 */
u8 Object2A_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject2AVariants[entity->spawnParam1].graphics, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sObject2AVariants[entity->spawnParam1].palette,
                                                      sObject2AVariants[entity->spawnParam1].paletteIndex);
    Sprite_Attach(entity, sObject2AVariants[entity->spawnParam1].sprites, gfx, entity->basePalette);
    entity->drawFunc = Object2A_Draw;
    return 1;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Object2A_Draw = Object34Part_Draw */

/**
 * @brief 577DC | Same as Object34Part_Draw.
 */
void Object2A_Draw(struct EwramData_EntityData *entity)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;

    entity->posX.whole = entity->ext.layerObject.drawX;
    entity->posY.whole = entity->ext.layerObject.drawY;
    Sprite_Draw(entity);
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}
