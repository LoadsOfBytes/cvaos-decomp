/*
 * Lilith: body and shots.
 */
#include "core.h"
#include "graphics.h"
#include "entity_util.h"
#include "player_core.h"
#include "sound.h"
#include "structs/ewram.h"
#include "gameplay.h"
#include "effects.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"

extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Lilith_UpdateWalk(struct EwramData_EntityData *);
extern void Lilith_UpdateAttack(struct EwramData_EntityData *);

void EnemyLilithUpdate(struct EwramData_EntityData *entity)
{
    /* Original reserves 0x30 bytes for this scratch area. */
    u32 box[12];
    u8 frame;
    if (entity->state != 2)
    {
        if (EntityIsOutsideScreen(entity, 4)) return;
        if (Entity_IsInRect(entity, -16, 0, 0x110, 0xB0)) Player_TrackEntity(entity);
    }
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 2 && (s8)entity->ext.lilith.hidden == 0)
            Hit_CheckWithPlayer(box, entity);
        return;
    }
    switch (entity->state)
    {
    case 0: Lilith_UpdateWalk(entity); break;
    case 1: Lilith_UpdateAttack(entity); break;
    case 2: Enemy_UpdateFragmentDeath(entity); break;
    case 3: Anim_Update(entity); return;
    }
    frame = entity->animFrame;
    Anim_Update(entity);
    if (frame != entity->animFrame) Entity_UpdateBoxes(entity);
    if (entity->state != 2 && (s8)entity->ext.lilith.hidden == 0)
        Hit_CheckWithPlayer(box, entity);
}

extern u8 Lilith_LoadGraphics(struct EwramData_EntityData *);
extern void Hit_EnableContact(struct EwramData_EntityData *, s32, void *, void *);
void Lilith_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void Lilith_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);

void EnemyLilithCreate(struct EwramData_EntityData *entity)
{
    if (!Lilith_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 4, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        Anim_StartIndexed(entity, 5, 0, 1);
        Anim_Update(entity);
        return;
    }
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, Lilith_DamagePlayerOnContact, Lilith_OnHit);
    else
        Hit_Enable(entity, 8, Lilith_DamagePlayerOnContact, (s32)Lilith_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->state = 0;
    entity->hidden = 1;
    entity->ext.lilith.hidden = 1;
    entity->velY.whole = 0x10000;
    entity->accelY.whole = 0x1800;
}

extern s32 Target_IsInRect(s16, s16, s16, s16);
extern s32 Entity_ClampXToRoom(struct EwramData_EntityData *, s32);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s32);
extern struct EwramData_EntityData *Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);

/**
 * @brief 948DC | Enemy AI like Succubus_UpdateWalk driven by acceleration: wakes
 * (animation 4, visible, song 0x1C8) on the player box 0x64 x 0x50, then
 * walks towards (state 2) / away from (state 3) the player with accelX
 * +-0x800, lunging after a landing like Succubus_UpdateWalk; every 60 frames of
 * walking (unk_4FC) a distant player (0xC8 px) triggers attack 10
 * (state = 1), a close one attack 0.
 */
void Lilith_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 speed;
    s32 speed2;
    s32 speed3;
    s32 found;
    s32 found2;
    s32 land;
    s32 wall;

    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        if (Target_IsInRect(entity->posX.part16.integer - 0x32, entity->posY.part16.integer - 0x28, 0x64, 0x50))
        {
            entity->subState++;
            Anim_StartIndexed(entity, 4, 0, 0);
            entity->hidden = 0;
            PlaySong(0x1C8);
        }
        break;
    case 1:
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 5, 0, 1);
            entity->subState++;
            entity->timer = 0;
            entity->ext.lilith.hidden = 0;
        }
        break;
    case 2:
        if (entity->timer++ > 0x3B)
        {
            entity->flipX = Entity_IsTargetRight(entity);
            entity->timer = 0;
        }
        if (entity->flipX == (s8)Target_GetFlipXOrOpposite(entity)
            && Target_IsInRect(entity->posX.part16.integer - 0x32, entity->posY.part16.integer - 0x28, 0x64, 0x50))
        {
            land = Player_GetFocusedActionFlags() & 0x60;
            if (!land)
            {
                entity->ext.lilith.dodgeChecked = land;
            }
            else if (!*(s8 *)&entity->ext.lilith.dodgeChecked)
            {
                if (!(RandomNumberGenerator() & 1))
                {
                    if (entity->flipX)
                        entity->velX.whole = -0x28000;
                    else
                        entity->velX.whole = 0x28000;
                    entity->subState = 4;
                    entity->timer = 0;
                    entity->accelX.whole = 0;
                    Afterimage_Create(entity, 0x38, 0xC, 0x10);
                    Afterimage_Create(entity, 0x38, 6, 0x10);
                    if ((RandomNumberGenerator() & 0x7F) <= 0x2F)
                        PlaySong(0x1C8);
                    break;
                }
                entity->ext.lilith.dodgeChecked = 1;
            }
        }
        if (entity->flipX)
            entity->accelX.whole = 0x800;
        else
            entity->accelX.whole = -0x800;
        speed = 0x10000;
        Entity_MoveWithCollision(entity, speed, -0x200000, 0x6000, 8);
        found = Target_IsInRect(entity->posX.part16.integer - 0x28, entity->posY.part16.integer - 0x28, 0x50, 0x50);
        if (found)
        {
            if (!(RandomNumberGenerator() & 1))
            {
                entity->state = 1;
                entity->subState = 0;
                entity->timer = 0;
                break;
            }
            entity->subState = 3;
            entity->timer = 0;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x1800;
            break;
        }
        wall = Entity_ClampXToRoom(entity, 4);
        if (wall)
        {
            entity->subState = 3;
            entity->timer = found;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x1800;
            break;
        }
        if (entity->ext.lilith.walkFrame++ > 0x3B)
        {
            entity->ext.lilith.walkFrame = wall;
            goto far;
        }
        break;
    case 3:
        entity->flipX = Entity_IsTargetRight(entity);
        if (entity->flipX == (s8)Target_GetFlipXOrOpposite(entity)
            && Target_IsInRect(entity->posX.part16.integer - 0x32, entity->posY.part16.integer - 0x28, 0x64, 0x50))
        {
            land = Player_GetFocusedActionFlags() & 0x60;
            if (!land)
            {
                entity->ext.lilith.dodgeChecked = land;
            }
            else if (!*(s8 *)&entity->ext.lilith.dodgeChecked)
            {
                if (!(RandomNumberGenerator() & 1))
                {
                    if (entity->flipX)
                        entity->velX.whole = -0x28000;
                    else
                        entity->velX.whole = 0x28000;
                    entity->subState = 4;
                    entity->timer = 0;
                    entity->accelX.whole = 0;
                    Afterimage_Create(entity, 0x38, 0xC, 0x10);
                    Afterimage_Create(entity, 0x38, 6, 0x10);
                    break;
                }
                entity->ext.lilith.dodgeChecked = 1;
            }
        }
        if (entity->flipX)
            entity->accelX.whole = -0x800;
        else
            entity->accelX.whole = 0x800;
        speed2 = 0x10000;
        Entity_MoveWithCollision(entity, speed2, -0x200000, 0x6000, 8);
        found2 = Target_IsInRect(entity->posX.part16.integer - 0x46, entity->posY.part16.integer - 0x28, 0x8C, 0x50);
        if (!found2)
        {
            if (!(RandomNumberGenerator() & 1))
            {
                entity->subState = 2;
                entity->timer = found2;
                entity->velY.whole = speed2;
                entity->accelY.whole = 0x1800;
                break;
            }
            entity->state = 1;
            entity->subState = 10;
            entity->timer = found2;
            break;
        }
        if (Entity_ClampXToRoom(entity, 4))
        {
            entity->subState = 2;
            entity->timer = 0;
            entity->velY.whole = speed2;
            entity->accelY.whole = 0x1800;
            break;
        }
        if (Target_IsInRect(entity->posX.part16.integer - 0x28, entity->posY.part16.integer - 0x28, 0x50, 0x50) && !(RandomNumberGenerator() & 1))
        {
            entity->state = 1;
            entity->subState = 0;
            entity->timer = 0;
            break;
        }
        if (entity->ext.lilith.walkFrame++ > 0x3B)
        {
            entity->ext.lilith.walkFrame = 0;
        far:
            if (Target_IsInRect(entity->posX.part16.integer - 0xC8, entity->posY.part16.integer - 0x28, 0x190, 0x50))
            {
                s32 r = RandomNumberGenerator() & 1;
                if (!r)
                {
                    entity->state = 1;
                    entity->subState = 10;
                    entity->timer = r;
                }
            }
        }
        break;
    case 4:
        EntityDampVelocityX(entity, 0x18);
        speed3 = 0x10000;
        Entity_MoveWithCollision(entity, speed3, -0x200000, 0x40000, 8);
        Entity_ClampXToRoom(entity, 4);
        if (entity->timer++ > 0x27)
        {
            entity->subState = 2;
            entity->timer = 0;
            entity->velX.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = speed3;
            entity->accelY.whole = 0x1800;
            entity->ext.lilith.dodgeChecked = 0;
        }
        break;
    }
}

extern void Lilith_SpawnShot(struct EwramData_EntityData *, s32, s32);

/**
 * @brief 94D28 | Enemy attack AI (state != 0 branch), all states damp velX
 * and apply gravity (Entity_MoveWithCollision): states 0-2 lunge in animation 7 (dust
 * via Afterimage_Create, sound 0x85; ext.lilithLunge.hit chains a second lunge), state 3
 * recovers for 30 frames; states 10-12 fire a projectile (Lilith_SpawnShot,
 * sound 0x7B) in animation 6. Both paths return to the walk animation 5.
 */
void Lilith_UpdateAttack(struct EwramData_EntityData *entity)
{
    s32 speed;

    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 7, 0, 0);
        entity->subState++;
        entity->accelX.whole = 0;
        entity->timer = 0;
    case 1:
        EntityDampVelocityX(entity, 0x18);
        Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x40000, 8);
        Entity_ClampXToRoom(entity, 4);
        if (entity->animStep <= 3)
            break;
        if (entity->flipX)
            entity->velX.whole = 0x30000;
        else
            entity->velX.whole = -0x30000;
        entity->subState++;
        entity->timer = 0;
        Afterimage_Create(entity, 0x38, 8, 0x10);
        Afterimage_Create(entity, 0x38, 4, 0x10);
        entity->ext.lilithLunge.hit = 0;
        PlaySong(0x85);
        break;
    case 2:
        EntityDampVelocityX(entity, 0x18);
        speed = 0x10000;
        Entity_MoveWithCollision(entity, speed, -0x200000, 0x40000, 8);
        Entity_ClampXToRoom(entity, 4);
        if (*(s8 *)&entity->ext.lilithLunge.hit)
            entity->animTimer = 0x64;
        if (entity->animStep <= 0xC)
            break;
        entity->flipX = Entity_IsTargetRight(entity);
        if (*(s8 *)&entity->ext.lilithLunge.hit)
        {
            if (entity->flipX)
                entity->velX.whole = -0x20000;
            else
                entity->velX.whole = 0x20000;
            entity->subState++;
            entity->timer = 0;
            entity->accelX.whole = 0;
            entity->animTimer = 0;
            break;
        }
        entity->state = 0;
        entity->subState = 3;
        entity->timer = 0;
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
        entity->velY.whole = speed;
        entity->accelY.whole = 0x1800;
        Anim_StartIndexed(entity, 5, 0, 1);
        break;
    case 3:
        EntityDampVelocityX(entity, 0x18);
        speed = 0x10000;
        Entity_MoveWithCollision(entity, speed, -0x200000, 0x40000, 8);
        Entity_ClampXToRoom(entity, 4);
        if (entity->timer++ > 0x1D)
        {
            entity->state = 0;
            entity->subState = 2;
            entity->timer = 0;
            entity->velX.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x1800;
            entity->ext.lilith.dodgeChecked = 0;
            Anim_StartIndexed(entity, 5, 0, 1);
        }
        break;
    case 10:
        Anim_StartIndexed(entity, 6, 0, 0);
        entity->subState++;
        entity->accelX.whole = 0;
        entity->timer = 0;
    case 11:
        EntityDampVelocityX(entity, 0x18);
        Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x40000, 8);
        Entity_ClampXToRoom(entity, 4);
        if (entity->animStep > 4)
        {
            Lilith_SpawnShot(entity, 0, -0x1C0000);
            entity->subState++;
            PlaySong(0x7B);
        }
        break;
    case 12:
        EntityDampVelocityX(entity, 0x18);
        speed = 0x10000;
        Entity_MoveWithCollision(entity, speed, -0x200000, 0x40000, 8);
        Entity_ClampXToRoom(entity, 4);
        if (entity->animDone)
        {
            entity->flipX = Entity_IsTargetRight(entity);
            entity->state = 0;
            entity->subState = 2;
            entity->timer = 0;
            entity->velX.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x1800;
            Anim_StartIndexed(entity, 5, 0, 1);
        }
        break;
    }
}

extern s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s16, s32);

void Lilith_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
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
        PlaySong(0x1CC);
        Anim_StartIndexed(entity, 9, 0, 0);
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->state = 2;
        entity->subState = 0;
    }
}

extern void LilithShot_Update(struct EwramData_EntityData *);
extern void LilithShot_DamagePlayerOnContact(struct EwramData_EntityData *);
void LilithShot_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern u8 sLilithSuccubusGfx[], sLilithSuccubusSpriteSheet[];
/**
 * @brief Spawn the wavy shot at owner + (dx, dy), two layers below
 * (LilithShot_Update).
 */
void Lilith_SpawnShot(struct EwramData_EntityData *owner, s32 dx, s32 dy)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)LilithShot_Update);
    u32 box = 0x0A0AFBFB;
    if (entity != NULL)
    {
        s32 graphics;
        entity->posX.whole = owner->posX.whole + dx;
        entity->posY.whole = owner->posY.whole + dy;
        entity->spriteLayer = owner->spriteLayer - 2;
        graphics = ObjGfx_Find(sLilithSuccubusGfx);
        entity->basePalette = owner->basePalette;
        Sprite_Attach(entity, sLilithSuccubusSpriteSheet, graphics, entity->basePalette);
        entity->animFrame = 0x36;
        Hit_Enable(entity, 8, LilithShot_DamagePlayerOnContact, (s32)LilithShot_OnHit);
        HitBox_SetAttack(entity, &box);
        HitBox_SetBody(entity, &box);
        entity->hit.flags &= ~4;
        entity->hit.flags &= ~2;
        entity->enemyId = owner->enemyId;
        entity->attack = owner->attack;
        entity->scrollWithBg = 1;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        if (owner->flipX)
        {
            entity->ext.lilithShot.angle = 0;
            entity->timer = 21;
        }
        else
        {
            entity->ext.lilithShot.angle = 0x8000;
            entity->timer = 7;
        }
        entity->ext.lilithShot.trail0 = Afterimage_Create(entity, 0, 10, 0);
        entity->ext.lilithShot.trail1 = Afterimage_Create(entity, 0, 5, 0);
    }
}

void LilithShot_Update(struct EwramData_EntityData *entity)
{
    s32 buffer[4];
    switch (entity->state)
    {
    case 0:
        entity->timer++;
        if (entity->timer <= 14) entity->ext.lilithShot.angle += 0x400;
        else if (entity->timer <= 28)
        {
            entity->ext.lilithShot.angle -= 0x400;
            if (entity->timer > 27) entity->timer = 0;
        }
        EntitySetVelocityFromAngle(entity, entity->ext.lilithShot.angle, 0x10000);
        EntityMove(entity);
        Hit_CheckWithPlayer(buffer, entity);
        if (EntityIsOutsideScreen(entity, 1)) entity->deleteRequested = 1;
        break;
    case 1: entity->deleteRequested = 1; break;
    case 2: entity->deleteRequested = 1; break;
    }
    if (entity->deleteRequested)
    {
        if (entity->ext.lilithShot.trail0 != NULL) Entity_EndIfActive(entity->ext.lilithShot.trail0);
        if (entity->ext.lilithShot.trail1 != NULL) Entity_EndIfActive(entity->ext.lilithShot.trail1);
    }
}

/* LilithShot_DamagePlayerOnContact. */
/**
 * @brief 95250 | Player_ApplyContactDamage with (attack, 0x80, 0, 0) and state 2.
 */
void LilithShot_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0x80;
    contactDamage.unused = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
    entity->state = 2;
    entity->subState = 0;
}

/* LilithShot_OnHit. */

/**
 * @brief 95288 | Hit: call Player_CalculateAttackPower on the damage value (result
 * unused) and go to state 1.
 */
void LilithShot_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    Player_CalculateAttackPower(hit[2]);
    entity->state = 1;
    entity->subState = 0;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Lilith_LoadGraphics = Object32_LoadGraphics */

extern u8 sLilithSuccubusPalette[];
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

/**
 * @brief 952A0 | Same as Object32_LoadGraphics.
 */
u8 Lilith_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sLilithSuccubusGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sLilithSuccubusPalette, 1);
    Sprite_Attach(entity, sLilithSuccubusSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/* Lilith_DamagePlayerOnContact. */
/**
 * @brief 952EC | Set ext.lilithLunge.hit and Player_ApplyContactDamage with (attack, 0, 0, 1)
 * at the entity position.
 */
void Lilith_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    entity->ext.lilithLunge.hit = 1;
    Player_ApplyContactDamage(&contactDamage);
}
