/*
 * Persephone: body and attack box.
 */
#include "graphics.h"
#include "entity_util.h"
#include "player_core.h"
#include "core.h"
#include "sound.h"
#include "structs/ewram.h"
#include "bg_and_collision.h"
#include "gameplay.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "damage_numbers.h"
#include "entity_motion.h"
#include "entity_hit.h"
#include "constants/hit_flags.h"
#include "constants/songs.h"

void Persephone_UpdateWalk(struct EwramData_EntityData *);
void Persephone_UpdateAttack(struct EwramData_EntityData *);
void Persephone_UpdatePreview(struct EwramData_EntityData *);
void Persephone_UpdateKnockdown(struct EwramData_EntityData *);
u8 Persephone_LoadGraphics(struct EwramData_EntityData *);
void Persephone_DamagePlayerOnContact(struct EwramData_EntityData *);
void Persephone_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
void Persephone_SpawnAttackBox(struct EwramData_EntityData *, s32, s32);
void PersephoneAttackBox_Update(struct EwramData_EntityData *);

/* States of Persephone (entity->state), named after the update function the dispatcher calls. */
enum {
    PERSEPHONE_STATE_WALK = 0,
    PERSEPHONE_STATE_ATTACK = 1,
    PERSEPHONE_STATE_KNOCKDOWN = 3,
    PERSEPHONE_STATE_PREVIEW = 4,
};

/* Dispatch Persephone's AI, death, reaction and display states. */
void EnemyPersephoneUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    if (entity->state != 2)
    {
        if (EntityIsOutsideScreen(entity, 4)) return;
        if (Entity_IsInRect(entity, -16, 0, 272, 176)) Player_TrackEntity(entity);
    }
    if (EntityIsOutsideScreen(entity, 4)) entity->hidden = 1;
    else entity->hidden = 0;
    if (entity->state != 2) Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity) && entity->state != 2)
    {
        Hit_CheckWithPlayer(&result, entity);
        return;
    }
    if (entity->hitFlashTimer && entity->state != 3 && entity->state != 2)
    {
        Hit_CheckWithPlayer(&result, entity);
        return;
    }
    switch (entity->state)
    {
    case PERSEPHONE_STATE_WALK: Persephone_UpdateWalk(entity); break;
    case PERSEPHONE_STATE_ATTACK: Persephone_UpdateAttack(entity); break;
    case 2:
        entity->accelY.whole = 0x2800;
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
        Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x80000, 8);
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0) entity->deleteRequested = 1;
        break;
    case PERSEPHONE_STATE_PREVIEW:
        Persephone_UpdatePreview(entity);
        Anim_Update(entity);
        return;
    case PERSEPHONE_STATE_KNOCKDOWN: Persephone_UpdateKnockdown(entity); break;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}

/* Initialize Persephone's graphics, collisions and initial facing. */
void EnemyPersephoneCreate(struct EwramData_EntityData *entity)
{
    if (!Persephone_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = PERSEPHONE_STATE_PREVIEW;
        return;
    }
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, Persephone_DamagePlayerOnContact, Persephone_OnHit);
    else
        Hit_Enable(entity, 8, Persephone_DamagePlayerOnContact, Persephone_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->flipX = Entity_IsTargetRight(entity);
    entity->state = PERSEPHONE_STATE_WALK;
    ENTITY_OWNER(entity) = entity;
}

/**
 * @brief 7B680 | Enemy walk/hop AI (subState): wake when the player is in a
 * 0xF0 x 0x60 box around it (or Enemy_IsDamaged), animation 1 with song 0x1C9
 * on frame 5, hop towards the player (velX +-1 px, velY 1 px, gravity
 * 0x2800), land and attack (state = 1) when the player is close; states
 * 5 / 6 walk towards / away from the player at half speed and switch on the
 * player box (Target_IsInRect), a wall (Entity_ClampXToRoom) or randomly.
 */
void Persephone_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 found;
    s32 r;
    s32 speed;
    s32 found2;

    switch (entity->subState)
    {
    case 0:
        if (Target_IsInRect((s16)(entity->posX.part16.integer - 0x78), (s16)(entity->posY.part16.integer - 0x30), 0xF0, 0x60) || Enemy_IsDamaged(entity))
        {
            entity->subState++;
            entity->flipX = Entity_IsTargetRight(entity);
            Anim_StartIndexed(entity, 1, 0, 0);
            entity->timer = 0;
        }
        break;
    case 1:
        if (++entity->timer == 5)
        {
            if (entity->spawnParam0)
                PlaySong(0x1C9);
            else if ((RandomNumberGenerator() & 0x7F) < 0x40)
                PlaySong(0x1C9);
        }
        if (entity->animDone)
        {
            entity->subState++;
            entity->flipX = Entity_IsTargetRight(entity);
            Anim_StartIndexed(entity, 2, 0, 1);
            if (entity->flipX)
                entity->velX.whole = 0x10000;
            else
                entity->velX.whole = -0x10000;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
            entity->timer = 0;
        }
        break;
    case 2:
        if (entity->timer++ > 0x1D)
        {
            entity->flipX = Entity_IsTargetRight(entity);
            if (entity->flipX)
                entity->velX.whole = 0x10000;
            else
                entity->velX.whole = -0x10000;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
            entity->timer = 0;
        }
        if (Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x80000, 8) & 4)
        {
            if (Target_IsInRect((s16)(entity->posX.part16.integer - 0x28), (s16)(entity->posY.part16.integer - 0x30), 0x50, 0x60))
            {
                entity->subState++;
                Anim_StartIndexed(entity, 3, 0, 0);
            }
        }
        break;
    case 3:
        if (entity->animDone)
        {
            entity->state = PERSEPHONE_STATE_ATTACK;
            entity->subState = 0;
            entity->timer = 0;
        }
        break;
    case 5:
        if (entity->timer++ > 0x1D)
        {
            entity->flipX = Entity_IsTargetRight(entity);
            entity->timer = 0;
        }
        if (entity->flipX)
            entity->velX.whole = 0x8000;
        else
            entity->velX.whole = -0x8000;
        speed = 0x10000;
        Entity_MoveWithCollision(entity, speed, -0x200000, 0x80000, 8);
        found = Target_IsInRect((s16)(entity->posX.part16.integer - 0x28), (s16)(entity->posY.part16.integer - 0x30), 0x50, 0x60);
        if (found)
        {
            if (!(RandomNumberGenerator() & 1))
            {
                entity->state = PERSEPHONE_STATE_ATTACK;
                entity->subState = 0;
                entity->timer = 0;
                break;
            }
            entity->subState = 6;
            entity->timer = 0;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x2800;
            break;
        }
        r = Entity_ClampXToRoom(entity, 1);
        if (r)
        {
            entity->subState = 6;
            entity->timer = found;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x2800;
            break;
        }
        if (EntityIsOutsideScreen(entity, 4))
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->subState = r;
        }
        break;
    case 6:
        entity->flipX = Entity_IsTargetRight(entity);
        if (entity->flipX)
            entity->velX.whole = -0x8000;
        else
            entity->velX.whole = 0x8000;
        speed = 0x10000;
        Entity_MoveWithCollision(entity, speed, -0x200000, 0x80000, 8);
        found2 = Target_IsInRect((s16)(entity->posX.part16.integer - 0x3C), (s16)(entity->posY.part16.integer - 0x30), 0x78, 0x60);
        if (!found2)
        {
            entity->subState = 5;
            entity->timer = found2;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x2800;
            break;
        }
        if (Entity_ClampXToRoom(entity, 1))
        {
            entity->subState = 5;
            entity->timer = 0;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x2800;
            break;
        }
        if (Target_IsInRect((s16)(entity->posX.part16.integer - 0x28), (s16)(entity->posY.part16.integer - 0x30), 0x50, 0x60) && !(RandomNumberGenerator() & 1))
        {
            entity->state = PERSEPHONE_STATE_ATTACK;
            entity->subState = 0;
            entity->timer = 0;
        }
        break;
    }
}

/* Persephone_UpdateAttack: enemy behaviour states; matched. */

/**
 * @brief Attack state machine (state 1): approach (animation 4, sound 0x1CA) and
 * swing, spawning a one-frame hit box in front (Persephone_SpawnAttackBox,
 * sound 0x85); then back to state 0.
 */
void Persephone_UpdateAttack(struct EwramData_EntityData *entity)
{
    s32 d;
    s32 v;
    s32 found;
    s32 r;
    s32 res;
    s32 speed;
    s32 speed3;

    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 4, 0, 0);
        entity->subState++;
        entity->ext.persephone.contactHit = 0;
        entity->flipX = Entity_IsTargetRight(entity);
        v = Entity_GetTargetDistanceX(entity);
        if (v < 0)
            v = -v;
        if (v > 0x20 && !(RandomNumberGenerator() & 1))
        {
            entity->subState = 10;
            break;
        }
    case 1:
        if (entity->animStep > 1)
        {
            PlaySong(0x1CA);
            entity->subState++;
            entity->timer++;
            v = Entity_GetTargetDistanceX(entity);
            if (v < 0)
                v = -v;
            if (v <= 7)
                v = -0x10000;
            else if (v <= 0xF)
                v = -0x8000;
            else if (v <= 0x1F)
                v = 0x10000;
            else if (v <= 0x2F)
                v = 0x34000;
            else
                v = 0x40000;
            if (entity->flipX)
                entity->velX.whole = v;
            else
                entity->velX.whole = -v;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
        }
        break;
    case 2:
        if (entity->animStep > 4)
            entity->velX.whole = 0;
        EntityDampVelocityX(entity, 8);
        speed = 0x10000;
        res = Entity_MoveWithCollision(entity, speed, -0x200000, 0x80000, 8);
        if ((res & 4) && entity->animDone)
        {
            if (entity->timer > 1 || (s8)entity->ext.persephone.contactHit == 1)
            {
                entity->subState = 3;
                entity->timer = 0;
                entity->velY.whole = speed;
                entity->accelY.whole = 0x2800;
                Anim_StartIndexed(entity, 5, 0, 1);
                break;
            }
            entity->subState = 0;
            break;
        }
        if (*(u16 *)&entity->animStep == 3)
        {
            Persephone_SpawnAttackBox(entity, entity->posX.whole, entity->posY.whole);
            PlaySong(SE_SWING);
        }
        break;
    case 3:
        entity->flipX = Entity_IsTargetRight(entity);
        if (entity->flipX)
            entity->velX.whole = -0x8000;
        else
            entity->velX.whole = 0x8000;
        speed3 = 0x10000;
        Entity_MoveWithCollision(entity, speed3, -0x200000, 0x80000, 8);
        found = Target_IsInRect((s16)(entity->posX.part16.integer - 0x3C), (s16)(entity->posY.part16.integer - 0x30), 0x78, 0x60);
        if (!found)
        {
            entity->state = found;
            entity->subState = 5;
            entity->timer = found;
            entity->velY.whole = speed3;
            entity->accelY.whole = 0x2800;
            break;
        }
        r = Entity_ClampXToRoom(entity, 1);
        if (r)
        {
            entity->state = PERSEPHONE_STATE_WALK;
            entity->subState = 5;
            do { entity->timer = 0; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
            entity->velY.whole = speed3;
            entity->accelY.whole = 0x2800;
            break;
        }
        if (entity->timer++ <= 0x3B)
            break;
        entity->state = r;
        entity->subState = 6;
        entity->timer = r;
        entity->velY.whole = speed3;
        entity->accelY.whole = 0x2800;
        break;
    case 10:
        if (entity->animStep)
        {
            PlaySong(0x1CA);
            entity->subState++;
            entity->timer++;
            v = 0x14000;
            if (entity->flipX)
                entity->velX.whole = v;
            else
                entity->velX.whole = -v;
            entity->velY.whole = -0x20000;
            entity->accelY.whole = 0x2000;
            Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x80000, 8);
        }
        break;
    case 11:
        if (entity->animStep > 4)
            entity->animFlags |= 2;
        speed = 0x10000;
        res = Entity_MoveWithCollision(entity, speed, -0x200000, 0x80000, 8);
        if (res & 4)
        {
            if (entity->timer > 1 || (s8)entity->ext.persephone.contactHit == 1)
            {
                entity->subState = 3;
                entity->timer = 0;
                entity->velY.whole = speed;
                entity->accelY.whole = 0x2800;
                Anim_StartIndexed(entity, 5, 0, 1);
                break;
            }
            entity->subState = 0;
            break;
        }
        if (*(u16 *)&entity->animStep == 3)
        {
            Persephone_SpawnAttackBox(entity, entity->posX.whole, entity->posY.whole);
            PlaySong(SE_SWING);
        }
        break;
    }
}

/* Alternate Persephone's two display animations after a timed pause. */
/**
 * @brief Bestiary preview state (state 4): every 120 frames play animation 1 once,
 * then idle again.
 */
void Persephone_UpdatePreview(struct EwramData_EntityData *entity)
{
    s32 state = entity->subState;
    switch (state)
    {
    case 0:
        if (entity->timer++ > 119)
        {
            entity->subState++;
            Anim_StartIndexed(entity, 1, 0, 0);
        }
        break;
    case 1:
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->subState = 0;
            {
                s32 timer = RandomNumberGenerator() & 31;
                entity->timer = timer;
            }
        }
        break;
    }
}

/* Load graphics and choose the drawing callback for the room. */
/**
 * @brief Load the Persephone graphics and palette; in area 4 rooms 12 / 13 draw
 * with Entity_DrawWithOffsetCopy instead of Sprite_DrawEntity. Return 0 when no
 * graphics slot is free, else 1.
 */
u8 Persephone_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics = ObjGfx_LoadResource(sPersephoneGfx, 0);
    u16 x, y;
    s32 room;
    if (graphics < 0) return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sPersephonePalette, 0);
    Sprite_Attach(entity, sPersephoneSpriteSheet, graphics, entity->basePalette);
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    room = GetRoomFromMapPosition((s16)x, (s16)y);
    if (GetAreaFromMapPosition((s16)x, (s16)y) == 4 && (u32)(room - 12) <= 1)
        entity->drawFunc = Entity_DrawWithOffsetCopy;
    else
        entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/* Apply damage, choose death, or occasionally react to attack types 3 and 4. */
/**
 * @brief Hit: resolve, flash, subtract and show the damage; at HP <= 0 drop the
 * rewards and die (song 0x1CC, 70 frames, state 2); a hit of type 3 / 4
 * knocks it down with 1/4 chance (animation 6, state 3).
 */
void Persephone_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 attack = Player_CalculateAttackPower(hit[2]);
    u8 *attackType = &attacker->hit.type;
    s16 damage = Hit_ResolveDamage(entity, attacker, hit, attack);
    s32 zero;
    entity->hitFlashTimer = 8;
    {
        s32 hp = entity->hp;
        damage = (s16)damage;
        zero = 0;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x1CC);
        Anim_StartIndexed(entity, 7, 0, 0);
        entity->palette = entity->basePalette;
        entity->timer = 70;
        entity->state = 2;
        entity->subState = zero;
    }
    else if ((u8)(*attackType - 3) <= 1 && (RandomNumberGenerator() & 255) <= 63)
    {
        Anim_StartIndexed(entity, 6, 0, 0);
        entity->state = PERSEPHONE_STATE_KNOCKDOWN;
        entity->subState = zero;
        entity->timer = zero;
    }
}

/* Create a short-lived attack box and advance its collision lifetime. */
/**
 * @brief Spawn an invisible one-frame attack box {-20, -24, 12, 14} at (x, y)
 * facing like parent, with its attack.
 */
void Persephone_SpawnAttackBox(struct EwramData_EntityData *parent, s32 x, s32 y)
{
    struct EwramData_EntityData *part = EntityCreateInRange(ENTITY_SLOT_EFFECT_FIRST, ENTITY_SLOT_EFFECT_LAST, PersephoneAttackBox_Update);
    struct SpriteBox box = {-20, -24, 12, 14};
    if (part)
    {
        s32 graphics;
        part->posX.whole = x;
        part->posY.whole = y;
        part->flipX = parent->flipX;
        part->spriteLayer = parent->spriteLayer - 1;
        graphics = ObjGfx_Find(sPersephoneGfx);
        Sprite_Attach(part, sPersephoneSpriteSheet, graphics, parent->basePalette);
        part->attack = parent->attack;
        ENTITY_OWNER(part) = parent;
        part->animFrame = 20;
        part->ext.persephone.boxTime = 1;
        part->scrollWithBg = 1;
        part->drawFunc = Sprite_DrawEntity;
        Hit_Enable(part, 8, Persephone_DamagePlayerOnContact, 0);
        HitBox_SetAttack(part, &box);
        part->hit.flags &= ~HIT_NO_ATTACK;
        part->hit.flags |= HIT_NO_DAMAGE;
    }
}

/* Create a short-lived attack box and advance its collision lifetime. */
/**
 * @brief Attack box update: contact test, delete when its counter (ext.persephone.boxTime) ends.
 */
void PersephoneAttackBox_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    Hit_CheckWithPlayer(&result, entity);
    if (entity->ext.persephone.boxTime-- <= 0) EntityDelete(entity);
}

/* Fall during the reaction, then resume the landing phase. */
/**
 * @brief Knock-down state (state 3): fall with gravity and, on landing, resume
 * the walk state (sub-state 6, animation 5).
 */
void Persephone_UpdateKnockdown(struct EwramData_EntityData *entity)
{
    s32 gravity = 0x2800;
    s32 zero;
    s32 initialSpeed;
    entity->accelY.whole = gravity;
    zero = 0;
    entity->velX.whole = zero;
    entity->accelX.whole = zero;
    initialSpeed = 0x10000;
    if (Entity_MoveWithCollision(entity, initialSpeed, -0x200000, 0x80000, 8) & 4)
    {
        entity->state = zero;
        entity->subState = 6;
        entity->timer = zero;
        entity->velY.whole = initialSpeed;
        entity->accelY.whole = gravity;
        Anim_StartIndexed(entity, 5, 0, 1);
    }
}

/* Report a hit effect and signal the parent that contact occurred. */
/**
 * @brief Contact (body and attack box): Player_ApplyContactDamage(attack, 0, 0, 0)
 * and set ext.persephone.contactHit on the entity in ENTITY_OWNER (the owner of an attack
 * box).
 */
void Persephone_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage effect;
    effect.attack = entity->attack;
    effect.attributes = 0;
    effect.unused = 0;
    effect.reaction = 0;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
    (ENTITY_OWNER(entity))->ext.persephone.contactHit = 1;
}
