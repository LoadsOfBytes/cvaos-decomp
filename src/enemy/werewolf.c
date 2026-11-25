/*
 * Werewolf: body and shockwave.
 */
#include "player_core.h"
#include "graphics.h"
#include "entity_util.h"
#include "sound.h"
#include "core.h"
#include "structs/ewram.h"
#include "gameplay.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "werejaguar.h"
#include "data/data_118B48.h"
#include "hitbox.h"
#include "damage_numbers.h"
#include "entity_motion.h"
#include "entity_hit.h"
#include "constants/hit_flags.h"
#include "constants/songs.h"

void Werewolf_UpdateWalk(struct EwramData_EntityData *);
void Werewolf_UpdateAttack(struct EwramData_EntityData *);
void Werewolf_UpdateRecoil(struct EwramData_EntityData *);
void Werewolf_SpawnShockwave(struct EwramData_EntityData *, s32);
void WerewolfShockwave_Update(struct EwramData_EntityData *);
u8 WerewolfShockwave_LoadGraphics(struct EwramData_EntityData *);
void WerewolfShockwave_EnableHitBoxes(struct EwramData_EntityData *);
u8 Werewolf_LoadGraphics(struct EwramData_EntityData *entity);
void WerewolfShockwave_DamagePlayerOnContact(struct EwramData_EntityData *entity);

/* Werewolf update dispatcher and jump state. */

/* States of Werewolf (entity->state), named after the update function the dispatcher calls. */
enum {
    WEREWOLF_STATE_WALK = 0,
    WEREWOLF_STATE_ATTACK = 1,
    WEREWOLF_STATE_DEATH = 2,
    WEREWOLF_STATE_RECOIL = 3,
    WEREWOLF_STATE_ANIM_ONLY = 4,
};

void EnemyWerewolfUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    if (entity->state != WEREWOLF_STATE_DEATH)
    {
        if (EntityIsOutsideScreen(entity, 4))
            return;
        if (Entity_IsInRect(entity, -0x10, 0, 0x110, 0xB0))
            Player_TrackEntity(entity);
    }
    if (EntityIsOutsideScreen(entity, 4))
        entity->hidden = 1;
    else
        entity->hidden = 0;
    if (entity->state != WEREWOLF_STATE_DEATH)
        Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity))
    {
        if (entity->state != WEREWOLF_STATE_DEATH)
        {
            Hit_CheckWithPlayer(&result, entity);
            return;
        }
    }
    if (entity->hitFlashTimer != 0 && entity->state != 3 && entity->state != WEREWOLF_STATE_DEATH)
    {
        Hit_CheckWithPlayer(&result, entity);
        return;
    }
    switch (entity->state)
    {
    case WEREWOLF_STATE_WALK: Werewolf_UpdateWalk(entity); break;
    case WEREWOLF_STATE_ATTACK: Werewolf_UpdateAttack(entity); break;
    case WEREWOLF_STATE_DEATH: Werejaguar_UpdateDeath(entity); break;
    case WEREWOLF_STATE_RECOIL: Werewolf_UpdateRecoil(entity); break;
    case WEREWOLF_STATE_ANIM_ONLY:
        Anim_Update(entity);
        return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}

/* Werewolf ground movement and transition into an attack. */

/**
 * @brief Walk state machine (state 0): land, face the target and wait until it is
 * in the 160 x 160 box, then approach and start the attack (state 1).
 */
void Werewolf_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 flags;
    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
        entity->velY.whole = 0x20000;
        entity->accelY.whole = 0x2800;
        flags = Entity_MoveWithCollision(entity, 0x20000, -0x200000, 0x80000, 0xA);
        if (flags & 4)
        {
            if (Target_IsInRect((s16)(entity->posX.part16.integer - 0x50),
                            (s16)(entity->posY.part16.integer - 0x50), 0xA0, 0xA0)
                || Enemy_IsDamaged(entity))
            {
                entity->subState = 1;
                Anim_StartIndexed(entity, 1, 0, 1);
            }
        }
        break;
    case 1:
        entity->flipX = Entity_IsTargetRight(entity);
        if (!EntityIsOutsideScreen(entity, 3))
        {
            if (entity->animTimer == 0)
            {
                if (entity->flipX)
                    entity->velX.whole = 0x10000;
                else
                    entity->velX.whole = -0x10000;
                if (entity->animStep == 3)
                    entity->velX.whole *= 4;
            }
            else
                entity->velX.whole = 0;
            flags = Entity_MoveWithCollision(entity, 0x20000, -0x200000, 0x80000, 0xA);
        if (flags & 4)
            {
                if (Target_IsInRect((s16)(entity->posX.part16.integer - 0x3C),
                                (s16)(entity->posY.part16.integer - 0x28), 0x78, 0x50))
                {
                    entity->state = WEREWOLF_STATE_ATTACK;
                    entity->subState = 0;
                    entity->velX.whole = 0;
                    entity->timer = 0;
                    Anim_StartIndexed(entity, 0, 0, 1);
                }
            }
        }
        else
        {
            entity->subState = 0;
            Anim_StartIndexed(entity, 0, 0, 1);
        }
        break;
    }
}

/* Werewolf attack phases, 0x0806F024..0x0806F41B. */

/**
 * @brief Attack state machine (state 1): randomly a dash (sound 0x77) or a ground
 * slam spawning shockwaves (Werewolf_SpawnShockwave at 0x40, then 0x30 and
 * 0x50 px, sound 0x79); then back to state 0.
 */
void Werewolf_UpdateAttack(struct EwramData_EntityData *entity)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    s32 flags;
    s32 x;
    switch (entity->subState)
    {
    case 0:
        flags = Entity_MoveWithCollision(entity, 0x20000, -0x200000, 0x80000, 0xA);
        if (!(flags & 4)) break;
        if (entity->timer++ <= 0x1D) break;
        if (RandomNumberGenerator() & 1)
        {
            entity->subState = 1;
            Anim_StartIndexed(entity, 2, 0, 0);
            entity->animDone = 0;
            break;
        }
        else
        {
            if (entity->flipX)
                x = entity->posX.part16.integer + 0x40;
            else
                x = entity->posX.part16.integer - 0x20;
            if (!Tile_Classify((s16)(background->xPos.part16.integer + x),
                             (s16)(background->yPos.part16.integer + entity->posY.part16.integer)))
            {
                entity->subState = 3;
                Anim_StartIndexed(entity, 3, 0, 0);
            }
            else
            {
                entity->subState = 1;
                Anim_StartIndexed(entity, 2, 0, 0);
            }
        }
        entity->animDone = 0;
        break;
    case 1:
        if (entity->animStep <= 1) break;
        entity->subState++;
        if (entity->flipX) entity->velX.whole = 0x50000;
        else entity->velX.whole = -0x50000;
        entity->accelX.whole = 0;
        entity->velY.whole = 0x20000;
        entity->accelY.whole = 0x2800;
        entity->timer = 0;
        entity->ext.werewolf.dashHit = 0;
        /* fall through */
    case 2:
        if (entity->ext.werewolf.dashHit != 0)
        {
            entity->ext.werewolf.dashHit = 0;
            Anim_StartIndexed(entity, 0, 0, 0);
            entity->state = WEREWOLF_STATE_RECOIL;
            entity->subState = 0;
            break;
        }
        if (entity->animDone)
        {
            entity->velX.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = 0x20000;
            entity->accelY.whole = 0x2800;
            Entity_MoveWithCollision(entity, 0x20000, -0x200000, 0x80000, 0xA);
            entity->state = WEREWOLF_STATE_WALK;
            entity->subState = 1;
            Anim_StartIndexed(entity, 1, 0, 1);
            break;
        }
        if (entity->animStep > 2)
        {
            EntityDampVelocityX(entity, 4);
            Entity_MoveWithCollision(entity, 0x20000, -0x200000, 0x80000, 0xA);
            break;
        }
        EntityDampVelocityX(entity, 0x10);
        flags = Entity_MoveWithCollision(entity, 0x20000, -0x200000, 0x80000, 0xA);
        if (!(entity->timer++ & 3) && (flags & 4))
            Effect_SpawnRisingParticleInBox(entity->posX.whole + entity->velX.whole * 6,
                         entity->posY.whole - 0x20000, 0x60000, 0x40000, entity->spriteLayer - 1, 1);
        break;
    case 3:
        if (entity->animStep <= 1) break;
        entity->subState++;
        entity->animTimer = 0;
        entity->timer = 0;
        entity->animDone = 0;
        break;
    case 4:
        if (!(entity->timer & 7))
        {
            PlaySong(0x77);
            if (entity->flipX)
                Effect_SpawnRandom(entity->posX.whole - 0xE0000, entity->posY.whole - 0x140000, 0x40000, 0x40000, entity->spriteLayer - 1);
            else
                Effect_SpawnRandom(entity->posX.whole + 0xE0000, entity->posY.whole - 0x140000, 0x40000, 0x40000, entity->spriteLayer - 1);
        }
        entity->timer++;
        entity->animTimer = 0;
        if (entity->timer == 0x1E) entity->animTimer = 0xC8;
        if (entity->timer == 0x1F) Werewolf_SpawnShockwave(entity, 0x40);
        if (entity->timer > 0x23)
        {
            entity->subState++;
            Werewolf_SpawnShockwave(entity, 0x30);
            Werewolf_SpawnShockwave(entity, 0x50);
            PlaySong(0x79);
        }
        break;
    case 5:
        if (entity->animStep <= 3) break;
        if (entity->flipX) entity->velX.whole = 0x160000;
        else entity->velX.whole = -0x160000;
        entity->accelX.whole = 0;
        entity->velY.whole = 0x20000;
        entity->accelY.whole = 0x2800;
        Entity_MoveWithCollision(entity, 0x20000, -0x200000, 0x80000, 0xA);
        entity->subState++;
        entity->velX.whole = 0;
        break;
    case 6:
        if (!entity->animDone) break;
        entity->accelX.whole = 0;
        entity->velY.whole = 0x20000;
        entity->accelY.whole = 0x2800;
        Entity_MoveWithCollision(entity, 0x20000, -0x200000, 0x80000, 0xA);
        Anim_StartIndexed(entity, 0, 0, 1);
        Entity_UpdateBoxes(entity);
        entity->timer = 0;
        entity->subState++;
        break;
    case 7:
        if (++entity->timer > 0x2C)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->state = WEREWOLF_STATE_WALK;
            entity->subState = 1;
            entity->timer = 0;
        }
        break;
    }
}

/* Werewolf jump: face the player, launch, then land into animation 1. */
/**
 * @brief Recoil state machine (state 3): hop back from the target and resume on
 * landing.
 */
void Werewolf_UpdateRecoil(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        if (entity->flipX)
            entity->velX.whole = -0x18000;
        else
            entity->velX.whole = 0x18000;
        entity->accelX.whole = 0;
        entity->velY.whole = -0x24000;
        entity->accelY.whole = 0x3000;
        entity->subState++;
        /* fall through */
    case 1:
        if (Entity_MoveWithCollision(entity, 0x40000, -0x200000, 0x80000, 0xA) & 4)
        {
            entity->velX.whole = 0;
            entity->state = WEREWOLF_STATE_WALK;
            entity->subState = 1;
            Anim_StartIndexed(entity, 1, 0, 1);
        }
        break;
    }
}

/* Hit-effect strength follows the parent's current action phase. */
/**
 * @brief Contact: Player_ApplyContactDamage with half the attack, the full attack
 * and knockback mode 1 during the dash (state 1, sub-states 1..2; ext.werewolf.dashHit
 * records it).
 */
void Werewolf_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage effect;
    effect.attributes = 0;
    effect.unused = 0;
    effect.attack = entity->attack >> 1;
    effect.reaction = 0;
    entity->ext.werewolf.dashHit = 0;
    if (entity->state == 1 && (u8)(entity->subState - 1) <= 1)
    {
        effect.attack = entity->attack;
        effect.reaction = 1;
        entity->ext.werewolf.dashHit = 1;
    }
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
}

/* Werewolf hit: HP, death state, and interrupting a close attack. */

/**
 * @brief Hit: resolve, flash, subtract and show the damage; at HP <= 0 drop the
 * rewards and die (song 0x6E, animation 4, state 2); a hit of type 3 / 4
 * during the dash makes it recoil (state 3).
 */
void Werewolf_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hitData)
{
    s32 damage = Player_CalculateAttackPower(hitData[2]);
    struct EntityHit *hit = &attacker->hit;
    damage = Hit_ResolveDamage(entity, attacker, hitData, damage);
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
        PlaySong(SE_ENEMY_DEATH);
        Anim_StartIndexed(entity, 4, 0, 0);
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->state = WEREWOLF_STATE_DEATH;
        entity->subState = 0;
    }
    else if (entity->state == 1 && (u8)(entity->subState - 1) <= 1
             && (u8)(hit->type - 3) <= 1)
    {
        Anim_StartIndexed(entity, 4, 0, 1);
        entity->state = WEREWOLF_STATE_RECOIL;
        entity->subState = 0;
    }
}

/* Spawn and position a Werewolf attack entity. */

/**
 * @brief Spawn a ground shockwave offset px in front of parent, snapped to the
 * ground, with the parent's attack.
 */
void Werewolf_SpawnShockwave(struct EwramData_EntityData *entity, s32 offsetX)
{
    struct EwramData_EntityData *child = EntityCreateInRange(ENTITY_SLOT_OBJECT_FIRST, ENTITY_SLOT_OBJECT_LAST, WerewolfShockwave_Update);
    if (child != NULL)
    {
        if (!WerewolfShockwave_LoadGraphics(child))
            EntityDelete(child);
        WerewolfShockwave_EnableHitBoxes(child);
        child->scrollWithBg = 1;
        child->spriteLayer = 0x12;
        ENTITY_CREATOR(child) = entity;
        if (entity->flipX)
            child->posX.part16.integer = entity->posX.part16.integer + offsetX;
        else
            child->posX.part16.integer = entity->posX.part16.integer - offsetX;
        child->posY.part16.integer = entity->posY.part16.integer;
        Entity_SnapToGround(child);
        child->posY.part16.integer++;
        child->flipX = entity->flipX;
        child->attack = entity->attack;
    }
}

/* Werewolf helpers 0x0806F678..0x0806F817 (EnemyWerewolfCreate like
 * EnemyWerejaguarCreate without Hit_SetTimerOverride). */

/**
 * @brief 6F678 | Effect: states 0 / 1 go to 2; state 2 runs Hit_CheckWithPlayer
 * while animStep is 1..5 and animates until the end (state 3); state 3 sets
 * 0x53D bit 3.
 */
void WerewolfShockwave_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;

    switch (entity->state)
    {
        case 0:
        case 1:
            entity->state = 2;
            break;
        case 2:
            if ((u8)(entity->animStep - 1) <= 4)
                Hit_CheckWithPlayer(&result, entity);
            if (Anim_Update(entity) == 3)
                entity->state = 3;
            break;
        case 3:
            entity->deleteRequested = 1;
            break;
    }
}

/**
 * @brief 6F6D4 | Take the common effect graphics (palette 7, animation 0xA);
 * 0 when they are not loaded.
 */
u8 WerewolfShockwave_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_Find(sCommonObjectGfx);

    if (gfx < 0)
        return 0;
    Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, 7);
    Anim_StartIndexed(entity, 0xA, 0, 0);
    Anim_Update(entity);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 6F71C | Box slot 8 with WerewolfShockwave_DamagePlayerOnContact and the box 0x1810E8F8 (x -8,
 * y -24, w 16, h 24 - not a ROM address); clear flags bit 1, set bit 2.
 */
void WerewolfShockwave_EnableHitBoxes(struct EwramData_EntityData *entity)
{
    Hit_Enable(entity, 8, WerewolfShockwave_DamagePlayerOnContact, 0);
    HitBox_SetAttackRaw(entity, 0x1810E8F8);
    entity->hit.flags &= ~HIT_NO_ATTACK;
    entity->hit.flags |= HIT_NO_DAMAGE;
}

/**
 * @brief 6F750 | Player_ApplyContactDamage with (attack, 2, 0, 3) at the entity
 * position, then Player_SetKnockbackVelocity with (+-2 px by the facing, -5 px).
 */
void WerewolfShockwave_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;
    s32 knockbackX;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 2;
    contactDamage.unused = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    contactDamage.reaction = 3;
    Player_ApplyContactDamage(&contactDamage);
    knockbackX = -0x20000;
    if (entity->flipX)
        knockbackX = -knockbackX;
    Player_SetKnockbackVelocity(knockbackX, -0x50000);
}

/**
 * @brief 6F7A4 | Werewolf create: load the graphics (delete the enemy
 * when that fails), start the animation and set up the hit boxes; state 4
 * instead when bestiaryPreview is set.
 */
void EnemyWerewolfCreate(struct EwramData_EntityData *entity)
{
    if (!Werewolf_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = WEREWOLF_STATE_ANIM_ONLY;
        return;
    }
    Hit_EnableContact(entity, 8, Werewolf_DamagePlayerOnContact, Werewolf_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->state = WEREWOLF_STATE_WALK;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Werewolf_LoadGraphics = LegionWalkingCorpse_LoadGraphics */

/**
 * @brief 6F818 | Same as LegionWalkingCorpse_LoadGraphics.
 */
u8 Werewolf_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sWerewolfGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sWerewolfPalette, 0);
    Sprite_Attach(entity, sWerewolfSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}
