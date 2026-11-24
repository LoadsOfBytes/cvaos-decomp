/*
 * Minotaur: attacks, axe pieces and swing trail, hits, entry callbacks and contact damage.
 * Built without -f2003-patch (the contact callbacks need it off; the rest compiles the same either way).
 */
#include "minotaur.h"
#include "core.h"
#include "structs/ewram.h"
#include "player_level.h"
#include "sound.h"
#include "player_core.h"
#include "graphics.h"
#include "gameplay.h"
#include "syscalls.h"
#include "entity_util.h"
#include "structs/entity_render_flags.h"
#include "axe_armor.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "werejaguar.h"
#include "effects.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "damage_numbers.h"
#include "music.h"
#include "entity_hit.h"
#include "bg_and_collision.h"

void MinotaurAxe_DamagePlayerOnContact(struct EwramData_EntityData *);
void Minotaur_DamagePlayerOnContact(struct EwramData_EntityData *);

/* Probabilistic gate followed by a direction-dependent room obstacle probe. */

s32 Minotaur_CheckAttackChance(struct EwramData_EntityData *entity, s32 mask)
{
    u16 x, y, result;
    if (!entity->ext.minotaur.variant || (RandomNumberGenerator2() & mask) != 0)
        return 1;
    if (!(entity->ext.minotaur.flags & 2))
        return 0;
    if (Target_IsNearEntity(entity, 160, 120))
        return 1;
    x = GetEntityRoomXPositionIntegerWide(entity);
    y = GetEntityRoomYPositionIntegerWide(entity);
    if (entity->flipX)
        result = Collision_GetPushLeft((s16)x + 80, (s16)y);
    else
        result = Collision_GetPushRight((s16)x - 80, (s16)y);
    return (s16)result;
}

/* Ballistic fragment: bounce once against room geometry, then fall away. */
static inline s32 FragmentHitsGround(struct EwramData_EntityData *entity)
{
    s32 x = (s16)entity->posX.part16.integer;
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 y;
    x += (s16)bg->xPos.part16.integer;
    y = entity->posY.part16.integer;
    y += (s16)bg->yPos.part16.integer;
    return Collision_GetGroundOffsetWide(x, y + 4);
}
void MinotaurAxePiece_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        entity->velX.whole = RandomSigned(0x20000) - 0x8000;
        if (entity->flipX)
            entity->velX.whole = -entity->velX.whole;
        entity->velY.whole = -0x10000 - ((RandomNumberGenerator() & 0x1FFF) << 4);
        entity->state++;
        break;
    case 1:
        if (FragmentHitsGround(entity))
        {
            entity->animFrame = (RandomNumberGenerator() & 1) + 64;
            entity->velY.whole = -(entity->velY.whole >> 1);
            entity->state++;
        }
        else
        {
            entity->posX.whole += entity->velX.whole;
            entity->posY.whole += entity->velY.whole;
            entity->velY.whole += 0x3000;
        }
        break;
    case 2:
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += 0x3000;
        break;
    }
    if (!Entity_IsInRect(entity, -16, -16, 272, 192))
        entity->deleteRequested = 1;
}

void Minotaur_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
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
    if ((s16)entity->hp < 100 && !(entity->ext.minotaur.flags & 1))
        entity->ext.minotaur.flags &= ~2;
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->state = 3;
        Anim_StartIndexed(entity, 7, 0, 0);
        entity->ext.minotaur.flags &= ~1;
        if (entity->spawnParam0 != 0)
        {
            Player_SetSelectedTarget(NULL);
            gEwramData->gameState.bossEventFlags |= 0x100;
        }
    }
}

/**
 * @brief B0064 | Affine-rotated companion sprite of the parent (unk_4F8):
 * allocates an affine slot, follows the parent while its frame animScriptIndex is 4
 * and its pose (unk_4F8 - 4) is 0 or 1, swinging on a rotating arc and
 * growing over 7 frames; freed with its slot when the parent id changes.
 */
void MinotaurSwingTrail_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    u16 slot;
    s32 angle;
    s32 rot;
    s32 dx, dy;

    if (entity->state != 0 && ENTITY_INDEX(parent) != ENTITY_PARENT_INDEX(entity))
        entity->state = 2;
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, (void *)sMinotaurGfx, (void *)sMinotaurPalette, 0, (void *)sMinotaurSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        slot = ObjAffine_Alloc(1);
        if ((s16)slot < 0)
        {
            EntityDelete(entity);
            return;
        }
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->scrollWithBg = 1;
        Anim_StartIndexed(entity, 9, 0, 1);
        Anim_Update(entity);
        entity->spriteLayer = parent->spriteLayer;
        entity->state++;
    case 1:
        if (parent->animScriptIndex != 4)
        {
            entity->hidden = 1;
            return;
        }
        if ((u8)(parent->ext.minotaur.animStep - 4) > 1)
        {
            entity->timer = 0;
            entity->hidden = 1;
            entity->ext.minotaurTrail.angle = 0;
            return;
        }
        if (entity->timer > 6)
        {
            entity->hidden = 1;
            return;
        }
        entity->timer++;
        entity->hidden = 0;
        entity->subState = parent->ext.minotaur.animStep - 4;
        entity->flipX = parent->flipX;
        entity->posX.part16.integer = parent->posX.part16.integer;
        entity->posY.part16.integer = parent->posY.part16.integer - 0x10;
        entity->ext.minotaurTrail.angle += -0x400;
        rot = entity->ext.minotaurTrail.angle;
        angle = (rot - 0x6000) & 0xFFFF;
        dx = Sin(angle + 0x4000) * 56;
        dy = Sin(angle) << 6;
        if (entity->flipX)
        {
            dx = -dx;
            rot = -rot;
        }
        entity->posX.whole += dx;
        entity->posY.whole += dy;
        break;
    case 2:
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
        return;
    }
    ObjAffine_Set(rot, 0x14000 - (entity->timer << 11), 0x12000, entity->flipX, 1, entity->affineIndex);
}

/* Collision object attached to a rotating parent, at a radius of 16 pixels. */
void MinotaurAxeHitBox_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    if (entity->state != 0 && *(u8 *)&parent->ext != ENTITY_PARENT_INDEX(entity))
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        Hit_Enable(entity, 8, MinotaurAxe_DamagePlayerOnContact, 0);
        HitBox_SetAttackRaw(entity, 0x1414F6F6);
        entity->hit.flags = (entity->hit.flags & ~2) | 4;
        entity->state++;
        /* fall through */
    case 1:
        {
            s32 angle = parent->ext.minotaurAxe.angle;
            s32 yAngle = angle - 0x4000;
            s32 dx = Sin(angle) << 4;
            s32 dy = Sin(yAngle) << 4;
            entity->posX.whole = parent->posX.whole + dx;
            entity->posY.whole = parent->posY.whole + dy;
            if (parent->state == 1)
                Hit_CheckWithPlayer(&result, entity);
        }
        break;
    }
}

/* Weapon carried by an enemy: follows the parent's animation, falls and breaks. */
static inline struct WeaponFrame *FindFrame(struct WeaponFrame *frame, struct EwramData_EntityData *parent)
{
    if (frame == NULL)
        return NULL;
    {
        u8 key = parent->ext.minotaur.animStep;
        while (frame->frame != -1)
        {
            if (key <= frame->frame)
                return frame;
            frame++;
        }
    }
    return NULL;
}

static inline s32 ScreenY(u16 y) { struct BgLayerState *bg = &gEwramData->bgInfo[1]; s32 b = bg->yPos.part16.integer; return (s16)y - b; }
static inline s32 ScreenX(s32 x) { struct BgLayerState *bg = &gEwramData->bgInfo[1]; return x - bg->xPos.part16.integer; }

#define SPIN(e) (e->ext.minotaurAxe.spin)
#define SPIN_SPEED(e) (e->ext.minotaurAxe.spinSpeed)
#define DROPPED(e) (e->ext.minotaurAxe.dropped)
#define TO_PIXELS(d) ((d) < 0 ? -(-(d) >> 16) : (d) >> 16)

void MinotaurAxe_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    s16 offset[2];
    s32 angle;

    if (DROPPED(entity) == 0)
    {
        if ((s16)parent->hp <= 0)
            DROPPED(entity) = 1;
        if (DROPPED(entity) == 0 && entity->state == 4
            && ENTITY_INDEX(parent) == ENTITY_PARENT_INDEX(entity)
            && (parent->ext.minotaur.flags & 1))
        {
            entity->state = 1;
            PlaySong(0x69);
        }
    }
    switch (entity->state)
    {
    case 0:
    {
        u16 slot;
        if (!Entity_LoadGraphics(entity, sMinotaurGfx, sMinotaurPalette, 0, sMinotaurSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        slot = ObjAffine_Alloc(1);
        if ((s16)slot < 0)
        {
            EntityDelete(entity);
            return;
        }
        if (!(((u8 *)gEwramData)[0x8E] & 0x40))
        {
            struct EwramData_EntityData *hitbox;
            entity->attack += entity->attack >> 2;
            hitbox = Entity_CreateObject(MinotaurAxeHitBox_Update);
            if (hitbox != NULL)
            {
                ENTITY_OWNER(hitbox) = entity;
                ENTITY_PARENT_INDEX(hitbox) = ENTITY_INDEX(entity);
                hitbox->attack = entity->attack;
                hitbox->hp = entity->hp;
            }
        }
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->scrollWithBg = 1;
        Anim_StartIndexed(entity, 8, 0, 1);
        Anim_Update(entity);
        entity->spriteLayer = parent->spriteLayer;
        entity->state++;
    }
    case 1:
        if (ENTITY_INDEX(parent) != ENTITY_PARENT_INDEX(entity))
        {
            entity->state = 6;
            break;
        }
        if (parent->state == 3)
        {
            entity->state = 3;
            break;
        }

        if (!(parent->ext.minotaur.flags & 1))
        {
            entity->state = 2;
            break;
        }
        entity->flipX = parent->flipX;
        {
            s16 *ofs = offset;
            Sprite_GetPartCornerOffset(parent, 0, ofs);
            entity->posX.part16.integer = parent->posX.part16.integer + offset[0];
            entity->posY.part16.integer = ofs[1] + parent->posY.part16.integer;
        }
        {
            struct WeaponFrame *found = FindFrame(sMinotaurAxeFrames[parent->animScriptIndex], parent);
            if (found == NULL)
                break;
            SPIN(entity) = found->angle;
            if (entity->flipX)
                entity->posX.part16.integer -= found->dx;
            else
                entity->posX.part16.integer += found->dx;
            entity->posY.part16.integer += found->dy;
        }
        break;
    case 2:
        PlaySong(0x86);
        entity->velX.whole = -0x8000;
        if (entity->flipX)
            entity->velX.whole = -entity->velX.whole;
        entity->velY.whole = -0x42000;
        entity->accelY.whole = 0x1800;
        SPIN_SPEED(entity) = 0xF200;
        entity->state = 4;
        break;
    case 3:
        entity->velX.part16.integer = 0;
        entity->velY.whole = -0x30000;
        entity->accelY.whole = 0x2000;
        SPIN_SPEED(entity) = 0xF800;
        entity->state = 4;
        break;
    case 4:
    {
        u16 rx, ry;
        s32 dx, dy, hit, i, drop;
        struct EwramData_EntityData *piece;
        s32 graphics;
        SPIN(entity) += SPIN_SPEED(entity);
        EntityAccelerateY(entity, 0x80000);
        EntityMove(entity);
        if (entity->velY.whole <= 0x2FFFF)
            break;
        if (entity->velY.whole > 0x60000 && EntityIsOutsideScreen(entity, 4))
        {
            if (DROPPED(entity) == 0)
                parent->ext.minotaur.flags &= ~2;
            entity->state = 6;
            break;
        }
        angle = (u16)SPIN(entity);
        if ((u32)(angle - 0x9000) > 0x2000)
            break;
        angle = SPIN(entity);
        if (entity->flipX)
            angle = -angle;
        rx = GetEntityRoomXPositionIntegerWide(entity);
        ry = GetEntityRoomYPositionIntegerWide(entity);
        angle -= 0x4000;
        dx = Sin(angle + 0x4000) << 4;
        dy = Sin(angle) << 4;
        rx = (s16)rx + TO_PIXELS(dx);
        ry = (s16)ry + TO_PIXELS(dy);
        hit = Collision_GetPushUp((s16)rx, (s16)ry + 10) << 16;
        if (hit == 0)
            break;
        PlaySong(0x7F);
        graphics = ObjGfx_Find(sCommonObjectGfx);
        i = 0;
        drop = hit;
        goto test;
    loop:
        {
            Sprite_Attach(piece, sCommonObjectSpriteSheet, graphics, 6);
            piece->animFrame = Mod(RandomNumberGenerator() >> 4, 3) + 0x40;
            piece->posX.part16.integer = ScreenX((s16)rx + RandomSigned(4));
            piece->posY.part16.integer = ScreenY(ry) - 8;
            piece->flipX = RandomNumberGenerator() & 1;
            piece->spriteLayer = 0x11;
            piece->scrollWithBg = 1;
            piece->drawFunc = Sprite_DrawEntity;
            i++;
        }
    test:
        if (i <= 3 && (piece = EntityCreateInRange(0x49, 0xCF, MinotaurAxePiece_Update)) != NULL)
            goto loop;
        if (ENTITY_INDEX(parent) == ENTITY_PARENT_INDEX(entity))
            parent->ext.minotaur.flags &= ~2;
        entity->posY.whole += drop;
        entity->state = 5;
        entity->timer = 0xC0;
        break;
    }
    case 5:
        if (EntityIsOutsideScreen(entity, 4))
        {
            entity->state = 6;
            break;
        }
        if (entity->timer == 0x20)
            entity->drawSkip = 1;
        if (entity->drawSkip && !(entity->timer & 3))
        {
            s32 x = entity->posX.whole + (RandomSigned(0x100) << 13);
            s32 y = entity->posY.whole + ({ s32 t = (RandomSigned(0x100) << 13) + 0x100000; t; });
            Effect_SpawnShared(x, y, RandomNumberGenerator() & 1, 8);
        }
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->state = 6;
        break;
    case 6:
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
        return;
    }
    angle = SPIN(entity);
    if (entity->flipX)
        angle = -angle;
    entity->ext.minotaurAxe.angle = angle;
    ObjAffine_Set(angle, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
}

/* Minotaur idle: choose approach or attack using two distance windows. */
void Minotaur_UpdateIdle(struct EwramData_EntityData *entity)
{
    Entity_FaceTarget(entity);
    if (entity->animTimer == 0)
        Entity_FaceTarget(entity);
    if (Target_IsNearEntity(entity, 200, 120) || Enemy_IsDamaged(entity))
    {
        s32 result = Minotaur_CheckAttackChance(entity, 1);
        if (result == 0)
        {
            entity->state = 2;
            entity->stage = result;
            entity->subState = result;
        }
        else if (entity->ext.minotaur.flags & 2)
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
        }
    }
    else if (Target_IsNearEntity(entity, 280, 120) || Enemy_IsDamaged(entity))
    {
        s32 result = Minotaur_CheckAttackChance(entity, 63);
        if (result == 0)
        {
            entity->state = 2;
            entity->stage = result;
            entity->subState = result;
        }
    }
}

/* Minotaur approach animation, movement, then close-range attack. */
void Minotaur_UpdateApproach(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->subState = 1;
        break;
    case 1:
        if (entity->animFlags & 4)
        {
            Anim_StartIndexed(entity, 3, 0, 1);
            entity->subState = 2;
        }
        break;
    case 2:
        Entity_Walk(entity, 0x8000, sMinotaurWalkProfile, 13);
        if (Target_IsNearEntity(entity, 100, 80))
        {
            Anim_StartIndexed(entity, 4, 0, 0);
            entity->subState = 3;
        }
        break;
    case 3:
        if (*(u16 *)&entity->animStep == 3)
            PlaySong(0x86);
        if (entity->animFlags & 4)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->state = 0;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 4:
        break;
    }
}

/* Minotaur attack: windup, decelerating charge, recovery or follow-up. */
void Minotaur_UpdateAttack(struct EwramData_EntityData *entity)
{
    s32 remaining;
    switch (entity->subState)
    {
    case 0:
        {
            s32 flag = entity->ext.minotaur.flags & 2;
            if (flag == 0)
            {
                Anim_StartIndexed(entity, 5, 0, 1);
                entity->subState = 3;
                entity->timer = flag;
            }
            else
            {
                entity->ext.minotaur.flags &= ~1;
                Anim_StartIndexed(entity, 7, 0, 0);
                entity->subState = 1;
            }
        }
        break;
    case 1:
        if (entity->animFlags & 4)
        {
            entity->subState = 2;
            entity->timer = 8;
        }
        break;
    case 2:
        remaining = entity->timer;
        if (remaining == 0)
        {
            Anim_StartIndexed(entity, 5, 0, 1);
            entity->subState = 3;
            Afterimage_Create(entity, 40, 12, 1);
            Afterimage_Create(entity, 40, 6, 0);
        }
        else
            goto decrementTimer;
        break;
    case 3:
        Entity_Walk(entity, 0x34000 - entity->timer * 0x1400, sMinotaurWalkProfile, 12);
        if (++entity->timer == 24)
            Anim_StartIndexed(entity, 6, 0, 0);
        if (entity->timer > 40)
        {
            if (!(entity->ext.minotaur.flags & 2))
            {
                entity->subState = 4;
                entity->timer = 48;
            }
            else
            {
                entity->subState = 5;
                Anim_StartIndexed(entity, 4, 0, 0);
                entity->ext.minotaur.flags |= 1;
            }
        }
        break;
    case 4:
        {
            s32 timer;
            remaining = entity->timer;
            timer = (u8)remaining;
            if (timer != 0)
            {
decrementTimer:
                entity->timer = remaining - 1;
            }
            else
            {
                Anim_StartIndexed(entity, 1, 0, 1);
                entity->state = timer;
                entity->stage = timer;
                entity->subState = timer;
            }
        }
        break;
    case 5:
        if (*(u16 *)&entity->animStep == 3)
            PlaySong(0x86);
        if (entity->animFlags & 4)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->state = 0;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    }
}

/* Minotaur initialisation shared by Minotaur (variant 0) and Red Minotaur
 * (variant 1).  Loads graphics for the variant,
 * stores the variant in unk_4F9, creates the two helper entities MinotaurAxe_Update
 * and MinotaurSwingTrail_Update (any failure deletes the Minotaur), starts animation 1 and
 * either enters state 4 (bestiaryPreview set) or registers the hit/death callbacks
 * and enters state 0. */

/* Two child entities inherit the parent pointer, low unk_4F4 byte, attack
 * and the hp value of the Minotaur. */
static inline void AttachChild(struct EwramData_EntityData *child, struct EwramData_EntityData *entity)
{
    ENTITY_OWNER(child) = entity;
    ENTITY_PARENT_INDEX(child) = ENTITY_INDEX(entity);
    child->attack = entity->attack;
    /* Signed 16-bit view of hp; the u16 field allocates registers differently. */
    *(s16 *)&child->hp = *(s16 *)&entity->hp;
}

void Minotaur_Init(struct EwramData_EntityData *entity, s32 variant)
{
    struct EwramData_EntityData *child;
    if (!Entity_LoadGraphics(entity, sMinotaurGfx, sMinotaurPalette, variant, sMinotaurSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    entity->ext.minotaur.variant = variant;
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x13;
    child = Entity_CreateObject(MinotaurAxe_Update);
    if (child == NULL)
    {
        EntityDelete(entity);
        return;
    }
    AttachChild(child, entity);
    child = Entity_CreateObject(MinotaurSwingTrail_Update);
    if (child == NULL)
    {
        EntityDelete(entity);
        return;
    }
    AttachChild(child, entity);
    entity->ext.minotaur.flags |= 3;
    Anim_StartIndexed(entity, 1, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        return;
    }
    Entity_EnableBoxes(entity, Minotaur_DamagePlayerOnContact, Minotaur_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->posY.part16.integer++;
    entity->scrollWithBg = 1;
    entity->state = 0;
    if (entity->spawnParam0 != 0)
        Player_SetSelectedTarget(entity);
}

/* Minotaur update: pause handling, state dispatch, contact and animation. */

void Minotaur_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    Entity_HideAboveY(entity, -16);
    if (!EntityIsOutsideScreen(entity, 6))
        Player_TrackEntity(entity);
    if (entity->state != 3)
        Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity) && entity->state != 3)
    {
        Hit_CheckWithPlayer(&result, entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        if (entity->spawnParam0 != 0 && !gEwramData->gameState.linkPlay)
            Music_SetCurrent(16);
        Minotaur_UpdateIdle(entity);
        break;
    case 1:
        Minotaur_UpdateApproach(entity);
        break;
    case 2:
        Minotaur_UpdateAttack(entity);
        break;
    case 3:
        Werejaguar_UpdateDeath(entity);
        break;
    case 4:
        Anim_Update(entity);
        return;
    }
    if (entity->state != 3)
        Hit_CheckWithPlayer(&result, entity);
    entity->ext.minotaur.animStep = entity->animStep;
    Entity_AnimateWithBoxes(entity);
}

/* Minotaur / Red Minotaur helpers 0x080B0CCC..0x080B0D5B. */
/**
 * @brief B0CCC | Player_ApplyContactDamage with (attack, 0, 0, 0) at the entity position.
 */
void Minotaur_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief B0CFC | Player_ApplyContactDamage with (attack, 1, 0, 1) at the entity position
 * (same as Buer_DamagePlayerOnContact).
 */
void MinotaurAxe_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 1;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief B0D2C | Minotaur create: Minotaur_Init variant 0.
 */
void EnemyMinotaurCreate(struct EwramData_EntityData *entity)
{
    Minotaur_Init(entity, 0);
}

/**
 * @brief B0D38 | Minotaur update.
 */
void EnemyMinotaurUpdate(struct EwramData_EntityData *entity)
{
    Minotaur_Update(entity);
}

/**
 * @brief B0D44 | Red Minotaur create: variant 1.
 */
void EnemyRedMinotaurCreate(struct EwramData_EntityData *entity)
{
    Minotaur_Init(entity, 1);
}

/**
 * @brief B0D50 | Red Minotaur update: same as Minotaur.
 */
void EnemyRedMinotaurUpdate(struct EwramData_EntityData *entity)
{
    Minotaur_Update(entity);
}

/* Axe Armor 0x080B0D5C..0x080B136F; only matches without -f2003-patch. */

/**
 * @brief Initialize and update the spinning thrown axe; release its affine slot on destruction.
 */
void AxeArmorAxe_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    u16 slot;

    if (entity->state != 0)
    {
        Hit_CheckWithPlayer(&result, entity);
        if ((s16)entity->hp <= 0)
        {
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0x80000, 0x80000, entity->spriteLayer - 1);
            entity->state = 2;
        }
        else if (gEwramData->gameState.timeStopTimer != 0)
        {
            return;
        }
    }
    switch (entity->state)
    {
        case 0:
            if (!Entity_LoadGraphics(entity, sAxeArmorGfx, sAxeArmorPalette, 0, sAxeArmorSpriteSheet))
            {
                EntityDelete(entity);
                return;
            }
            slot = ObjAffine_Alloc(1);
            if ((s16)slot < 0)
            {
                EntityDelete(entity);
                return;
            }
            entity->affineIndex = slot;
            entity->affine = 1;
            Anim_StartIndexed(entity, 3, 0, 0);
            Anim_Update(entity);
            entity->spriteLayer = 0x14;
            entity->scrollWithBg = 1;
            entity->velX.whole = -0x28000;
            entity->accelX.whole = 0x800;
            entity->ext.axeArmorAxe.spin = 0x800;
            if (entity->stage == 1)
            {
                entity->posY.part16.integer -= 20;
                entity->ext.axeArmorAxe.spin = -entity->ext.axeArmorAxe.spin;
            }
            if (entity->flipX)
            {
                entity->velX.whole = -entity->velX.whole;
                entity->accelX.whole = -entity->accelX.whole;
                entity->ext.axeArmorAxe.spin = -entity->ext.axeArmorAxe.spin;
            }
            entity->state = 1;
            PlaySong(0x86);
            entity->hp = 1;
            Hit_Enable(entity, 10, Enemy_OnContactMask1, AxeArmorAxe_ClearHealth);
            HitBox_SetAttackRaw(entity, 0x1010F8F8);
            HitBox_SetBodyRaw(entity, 0x1010F8F8);
            break;
        case 1:
            EntityAccelerateAndMoveX(entity, 0x28000);
            break;
        case 2:
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
    }
    entity->ext.axeArmorAxe.angle += entity->ext.axeArmorAxe.spin;
    ObjAffine_Set(entity->ext.axeArmorAxe.angle, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
    if (EntityIsOutsideScreen(entity, 4))
        entity->state = 2;
}

/**
 * @brief Walk at animation-dependent speed; turn or enter attack state 1 at the designated animation step.
 */
void AxeArmor_UpdateWalking(struct EwramData_EntityData *entity)
{
    u8 frame;
    s32 speed;
    s32 result;
    s32 attack;
    s32 distance;

    if (entity->subState != 0)
        return;
    // animStep and animTimer read together: frame 0x11 and animTimer 0
    if (*(u16 *)&entity->animStep == 0x11 && entity->spawnParam0 != 0
        && entity->timer++ > entity->spawnParam0)
    {
        entity->timer = entity->subState;
        entity->flipX ^= 1;
    }
    frame = entity->animStep % 9;
    speed = frame * 0x2800 + 0x1000;
    if (frame > 6)
        speed = 0;
    result = Entity_Walk(entity, speed, sAxeArmorWalkProfile, 20);
    if (entity->spawnParam0 == 0 && (result & 0x1B) != 8)
        entity->flipX ^= 1;
    if (entity->animTimer == 0)
    {
        if (entity->animStep != 0x11)
            return;
        // attack = target in front && distance <= 99, spelled out for the
        // original flag order
        if ((entity->flipX && (s8)Entity_GetTargetDirection(entity) == 1)
            || (!entity->flipX && (s8)Entity_GetTargetDirection(entity) == -1))
        {
            distance = Entity_GetTargetDistance(entity);
            attack = 1;
            if (distance <= 99)
                goto test;
        }
        attack = 0;
    test:
        if (attack)
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
        }
    }
}
