/*
 * Ectoplasm: update, create, floating, hits, graphics and particles.
 */
#include "core.h"
#include "graphics.h"
#include "entity_util.h"
#include "player_core.h"
#include "structs/ewram.h"
#include "syscalls.h"
#include "sound.h"

extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Ectoplasm_UpdateFloat(struct EwramData_EntityData *);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *);
extern void Ectoplasm_UpdatePreview(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);

void EnemyEctoplasmUpdate(struct EwramData_EntityData *entity)
{
    u32 box[4];
    if (entity->state != 1)
    {
        if (EntityIsOutsideScreen(entity, 4)) return;
        if (Entity_IsInRect(entity, -16, 0, 0x110, 0xB0)) Player_TrackEntity(entity);
    }
    if (EntityIsOutsideScreen(entity, 4)) entity->hidden = 1;
    else entity->hidden = 0;
    if (entity->state != 1) Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity) && entity->state != 1)
    {
        Hit_CheckWithPlayer(box, entity);
        return;
    }
    switch (entity->state)
    {
    case 0: Ectoplasm_UpdateFloat(entity); break;
    case 1: EntityAccelerateAndMove(entity, 0x40000); Enemy_UpdateFragmentDeath(entity); break;
    case 2: Ectoplasm_UpdatePreview(entity); Anim_Update(entity); return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}

extern u8 Ectoplasm_LoadGraphics(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *);
extern void Hit_EnableContact(struct EwramData_EntityData *, s32, void *, void *);
extern void Hit_Enable(struct EwramData_EntityData *, s32, void *, void *);
void Ectoplasm_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void Ectoplasm_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);

void EnemyEctoplasmCreate(struct EwramData_EntityData *entity)
{
    struct EwramData **ewram = &gEwramData;
    struct BgLayerState *background = &(*ewram)->bgInfo[1];
    if (!Ectoplasm_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if ((*ewram)->gameState.bestiaryPreview)
    {
        entity->ext.ectoplasm.pointX = entity->posX.whole + background->xPos.whole;
        entity->ext.ectoplasm.pointY = entity->posY.whole + background->yPos.whole;
        entity->state = 2;
        return;
    }
    entity->spriteLayer = 3;
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, Ectoplasm_DamagePlayerOnContact, Ectoplasm_OnHit);
    else Hit_Enable(entity, 8, Ectoplasm_DamagePlayerOnContact, Ectoplasm_OnHit);
    Entity_UpdateBoxes(entity);
    entity->state = 0;
    entity->ext.ectoplasm.pointX = entity->posX.whole + background->xPos.whole;
    entity->ext.ectoplasm.pointY = entity->posY.whole + background->yPos.whole;
    entity->scrollWithBg = 1;
    entity->accelY.whole = -0x100;
    entity->timer = 0x1E;
}

/* Flying enemy: approaches a random target near Target_GetX/Target_GetY and re-targets. */
extern s32 Target_IsInRect(s16, s16, s16, s16);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);

static inline void SetTarget(struct EwramData_EntityData *entity, struct BgLayerState *bg, s32 r, s32 dy)
{
    s32 x = Target_GetX(entity);
    s32 bx = bg->xPos.whole;
    if (!(RandomNumberGenerator() & 1))
        entity->ext.ectoplasm.pointX = x - r + bx;
    else
        entity->ext.ectoplasm.pointX = x + r + bx;
    x = Target_GetY(entity) - RandomNumberGenerator() % 0x300000;
    x += bg->yPos.whole + dy;
    entity->ext.ectoplasm.pointY = x;
}

void Ectoplasm_UpdateFloat(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 state = entity->subState;
    s32 d;
    s32 dy;
    s32 speed;

    switch (state)
    {
    case 0:
        { s32 bx_ = bg->xPos.whole; entity->posX.whole = entity->ext.ectoplasm.pointX - bx_; }
        dy = entity->ext.ectoplasm.pointY - (entity->posY.whole + bg->yPos.whole);
        entity->posY.whole += dy + Sin(entity->ext.ectoplasm.bobPhase + 0x4000) * 3;
        entity->ext.ectoplasm.bobPhase += 0x200;
        entity->timer++;
        if (Target_IsInRect(entity->posX.part16.integer - 0x5A, entity->posY.part16.integer - 0x64, 0xB4, 0xC8))
        {
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 1:
        if (!entity->ext.ectoplasm.retarget)
        {
            entity->ext.ectoplasm.retarget = 1;
            { s32 r = RandomNumberGenerator() % 0x300000 + 0x100000;
            SetTarget(entity, bg, r, -0x100000); }
        }
        else
        {
            entity->ext.ectoplasm.retarget = 0;
            { s32 r = RandomNumberGenerator() % 0x300000 + 0x300000;
            SetTarget(entity, bg, r, -0x200000); }
        }
        entity->subState++;
        entity->timer = 0;
    case 2:
        d = entity->ext.ectoplasm.pointX - (entity->posX.whole + bg->xPos.whole);
        dy = entity->ext.ectoplasm.pointY - (entity->posY.whole + bg->yPos.whole);
        if (d > 0x40000)
            entity->accelX.whole = 0x400;
        else if (d < -0x40000)
            entity->accelX.whole = -0x400;
        else
            entity->accelX.whole = 0;
        if (dy > 0x40000)
            entity->accelY.whole = 0x400;
        else if (dy < -0x40000)
            entity->accelY.whole = -0x400;
        else
            entity->accelY.whole = 0;
        EntityDampVelocityX(entity, 0x20);
        EntityDampVelocityY(entity, 0x20);
        speed = 0x10000;
        EntityAccelerateAndMove(entity, speed);
        if (Player_GetFocusedActionFlags() & 0x360)
        {
            EntityDampVelocityX(entity, 0x20);
            EntityDampVelocityY(entity, 0x20);
            EntityAccelerateAndMove(entity, speed);
        }
        if ((d < 0 ? -d : d) <= 0x3FFFF && (dy < 0 ? -dy : dy) <= 0x3FFFF)
            entity->subState = 1;
        if (entity->timer++ > 0x77)
            entity->subState = 1;
        break;
    }
}

extern s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s16, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);

void Ectoplasm_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
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
    {
        s32 dx = entity->posX.whole - attacker->posX.whole;
        s32 dy = entity->posY.whole - attacker->posY.whole;
        u16 angle = ArcTan2(dx < 0 ? -((-dx) >> 16) : dx >> 16, dy < 0 ? -((-dy) >> 16) : dy >> 16);
        s32 speed;
        dx = Sin(angle + 0x4000);
        dy = Sin(angle);
        speed = 0x10000;
        entity->velX.whole = FixedMultiply(dx, speed);
        entity->velY.whole = FixedMultiply(dy, speed);
    }
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x6D);
        Anim_StartIndexed(entity, 1, 0, 1);
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->state = 1;
        entity->subState = 0;
        entity->scrollWithBg = 1;
        entity->velX.whole = 0;
        entity->velY.whole = 0;
        entity->accelX.whole = 0;
        entity->accelY.whole = 0x1000;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Ectoplasm_LoadGraphics = Object25_LoadGraphics
 *
 * Ectoplasm_UpdatePreview..EctoplasmParticle_Update decompiled locally. */

extern u8 sEctoplasmGfx[];
extern u8 sEctoplasmPalette[];
extern u8 sEctoplasmSpriteSheet[];
void Ectoplasm_Draw(struct EwramData_EntityData *entity);
void Ectoplasm_SpawnParticle(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY, s32 frame);
void EctoplasmParticle_Update(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *updateFunc);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *entity, s32 maxSpeed);
extern s32 EntityIsOutsideScreen(struct EwramData_EntityData *entity, s32 margin);
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

/**
 * @brief 999F0 | Same as Object25_LoadGraphics.
 */
u8 Ectoplasm_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sEctoplasmGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sEctoplasmPalette, 0);
    Sprite_Attach(entity, sEctoplasmSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Ectoplasm_Draw;
    return 1;
}

/**
 * @brief 99A3C | Substate 0: follow the room position (unk_4F8, unk_4FC)
 * relative to background 1, bobbing vertically by 3 * cos(unk_500).
 */
void Ectoplasm_UpdatePreview(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 baseY;

    if (entity->subState == 0)
    {
        entity->posX.whole = entity->ext.ectoplasm.pointX - bg->xPos.whole;
        baseY = entity->ext.ectoplasm.pointY - bg->yPos.whole;
        entity->posY.whole = baseY + Sin(entity->ext.ectoplasm.bobPhase + 0x4000) * 3;
        entity->ext.ectoplasm.bobPhase += 0x200;
        entity->timer++;
    }
}

/**
 * @brief 99A90 | Draw; while visible (and unless paused) spawn a particle
 * of type 3 or 9 at a random offset every 8 frames.
 */
void Ectoplasm_Draw(struct EwramData_EntityData *entity)
{
    s32 offsetX;
    s32 offsetY;

    Sprite_Draw(entity);
    if (gEwramData->paused && !gEwramData->gameState.bestiaryPreview)
        return;
    if (EntityIsOutsideScreen(entity, 6))
        return;

    offsetX = (RandomNumberGenerator() & 0xFFFFF) - 0x80000;
    offsetY = -(RandomNumberGenerator() & 0x7FFFF);
    if ((gEwramData->frameCounter & 7) == 0)
        Ectoplasm_SpawnParticle(entity, offsetX, offsetY, 3);
    if ((gEwramData->frameCounter & 7) == 4)
        Ectoplasm_SpawnParticle(entity, offsetX, offsetY, 9);
}

/**
 * @brief 99B30 | Player_ApplyContactDamage with (attack, 0x80, 0, 0) at the entity
 * position.
 */
void Ectoplasm_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0x80;
    contactDamage.unused = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 99B60 | Spawn a rising particle (update EctoplasmParticle_Update) at the
 * entity position plus (offsetX, offsetY) with frame frame.
 */
void Ectoplasm_SpawnParticle(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY, s32 frame)
{
    struct EwramData_EntityData *child = Entity_CreateObject((u32 *)EctoplasmParticle_Update);
    s32 gfx;

    if (child != NULL)
    {
        child->posX.whole = entity->posX.whole + offsetX;
        child->posY.whole = entity->posY.whole + offsetY;
        child->spriteLayer = entity->spriteLayer + 1;
        gfx = ObjGfx_Find(sEctoplasmGfx);
        Sprite_Attach(child, sEctoplasmSpriteSheet, gfx, entity->basePalette);
        child->animFrame = frame;
        child->flipX = RandomNumberGenerator() % 2;
        child->velY.whole = -0x10000;
        child->accelY.whole = -0x400;
        child->scrollWithBg = 1;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}

/**
 * @brief 99C08 | Particle update: rise, advance the frame every 3 frames
 * and delete at frame 9 or 13 or when off screen.
 */
void EctoplasmParticle_Update(struct EwramData_EntityData *entity)
{
    EntityAccelerateAndMove(entity, 0x20000);
    if (entity->timer++ > 2)
    {
        entity->timer = 0;
        entity->animFrame++;
        if (entity->animFrame == 9 || entity->animFrame == 0xD)
        {
            EntityDelete(entity);
            return;
        }
    }
    if (EntityIsOutsideScreen(entity, 6))
        EntityDelete(entity);
}
