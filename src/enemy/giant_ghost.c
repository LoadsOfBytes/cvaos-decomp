/*
 * Giant Ghost: body, aura, hands, aura trail and fragments.
 */
#include "core.h"
#include "graphics.h"
#include "gameplay.h"
#include "structs/ewram.h"
#include "sound.h"
#include "syscalls.h"
#include "player_core.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "palette.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "damage_numbers.h"
#include "hitbox.h"
#include "constants/songs.h"

void GiantGhost_UpdateMove(struct EwramData_EntityData *);
void GiantGhost_UpdateAttack(struct EwramData_EntityData *);
void GiantGhost_SpawnFragment(s32, s32, s32, s32, s32, s32);
u8 GiantGhost_LoadGraphics(struct EwramData_EntityData *entity);
void GiantGhostAura_Update(struct EwramData_EntityData *entity);
void GiantGhostHandA_Update(struct EwramData_EntityData *entity);
void GiantGhostHandB_Update(struct EwramData_EntityData *entity);
void GiantGhost_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void GiantGhost_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *other, u16 *hit);
void GiantGhost_SpawnAuraTrail(struct EwramData_EntityData *, s32, s32);
void GiantGhostAuraTrail_Update(struct EwramData_EntityData *);

/* States of GiantGhost (entity->state), named after the update function the dispatcher calls. */
enum {
    GIANT_GHOST_STATE_MOVE = 0,
    GIANT_GHOST_STATE_ATTACK = 1,
    GIANT_GHOST_STATE_ANIM_ONLY = 3,
};

/* Dispatch Giant Ghost states and emit particles during its death fade. */
void EnemyGiantGhostUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    if (Enemy_PreUpdate(entity)) return;
    switch (entity->state)
    {
    case GIANT_GHOST_STATE_MOVE: GiantGhost_UpdateMove(entity); break;
    case GIANT_GHOST_STATE_ATTACK: GiantGhost_UpdateAttack(entity); break;
    case 2:
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0) entity->deleteRequested = 1;
        if (entity->timer > 40 && (entity->timer & 7) == 1)
        {
            s32 x = (RandomNumberGenerator() & 0x1FFFFF) - 0x100000;
            s32 y = -(RandomNumberGenerator() & 0x1FFFFF);
            s32 vx = (RandomNumberGenerator() & 0x7FFF) - 0x4000;
            s32 type = RandomNumberGenerator() & 1;
            GiantGhost_SpawnFragment(entity->posX.whole + x, entity->posY.whole + y, vx, 0, entity->spriteLayer - 1, type);
        }
        break;
    case GIANT_GHOST_STATE_ANIM_ONLY:
        Anim_Update(entity);
        return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}

/**
 * @brief 7D12C | Giant Ghost create: three parts Entity_CreateObject
 * (GiantGhostAura_Update / GiantGhostHandA_Update / GiantGhostHandB_Update, animations 6 / 8 / 7)
 * sharing the graphics sGiantGhostSpriteSheet and the palette, the first one in
 * front with its own palette (ObjAffine_Alloc) and blending; frozen time
 * keeps the parts 32 px left and 40 px up and gives state 3; otherwise box
 * slot 8 (Hit_EnableContact when Anim_GetMaxBoxCount > 1), state 0 and GiantGhost_ResetAnimation.
 * Any failed allocation deletes it.
 */
void EnemyGiantGhostCreate(struct EwramData_EntityData *entity)
{
    s32 i;
    struct EwramData_EntityData *part;
    u16 palette;

    if ((u8)GiantGhost_LoadGraphics(entity) == 0)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    for (i = 0; i <= 2; i++)
    {
        part = NULL;
        switch (i)
        {
        case 0:
            part = Entity_CreateObject(GiantGhostAura_Update);
            break;
        case 1:
            part = Entity_CreateObject(GiantGhostHandA_Update);
            break;
        case 2:
            part = Entity_CreateObject(GiantGhostHandB_Update);
            break;
        }
        if (part != NULL)
        {
        if (i == 0)
            part->spriteLayer = entity->spriteLayer + 1;
        else
            part->spriteLayer = entity->spriteLayer - 1;
        Sprite_Attach(part, sGiantGhostSpriteSheet, ObjGfx_Find(sGiantGhostGfx), entity->basePalette);
        part->enemyId = entity->enemyId;
        part->basePalette = entity->basePalette;
        part->scrollWithBg = 0;
        part->drawFunc = Sprite_DrawEntity;
        ENTITY_OWNER(part) = entity;
        part->ext.giantGhostHand.index = i;
        part->posX.whole = entity->posX.whole;
        part->posY.whole = entity->posY.whole;
        switch (i)
        {
        case 0:
            Anim_StartIndexed(part, 6, 0, 1);
            break;
        case 1:
            Anim_StartIndexed(part, 8, 0, 1);
            break;
        case 2:
            Anim_StartIndexed(part, 7, 0, 1);
            break;
        }
        Anim_Update(part);
        if (i == 0)
        {
            palette = ObjAffine_Alloc(1);
            if ((s16)palette < 0)
            {
                EntityDelete(entity);
                EntityDelete(part);
                return;
            }
            part->affineIndex = palette;
            part->affine = 1;
            part->doubleSize = 1;
            part->objMode = 1;
        }
        if (gEwramData->gameState.bestiaryPreview)
        {
            part->ext.giantGhostHand.pointX = part->posX.whole - 0x200000;
            part->ext.giantGhostHand.pointY = part->posY.whole - 0x280000;
            part->ext.giantGhostHand.swayDir = 0;
        }
        continue;
        }
        EntityDelete(entity);
        return;
    }
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = GIANT_GHOST_STATE_ANIM_ONLY;
        return;
    }
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, GiantGhost_DamagePlayerOnContact, GiantGhost_OnHit);
    else
        Hit_Enable(entity, 8, GiantGhost_DamagePlayerOnContact, GiantGhost_OnHit);
    Entity_UpdateBoxes(entity);
    entity->state = GIANT_GHOST_STATE_MOVE;
    GiantGhost_ResetAnimation(entity);
}

/* Flying enemy that circles the player: approach / coast / brake, and after
 * being hit (unk_505) the same three phases away from the player. */

#define SPEED(e) (e->ext.giantGhost.speed)
#define HIT(e) (e->ext.giantGhost.hitPlayer)

/* Aim the velocity at a point 0x30 px above the player.  `px` must be a
 * block-local variable: it lives inside one basic block, so local-alloc
 * gives it r4 before the entity pointer is allocated (entity ends up in r5). */
#define AIM_AT_PLAYER(e)                                                                         \
    do {                                                                                         \
        s32 px = Target_GetX(e);                                                                \
        px += bg->xPos.whole;                                                                    \
        Entity_AimVelocityAtPoint(e, px, Target_GetY(e) + bg->yPos.whole - 0x300000, SPEED(e));              \
    } while (0)

/**
 * @brief 7D33C | Movement state machine (subState): 0 choose, 1 accelerate
 * towards the player, 2 coast, 3 brake; 10-13 the same sequence flying away
 * after a hit.  The sprite is hidden while it is vertically off screen.
 */
void GiantGhost_UpdateMove(struct EwramData_EntityData *entity)
{
    s32 hit1;
    s32 hit3;
    s32 hit13;
    s32 x;
    struct BgLayerState *bg = gEwramData->bgInfo + 1;

    if ((u16)(entity->posY.part16.integer + 0x1C) > 0xDC)
        entity->hidden = 1;
    else
        entity->hidden = 0;
    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        x = RandomNumberGenerator() % 3;
        if (x == 0 && Target_IsInRect((s16)(entity->posX.part16.integer - 0x50), (s16)(entity->posY.part16.integer - 0x50), 0xA0, 0xA0))
        {
            entity->state = GIANT_GHOST_STATE_ATTACK;
            entity->subState = x;
            SPEED(entity) = x;
            Anim_StartIndexed(entity, 0, 0, 0);
        }
        else
        {
            entity->timer = 0;
            entity->subState++;
            HIT(entity) = 0;
            Anim_StartIndexed(entity, 3, 0, 0);
        }
        break;
    case 1:
        hit1 = (s8)HIT(entity);
        if (hit1)
        {
            entity->subState = 10;
            HIT(entity) = 0;
            Anim_StartIndexed(entity, 2, 0, 1);
            break;
        }
        if (SPEED(entity) < 0xC000)
            SPEED(entity) += 0x1000;
        AIM_AT_PLAYER(entity);
        EntityMove(entity);
        if (++entity->timer > 0x13)
        {
            entity->subState++;
            entity->timer = hit1;
        }
        break;
    case 2:
        x = (s8)HIT(entity);
        if (x)
        {
            entity->subState = 10;
            HIT(entity) = 0;
            Anim_StartIndexed(entity, 2, 0, 1);
            break;
        }
        EntityMove(entity);
        if (++entity->timer > 0x13)
        {
            entity->subState++;
            entity->timer = x;
            Anim_StartIndexed(entity, 0, 0, 1);
        }
        break;
    case 3:
        hit3 = (s8)HIT(entity);
        if (hit3)
        {
            entity->subState = 10;
            HIT(entity) = 0;
            Anim_StartIndexed(entity, 2, 0, 1);
            break;
        }
        if (SPEED(entity) > 0x1000)
            SPEED(entity) -= 0x800;
        AIM_AT_PLAYER(entity);
        EntityMove(entity);
        if (++entity->timer > 0x1D)
        {
            entity->subState = hit3;
            entity->timer = hit3;
        }
        break;
    case 10:
        SPEED(entity) = 0;
        entity->subState++;
        entity->timer = 0;
    case 11:
        if (SPEED(entity) < 0xC000)
            SPEED(entity) += 0x1000;
        AIM_AT_PLAYER(entity);
        entity->velX.whole = -entity->velX.whole;
        entity->velY.whole = -SPEED(entity);
        EntityMove(entity);
        if (++entity->timer > 0x13)
        {
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 12:
        x = (s8)HIT(entity);
        if (x)
        {
            entity->subState = 10;
            HIT(entity) = 0;
            break;
        }
        EntityMove(entity);
        if (++entity->timer > 0x13)
        {
            entity->subState++;
            entity->timer = x;
            Anim_StartIndexed(entity, 0, 0, 1);
        }
        break;
    case 13:
        hit13 = (s8)HIT(entity);
        if (hit13)
        {
            entity->subState = 10;
            HIT(entity) = 0;
            break;
        }
        if (SPEED(entity) > 0x1000)
            SPEED(entity) -= 0x800;
        AIM_AT_PLAYER(entity);
        entity->velX.whole = -entity->velX.whole;
        entity->velY.whole = -entity->velY.whole;
        EntityMove(entity);
        if (++entity->timer > 0x1D)
        {
            entity->subState = hit13;
            entity->timer = hit13;
        }
        break;
    }
}

/**
 * @brief 7D65C | Swooping enemy attack (subState): backs away from the
 * player (Entity_AimVelocityAtPoint towards the player 0x10 px up, velocity negated)
 * accelerating unk_4F8 up to 0xC000, coasts, backs away decelerating, turns
 * to the player, then (song 0x87, animation 3) dives at the stored player
 * position (unk_4FC / unk_500, 0x20 px up, speed 2 px), brakes for 45
 * frames and returns to idle (state = 0) after 30 frames.
 */
void GiantGhost_UpdateAttack(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = gEwramData->bgInfo + 1;
    s32 x;

    switch (entity->subState)
    {
    case 0:
        if (entity->ext.giantGhost.speed < 0xC000)
            entity->ext.giantGhost.speed += 0x1000;
        x = Target_GetX(entity);
        x += bg->xPos.whole;
        Entity_AimVelocityAtPoint(entity, x, Target_GetY(entity) + bg->yPos.whole - 0x100000, entity->ext.giantGhost.speed);
        entity->velX.whole = -entity->velX.whole;
        entity->velY.whole = -entity->velY.whole;
        EntityMove(entity);
        if (++entity->timer > 0x13)
        {
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 1:
        EntityMove(entity);
        if (++entity->timer > 0x13)
        {
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 2:
        if (entity->ext.giantGhost.speed > 0x1000)
            entity->ext.giantGhost.speed -= 0x800;
        x = Target_GetX(entity);
        x += bg->xPos.whole;
        Entity_AimVelocityAtPoint(entity, x, Target_GetY(entity) + bg->yPos.whole - 0x100000, entity->ext.giantGhost.speed);
        entity->velX.whole = -entity->velX.whole;
        entity->velY.whole = -entity->velY.whole;
        EntityMove(entity);
        if (++entity->timer > 0x1D)
        {
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 3:
        entity->flipX = Entity_IsTargetRight(entity);
        if (++entity->timer > 0x13)
        {
            entity->subState++;
            entity->timer = 0;
            PlaySong(SE_DASH);
            entity->ext.giantGhost.diveX = Target_GetX(entity) + bg->xPos.whole;
            entity->ext.giantGhost.diveY = Target_GetY(entity) + bg->yPos.whole - 0x100000;
            Anim_StartIndexed(entity, 3, 0, 0);
        }
        break;
    case 4:
        if (Entity_AimVelocityAtPoint(entity, entity->ext.giantGhost.diveX, entity->ext.giantGhost.diveY - 0x200000, 0x20000))
        {
            entity->subState++;
            Anim_StartIndexed(entity, 4, 0, 1);
            entity->timer = 0;
        }
        EntityMove(entity);
        break;
    case 5:
        EntityDampVelocityX(entity, 8);
        EntityDampVelocityY(entity, 8);
        EntityMove(entity);
        if (entity->timer++ > 0x2C)
        {
            entity->subState++;
            Anim_StartIndexed(entity, 0, 0, 0);
            entity->timer = 0;
        }
        break;
    case 6:
        if (entity->timer++ > 0x1D)
        {
            entity->state = GIANT_GHOST_STATE_MOVE;
            entity->subState = 0;
            entity->timer = 0;
            entity->ext.giantGhost.hitPlayer = 0;
        }
        break;
    }
}

/**
 * @brief 7D88C | Hit callback: flash (hitFlashTimer = 8), subtract the damage
 * (Hit_ResolveDamage) from the HP in hp and show it (Entity_ShowDamage); at 0 HP
 * release the hit box, play song 0x6B, animation 5, palette from basePalette,
 * death state 2 (timer 0x5A) and burst into four random fragments
 * (GiantGhost_SpawnFragment) one layer below.
 */
void GiantGhost_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *other, u16 *hit)
{
    s32 damage = Player_CalculateAttackPower(hit[2]);
    s32 i;
    damage = Hit_ResolveDamage(entity, other, hit, damage);
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
        PlaySong(0x6B);
        Anim_StartIndexed(entity, 5, 0, 0);
        entity->palette = entity->basePalette;
        entity->timer = 0x5A;
        entity->state = 2;
        entity->subState = 0;
        entity->objMode = 1;
        for (i = 0; i < 4; i++)
        {
            s32 dx = (RandomNumberGenerator() & 0xFFFFF) - 0x80000;
            s32 dy = -(RandomNumberGenerator() & 0xFFFFF);
            s32 vx = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
            s32 vy = -(RandomNumberGenerator() & 0x1FFFF) - 0x10000;
            s32 flip = RandomNumberGenerator() & 1;
            GiantGhost_SpawnFragment(entity->posX.whole + dx, entity->posY.whole + dy, vx, vy, entity->spriteLayer - 1, flip);
        }
    }
}

/* Follow the main body, pulse its affine scale, then shrink away on death. */
static inline s32 Shrink(s32 value, s32 duration) { return value - value / duration; }
/**
 * @brief Aura part update: sit above the owner (type 0x13) with its visibility,
 * palette and facing, pulse its affine scale and leave fading trails
 * (GiantGhost_SpawnAuraTrail) every 4 frames; when the owner dies (state 2)
 * fade out for 110 frames.
 */
void GiantGhostAura_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    if (entity->state == 0)
    {
        s32 pulse;
        if (parent && parent->updateFunc && parent->enemyId == 0x13)
        {
            entity->hidden = parent->hidden;
            entity->palette = parent->palette;
            entity->flipX = parent->flipX;
            if (!Enemy_IsStunnedNoFlash(parent))
            {
                entity->ext.giantGhostAura.phase += 0x400;
                Anim_Update(entity);
                entity->timer++;
                if ((entity->timer & 3) == 0)
                {
                    s32 x = (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
                    s32 y = (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
                    GiantGhost_SpawnAuraTrail(entity, entity->posX.whole + x, entity->posY.whole + y);
                }
            }
            if (entity->flipX) entity->posX.part16.integer = parent->posX.part16.integer - 4;
            else entity->posX.part16.integer = parent->posX.part16.integer + 4;
            entity->posY.part16.integer = parent->posY.part16.integer - 18;
            pulse = Sin(entity->ext.giantGhostAura.phase) / 16;
            entity->velX.whole = pulse + 0x1B000;
            entity->velY.whole = pulse + 0x1D000;
            ObjAffine_SetBiased(0, entity->velX.whole, entity->velY.whole, entity->flipX, entity->flipY, entity->affineIndex);
            if (parent->state == 2)
            {
                entity->state = 1;
                entity->subState = 0;
                entity->timer = 110;
            }
            return;
        }
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
    else
    {
        s32 scaleX, scaleY;
        if (entity->subState == 0)
        {
            entity->subState++;
            entity->scrollWithBg = 1;
        }
        scaleX = Shrink(entity->velX.whole, entity->timer);
        entity->velX.whole = scaleX;
        scaleY = Shrink(entity->velY.whole, entity->timer);
        entity->velY.whole = scaleY;
        if (scaleX < 0x10000) entity->shrinkPartOffsets = 1;
        ObjAffine_SetBiased(0, entity->velX.whole, entity->velY.whole, entity->flipX, entity->flipY, entity->affineIndex);
        Anim_Update(entity);
        if ((s8)--entity->timer <= 0)
        {
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
        }
    }
}

#define FIXED_TO_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)

/**
 * @brief 7DBB4 | Giant Ghost hand (owner type 0x13 in enemyId): copies the
 * owner's visibility, palette and facing; while gEwramData bestiaryPreview is set
 * it sways a target point (unk_4FC / unk_500, direction unk_506 flipped
 * every 60 frames) and moves towards it at 3x sine speed from the owner
 * (1 / 0x1E px above), otherwise (unless Enemy_IsStunnedNoFlash) it reaches for the
 * player (Target_GetX / Target_GetY) from 1 px in front of the owner.
 * Vanishes (deleteRequested) when the owner enters state 2.
 */
void GiantGhostHandA_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    s32 x;
    s32 y;
    u16 angle;
    s32 c;
    s32 s;

    if (entity->state == 0)
    {
        if (parent && parent->updateFunc && parent->enemyId == 0x13)
        {
            entity->hidden = parent->hidden;
            entity->palette = parent->palette;
            entity->flipX = parent->flipX;
            if (gEwramData->gameState.bestiaryPreview)
            {
                if (entity->ext.giantGhostHand.swayDir)
                {
                    entity->ext.giantGhostHand.pointX -= 0x10000;
                    entity->ext.giantGhostHand.pointY -= 0x10000;
                    if ((s8)++entity->ext.giantGhostHand.swayFrame > 0x3B)
                    {
                        entity->ext.giantGhostHand.swayFrame = 0;
                        entity->ext.giantGhostHand.swayDir ^= 1;
                    }
                }
                else
                {
                    entity->ext.giantGhostHand.pointX += 0x10000;
                    entity->ext.giantGhostHand.pointY += 0x10000;
                    if ((s8)++entity->ext.giantGhostHand.swayFrame > 0x3B)
                    {
                        entity->ext.giantGhostHand.swayFrame = 0;
                        entity->ext.giantGhostHand.swayDir ^= 1;
                    }
                }
                x = parent->posX.whole - 0x10000;
                y = parent->posY.whole - 0x1E0000;
                x = entity->ext.giantGhostHand.pointX - x;
                y = entity->ext.giantGhostHand.pointY - y;
                angle = ArcTan2(FIXED_TO_INT(x), FIXED_TO_INT(y));
                x = Sin(angle + 0x4000);
                y = Sin(angle);
                entity->velX.whole = x * 3;
                entity->velY.whole = y * 2;
                if (entity->flipX)
                    entity->velX.whole = x * 3 - 0x10000;
                else
                    entity->velX.whole = x * 3 + 0x10000;
            }
            else if (!Enemy_IsStunnedNoFlash(parent))
            {
                if (entity->flipX)
                    x = parent->posX.whole + 0x10000;
                else
                    x = parent->posX.whole - 0x10000;
                y = parent->posY.whole - 0x10000;
                x = Target_GetX(entity) - x;
                y = Target_GetY(entity) - y;
                angle = ArcTan2(FIXED_TO_INT(x), FIXED_TO_INT(y));
                x = Sin(angle + 0x4000);
                y = Sin(angle);
                entity->velX.whole = x * 3;
                entity->velY.whole = y * 2;
                if (entity->flipX)
                    entity->velX.whole = x * 3 - 0x10000;
                else
                    entity->velX.whole = x * 3 + 0x10000;
            }
            entity->posX.whole = parent->posX.whole + entity->velX.whole;
            entity->posY.whole = parent->posY.whole + entity->velY.whole;
            if (parent->state == 2)
            {
                entity->state = 1;
                entity->deleteRequested = 1;
            }
            return;
        }
        EntityDelete(entity);
        return;
    }
    entity->deleteRequested = 1;
}

/**
 * @brief 7DE14 | Second Giant Ghost hand: like GiantGhostHandA_Update but at 2x sine
 * speed, swaying around 4 / 0x24 px from the owner and reaching for the
 * player from 4 px in front of / 4 px above the owner.
 */
void GiantGhostHandB_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    s32 x;
    s32 y;
    u16 angle;
    s32 c;
    s32 s;

    if (entity->state == 0)
    {
        if (parent && parent->updateFunc && parent->enemyId == 0x13)
        {
            entity->hidden = parent->hidden;
            entity->palette = parent->palette;
            entity->flipX = parent->flipX;
            if (gEwramData->gameState.bestiaryPreview)
            {
                if (entity->ext.giantGhostHand.swayDir)
                {
                    entity->ext.giantGhostHand.pointX -= 0x10000;
                    entity->ext.giantGhostHand.pointY -= 0x10000;
                    if ((s8)++entity->ext.giantGhostHand.swayFrame > 0x3B)
                    {
                        entity->ext.giantGhostHand.swayFrame = 0;
                        entity->ext.giantGhostHand.swayDir ^= 1;
                    }
                }
                else
                {
                    entity->ext.giantGhostHand.pointX += 0x10000;
                    entity->ext.giantGhostHand.pointY += 0x10000;
                    if ((s8)++entity->ext.giantGhostHand.swayFrame > 0x3B)
                    {
                        entity->ext.giantGhostHand.swayFrame = 0;
                        entity->ext.giantGhostHand.swayDir ^= 1;
                    }
                }
                x = parent->posX.whole - 0x40000;
                y = parent->posY.whole - 0x240000;
                x = entity->ext.giantGhostHand.pointX - x;
                y = entity->ext.giantGhostHand.pointY - y;
                angle = ArcTan2(FIXED_TO_INT(x), FIXED_TO_INT(y));
                x = Sin(angle + 0x4000);
                y = Sin(angle);
                entity->velX.whole = x * 2;
                entity->velY.whole = y * 2;
                if (entity->flipX)
                    entity->velX.whole = x * 2 - 0x10000;
                else
                    entity->velX.whole = x * 2 + 0x10000;
            }
            else if (!Enemy_IsStunnedNoFlash(parent))
            {
                if (entity->flipX)
                    x = parent->posX.whole + 0x40000;
                else
                    x = parent->posX.whole - 0x40000;
                y = parent->posY.whole - 0x40000;
                x = Target_GetX(entity) - x;
                y = Target_GetY(entity) - y;
                angle = ArcTan2(FIXED_TO_INT(x), FIXED_TO_INT(y));
                x = Sin(angle + 0x4000);
                y = Sin(angle);
                entity->velX.whole = x * 2;
                entity->velY.whole = y * 2;
                if (entity->flipX)
                    entity->velX.whole = x * 2 - 0x10000;
                else
                    entity->velX.whole = x * 2 + 0x10000;
            }
            entity->posX.whole = parent->posX.whole + entity->velX.whole;
            entity->posY.whole = parent->posY.whole + entity->velY.whole;
            if (parent->state == 2)
            {
                entity->state = 1;
                entity->deleteRequested = 1;
            }
            return;
        }
        EntityDelete(entity);
        return;
    }
    entity->deleteRequested = 1;
}

/* Create and retire an affine afterimage with its own matrix allocation. */
/**
 * @brief Spawn a semi-transparent affine trail sprite (animation 6) at (x, y)
 * with the scale of parent (kept in velX / velY); GiantGhostAuraTrail_Update.
 */
void GiantGhost_SpawnAuraTrail(struct EwramData_EntityData *parent, s32 x, s32 y)
{
    struct EwramData_EntityData *part = Entity_CreateObject(GiantGhostAuraTrail_Update);
    if (part)
    {
        s32 graphics;
        u16 matrix;
        part->posX.whole = x;
        part->posY.whole = y;
        part->velX.whole = parent->velX.whole;
        part->velY.whole = parent->velY.whole;
        part->flipX = parent->flipX;
        part->spriteLayer = parent->spriteLayer + 1;
        graphics = ObjGfx_Find(sGiantGhostGfx);
        Sprite_Attach(part, sGiantGhostSpriteSheet, graphics, parent->basePalette);
        part->hidden = parent->hidden;
        matrix = ObjAffine_Alloc(1);
        if ((s16)matrix < 0)
        {
            EntityDelete(part);
            return;
        }
        part->affineIndex = matrix;
        part->affine = 1;
        part->doubleSize = 1;
        part->objMode = 1;
        if ((u16)(part->posY.part16.integer + 28) > 220) part->hidden = 1;
        else part->hidden = 0;
        Anim_StartIndexed(part, 6, 0, 1);
        Anim_Update(part);
        ObjAffine_SetBiased(0, part->velX.whole, part->velY.whole, part->flipX, part->flipY, part->affineIndex);
        part->timer = 4;
        part->scrollWithBg = 1;
        part->drawFunc = Sprite_DrawEntity;
    }
}

/* Create and retire an affine afterimage with its own matrix allocation. */
/**
 * @brief Trail update: hide off screen vertically, keep the affine scale, animate;
 * free the matrix and delete when the timer ends.
 */
void GiantGhostAuraTrail_Update(struct EwramData_EntityData *entity)
{
    if ((u16)(entity->posY.part16.integer + 28) > 220) entity->hidden = 1;
    else entity->hidden = 0;
    ObjAffine_SetBiased(0, entity->velX.whole, entity->velY.whole, entity->flipX, entity->flipY, entity->affineIndex);
    Anim_Update(entity);
    if ((s8)entity->timer-- <= 0)
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
}

/* Giant Ghost's graphics, collision effect and death particles. */
/**
 * @brief Spawn a falling fragment at (x, y) moving with (vx, vy) under gravity on
 * layer priority, frame chosen by type (four on death).
 */
void GiantGhost_SpawnFragment(s32 x, s32 y, s32 vx, s32 vy, s32 priority, s32 type)
{
    struct EwramData_EntityData *part = EntityCreateInRange(ENTITY_SLOT_EFFECT_FIRST, ENTITY_SLOT_EFFECT_LAST, Effect_UpdateTimedParticle);
    if (part)
    {
        s32 zero;
        s32 graphics;
        u8 *priorityField;
        part->posX.whole = x;
        part->posY.whole = y;
        part->velX.whole = vx;
        part->velY.whole = vy;
        part->accelY.whole = 0x2000;
        priorityField = &part->spriteLayer;
        zero = 0;
        *priorityField = priority;
        part->timer = 200;
        part->ext.particle.flags = zero;
        part->ext.particle.flipFlags = 3;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(part, sCommonObjectSpriteSheet, graphics, 6);
        if (type == 0) part->animFrame = 64;
        else part->animFrame = 65;
        part->objMode = 1;
        part->scrollWithBg = 1;
        part->drawFunc = Sprite_DrawEntity;
    }
}

/* Giant Ghost's graphics, collision effect and death particles. */
/**
 * @brief Load the Giant Ghost graphics and palette and start its palette
 * animation (sGiantGhostPaletteAnim); return 0 when no graphics slot is free, else 1.
 */
u8 GiantGhost_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics = ObjGfx_LoadResource(sGiantGhostGfx, 0);
    u8 result;
    if (graphics >= 0)
    {
        entity->basePalette = ObjPalette_FindOrLoadVariant(sGiantGhostPalette, 0);
        PaletteAnim_Start(sGiantGhostPalette, sGiantGhostPaletteAnim, entity->basePalette);
        Sprite_Attach(entity, sGiantGhostSpriteSheet, graphics, entity->basePalette);
        entity->drawFunc = Sprite_DrawEntity;
        result = 1;
    }
    else
        result = 0;
    return result;
}

/* Giant Ghost's graphics, collision effect and death particles. */
/**
 * @brief Contact: mark it (unk_505 = 1) and Player_ApplyContactDamage with
 * attribute 0x10 at the entity position.
 */
void GiantGhost_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage effect;
    effect.attack = entity->attack;
    effect.attributes = 16;
    effect.unused = 0;
    effect.reaction = 0;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    entity->ext.giantGhost.hitPlayer = 1;
    Player_ApplyContactDamage(&effect);
}
