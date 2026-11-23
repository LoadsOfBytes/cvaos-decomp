/*
 * Gorgon: body, breath and death pieces. The last part of the range also holds the entry points of Catoblepas.
 */
#include "core.h"
#include "gorgon_entity.h"
#include "graphics.h"
#include "entity_util.h"
#include "player_core.h"
#include <string.h>
#include "sound.h"
#include "gameplay.h"
#include "structs/ewram.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"

/* Dispatch Gorgon/Catoblepas states and support an overridden single frame. */
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern void Gorgon_UpdateWalk(struct EwramData_EntityData *);
extern void Gorgon_UpdateAttack(struct EwramData_EntityData *);
extern void Gorgon_SpawnDeathPieces(struct EwramData_EntityData *);
struct SingleFrameAnimation {
    u16 count;
    u16 format;
    u8 frame;
    u8 duration;
    u16 unk_6;
};
extern struct SingleFrameAnimation sGorgonFrameAnim;
void EnemyGorgonUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct SingleFrameAnimation animation = sGorgonFrameAnim;
    entity->ext.gorgon.flags = (u8)entity->ext.gorgon.flags & 0xFE;
    if (entity->state != 2)
    {
        if (EntityIsOutsideScreen(entity, 4)) goto finished;
        if (Entity_IsInRect(entity, -16, 0, 272, 176)) Player_TrackEntity(entity);
        if (entity->state != 2) Player_QueueEntity(entity, 1);
    }
    if (EntityIsOutsideScreen(entity, 4)) entity->hidden = 1;
    else entity->hidden = 0;
    if (entity->state != 2) Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity))
    {
        entity->velX.whole = 0;
        if (entity->state != 2)
        {
            Hit_CheckWithPlayer(collision, entity);
            goto finished;
        }
    }
    switch (entity->state)
    {
    case 0: Gorgon_UpdateWalk(entity); break;
    case 1: Gorgon_UpdateAttack(entity); break;
    case 2:
        if (entity->stage == 0)
        {
            entity->stage = 1;
            Gorgon_SpawnDeathPieces(entity);
        }
        entity->ext.gorgon.flags |= 1;
        entity->ext.gorgon.forcedFrame = 23;
        if ((entity->timer & 3) == 0)
        {
            s32 x = RandomNumberGenerator() % 0x280000 - 0x140000;
            RandomNumberGenerator();
            Effect_SpawnRisingParticle(entity->posX.whole + x, entity->posY.whole, entity->spriteLayer - 1, 1);
        }
        if ((s8)--entity->timer <= 0) entity->deleteRequested = 1;
        Enemy_UpdateFragmentDeath(entity);
        break;
    case 3:
        Anim_Update(entity);
        goto finished;
    }
    {
        u8 *initial = &entity->animFrame;
        u8 previous = *initial;
        s32 override = entity->ext.gorgon.flags & 1;
        u8 *frame = initial;
        if (override)
        {
            Anim_Start(entity, (struct AnimScript *)&animation, 0, 1);
            animation.frame = entity->ext.gorgon.forcedFrame;
            Anim_Update(entity);
        }
        else Anim_Update(entity);
        if (previous != *frame) Entity_UpdateBoxes(entity);
    }
    entity->ext.gorgon.hit = 0;
    Enemy_CheckHitPlayer(entity);
finished:
    return;
}

/* Initialize the shared Gorgon/Catoblepas body and its collision callbacks. */
extern u8 Gorgon_LoadGraphics(struct EwramData_EntityData *);
extern void Hit_EnableContact(struct EwramData_EntityData *, s32, void *, void *);
extern void Gorgon_DamagePlayerOnContact(struct EwramData_EntityData *);
void Gorgon_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
void EnemyGorgonCreate(struct EwramData_EntityData *entity)
{
    if (!Gorgon_LoadGraphics(entity))
    {
        EntityDelete(entity);
        goto finished;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 9;
    if (gEwramData->gameState.bestiaryPreview) entity->state = 3;
    else
    {
        if (Anim_GetMaxBoxCount(entity) > 1)
            Hit_EnableContact(entity, 8, Gorgon_DamagePlayerOnContact, Gorgon_OnHit);
        else
            Hit_Enable(entity, 8, Gorgon_DamagePlayerOnContact, (s32)Gorgon_OnHit);
        Entity_UpdateBoxes(entity);
        Entity_SnapToGround(entity);
        entity->state = 0;
    }
finished:
    return;
}

/* Ground activation, timed gait, turning and backward retreat. */
extern s8 sGorgonWalk[16], sGorgonTurn[12];
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s16);
extern s32 Target_IsInRect(s16, s16, s32, s32);
#define GAIT(e) (e->ext.gorgon.gait)
#define FRAME(e) ((e)->ext.raw.unk_4FC.part8.unk_4FE)
#define CLOCK(e) (e->ext.gorgon.clock)
static inline s32 TileAhead(struct EwramData_EntityData *entity, struct BgLayerState *background, s32 offset)
{
    return Tile_Classify((s16)(background->xPos.part16.integer + entity->posX.part16.integer + offset),
                       (s16)(background->yPos.part16.integer + entity->posY.part16.integer));
}
/**
 * @brief Walk state machine (state 0): fall to the ground, wake when the target is
 * in the 180 x 120 box or the Gorgon is damaged, then walk with the gait
 * tables sGorgonWalk / sGorgonTurn and start the breath attack.
 */
void Gorgon_UpdateWalk(struct EwramData_EntityData *entity)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    s8 walk[16], turn[12];
    s32 phase;
    memcpy(walk, sGorgonWalk, sizeof(walk));
    memcpy(turn, sGorgonTurn, sizeof(turn));
    phase = entity->subState;
    switch (phase)
    {
    case 0:
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0x2000;
        if (!(Entity_MoveWithCollision(entity, 0x40000, -0x200000, 0x80000, 24) & 4)) break;
        if (Target_IsInRect((s16)(entity->posX.part16.integer - 90),
                         (s16)(entity->posY.part16.integer - 60), 180, 120) || Enemy_IsDamaged(entity))
        {
            entity->subState++;
            CLOCK(entity) = phase;
            GAIT(entity) = phase;
            entity->velX.whole = phase;
        }
        break;
    case 1:
    {
        s32 offset;
        entity->ext.gorgon.flags |= 1;
        CLOCK(entity)++;
        if (CLOCK(entity) >= walk[(s8)GAIT(entity) * 2 + 1])
        {
            CLOCK(entity) = 0;
            GAIT(entity)++;
            if ((s8)GAIT(entity) > 7) GAIT(entity) = 0;
        }
        FRAME(entity) = walk[(s8)GAIT(entity) * 2];
        if ((u16)GORGON_FRAME_CLOCK(entity) == 7)
        {
            s32 facing;
            offset = entity->flipX ? 32 : -32;
            facing = entity->flipX;
            if (facing != Entity_IsTargetRight(entity) || TileAhead(entity, background, offset))
            {
                entity->subState = 2;
                CLOCK(entity) = 0;
                GAIT(entity) = 0;
                entity->velX.whole = 0;
                break;
            }
        }
        if (RandomNumberGenerator() % 100 <= 50)
        {
            s32 facing = entity->flipX << 6;
            offset = 80;
            if (facing) offset = 0;
            if (Target_IsInRect((s16)(entity->posX.part16.integer - offset),
                             (s16)(entity->posY.part16.integer - 24), 80, 24)
                && (u16)GORGON_FRAME_CLOCK(entity) == 0x805)
            {
                entity->state = 1;
                entity->subState = 0;
                entity->timer = 0;
                entity->velX.whole = 0;
                break;
            }
        }
        if (entity->ext.gorgon.hit)
        {
            offset = entity->flipX ? -32 : 32;
            if (!TileAhead(entity, background, offset))
            {
                entity->subState = 3;
                CLOCK(entity) = 60;
                entity->velX.whole = 0;
                entity->timer = 0;
                break;
            }
        }
        Entity_MoveWithCollision(entity, 0x40000, -0x200000, 0x80000, 24);
        if (FRAME(entity) == 4 || FRAME(entity) == 7) entity->velX.whole = 0;
        else entity->velX.whole = entity->flipX ? 0x8000 : -0x8000;
        break;
    }
    case 2:
    {
        entity->ext.gorgon.flags |= 1;
        CLOCK(entity)++;
        if (CLOCK(entity) >= turn[(s8)GAIT(entity) * 2 + 1])
        {
            CLOCK(entity) = 0;
            GAIT(entity)++;
            if ((s8)GAIT(entity) > 5)
            {
                entity->subState = 1;
                GAIT(entity) = phase;
                entity->velX.whole = 0;
                entity->accelX.whole = 0;
                entity->accelY.whole = 0;
                entity->flipX ^= 1;
                entity->ext.gorgon.flags |= 1;
                FRAME(entity) = walk[(s8)GAIT(entity) * 2];
                break;
            }
        }
        FRAME(entity) = turn[(s8)GAIT(entity) * 2];
        entity->velX.whole = 0;
        if ((GORGON_WALK_WORD(entity) & 0xFF00FF00) == 0x100)
        {
            entity->accelX.whole = (gEwramData->playerState.player->posX.whole - entity->posX.whole) / 2;
            entity->velX.whole = -entity->accelX.whole;
            FRAME(entity)--;
        }
        if ((GORGON_WALK_WORD(entity) & 0xFF00FF00) == 0x200)
        {
            entity->accelY.whole = gEwramData->playerState.player->posX.whole - entity->posX.whole;
            entity->velX.whole = -entity->accelY.whole;
            FRAME(entity)--;
        }
        if ((GORGON_WALK_WORD(entity) & 0xFF00FF00) == 0x300)
        {
            entity->velX.whole = entity->flipX ? 0x10000 : -0x10000;
            FRAME(entity)--;
        }
        if ((GORGON_WALK_WORD(entity) & 0xFF00FF00) == 0x400)
        {
            entity->velX.whole = -(entity->accelX.whole + entity->accelY.whole);
            FRAME(entity)--;
        }
        if ((GORGON_WALK_WORD(entity) & 0xFF00FF00) == 0x500)
        {
            entity->velX.whole = entity->flipX ? -0x10000 : 0x10000;
            FRAME(entity)--;
        }
        break;
    }
    case 3:
        entity->ext.gorgon.flags |= 1;
        CLOCK(entity)++;
        if (CLOCK(entity) >= walk[(s8)GAIT(entity) * 2 + 1])
        {
            CLOCK(entity) = 0;
            GAIT(entity)--;
            if ((s8)GAIT(entity) < 0) GAIT(entity) = 7;
        }
        FRAME(entity) = walk[(s8)GAIT(entity) * 2];
        Entity_MoveWithCollision(entity, 0x40000, -0x200000, 0x80000, 24);
        if (FRAME(entity) == 4 || FRAME(entity) == 7)
        {
            entity->velX.whole = 0;
            if (entity->timer > 21)
            {
                entity->subState = 1;
                entity->accelX.whole = 0;
                entity->accelY.whole = 0;
                break;
            }
        }
        else entity->velX.whole = entity->flipX ? -0x8000 : 0x8000;
        if (++entity->timer > 59) entity->timer = 60;
        break;
    }
}

/* Sweep the attack direction while emitting effects on alternate frames. */
extern void GorgonBreath_Spawn(struct EwramData_EntityData *, s32, s32, s32, s32);
/**
 * @brief Attack state machine (state 1): play animation 2 and, from its first step
 * on, emit breath puffs (GorgonBreath_Spawn) for the attack duration, then
 * return to state 0.
 */
void Gorgon_UpdateAttack(struct EwramData_EntityData *entity)
{
    s32 phase = entity->subState;
    switch (phase)
    {
    case 0:
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->subState++;
        break;
    case 1:
        if (entity->animStep != 0)
        {
            entity->subState = 2;
            if (entity->flipX)
            {
                entity->ext.gorgon.breathAngle = 0;
                entity->ext.gorgon.breathDir = phase;
            }
            else
            {
                entity->ext.gorgon.breathAngle = 0x8000;
                entity->ext.gorgon.breathDir = 0;
            }
            entity->ext.gorgon.breathStep = 3;
        }
        break;
    case 2:
        entity->timer++;
        if (entity->timer == 30 || entity->timer == 50) PlaySong(0x67);
        if ((u8)(entity->timer - 30) <= 40 && !(entity->timer & 1))
        {
            s32 x, y, vx, vy, speed;
            if (entity->flipX) x = 0x2A0000;
            else x = -0x2A0000;
            y = -0x120000;
            vx = Sin((u16)entity->ext.gorgon.breathAngle + 0x4000);
            vy = Sin((u16)entity->ext.gorgon.breathAngle);
            speed = 0x30000;
            vx = FixedMultiply(vx, speed);
            vy = FixedMultiply(vy, speed);
            GorgonBreath_Spawn(entity, x, y, vx, vy);
            if ((s8)entity->ext.gorgon.breathDir == 0)
                entity->ext.gorgon.breathAngle += 0x400;
            else entity->ext.gorgon.breathAngle -= 0x400;
            if ((s8)++entity->ext.gorgon.breathStep > 6)
            {
                entity->ext.gorgon.breathStep = 0;
                entity->ext.gorgon.breathDir ^= 1;
            }
        }
        if (entity->animDone)
        {
            s32 zero = 0;
            entity->state = zero;
            entity->subState = 1;
            entity->ext.gorgon.clock = zero;
            entity->ext.gorgon.gait = zero;
            entity->velX.whole = zero;
            entity->accelX.whole = zero;
            entity->accelY.whole = zero;
            entity->timer = zero;
        }
        break;
    }
}

/* Apply damage and signal a hit to the body's current behavior. */
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *, s16);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s32, s32);
/**
 * @brief Hit: resolve the damage, flash (hitFlashTimer = 8), subtract it from the HP and
 * show it, mark the Gorgon as hit (unk_500 = 1); at HP <= 0 drop the
 * rewards and die (song 0x71, 60 frames, state 2).
 */
void Gorgon_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s16 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));
    s32 zero;
    entity->hitFlashTimer = 8;
    {
        s32 hp = entity->hp;
        zero = 0;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    entity->ext.gorgon.hit = 1;
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x71);
        entity->palette = entity->basePalette;
        entity->timer = 60;
        entity->state = 2;
        entity->subState = zero;
        entity->stage = zero;
    }
}

/* Create a hit effect relative to the body with supplied initial velocity. */
extern void GorgonBreath_Update(struct EwramData_EntityData *);
extern void GorgonBreath_DamagePlayerOnContact(struct EwramData_EntityData *);
extern u8 sGorgonGfx[], sGorgonSpriteSheet[];
/**
 * @brief Spawn a breath puff at offset (x, y) from parent moving with (vx, vy);
 * contact damage GorgonBreath_DamagePlayerOnContact (attribute 0x100).
 */
void GorgonBreath_Spawn(struct EwramData_EntityData *parent, s32 x, s32 y, s32 vx, s32 vy)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, (u32 *)GorgonBreath_Update);
    struct SpriteBox box = {-4, -4, 8, 8};
    if (part)
    {
        s32 graphics;
        part->posX.whole = parent->posX.whole + x;
        part->posY.whole = parent->posY.whole + y;
        part->velX.whole = vx;
        part->velY.whole = vy;
        part->spriteLayer = parent->spriteLayer - 1;
        graphics = ObjGfx_Find(sGorgonGfx);
        part->basePalette = parent->basePalette;
        Sprite_Attach(part, sGorgonSpriteSheet, graphics, parent->basePalette);
        part->ext.gorgon.forcedFrame = 25;
        Hit_Enable(part, 8, GorgonBreath_DamagePlayerOnContact, 0);
        HitBox_SetAttack(part, &box);
        part->hit.flags |= 4;
        part->hit.flags &= ~2;
        part->attack = parent->attack;
        ENTITY_OWNER(part) = parent;
        part->scrollWithBg = 1;
        part->drawFunc = (u32 *)Sprite_DrawEntity;
        part->timer = 10;
    }
}

/* Update a tumbling hit effect with a mutable one-frame animation. */
/* Animation format 1 embeds four-byte frames after the common header. */
/**
 * @brief Breath puff update: slow down, flicker with random flips, grow its hit box
 * at frame 25 and vanish after 40 frames; contact test every other frame.
 */
void GorgonBreath_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct SpriteBox box;
    struct SingleFrameAnimation animation = sGorgonFrameAnim;
    box = (struct SpriteBox){-7, -7, 14, 14};
    EntityDampVelocityX(entity, 24);
    EntityDampVelocityY(entity, 24);
    entity->flipX = RandomNumberGenerator() % 2;
    entity->flipY = RandomNumberGenerator() % 2;
    EntityMove(entity);
    if (entity->timer == 25)
    {
        entity->ext.gorgon.forcedFrame = 26;
        HitBox_SetAttack(entity, &box);
    }
    Anim_Start(entity, (struct AnimScript *)&animation, 0, 1);
    animation.frame = entity->ext.gorgon.forcedFrame;
    Anim_Update(entity);
    if (entity->timer++ > 39) EntityDelete(entity);
    else
    {
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if (entity->timer & 1) Hit_CheckWithPlayer(collision, entity);
    }
}

/* Release six pieces with distinct frames and randomized horizontal velocities. */
extern void GorgonPiece_Update(struct EwramData_EntityData *);
/**
 * @brief Spawn the six body pieces (frames 17..22) that drop away when the Gorgon
 * dies, with a random horizontal speed and lifetime 40..55 frames.
 */
void Gorgon_SpawnDeathPieces(struct EwramData_EntityData *parent)
{
    s32 index;
    for (index = 5; index >= 0; index--)
    {
        struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, (u32 *)GorgonPiece_Update);
        if (part)
        {
            s32 graphics;
            s32 lifetime;
            part->posX.whole = parent->posX.whole;
            part->posY.whole = parent->posY.whole;
            part->velX.whole = (RandomNumberGenerator() & 0xFFFF) - 0x8000;
            part->velY.whole = 0;
            part->accelY.whole = 0x1400;
            part->spriteLayer = parent->spriteLayer;
            graphics = ObjGfx_Find(sGorgonGfx);
            Sprite_Attach(part, sGorgonSpriteSheet, graphics, parent->basePalette);
            part->animFrame = index + 17;
            part->flipX = parent->flipX;
            part->scrollWithBg = 1;
            part->drawFunc = (u32 *)Sprite_DrawEntity;
            lifetime = RandomNumberGenerator() & 15;
            part->timer = lifetime + 40;
        }
    }
}

/* GorgonBreath_DamagePlayerOnContact: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */

/**
 * @brief 899CC | Like ArachneBeam_DamagePlayerOnContact, but unk_2 = 0x100.
 */
void GorgonBreath_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0x100;
    contactDamage.unused = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/* Let debris settle, flash near the end of its life and remove it. */
/**
 * @brief Falling body piece: from frame 40 down, slow horizontally, fall and blink;
 * delete when the timer ends.
 */
void GorgonPiece_Update(struct EwramData_EntityData *entity)
{
    if (entity->timer <= 40)
    {
        EntityDampVelocityX(entity, 8);
        EntityAccelerateAndMove(entity, 0x80000);
        if (entity->timer <= 40) entity->hidden = Entity_GetVisibilityPattern(entity->timer);
    }
    if ((s8)--entity->timer <= 0) EntityDelete(entity);
}

/* Gorgon and Catoblepas share graphics and select different palette variants. */
extern s8 ObjPalette_FindOrLoadVariant(void *, u8);
extern u8 sGorgonGfx[], sGorgonPalette[], sGorgonSpriteSheet[];
/**
 * @brief Load the Gorgon graphics with palette variant 0 for type 0x33, else 1;
 * return 0 when no graphics slot is free, else 1.
 */
u8 Gorgon_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics = ObjGfx_LoadResource(sGorgonGfx, 0);
    if (graphics < 0) return 0;
    if (entity->enemyId == 0x33)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sGorgonPalette, 0);
    else
        entity->basePalette = ObjPalette_FindOrLoadVariant(sGorgonPalette, 1);
    Sprite_Attach(entity, sGorgonSpriteSheet, graphics, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Gorgon_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */

static inline void Entity_DamagePlayerOnContact(struct EwramData_EntityData *entity, u16 attributes, u16 unused, u16 reaction)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = attributes;
    contactDamage.unused = unused;
    contactDamage.reaction = reaction;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 89AB4 | Same as Enemy_DamagePlayerOnContact.
 */
void Gorgon_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/* Forward this enemy variant to its shared implementation. */
void EnemyCatoblepasUpdate(struct EwramData_EntityData *entity)
{
    EnemyGorgonUpdate(entity);
}

/* Forward this enemy variant to its shared implementation. */
void EnemyCatoblepasCreate(struct EwramData_EntityData *entity)
{
    EnemyGorgonCreate(entity);
}
