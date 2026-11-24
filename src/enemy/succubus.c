/*
 * Succubus.
 */
#include "core.h"
#include "player_core.h"
#include "graphics.h"
#include "entity_util.h"
#include "structs/ewram.h"
#include "sound.h"
#include "structs/entity_render_flags.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "hitbox.h"
#include "damage_numbers.h"
#include "entity_motion.h"
#include "effects.h"
#include "player_soma.h"
#include "entity_hit.h"

void Succubus_UpdateWalk(struct EwramData_EntityData *entity);
void Succubus_UpdateAttack(struct EwramData_EntityData *entity);
u8 Succubus_LoadGraphics(struct EwramData_EntityData *);
void Succubus_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void Succubus_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);

/**
 * @brief 93C34 | Succubus update (as EnemyDevilUpdate): skipped while off
 * screen unless dying, hidden off screen (hidden), states 0 / 1
 * (Succubus_UpdateWalk / Succubus_UpdateAttack), 2 dying, 3 animation only; contact test
 * unless dying or unk_4FF is set. The "previous frame" compared before
 * Entity_UpdateBoxes is uninitialised in the original.
 */
void EnemySuccubusUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    u8 previous;

    if (entity->state != 2)
    {
        if (EntityIsOutsideScreen(entity, 4))
            return;
        if (Entity_IsInRect(entity, -16, 0, 272, 176))
            Player_TrackEntity(entity);
    }
    if (EntityIsOutsideScreen(entity, 4))
        entity->hidden = 1;
    else
        entity->hidden = 0;
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 2 && (s8)entity->ext.succubus.holding == 0)
            Hit_CheckWithPlayer(&result, entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        Succubus_UpdateWalk(entity);
        break;
    case 1:
        Succubus_UpdateAttack(entity);
        break;
    case 2:
        Enemy_UpdateFragmentDeath(entity);
        break;
    case 3:
        Anim_Update(entity);
        return;
    }
    Anim_Update(entity);
    if (previous != entity->animFrame)
        Entity_UpdateBoxes(entity);
    if (entity->state != 2 && (s8)entity->ext.succubus.holding == 0)
        Hit_CheckWithPlayer(&result, entity);
}

void EnemySuccubusCreate(struct EwramData_EntityData *entity)
{
    if (!Succubus_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 10, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        Anim_StartIndexed(entity, 1, 0, 1);
        Anim_Update(entity);
        return;
    }
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, Succubus_DamagePlayerOnContact, Succubus_OnHit);
    else
        Hit_Enable(entity, 8, Succubus_DamagePlayerOnContact, Succubus_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->spriteLayer = 9;
    if (entity->spawnParam0 == 0)
    {
        entity->state = 0;
        entity->ext.succubus.intro = 1;
        entity->subState = 0;
    }
    else
    {
        Anim_StartIndexed(entity, 1, 0, 1);
        entity->subState = 2;
        entity->ext.succubus.intro = 0;
        entity->ext.succubus.holding = 0;
    }
    entity->velY.whole = 0x10000;
    entity->accelY.whole = 0x1800;
}

/**
 * @brief 93E04 | Enemy AI (subState): growls (song 0x1C7) once the player is
 * near and wakes when unk_4FC is set or the player is in front; walks
 * towards (state 2) / away from (state 3) the player at 0.375 px, turning
 * every 30 frames. Facing the player (Target_GetFlipXOrOpposite) within 0x32 px, after a
 * landing (Player_GetFocusedActionFlags & 0x60) it may lunge (velX 2.5 px, two dust puffs
 * via Afterimage_Create, state 4 for 40 frames) once per landing (unk_4FD);
 * attacks (state = 1) at random when the player is in the 0x50 box.
 */
void Succubus_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 speed;
    s32 speed2;
    s32 speed3;
    s32 found;
    s32 found2;
    s32 land;
    s32 facing;

    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        if (!*(s8 *)&entity->ext.succubus.seen
            && Target_IsInRect((s16)(entity->posX.part16.integer - 0x5A), (s16)(entity->posY.part16.integer - 0x40), 0xB4, 0x46))
        {
            entity->ext.succubus.seen = 1;
            PlaySong(0x1C7);
        }
        if (entity->ext.succubus.provoked == 1
            || Target_IsInRect((s16)(entity->posX.part16.integer - 0x1E), (s16)(entity->posY.part16.integer - 0x40), 0x3C, 0x46))
        {
            entity->subState++;
            Anim_StartIndexed(entity, 0, 0, 0);
        }
        break;
    case 1:
        if (*(u16 *)&entity->animStep == 5)
            PlaySong(0x1C8);
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->subState++;
            entity->timer = 0;
            entity->ext.succubus.intro = 0;
            entity->ext.succubus.holding = 0;
        }
        break;
    case 2:
        if (entity->timer++ > 0x1D)
        {
            entity->timer = 0;
            entity->flipX = Entity_IsTargetRight(entity);
            if (entity->flipX)
                entity->velX.whole = 0x6000;
            else
                entity->velX.whole = -0x6000;
        }
        if (entity->flipX == (s8)Target_GetFlipXOrOpposite(entity)
            && Target_IsInRect((s16)(entity->posX.part16.integer - 0x32), (s16)(entity->posY.part16.integer - 0x50), 0x64, 0xA0))
        {
            land = Player_GetFocusedActionFlags() & 0x60;
            if (!land)
            {
                entity->ext.succubus.dodgeChecked = land;
            }
            else if (!*(s8 *)&entity->ext.succubus.dodgeChecked)
            {
                if (!(RandomNumberGenerator() & 1))
                {
                    if (entity->flipX)
                        entity->velX.whole = -0x28000;
                    else
                        entity->velX.whole = 0x28000;
                    entity->subState = 4;
                    entity->timer = 0;
                    Afterimage_Create(entity, 0x38, 0xC, 0x10);
                    Afterimage_Create(entity, 0x38, 6, 0x10);
                    if ((RandomNumberGenerator() & 0x7F) <= 0x2F)
                        PlaySong(0x1C8);
                    break;
                }
                entity->ext.succubus.dodgeChecked = 1;
            }
        }
        speed = 0x10000;
        Entity_MoveWithCollision(entity, speed, -0x200000, 0x40000, 8);
        found = Target_IsInRect((s16)(entity->posX.part16.integer - 0x28), (s16)(entity->posY.part16.integer - 0x50), 0x50, 0xA0);
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
        if (Entity_ClampXToRoom(entity, 4))
        {
            entity->subState = 3;
            entity->timer = found;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x1800;
        }
        break;
    case 3:
        if (entity->timer++ > 0x1D)
        {
            entity->timer = 0;
            entity->flipX = Entity_IsTargetRight(entity);
            if (entity->flipX)
                entity->velX.whole = -0x6000;
            else
                entity->velX.whole = 0x6000;
        }
        if (entity->flipX == (s8)Target_GetFlipXOrOpposite(entity)
            && Target_IsInRect((s16)(entity->posX.part16.integer - 0x32), (s16)(entity->posY.part16.integer - 0x50), 0x64, 0xA0))
        {
            land = Player_GetFocusedActionFlags() & 0x60;
            if (!land)
            {
                entity->ext.succubus.dodgeChecked = land;
            }
            else if (!*(s8 *)&entity->ext.succubus.dodgeChecked)
            {
                if (!(RandomNumberGenerator() & 1))
                {
                    if (entity->flipX)
                        entity->velX.whole = -0x28000;
                    else
                        entity->velX.whole = 0x28000;
                    entity->subState = 4;
                    entity->timer = 0;
                    Afterimage_Create(entity, 0x38, 0xC, 0x10);
                    Afterimage_Create(entity, 0x38, 6, 0x10);
                    break;
                }
                entity->ext.succubus.dodgeChecked = 1;
            }
        }
        speed2 = 0x10000;
        Entity_MoveWithCollision(entity, speed2, -0x200000, 0x40000, 8);
        found2 = Target_IsInRect((s16)(entity->posX.part16.integer - 0x46), (s16)(entity->posY.part16.integer - 0x50), 0x8C, 0xA0);
        if (!found2)
        {
            entity->subState = 2;
            entity->timer = found2;
            entity->velY.whole = speed2;
            entity->accelY.whole = 0x1800;
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
        if (Target_IsInRect((s16)(entity->posX.part16.integer - 0x28), (s16)(entity->posY.part16.integer - 0x50), 0x50, 0xA0) && !(RandomNumberGenerator() & 1))
        {
            entity->state = 1;
            entity->subState = 0;
            entity->timer = 0;
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
            entity->velY.whole = speed3;
            entity->accelY.whole = 0x1800;
            entity->ext.succubus.dodgeChecked = 0;
        }
        break;
    }
}

/**
 * @brief Attack state machine (state 1): dash at 3 px per frame (sound 0x1C6) in
 * grab mode (unk_500 = 1); when Succubus_DamagePlayerOnContact latched on
 * (unk_501) hold the player (sub-states 10.., sound 0x7B), then let go
 * (unk_501 = unk_4FF = 0) and return to state 0.
 */
void Succubus_UpdateAttack(struct EwramData_EntityData *entity)
{
    s32 speed;
    s32 speed2;
    s32 hit;
    s32 dx;
    s32 v;

    switch (entity->subState)
    {
    case 0:
        EntityDampVelocityX(entity, 0x10);
        Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x40000, 8);
        Entity_ClampXToRoom(entity, 4);
        if (entity->timer++ > 0x27)
        {
            Anim_StartIndexed(entity, 2, 0, 0);
            entity->subState++;
            entity->velX.whole = 0;
            entity->timer = 0;
            PlaySong(0x1C6);
        }
        break;
    case 1:
        entity->flipX = Entity_IsTargetRight(entity);
        EntityDampVelocityX(entity, 0x18);
        Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x40000, 8);
        Entity_ClampXToRoom(entity, 4);
        if (entity->animStep > 2)
        {
            if (entity->flipX)
                entity->velX.whole = 0x30000;
            else
                entity->velX.whole = -0x30000;
            entity->subState++;
            entity->timer = 0;
            entity->ext.succubus.lunging = 1;
            entity->ext.succubus.grabbed = 0;
            Afterimage_Create(entity, 0x38, 0xC, 0x10);
            Afterimage_Create(entity, 0x38, 6, 0x10);
        }
        break;
    case 2:
        EntityDampVelocityX(entity, 0x18);
        speed = 0x10000;
        Entity_MoveWithCollision(entity, speed, -0x200000, 0x40000, 8);
        Entity_ClampXToRoom(entity, 4);
        hit = *(s8 *)&entity->ext.succubus.grabbed;
        if (hit)
        {
            entity->subState = 10;
            entity->timer = 0x10;
            entity->ext.succubus.drainCount = 0;
            entity->ext.succubus.drainTimer = 0x14;
            entity->ext.succubus.escapePresses = 0;
            break;
        }
        if (entity->timer++ > 0x1D)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->state = hit;
            entity->subState = 2;
            entity->timer = hit;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x1800;
            entity->ext.succubus.lunging = hit;
            entity->ext.succubus.drainCount = hit;
            entity->ext.succubus.drainTimer = hit;
        }
        break;
    case 10:
    {
        u8 t; register u32 z_ asm("r0"); /* FAKEMATCH: r0 live across the test */
        entity->flipX = Entity_IsTargetRight(entity);
        gEwramData->playerState.enemyTarget->flipX = entity->flipX;
        t = entity->timer;
        if (({ asm("" : "=r"(z_)); /* FAKEMATCH: r0 live across the test */ t; }))
        {
            struct EwramData_EntityData *player;
            s32 y, ex, ey; register s32 n asm("r8"); /* FAKEMATCH: divisor in r8 */
            {
                u8 b_ = *(ENTITY_RENDER_FLAGS_ADDRESS(entity)); u32 k_;
                asm("" :: "r"(z_)); /* FAKEMATCH: r0 live across the test */
                k_ = 0x40;
                k_ &= b_;
                if (k_)
                    dx = -0xC0000;
                else
                    dx = 0xC0000;
            }
            player = gEwramData->playerState.player;
            dx = player->posX.whole + dx - (ex = entity->posX.whole);
            y = player->posY.whole; y -= (ey = entity->posY.whole);
            n = *(volatile u8 *)&entity->timer; /* FAKEMATCH: volatile read forces the reload of the original */
            dx /= n;
            y /= n;
            entity->posX.whole = ex + dx;
            entity->posY.whole = ey + y;
            entity->timer = t - 1;
        }
        v = ({ s32 t = entity->ext.succubus.drainTimer + 1; entity->ext.succubus.drainTimer = t; (s8)t; });
        if (v > 0x2C)
        {
            entity->ext.succubus.drainTimer = 0;
            if (({ s32 t = entity->ext.succubus.drainCount + 1; entity->ext.succubus.drainCount = t; (s8)t; }) > 4)
                goto check;
            {
                struct PlayerContactDamage effect;
                effect.attack = entity->attack;
                effect.attributes = 1;
                effect.unused = 0;
                effect.reaction = 0;
                effect.sourceX = entity->posX.whole;
                effect.sourceY = entity->posY.whole;
                Player_ApplyContactDamage(&effect);
            }
            PlaySong(0x7B);
            {
                register u32 z0_ asm("r0"); register u32 z2_ asm("r2"); u8 b_; register u32 k_ asm("r0"); /* FAKEMATCH: and result in r0 */
                asm("" : "=r"(z0_), "=r"(z2_)); /* FAKEMATCH: r0/r2 live across the flag load (reload rotation) */
                b_ = *(ENTITY_RENDER_FLAGS_ADDRESS(entity));
                asm("" :: "r"(z0_), "r"(z2_)); /* FAKEMATCH: r0/r2 live across the flag load (reload rotation) */
                k_ = 0x40;
                k_ &= b_;
                if (k_)
                    dx = 0x10000;
                else
                    dx = -0x10000;
            }
            entity->posX.whole += dx;
        }
        else if (v == 10)
        {
            if (entity->flipX)
                dx = -0x10000;
            else
                dx = 0x10000;
            entity->posX.whole += dx;
        }
    check:
        if ((gEwramData->inputData.playerNewInput & 0x3FF) && *(s8 *)&entity->ext.succubus.escapePresses <= 0x63)
            entity->ext.succubus.escapePresses++;
        if (Player_StartScriptedAnimation(0xE, 0, 0, 0) || *(s8 *)&entity->ext.succubus.drainCount > 4 || *(s8 *)&entity->ext.succubus.escapePresses > 0x1D)
        {
            Anim_StartIndexed(entity, 3, 0, 0);
            {
                register u32 z0_ asm("r0"); register u32 z2_ asm("r2"); u8 b_; register u32 k_ asm("r0"); /* FAKEMATCH: and result in r0 */
                asm("" : "=r"(z0_), "=r"(z2_)); /* FAKEMATCH: r0/r2 live across the flag load (reload rotation) */
                b_ = *(ENTITY_RENDER_FLAGS_ADDRESS(entity));
                asm("" :: "r"(z0_), "r"(z2_)); /* FAKEMATCH: r0/r2 live across the flag load (reload rotation) */
                k_ = 0x40;
                k_ &= b_;
                if (k_)
                    entity->velX.whole = -0x28000;
                else
                    entity->velX.whole = 0x28000;
            }
            entity->accelX.whole = 0;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x1800;
            entity->subState++;
            entity->timer = 0;
            entity->ext.succubus.dodgeChecked = 1;
            entity->ext.succubus.lunging = 0;
            entity->ext.succubus.grabbed = 0;
            entity->ext.succubus.holding = 0;
            entity->ext.succubus.escapePresses = 0;
        }
        break;
    }
    case 11:
        EntityDampVelocityX(entity, 0x18);
        speed2 = 0x10000;
        Entity_MoveWithCollision(entity, speed2, -0x200000, 0x40000, 8);
        Entity_ClampXToRoom(entity, 4);
        if (entity->timer++ > 0x27)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->state = 0;
            entity->subState = 2;
            entity->timer = 0;
            entity->velY.whole = speed2;
            entity->accelY.whole = 0x1800;
            entity->ext.succubus.dodgeChecked = 0;
        }
        break;
    }
}

/**
 * @brief Hit: resolve the damage, flash, wake it (unk_4FC = 1), subtract and show
 * the damage; at HP <= 0 drop the rewards and die (state 2).
 */
void Succubus_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 damage = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(entity, attacker, hit, damage);
    entity->hitFlashTimer = 8;
    entity->ext.succubus.provoked = 1;
    {
        u16 hp = entity->hp;
        damage <<= 16;
        damage >>= 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, (s16)damage, 0);
    if ((s8)entity->ext.succubus.intro != 0)
    {
        entity->ext.succubus.holding = 1;
        if ((s16)entity->hp > 0) return;
        entity->hp = 1;
    }
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

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Succubus_LoadGraphics = Object25_LoadGraphics
 * plus Succubus_DamagePlayerOnContact (decompiled locally). */

/**
 * @brief 946C4 | Same as Object25_LoadGraphics.
 */
u8 Succubus_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sLilithSuccubusGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sLilithSuccubusPalette, 0);
    Sprite_Attach(entity, sLilithSuccubusSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 94710 | When not lunging, Player_ApplyContactDamage with (attack, 0, 0, 0)
 * at the entity position; otherwise grab the player (grabbed = holding = 1,
 * escape presses = 0).
 */
void Succubus_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    if ((s8)entity->ext.succubus.lunging == 0)
    {
        contactDamage.attack = entity->attack;
        contactDamage.attributes = 0;
        contactDamage.unused = 0;
        contactDamage.reaction = 0;
        contactDamage.sourceX = entity->posX.whole;
        contactDamage.sourceY = entity->posY.whole;
        Player_ApplyContactDamage(&contactDamage);
    }
    else
    {
        entity->ext.succubus.grabbed = 1;
        entity->ext.succubus.holding = 1;
        entity->ext.succubus.escapePresses = 0;
    }
}
