/*
 * Bomber Armor and Rock Armor: bodies, bombs, rocks and rock pieces.
 */
#include "core.h"
#include "graphics.h"
#include "player_core.h"
#include "gameplay.h"
#include "entity_util.h"
#include "structs/ewram.h"
#include "sound.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "obj_palette.h"
#include "effects.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "entity_hit.h"
#include "bg_and_collision.h"
#include "constants/hit_flags.h"
#include "constants/songs.h"


void BomberArmorBomb_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void ArmorProjectile_OnHit(struct EwramData_EntityData *entity);
void BomberArmorBombTrail_Update(struct EwramData_EntityData *entity);
void RockArmorRock_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void BomberRockArmor_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

/**
 * @brief B243C | Bouncing boss projectile (like RockArmorRock_Update): arcs towards
 * the player leaving BomberArmorBombTrail_Update trails every fourth frame, halves and
 * reverses its arc on wall hits, explodes (Effect_SpawnParticleTemplateList) 16 frames after
 * landing or when its HP runs out. Frozen while timeStopTimer is set.
 */
void BomberArmorBomb_Update(struct EwramData_EntityData *entity)
{
    struct HitResult hit;
    struct EwramData_EntityData *trail;
    s32 result;

    Hit_CheckWithPlayer(&hit, entity);
    if (entity->state != 0)
    {
        if ((s16)entity->hp == 0 && entity->state != 3)
        {
            Effect_SpawnParticleTemplateList((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer, 1);
            entity->state = 3;
        }
        if (entity->state != 3 && gEwramData->gameState.timeStopTimer != 0)
            return;
    }
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, (void *)sBomberArmorRockArmorGfx, (void *)sBomberArmorRockArmorPalette, 0, (void *)sBomberArmorSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        Anim_StartIndexed(entity, 3, 0, 1);
        entity->spriteLayer = 0x13;
        Hit_Enable(entity, 8, BomberArmorBomb_DamagePlayerOnContact, ArmorProjectile_OnHit);
        HitBox_SetAttackRaw(entity, 0x3030E8E8);
        HitBox_SetBodyRaw(entity, 0x1010F8F8);
        entity->hit.flags |= HIT_NO_ATTACK;
        entity->ext.bomberArmorBomb.speed = -0x2000 - (Entity_GetTargetDistance(entity) << 10);
        entity->posY.whole += -0x3A0000;
        if (entity->flipX)
            entity->ext.bomberArmorBomb.speed = -entity->ext.bomberArmorBomb.speed;
        entity->velY.whole = -0x28000;
        entity->accelY.whole = 0x2000;
        entity->attack += entity->attack >> 2;
        entity->scrollWithBg = 1;
        entity->state = 1;
    case 1:
        if ((entity->timer & 3) == 0)
        {
            trail = EntityCreateInRange(ENTITY_SLOT_EFFECT_FIRST, ENTITY_SLOT_EFFECT_LAST, BomberArmorBombTrail_Update);
            if (trail != NULL)
            {
                trail->posX.whole = entity->posX.whole;
                trail->posY.whole = entity->posY.whole - 0x80000;
            }
        }
        entity->timer++;
        result = Entity_Walk(entity, entity->ext.bomberArmorBomb.speed, (void *)sBomberArmorBombWalkProfile, 2);
        if (result & 3)
            entity->ext.bomberArmorBomb.speed = -(s32)entity->ext.bomberArmorBomb.speed / 2;
        if (result & 8)
        {
            entity->state = 2;
            entity->timer = 0x10;
        }
        break;
    case 2:
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->hit.flags &= ~HIT_NO_ATTACK;
        Effect_SpawnParticleTemplateList((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer, 0);
        entity->state = 3;
        break;
    case 3:
        PlaySong(SE_PROJECTILE_IMPACT);
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 3))
    {
        EntityDelete(entity);
        return;
    }
    Anim_Update(entity);
}

/**
 * @brief B264C | Debris piece (file 0x080B243C): graphics sBomberArmorRockArmorGfx /
 * sBomberArmorRockArmorPalette / sRockArmorSpriteSheet (palette 1, deleted when they cannot be
 * loaded), random animation 4..6, random velX -0x4000..0xC000 mirrored by
 * the facing and velY -1..-2 px; falls with gravity 0x4000 until it hits
 * the floor (Collision_GetGroundOffset 4 px below), bounces once at half speed and
 * keeps falling; finished outside the screen margin.
 */
void RockArmorRockPiece_Update(struct EwramData_EntityData *entity)
{
    s32 r, vx, x;
    struct BgLayerState *bg;

    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sBomberArmorRockArmorGfx, sBomberArmorRockArmorPalette, 1, sRockArmorSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        r = RandomNumberGenerator() & 3;
        if (r > 2)
            r = 2;
        Anim_StartIndexed(entity, r + 4, 0, 1);
        Anim_Update(entity);
        vx = RandomSigned(0x10000) - 0x4000;
        entity->velX.whole = vx;
        if (entity->flipX)
            entity->velX.whole = -vx;
        entity->velY.whole = -0x10000 - ((RandomNumberGenerator() & 0x1FFF) << 4);
        entity->state++;
        break;
    case 1:
        x = (s16)entity->posX.part16.integer;
        bg = &gEwramData->bgInfo[1];
        if (Collision_GetGroundOffsetWide(x + (s16)bg->xPos.part16.integer,
                         (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer + 4))
        {
            entity->velY.whole = -(entity->velY.whole >> 1);
            entity->state++;
            break;
        }
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += 0x4000;
        break;
    case 2:
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += 0x4000;
        break;
    }
    if (!Entity_IsInRect(entity, -16, -16, 0x110, 0xC0))
        entity->deleteRequested = 1;
}

/**
 * @brief B2798 | Thrown boss projectile: load its graphics (deleted when
 * that fails), hit box 0x2020F0F0, arc towards the player via Entity_Walk;
 * on landing or when its HP runs out burst into eight RockArmorRockPiece_Update pieces.
 * Frozen while timeStopTimer is set; deleted when far outside the screen.
 */
void RockArmorRock_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    s32 i;
    struct EwramData_EntityData *piece;
    s32 box;

    Hit_CheckWithPlayer(&result, entity);
    if (entity->state != 0)
    {
        if ((s16)entity->hp == 0 && entity->state != 2)
            entity->state = 2;
        if (entity->state != 2 && gEwramData->gameState.timeStopTimer != 0)
            return;
    }
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, (void *)sBomberArmorRockArmorGfx, (void *)sBomberArmorRockArmorPalette, 1, (void *)sRockArmorSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        Anim_StartIndexed(entity, 3, 0, 1);
        Anim_Update(entity);
        entity->spriteLayer = 0x13;
        Hit_Enable(entity, 10, RockArmorRock_DamagePlayerOnContact, ArmorProjectile_OnHit);
        box = 0x2020F0F0;
        HitBox_SetAttackRaw(entity, box);
        HitBox_SetBodyRaw(entity, box);
        entity->ext.rockArmorRock.speed = -0x2000 - (Entity_GetTargetDistance(entity) << 10);
        entity->posY.whole += -0x3A0000;
        if (entity->flipX)
            entity->ext.rockArmorRock.speed = -entity->ext.rockArmorRock.speed;
        entity->velY.whole = -0x20000;
        entity->accelY.whole = 0x2000;
        entity->attack += entity->attack >> 2;
        entity->scrollWithBg = 1;
        entity->state = 1;
    case 1:
        if (Entity_Walk(entity, entity->ext.rockArmorRock.speed, (void *)sBomberArmorBombWalkProfile, 2))
            entity->state = 2;
        break;
    case 2:
        PlaySong(0x80);
        for (i = 0; i <= 7; i++)
        {
            piece = EntityCreateInRange(ENTITY_SLOT_EFFECT_FIRST, ENTITY_SLOT_EFFECT_LAST, RockArmorRockPiece_Update);
            if (piece == NULL)
                break;
            piece->posX.whole = entity->posX.whole + RandomSigned(0x100000);
            piece->posY.whole = entity->posY.whole + RandomSigned(0x100000) - 0x80000;
            piece->flipX = RandomNumberGenerator() & 1;
            piece->spriteLayer = 0x12;
            piece->scrollWithBg = 1;
        }
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 3))
        EntityDelete(entity);
}

static inline s32 CanAttack(struct EwramData_EntityData *e, s32 range)
{
    s32 result;
    if (((e->flipX && (s8)Entity_GetTargetDirection(e) >= 0) || (!e->flipX && (s8)Entity_GetTargetDirection(e) <= 0))
        && Entity_GetTargetDistance(e) <= range)
        result = 1;
    else
        result = 0;
    return result;
}

static inline s32 Speed(s32 phase, s32 base) { return (phase << 14) + base; }
void BomberRockArmor_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 state = entity->subState;
    if (state == 0)
    {
        u8 phase;
        s32 speed;
        s32 collision;
        if (*(u16 *)&entity->animStep == 9 && entity->spawnParam0 != 0)
        {
            if (entity->timer++ > entity->spawnParam0)
            {
                entity->timer = state;
                entity->flipX ^= 1;
            }
        }
        phase = (u32)entity->animStep % 5;
        speed = Speed(phase, 0x2000);
        if (phase > 3) speed = 0;
        collision = Entity_Walk(entity, speed, sBomberRockArmorWalkProfile, 0x14);
        if (entity->spawnParam0 == 0 && (collision & 0x1B) != 8)
            entity->flipX ^= 1;
        if (entity->animTimer != 0) return;
        if (entity->animStep == 9 && CanAttack(entity, 99))
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
        }
    }
}

/**
 * @brief Attack state machine (state 1): animation 1 and at step 7 throw a
 * bouncing bomb (BomberArmorBomb_Update, sound 0x88); back to state 0 when
 * the animation ends.
 */
void BomberArmor_UpdateAttack(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->subState = 1;
        break;
    case 1:
        if (*(u16 *)&entity->animStep == 7)
        {
            struct EwramData_EntityData *child;
            PlaySong(0x88);
            child = Entity_CreateObject(BomberArmorBomb_Update);
            if (child != NULL)
            {
                ENTITY_INDEX(child) = ENTITY_INDEX(entity);
                ENTITY_OWNER(child) = entity;
                child->posX.whole = entity->posX.whole;
                child->posY.whole = entity->posY.whole;
                child->flipX = entity->flipX;
                child->hp = entity->hp;
                child->attack = entity->attack;
            }
        }
        if (entity->animFlags & 4)
        {
            if (CanAttack(entity, 99))
            {
                Anim_StartIndexed(entity, 0, 0, 0);
                entity->subState = 2;
            }
            else entity->subState = 3;
        }
        break;
    case 2:
        {
            u8 phase = (u32)entity->animStep % 5;
            s32 speed = Speed(phase, 0x2000);
            if (phase > 3) speed = 0;
            Entity_Walk(entity, -speed, sBomberRockArmorWalkProfile, 0x14);
            if (entity->animFlags & 4) entity->subState = 0;
        }
        break;
    case 3:
        entity->state = 0;
        entity->stage = 0;
        entity->subState = 0;
        Anim_StartIndexed(entity, 0, 0, 1);
        break;
    }
}

void RockArmor_UpdateAttack(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->subState = 1;
        break;
    case 1:
        if (*(u16 *)&entity->animStep == 7)
        {
            struct EwramData_EntityData *child;
            PlaySong(0x88);
            child = Entity_CreateObject(RockArmorRock_Update);
            if (child != NULL)
            {
                ENTITY_INDEX(child) = ENTITY_INDEX(entity);
                ENTITY_OWNER(child) = entity;
                child->posX.whole = entity->posX.whole;
                child->posY.whole = entity->posY.whole;
                child->flipX = entity->flipX;
                child->hp = entity->hp;
                child->attack = entity->attack;
            }
        }
        if (entity->animFlags & 4)
        {
            if (CanAttack(entity, 99))
                entity->subState = 0;
            else entity->subState = 2;
        }
        break;
    case 2:
        entity->state = 0;
        entity->stage = 0;
        entity->subState = 0;
        Anim_StartIndexed(entity, 0, 0, 1);
        break;
    }
}

s32 BomberRockArmor_UpdateDeath(struct EwramData_EntityData *entity)
{
    s32 state = entity->subState;
    switch (state)
    {
    case 0:
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->subState = 1;
        entity->velY.whole = -0x280000;
        entity->velX.whole = state;
        PlaySong(0x6F);
        break;
    case 1:
        {
            s32 x, y;
            entity->timer++;
            y = entity->posY.whole + entity->velY.whole;
            entity->velY.whole += 0x8000;
            entity->velX.whole += 0x5000;
            if (!(entity->timer & 3))
            {
                struct EwramData_EntityData *effect;
                s32 offset;
                x = Sin(entity->velX.whole + 0x4000);
                offset = RandomSigned(0x40000);
                x <<= 4; x = entity->posX.whole + x + offset;
                effect = Effect_SpawnShared(x, y, 0xF, 7);
                if (effect != NULL)
                {
                    effect->velY.whole = 0x8000;
                    effect->accelY.whole = -0x2000;
                }
            }
            if (!(entity->timer & 7))
            {
                PlaySong(0x72);
                x = entity->posX.whole + (RandomSigned(0x10) << 16);
                y += RandomSigned(4) << 16;
                Effect_SpawnShared(x, y, 0x10, 6);
            }
            if (entity->animFlags & 4)
                return 1;
        }
        break;
    }
    return 0;
}

void EnemyBomberArmorCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sBomberArmorRockArmorGfx, sBomberArmorRockArmorPalette, 0, sBomberArmorSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x13;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        return;
    }
    Hit_EnableContact(entity, 8, Enemy_OnContact, BomberRockArmor_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->posY.part16.integer++;
    entity->scrollWithBg = 1;
    entity->state = 0;
}

void EnemyRockArmorCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sBomberArmorRockArmorGfx, sBomberArmorRockArmorPalette, 1, sRockArmorSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    entity->spriteLayer = 0x13;
    Enemy_ResetFromTemplate(entity);
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        return;
    }
    Hit_EnableContact(entity, 8, Enemy_OnContact, BomberRockArmor_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->scrollWithBg = 1;
    entity->state = 0;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   BomberRockArmor_OnHit = AxeArmor_OnHit */

/**
 * @brief B2F10 | Same as AxeArmor_OnHit.
 */
void BomberRockArmor_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    Entity_FaceTarget(entity);
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/**
 * @brief B2F5C | Clear the HP.
 */
void ArmorProjectile_OnHit(struct EwramData_EntityData *entity)
{
    entity->hp = 0;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   BomberArmorBomb_DamagePlayerOnContact = HeadStackProjectile_DamagePlayerOnContact
 *   RockArmorRock_DamagePlayerOnContact = DeathPillar_DamagePlayerOnContact */

/**
 * @brief B2F64 | Same as HeadStackProjectile_DamagePlayerOnContact.
 */
void BomberArmorBomb_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 2;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief B2F94 | Same as DeathPillar_DamagePlayerOnContact.
 */
void RockArmorRock_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/* Bomber Armor effect (Bomber Armor graphics). */

/**
 * @brief B2FC4 | One-shot effect: state 0 loads graphics (deleting the entity
 * on failure), starts animation 4 and sets spriteLayer = 0x12 and scrollWithBg.
 * Every frame deleteRequested is set once the animation returns 3.
 */
void BomberArmorBombTrail_Update(struct EwramData_EntityData *entity)
{
    if (entity->state == 0)
    {
        if (!Entity_LoadGraphics(entity, sBomberArmorRockArmorGfx, sBomberArmorRockArmorPalette, 0, sBomberArmorSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        Anim_StartIndexed(entity, 4, 0, 0);
        entity->spriteLayer = 0x12;
        entity->scrollWithBg = 1;
        entity->state++;
    }
    if (Anim_Update(entity) == 3)
        entity->deleteRequested = 1;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   EnemyBomberArmorUpdate = EnemyAxeArmorUpdate */

/**
 * @brief B3040 | Same as EnemyAxeArmorUpdate.
 */
void EnemyBomberArmorUpdate(struct EwramData_EntityData *entity)
{
    if (Enemy_PreUpdate(entity))
        return;
    switch (entity->state)
    {
        case 0:
            BomberRockArmor_UpdateWalk(entity);
            break;
        case 1:
            BomberArmor_UpdateAttack(entity);
            break;
        case 2:
            if (BomberRockArmor_UpdateDeath(entity))
            {
                EntityDelete(entity);
                return;
            }
            break;
        case 3:
            Anim_Update(entity);
            return;
    }
    Enemy_CheckHitPlayer(entity);
    Entity_AnimateWithBoxes(entity);
}

void EnemyRockArmorUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    if (Enemy_PreUpdate(entity)) return;
    switch (entity->state)
    {
    case 0: BomberRockArmor_UpdateWalk(entity); break;
    case 1: RockArmor_UpdateAttack(entity); break;
    case 2:
        if (BomberRockArmor_UpdateDeath(entity))
        {
            EntityDelete(entity);
            return;
        }
        break;
    case 3: Anim_Update(entity); return;
    }
    if (entity->state != 2) Hit_CheckWithPlayer(&result, entity);
    Entity_AnimateWithBoxes(entity);
}
