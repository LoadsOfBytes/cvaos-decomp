/*
 * Creaking Skull (enemy): body, joints, parts, guided effect and projectiles;
 * at the end the Giant Skeleton entry points (same original unit).
 * One translation unit for the consecutive ROM range of its code; the
 * functions keep their ROM order.
 */
#include "core.h"
#include "player_core.h"
#include "graphics.h"
#include "gameplay.h"
#include "entity_util.h"
#include "structs/ewram.h"
#include "sound.h"
#include <string.h>
#include "syscalls.h"
#include "boss_defeat.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "map_flags.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "player_level.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "damage_numbers.h"
#include "music.h"
#include "entity_hit.h"
#include "hit_shared_history.h"
#include "constants/songs.h"
#include "constants/hit_flags.h"

void CreakingSkull_UpdateMovement(struct EwramData_EntityData *entity);
void CreakingSkull_UpdateAttack(struct EwramData_EntityData *entity);
void CreakingSkull_UpdateBestiaryPreview(struct EwramData_EntityData *entity);

struct SkullBox {
    s32 x:8;
    s32 y:8;
    u32 w:8;
    u32 h:8;
};

#define TO_PIXELS(d) ((d) < 0 ? -(-(d) >> 16) : (d) >> 16)

extern void *memcpy(void *dst, const void *src, unsigned long size);

u8 CreakingSkull_LoadGraphics(struct EwramData_EntityData *entity);
void CreakingSkullPart_Update(struct EwramData_EntityData *entity);
void CreakingSkull_OnContact(struct EwramData_EntityData *entity);
void CreakingSkullPart_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
void CreakingSkull_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void CreakingSkull_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
void CreakingSkull_AttachJointSetAngle(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length);
void CreakingSkull_InterpolateJointsInOrder(struct EwramData_EntityData *entity, s8 *order);
void CreakingSkull_FaceTarget(struct EwramData_EntityData *entity);
void CreakingSkull_SaveJointPositions(struct EwramData_EntityData *entity);
void CreakingSkull_RestoreJointPositions(struct EwramData_EntityData *entity);
void CreakingSkull_AttachJoint(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length);
void CreakingSkull_SpawnGuidedEffect(struct EwramData_EntityData *entity, s32 lateTargetX, s32 lateTargetY, s32 earlyTargetX, s32 earlyTargetY, s32 x, s32 y);
void CreakingSkull_SpawnProjectile(struct EwramData_EntityData *owner, s32 x, s32 y, s32 vx, s32 vy);
void CreakingSkullProjectile_Update(struct EwramData_EntityData *);
void CreakingSkullProjectile_DamagePlayerOnContact(struct EwramData_EntityData *);
void CreakingSkullProjectile_OnHit(struct EwramData_EntityData *);

/* States of CreakingSkull (entity->state), named after the update function the dispatcher calls. */
enum {
    CREAKING_SKULL_STATE_MOVEMENT = 0,
    CREAKING_SKULL_STATE_ATTACK = 1,
    CREAKING_SKULL_STATE_BESTIARY_PREVIEW = 3,
};

/**
 * @brief 95320 | Creaking Skull update (as EnemyGreatArmorUpdate): clears
 * unk_505, skipped off screen unless dying; states 0 / 1 (CreakingSkull_UpdateMovement /
 * CreakingSkull_UpdateAttack), 2 dying frees the parts (Heap_Free) when done, 3
 * CreakingSkull_UpdateBestiaryPreview; the hit box (0x2C wide) follows part 5 of the 13 parts
 * (mirrored by flipX).
 */
void EnemyCreakingSkullUpdate(struct EwramData_EntityData *entity)
{
    u8 unused[48];
    struct SkullBox box;
    s32 *parts; /* EntityJointPart viewed as 12 words (needed for the addressing) */

    entity->ext.creakingSkull.unusedFlag = 0;
    if (entity->state != 2)
    {
        if (EntityIsOutsideScreen(entity, 6))
            return;
        if (Entity_IsInRect(entity, -16, 0, 272, 176))
            Player_TrackEntity(entity);
        if (entity->state != 2)
            Enemy_UpdateHitFlash(entity);
    }
    if (!Entity_IsTimeStopped(entity) || entity->state == 2)
    {
        switch (entity->state)
        {
        case CREAKING_SKULL_STATE_MOVEMENT:
            CreakingSkull_UpdateMovement(entity);
            break;
        case CREAKING_SKULL_STATE_ATTACK:
            CreakingSkull_UpdateAttack(entity);
            break;
        case 2:
            if ((s8)--entity->timer <= 0)
                entity->deleteRequested = 1;
            if (entity->deleteRequested && entity->ext.joint.parts != NULL)
                Heap_Free(entity->ext.joint.parts);
            return;
        case CREAKING_SKULL_STATE_BESTIARY_PREVIEW:
            CreakingSkull_UpdateBestiaryPreview(entity);
            return;
        }
    }
    parts = (s32 *)entity->ext.joint.parts;
    if (entity->flipX)
        box.x = -TO_PIXELS(parts[5 * 12 + 0]) - 0x14;
    else
        box.x = TO_PIXELS(parts[5 * 12 + 0]) - 0x14;
    box.y = TO_PIXELS(parts[5 * 12 + 1]) - 0x14;
    box.w = 0x2C;
    box.h = box.y < 0 ? -box.y : box.y;
    HitBox_SetAttack(entity, &box);
    HitBox_SetBody(entity, &box);
    Enemy_CheckHitPlayer(entity);
}

/* Creaking Skull create: allocates the 13-joint skeleton and one part entity per joint. */

void EnemyCreakingSkullCreate(struct EwramData_EntityData *entity)
{
    u8 frames0[13];
    u8 frames1[13];
    u8 anchors[13];
    s8 order[13];
    s8 chain[13];
    s32 i;
    s32 index;
    struct EwramData_EntityData *part;
    struct EntityJointPart *parts;
    const u8 *pose;
    const s32 *angles;
    u8 *frames;
    s32 frozen;
    struct EwramData **ewram = &gEwramData;

    memcpy(frames0, sCreakingSkullFrames0, sizeof(frames0));
    memcpy(frames1, sCreakingSkullFrames0, sizeof(frames1));
    memcpy(anchors, sCreakingSkullAnchors, sizeof(anchors));
    memcpy(order, sCreakingSkullOrder, sizeof(order));
    memcpy(chain, sUnk_08119063, sizeof(chain));

    if (entity->spawnParam0 != 0 && !(*ewram)->gameState.bossRushMode && ((*ewram)->gameState.bossEventFlags & BOSS_DEFEATED_CREAKING_SKULL))
    {
        EntityDelete(entity);
        return;
    }
    if (!CreakingSkull_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    entity->ext.joint.parts = Heap_Alloc(0x9C);
    if (entity->ext.joint.parts == NULL)
    {
        EntityDelete(entity);
        return;
    }
    entity->hidden = 1;
    entity->flipX = 0;
    entity->ext.joint.pose = 0;
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bossRushMode)
    {
        entity->hp += 1000;
        entity->attack += 20;
    }
    frames = frames0;
    parts = entity->ext.joint.parts;
    for (i = 0; i <= 12; i++)
    {
        index = order[i];
        part = Entity_CreateObject(CreakingSkullPart_Update);
        if (part != NULL)
        {
            part->spriteLayer = entity->spriteLayer;
            Sprite_Attach(part, sCreakingSkullSpriteSheet, ObjGfx_LoadResource(sCreakingSkullGfx, 0), entity->basePalette);
            part->enemyId = entity->enemyId;
            part->basePalette = entity->basePalette;
            part->scrollWithBg = 0;
            part->hidden = 0;
            part->attack = entity->attack;
            part->drawFunc = Sprite_DrawEntity;
            part->ext.jointPart.owner = entity;
            part->ext.jointPart.index = index;
            part->ext.jointPart.anchor = anchors[index];
            part->posX.whole = entity->posX.whole;
            part->posY.whole = entity->posY.whole;
            part->animFrame = frames[index];
            if ((u32)index <= 2 || index == 8 || index == 11)
            {
                u16 palette = ObjAffine_Alloc(1);
                if ((s16)palette < 0)
                {
                    EntityDelete(part);
                    EntityDelete(entity);
                    return;
                }
                part->affineIndex = palette;
                part->affine = 1;
                ObjAffine_Set(0, 0x10000, 0x10000, part->flipX, part->flipY, part->affineIndex);
            }
            if (!gEwramData->gameState.bestiaryPreview)
                Hit_Enable(part, 8, CreakingSkull_OnContact, CreakingSkullPart_OnHit);
            *(struct EwramData_EntityData **)parts[index].pad_2C = part;
        }
        else
        {
            Heap_Free(entity->ext.joint.parts);
            EntityDelete(entity);
            return;
        }
    }
    parts = entity->ext.joint.parts;
    pose = sCreakingSkullPartPoses[(s8)entity->ext.joint.pose];
    angles = sCreakingSkullPartAngles[(s8)entity->ext.joint.pose];
    for (index = 0; index <= 12; index++)
    {
        parts[index].offsetX = (s8)pose[index * 2] << 16;
        parts[index].offsetY = (s8)pose[index * 2 + 1] << 16;
        if (index == 2)
            parts[2].rotationAngle = angles[0];
        else
            parts[index].rotationAngle = 0x10000;
    }
    entity->timer = 1;
    CreakingSkull_AttachJointSetAngle(entity, 12, 11, -0x4000, 0x1F0000);
    CreakingSkull_AttachJointSetAngle(entity, 1, 0, -0x800, 0x1E0000);
    CreakingSkull_AttachJointSetAngle(entity, 2, 1, 0x6000, 0x1C0000);
    CreakingSkull_InterpolateJointsInOrder(entity, chain);
    CreakingSkull_AttachJointSetAngle(entity, 12, 11, 0, 0x1F0000);
    CreakingSkull_AttachJointSetAngle(entity, 1, 0, 0, 0x1E0000);
    CreakingSkull_AttachJointSetAngle(entity, 2, 1, 0, 0x1C0000);
    entity->timer = 1;
    CreakingSkull_InterpolateJointsInOrder(entity, chain);
    frozen = (u8)(*((u8 *)gEwramData + 0x8E) & 0x40);
    if (frozen)
    {
        entity->state = CREAKING_SKULL_STATE_BESTIARY_PREVIEW;
        entity->subState = 0x14;
        entity->timer = 0;
        return;
    }
    Hit_Enable(entity, 8, CreakingSkull_DamagePlayerOnContact, CreakingSkull_OnHit);
    CreakingSkull_FaceTarget(entity);
    Entity_SnapToGround(entity);
    entity->state = frozen;
    entity->subState = 0x14;
    entity->ext.joint.hurtMask = 0x106;
    switch (entity->enemyId)
    {
    case 0x21:
        entity->ext.creakingSkull.variant = 0;
        break;
    case 0x5C:
        entity->ext.creakingSkull.variant = 1;
        break;
    }
    if (entity->spawnParam0 != 0)
    {
        Player_SetSelectedTarget(entity);
        gEwramData->gameState.stateFlags |= STATE_BOSS_FIGHT;
        Music_SetCurrent(MUS_FORMIDABLE_FOE);
    }
}

/* Creaking Skull movement: alternating strides, pauses, and attack selection.
 * for compiler constraints and matching evidence. */
#define PHASE entity->subState
#define SUBPHASE entity->stage
#define TIMER entity->timer
#define FRAME entity->ext.joint.pose
#define ARMS(A) \
    CreakingSkull_AttachJointSetAngle(entity, 1, 0, A, 0x1E0000); \
    CreakingSkull_AttachJointSetAngle(entity, 2, 1, A, 0x1C0000);
#define POSE(A,B) \
    CreakingSkull_AttachJointSetAngle(entity, 12, 11, A, 0x1F0000); \
    ARMS(B)
#define GRAVITY() entity->velY.whole = 0x10000; entity->accelY.whole = 0x2800;
#define SPEED() \
    if (entity->flipX) entity->velX.whole = 0x190000 / (TIMER * 2 - 4); \
    else entity->velX.whole = -0x190000 / (TIMER * 2 - 4);
#define UPDATE() CreakingSkull_InterpolateJointsInOrder(entity, chain);
#define FALL() Entity_MoveWithCollisionDefault(entity, 0x10000, -0x300000);
#define COUNTDOWN() if ((s8)--TIMER > 0) break;
#define DISTANCE() \
    distance = (s16)Target_GetPixelX(entity) - (s16)entity->posX.part16.integer; \
    if (distance < 0) distance = -distance;
#define TERRAIN() Tile_Classify(camera->xPos.part16.integer + entity->posX.part16.integer + offset, camera->yPos.part16.integer + entity->posY.part16.integer)
/* The extra RNG call is present in the ROM even though its result is unused.
 * The debris offset and velocities are function-level variables (offset, vx, vy). */
#define DEBRIS(REVERSE) \
    if (!(TIMER & 15)) \
    { \
        s32 zero;\
        if (entity->flipX) { offset = -0x230000; if (REVERSE) vx = -0x4000 - (RandomNumberGenerator() & 0x3FFF); else vx = (RandomNumberGenerator() & 0x3FFF) + 0x4000; } \
        else { offset = 0x230000; if (REVERSE) vx = (RandomNumberGenerator() & 0x3FFF) + 0x4000; else vx = -0x4000 - (RandomNumberGenerator() & 0x3FFF); } \
        vy = -0x4000 - (RandomNumberGenerator() & 0x3FFF);\
        RandomNumberGenerator(); zero = 0; \
        Effect_SpawnParticleWithDamping(entity->posX.whole + offset, entity->posY.whole, vx, vy, entity->spriteLayer - 1, zero, 0x10, 0xF, 6); \
    } \
    if (!(TIMER & 7)) { if (TIMER & 8) PlaySong(0x118); else if ((RandomNumberGenerator() & 0x7F) <= 0x3F) PlaySong(0x118); }
/* Keep gravity outside the pose macro: duplicating it inside PROBES changes
 * which copies of the branch tails agbcc merges. */
#define BACKWARD_POSE() \
    SPEED() \
    entity->velX.whole = -entity->velX.whole; \
    PHASE = 10; FRAME = 9; entity->ext.creakingSkull.poseDelay = TIMER >> 1; \
    POSE(0x1000, 0x1800)
#define BACKWARD_EXISTING() BACKWARD_POSE() GRAVITY()
#define BACKWARD() TIMER = times0[entity->ext.creakingSkull.variant]; BACKWARD_EXISTING()
#define STOP() \
    TIMER = times0[entity->ext.creakingSkull.variant]; entity->velX.whole = 0; PHASE = 7; FRAME = 8; \
    POSE(-0x1000, 0x800) \
    GRAVITY()
#define FORWARD() \
    TIMER = times0[entity->ext.creakingSkull.variant]; PHASE = 1; FRAME = 9; entity->ext.creakingSkull.poseDelay = TIMER >> 1; \
    POSE(-0x1000, 0x1800) \
    SPEED()
#define PROBES(BLOCKED) \
    if (entity->flipX) offset = 50; else offset = -50; \
    DISTANCE() \
    if (distance <= 49 || TERRAIN()) \
    { \
        if (entity->flipX) offset = -50; else offset = 50; \
        if (!TERRAIN()) { BLOCKED } \
    } \
    GRAVITY()
#define ATTACK(S) entity->state = 1; PHASE = S;
/**
 * @brief Movement state (0): coordinate joint poses for strides, pauses,
 * turns and attack selection. Shared with Giant Skeleton; retain the
 * unused stack copy, extra RNG call and macro/temporary structure.
 */
void CreakingSkull_UpdateMovement(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    u8 times0[2], times1[2], times2[2], times3[2];
    s8 chain[13];
    u8 unused[2];
    s32 distance, offset, direction;
    s32 vx, vy;
    memcpy(times0, sCreakingSkullTimes0, 2);
    memcpy(times1, sCreakingSkullTimes1, 2);
    memcpy(times2, sCreakingSkullTimes2, 2);
    memcpy(times3, sUnk_08119076, 2);
    memcpy(chain, sUnk_08119063, 13);
    /* Retain the original unused stack copy. */
    memcpy(unused, sUnk_08119076, 2);
    switch (PHASE)
    {
    case 0:
        CreakingSkull_FaceTarget(entity);
        if (Target_IsNearEntity(entity, 200, 160) || Enemy_IsDamaged(entity))
        {
            PHASE = 2; SUBPHASE = 0; TIMER = times0[entity->ext.creakingSkull.variant]; FRAME = 5;
            CreakingSkull_AttachJointSetAngle(entity, 12, 11, -0x1000, 0x1F0000);
            CreakingSkull_AttachJointSetAngle(entity, 1, 0, -0x800, 0x1E0000);
            CreakingSkull_AttachJointSetAngle(entity, 2, 1, -0x1000, 0x1C0000);
            SPEED() GRAVITY()
        }
        break;
    case 1:
        UPDATE() FALL()
        if ((s8)--entity->ext.creakingSkull.poseDelay <= 0) FRAME = 4;
        DEBRIS(0)
        COUNTDOWN()
        TIMER = times0[entity->ext.creakingSkull.variant]; PHASE++; FRAME = 10; entity->ext.creakingSkull.poseDelay = TIMER >> 1;
        POSE(-0x1000, -0x1800)
        break;
    case 2:
        UPDATE() FALL()
        if ((s8)--entity->ext.creakingSkull.poseDelay <= 0) FRAME = 5;
        DEBRIS(0)
        if ((s8)--TIMER > 4) break;
        TIMER = times2[entity->ext.creakingSkull.variant]; PHASE++; SUBPHASE = 0; FRAME = 5;
        break;
    case 3:
        UPDATE() COUNTDOWN()
        if (SUBPHASE == 0)
        {
            SUBPHASE++; TIMER = 1;
            CreakingSkull_AttachJointSetAngle(entity, 12, 11, 0x13000, 0x1F0000);
            CreakingSkull_AttachJointSetAngle(entity, 1, 0, 0x17000, 0x1E0000);
            CreakingSkull_AttachJointSetAngle(entity, 2, 1, 0x1D000, 0x1C0000);
            break;
        }
        if (entity->enemyId == 0x21 && (RandomNumberGenerator2() & 0x7F) <= 0x1F)
        { ATTACK(20) break; }
        DISTANCE()
        if (distance <= 99)
        {
            if ((RandomNumberGenerator2() & 0x7F) <= 0x3F) { ATTACK(0) break; }
            if ((RandomNumberGenerator2() & 0x7F) <= 0x3F)
            { CreakingSkull_FaceTarget(entity); BACKWARD() break; }
        }
        direction = Entity_GetHorizontalRoomEdge(entity, 24);
        if (direction > 0)
        {
            direction--; CreakingSkull_FaceTarget(entity);
            if ((direction == 0 && !entity->flipX) || (direction == 1 && entity->flipX))
            { BACKWARD() break; }
        }
        CreakingSkull_FaceTarget(entity);
        TIMER = times1[entity->ext.creakingSkull.variant]; PHASE++; FRAME = 8;
        POSE(0x1000, 0x800)
        entity->velX.whole = 0;
        PROBES(TIMER = times0[entity->ext.creakingSkull.variant]; BACKWARD_POSE())
        break;
    case 4:
        UPDATE() FALL() COUNTDOWN()
        TIMER = times1[entity->ext.creakingSkull.variant]; PHASE++; FRAME = 7;
        POSE(0x1000, -0x800)
        break;
    case 5:
        UPDATE() FALL()
        if ((s8)--TIMER > 4) break;
        TIMER = times3[entity->ext.creakingSkull.variant]; PHASE++; SUBPHASE = 0; FRAME = 7;
        PlaySong(SE_HEAVY_STEP);
        break;
    case 6:
        UPDATE() COUNTDOWN()
        if (SUBPHASE == 0)
        {
            SUBPHASE++; TIMER = 1;
            CreakingSkull_AttachJointSetAngle(entity, 12, 11, 0x15000, 0x1F0000);
            CreakingSkull_AttachJointSetAngle(entity, 1, 0, 0x17000, 0x1E0000);
            CreakingSkull_AttachJointSetAngle(entity, 2, 1, 0x1D000, 0x1C0000);
            break;
        }
        if (entity->enemyId == 0x21 && (RandomNumberGenerator2() & 0x7F) <= 0x1F)
        { ATTACK(20) break; }
        DISTANCE()
        if (distance <= 99)
        {
            if ((RandomNumberGenerator2() & 0x7F) <= 0x3F) { ATTACK(0) break; }
            if ((RandomNumberGenerator2() & 0x7F) <= 0x3F)
            { CreakingSkull_FaceTarget(entity); STOP() break; }
        }
        direction = Entity_GetHorizontalRoomEdge(entity, 24);
        if (direction > 0)
        {
            direction--; CreakingSkull_FaceTarget(entity);
            if ((direction == 0 && !entity->flipX) || (direction == 1 && entity->flipX))
            { STOP() break; }
        }
        CreakingSkull_FaceTarget(entity);
        FORWARD()
        PROBES(entity->velX.whole = 0; PHASE = 7; FRAME = 8; POSE(-0x1000, 0x800))
        break;
    case 7:
        UPDATE() FALL() COUNTDOWN()
        TIMER = times0[entity->ext.creakingSkull.variant]; PHASE++; FRAME = 5;
        POSE(-0x1000, -0x800)
        break;
    case 8:
        UPDATE() FALL()
        if ((s8)--TIMER > 4) break;
        TIMER = times2[entity->ext.creakingSkull.variant]; PHASE++; SUBPHASE = 0; FRAME = 5;
        break;
    case 9:
        UPDATE() COUNTDOWN()
        if (SUBPHASE == 0)
        {
            SUBPHASE++; TIMER = 1;
            CreakingSkull_AttachJointSetAngle(entity, 12, 11, 0x13000, 0x1F0000);
            CreakingSkull_AttachJointSetAngle(entity, 1, 0, 0x17000, 0x1E0000);
            CreakingSkull_AttachJointSetAngle(entity, 2, 1, 0x1D000, 0x1C0000);
            break;
        }
        if ((RandomNumberGenerator2() & 0x7F) <= 0x3F)
        { DISTANCE() if (distance <= 99) { ATTACK(0) break; } }
        else if ((RandomNumberGenerator2() & 0x7F) <= 0x1F && entity->enemyId == 0x21)
        { ATTACK(20) break; }
        CreakingSkull_FaceTarget(entity);
        TIMER = times0[entity->ext.creakingSkull.variant]; PHASE = 4; FRAME = 8;
        POSE(0x1000, 0x800)
        entity->velX.whole = 0;
        PROBES(BACKWARD_POSE())
        break;
    case 10:
        UPDATE() FALL()
        if ((s8)--entity->ext.creakingSkull.poseDelay <= 0) FRAME = 4;
        DEBRIS(1)
        COUNTDOWN()
        TIMER = times0[entity->ext.creakingSkull.variant]; PHASE++; FRAME = 12; entity->ext.creakingSkull.poseDelay = TIMER >> 1;
        POSE(0x1000, -0x1800)
        break;
    case 11:
        UPDATE() FALL()
        if ((s8)--entity->ext.creakingSkull.poseDelay <= 0) FRAME = 7;
        DEBRIS(1)
        if ((s8)--TIMER > 4) break;
        TIMER = times2[entity->ext.creakingSkull.variant]; PHASE++; FRAME = 7;
        break;
    case 12:
        UPDATE() COUNTDOWN()
        if (SUBPHASE == 0)
        {
            SUBPHASE++; TIMER = 1;
            CreakingSkull_AttachJointSetAngle(entity, 12, 11, 0x15000, 0x1F0000);
            CreakingSkull_AttachJointSetAngle(entity, 1, 0, 0x17000, 0x1E0000);
            CreakingSkull_AttachJointSetAngle(entity, 2, 1, 0x1D000, 0x1C0000);
            break;
        }
        if ((RandomNumberGenerator2() & 0x7F) <= 0x3F)
        { DISTANCE() if (distance <= 99) { ATTACK(0) break; } }
        else if ((RandomNumberGenerator2() & 0x7F) <= 0x1F && entity->enemyId == 0x21)
        { ATTACK(20) break; }
        CreakingSkull_FaceTarget(entity);
        FORWARD()
        PROBES(entity->velX.whole = 0; PHASE = 7; FRAME = 8; POSE(-0x1000, 0x800))
        break;
    case 20:
        CreakingSkull_FaceTarget(entity);
        POSE(0, 0)
        TIMER = 14; FRAME = 1; PHASE++; SUBPHASE = 0;
        /* Fall through. */
    case 21:
        UPDATE() COUNTDOWN()
        PHASE++; TIMER = 24; FRAME = 2;
        break;
    case 22:
        UPDATE() COUNTDOWN()
        PHASE++; TIMER = 14; FRAME = 3;
        break;
    case 23:
        UPDATE() COUNTDOWN()
        PHASE++; TIMER = 24; FRAME = 0;
        break;
    case 24:
        UPDATE() COUNTDOWN()
        CreakingSkull_FaceTarget(entity);
        CreakingSkull_AttachJointSetAngle(entity, 12, 11, 0x14000, 0x1F0000);
        CreakingSkull_AttachJointSetAngle(entity, 1, 0, 0x17800, 0x1E0000);
        CreakingSkull_AttachJointSetAngle(entity, 2, 1, 0x1E000, 0x1C0000);
        PHASE = 21; TIMER = 14; FRAME = 1;
        if (Target_IsNearEntity(entity, 250, 160) || Enemy_IsDamaged(entity) == 1)
        {
            PHASE = 2; SUBPHASE = 0; TIMER = times0[entity->ext.creakingSkull.variant]; FRAME = 5;
            CreakingSkull_AttachJointSetAngle(entity, 12, 11, -0x1000, 0x1F0000);
            CreakingSkull_AttachJointSetAngle(entity, 1, 0, -0x800, 0x1E0000);
            CreakingSkull_AttachJointSetAngle(entity, 2, 1, -0x1000, 0x1C0000);
            SPEED() GRAVITY()
        }
        break;
    }
}

/* Creaking Skull main state machine: arm swings, ground smash debris, bone spit. */

#undef ARMS
#define ARMS(A, B, C, D)\
    CreakingSkull_AttachJointSetAngle(entity, 1, 0, A, 0x1E0000);\
    CreakingSkull_AttachJointSetAngle(entity, 2, 1, B, 0x1C0000);

#undef COUNTDOWN
#define COUNTDOWN()\
    CreakingSkull_InterpolateJointsInOrder(entity, chain);\
    if ((s8)--entity->timer > 0)\
        break;

/* Smash: three debris bursts along the arm. */
#define SMASH_DEBRIS(BX)\
    for (i = 0; i <= 2; i++)\
    {\
        s32 bx, vx, vy, rvx, rvy;\
\
        if (entity->flipX)\
        {\
            bx = BX;\
            step = 0xC0000;\
            vx = (RandomNumberGenerator() & 0x7FFF) + 0x10000;\
        }\
        else\
        {\
            bx = -(BX);\
            step = -0xC0000;\
            vx = -(RandomNumberGenerator() & 0x7FFF) + -0x10000;\
        }\
        vy = -0x8000 - (RandomNumberGenerator() & 0x7FFF);\
        Effect_SpawnParticleWithDamping(entity->posX.whole + bx + i * step, entity->posY.whole, vx, vy,\
                     entity->spriteLayer - 1, 0, 8, 0xF, 6);\
        rvx = (RandomNumberGenerator() & 0xFFFF) + -0x8000;\
        rvy = -(RandomNumberGenerator() & 0xFFFF) + -0x30000;\
        Effect_SpawnFallingPiece(entity->posX.whole + bx + i * step, entity->posY.whole, rvx, rvy,\
                     entity->spriteLayer - 1, RandomNumberGenerator() & 1);\
    }

/* Dragging the arm: dust along it and one rock. */
#define DRAG_DEBRIS(BX)\
    if (!(entity->timer & 3))\
        ScreenShake_RequestOffset(0, 0, 1);\
    if (entity->timer > 3)\
    {\
        for (i = 0; i <= 3; i++)\
        {\
\
\
            if (entity->flipX)\
            {\
                bx = BX;\
                step = 0xC0000;\
            }\
            else\
            {\
                bx = -(BX);\
                step = -0xC0000;\
            }\
            vx = (RandomNumberGenerator() & 0xFFFF) + -0x8000;\
            vy = -0x8000 - (RandomNumberGenerator() & 0xFFFF);\
            Effect_SpawnParticleWithDamping(entity->posX.whole + bx + i * step, entity->posY.whole, vx, vy,\
                         entity->spriteLayer - 1, 0, 0x10, 0xF, 6);\
        }\
        for (i = 0; i <= 0; i++)\
        {\
                s32 base, vx, vy;\
\
            if (entity->flipX)\
            {\
                bx = 0x700000;\
                step = 0x100000;\
                base = 0x8000;\
            }\
            else\
            {\
                bx = -0x700000;\
                step = -0x100000;\
                base = -0x8000;\
            }\
            vx = (RandomNumberGenerator() & 0xFFFF) + ({ s32 t = base + -0x8000; t; });\
            vy = -(RandomNumberGenerator() & 0xFFFF) - ((entity->timer - 2) << 16);\
            Effect_SpawnFallingPiece(entity->posX.whole + bx + i * step, entity->posY.whole, vx, vy,\
                         entity->spriteLayer - 1, RandomNumberGenerator() & 1);\
        }\
    }

/* Spit bones from the mouth. */
#define SPIT_BONES()\
    if (Entity_GetVisibilityPattern(entity->timer) == 1)\
    {\
        register s32 y0 asm("r5"); /* FAKEMATCH: pinned register */ \
        s32 x1, y1;\
\
        if (entity->flipX)\
        {\
            step = entity->posX.whole + -0xB0000;\
            bx = step - RandomNumberGenerator() % 0x280000 + 0xC0000;\
            x1 = entity->posX.whole + -0xF0000;\
        }\
        else\
        {\
            step = entity->posX.whole + 0xB0000;\
            bx = step + RandomNumberGenerator() % 0x280000 + -0xC0000;\
            x1 = entity->posX.whole + 0xF0000;\
        }\
        by = entity->posY.whole; y0 = by + -0x2B0000;\
        y1 = y0 - (RandomNumberGenerator() & 0x1FFFFF) + -0x80000;\
        vy = entity->posY.whole + -0x2F0000;\
        CreakingSkull_SpawnGuidedEffect(entity, step, y0, x1, vy, bx, y1);\
    }

/* Breath puffs, then the fireball. */
#define BREATH(SONG)\
    if (Entity_GetVisibilityPattern(entity->timer) == 1)\
    {\
\
        if (entity->flipX)\
            bx = RandomNumberGenerator() % 0x60000 + entity->posX.whole + 0x1D0000;\
        else\
            bx = RandomNumberGenerator() % 0x60000 + entity->posX.whole + -0x230000;\
        by = RandomNumberGenerator() % 0x60000 + entity->posY.whole; Effect_SpawnParticleAnim11(bx, by + -0x1F0000, 0, 0, entity->spriteLayer - 1);\
    }\
    if (!(entity->timer & 7))\
        PlaySong(SONG);\
    if ((s8)--entity->timer > 0)\
        break;\
    {\
        s32 vx;\
\
        if (entity->flipX)\
        {\
            bx = 0x240000;\
            vx = 0x30000;\
        }\
        else\
        {\
            bx = -0x240000;\
            vx = -0x30000;\
        }\
        CreakingSkull_SpawnProjectile(entity, bx, -0x190000, vx, 0);\
    }

#define RECOVER(STATE)\
    entity->timer = 1;\
    entity->subState = STATE;\
    entity->stage = 0;\
    entity->state = 0;\
    CreakingSkull_RestoreJointPositions(entity);\
    ARMS(0x17000, 0x1D000, , )\
    CreakingSkull_InterpolateJointsInOrder(entity, chain);

/**
 * @brief Attack state (1): coordinate arm swings, ground-smash/drag effects
 * and ranged attack sequences through joint poses and variant timing
 * tables. Preserve macros, signed counters and pinned registers.
 */
void CreakingSkull_UpdateAttack(struct EwramData_EntityData *entity)
{
    u8 swingTimes[3];
    u8 armTimes[3];
    u8 waitTimes[3];
    s8 spitTimes[3];
    s8 chain[13];
    s32 i;
    s32 step;
    s32 vx, vy;
    register s32 bx asm("r9"); /* FAKEMATCH: pinned register */
    s32 zero;
    s32 by;

    memcpy(swingTimes, sCreakingSkullSwingTimes, sizeof(swingTimes));
    memcpy(armTimes, sCreakingSkullArmTimes, sizeof(armTimes));
    memcpy(waitTimes, sCreakingSkullSwingTimes, sizeof(waitTimes));
    memcpy(spitTimes, sCreakingSkullSpitTimes, sizeof(spitTimes));
    memcpy(chain, sUnk_08119063, sizeof(chain));

    switch (entity->subState)
    {
    case 0:
        CreakingSkull_FaceTarget(entity);
        CreakingSkull_SaveJointPositions(entity);
        if ((s8)entity->ext.joint.pose == 5)
        {
            zero = 0;
            entity->ext.joint.pose = 0xD;
            entity->subState = 1;
        }
        else
        {
            zero = 0;
            entity->ext.joint.pose = 0x11;
            entity->subState = 0xB;
        }
        CreakingSkull_AttachJointSetAngle(entity, 0xC, 0xB, 0, 0x1F0000);
        ARMS(0x5000, 0x1000, , )
        entity->ext.jointAttack.phase = zero;
        entity->timer = swingTimes[entity->ext.creakingSkull.variant];
        break;
    case 1:
        COUNTDOWN()
        entity->ext.joint.pose = 0xE;
        entity->subState++;
        entity->timer = armTimes[entity->ext.creakingSkull.variant];
        ARMS(0x200, 0x200, , )
        break;
    case 2:
        COUNTDOWN()
        entity->timer = 4;
        entity->subState++;
        ARMS(0, 0, , )
        break;
    case 3:
        COUNTDOWN()
        entity->timer = 0xE;
        entity->subState++;
        entity->ext.joint.pose = 0xF;
        ARMS(-0x5A00, -0x7200, , )
        PlaySong(0x86);
        break;
    case 4:
        COUNTDOWN()
        entity->timer = 6;
        entity->subState++;
        entity->ext.joint.pose = 0x10;
        ARMS(0, 0x400, , )
        SMASH_DEBRIS(0x4C0000)
        ScreenShake_RequestOffset(0, 1, 2);
        PlaySong(0x80);
        break;
    case 5:
        CreakingSkull_InterpolateJointsInOrder(entity, chain);
        DRAG_DEBRIS(0x4C0000)
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = 6;
        entity->subState++;
        entity->ext.joint.pose = 0xF;
        ARMS(0, -0x400, , )
        break;
    case 6:
        COUNTDOWN()
        entity->timer = waitTimes[entity->ext.creakingSkull.variant];
        entity->subState++;
        entity->ext.joint.pose = 0xF;
        ARMS(0, 0, , )
        break;
    case 7:
        COUNTDOWN()
        entity->timer = 0x32;
        entity->subState++;
        entity->ext.joint.pose = 5;
        ARMS(0x800, 0x6000, , )
        break;
    case 11:
        COUNTDOWN()
        entity->ext.joint.pose = 0x12;
        entity->subState++;
        entity->timer = armTimes[entity->ext.creakingSkull.variant];
        ARMS(0x200, 0x200, , )
        break;
    case 12:
        COUNTDOWN()
        entity->timer = 2;
        entity->subState++;
        ARMS(0, 0, , )
        break;
    case 13:
        COUNTDOWN()
        entity->timer = 0xE;
        entity->subState++;
        entity->ext.joint.pose = 0x13;
        ARMS(-0x5A00, -0x7200, , )
        PlaySong(0x86);
        break;
    case 14:
        COUNTDOWN()
        entity->timer = 6;
        entity->subState++;
        entity->ext.joint.pose = 0x14;
        ARMS(0, 0x400, , )
        SMASH_DEBRIS(0x4C0000)
        ScreenShake_RequestOffset(0, 1, 2);
        PlaySong(0x80);
        break;
    case 15:
        CreakingSkull_InterpolateJointsInOrder(entity, chain);
        DRAG_DEBRIS(0x500000)
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = 6;
        entity->subState++;
        entity->ext.joint.pose = 0x13;
        ARMS(0, -0x400, , )
        break;
    case 16:
        COUNTDOWN()
        entity->timer = waitTimes[entity->ext.creakingSkull.variant];
        entity->subState++;
        entity->ext.joint.pose = 0x13;
        ARMS(0, 0, , )
        break;
    case 17:
        COUNTDOWN()
        entity->timer = 0x1E;
        entity->subState++;
        entity->ext.joint.pose = 7;
        ARMS(0x800, 0x6000, , )
        break;
    case 8:
    case 18:
        CreakingSkull_InterpolateJointsInOrder(entity, chain);
        if ((s8)--entity->timer > 4)
            break;
        entity->timer = 0xA;
        entity->subState++;
        CreakingSkull_AttachJoint(entity, 1, 0, 0x17000, 0x1E0000);
        CreakingSkull_AttachJoint(entity, 2, 1, 0x1D000, 0x1C0000);
        break;
    case 19:
        COUNTDOWN()
        RECOVER(6)
        break;
    case 20:
        CreakingSkull_FaceTarget(entity);
        CreakingSkull_SaveJointPositions(entity);
        if ((s8)entity->ext.joint.pose == 5)
        {
            zero = 0;
            entity->ext.joint.pose = 0x16;
            entity->subState = 0x15;
        }
        else
        {
            zero = 0;
            entity->ext.joint.pose = 0x1A;
            entity->subState = 0x1F;
        }
        CreakingSkull_AttachJointSetAngle(entity, 0xC, 0xB, 0, 0x1F0000);
        ARMS(0, 0, , )
        entity->ext.jointAttack.phase = zero;
        entity->timer = spitTimes[entity->ext.creakingSkull.variant];
        entity->ext.creakingSkull.poseDelay = entity->timer / 2;
        break;
    case 21:
        if ((s8)--*(s8 *)&entity->ext.creakingSkull.poseDelay <= 0)
            entity->ext.joint.pose = 0x15;
        COUNTDOWN()
        entity->subState++;
        entity->timer = 0x29;
        PlaySong(0x77);
        break;
    case 22:
        SPIT_BONES()
        if (!(entity->timer & 7))
            PlaySong(0x84);
        if ((s8)--entity->timer > 0)
            break;
        entity->subState++;
        entity->timer = 0xA;
        entity->ext.creakingSkull.poseDelay = 5;
        entity->ext.joint.pose = 0x17;
        break;
    case 24:
        BREATH(0x12E)
        entity->timer = 3;
        entity->subState++;
        entity->ext.joint.pose = 0x18;
        PlaySong(0x7A);
        break;
    case 25:
        COUNTDOWN()
        entity->subState++;
        entity->timer = 0x1E;
        entity->ext.joint.pose = 0x17;
        break;
    case 26:
        COUNTDOWN()
        entity->ext.joint.pose = 5;
        entity->subState++;
        entity->timer = 0x3C;
        break;
    case 9:
    case 27:
        COUNTDOWN()
        RECOVER(3)
        break;
    case 31:
        if ((s8)--*(s8 *)&entity->ext.creakingSkull.poseDelay <= 0)
            entity->ext.joint.pose = 0x19;
        COUNTDOWN()
        entity->subState++;
        entity->timer = 0x29;
        PlaySong(0x77);
        break;
    case 32:
        SPIT_BONES()
        if (!(entity->timer & 7))
            PlaySong(0x84);
        if ((s8)--entity->timer > 0)
            break;
        entity->subState++;
        entity->timer = 0xA;
        entity->ext.creakingSkull.poseDelay = 5;
        entity->ext.joint.pose = 0x1B;
        break;
    case 23:
    case 33:
        COUNTDOWN()
        entity->subState++;
        entity->timer = 0x28;
        break;
    case 34:
        BREATH(0x77)
        entity->timer = 3;
        entity->subState++;
        entity->ext.joint.pose = 0x1C;
        PlaySong(0x7A);
        break;
    case 35:
        COUNTDOWN()
        entity->subState++;
        entity->timer = 0x1E;
        entity->ext.joint.pose = 0x1B;
        break;
    case 36:
        COUNTDOWN()
        entity->ext.joint.pose = 7;
        entity->subState++;
        entity->timer = 0x3C;
        break;
    case 37:
        COUNTDOWN()
        RECOVER(6)
        break;
    }
}

/**
 * @brief Bestiary state (3): cycle four poses through the 13-joint chain
 * with the stored timers, then reset the joint angles and repeat.
 */
void CreakingSkull_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
{
    s8 parts[13];

    memcpy(parts, sUnk_08119063, sizeof(parts));
    switch (entity->subState)
    {
    case 0:
        entity->subState = 20;
    case 20:
        entity->timer = 14;
        entity->ext.joint.pose = 1;
        entity->subState++;
    case 21:
        CreakingSkull_InterpolateJointsInOrder(entity, parts);
        if ((s8)--entity->timer > 0)
            break;
        entity->subState++;
        entity->timer = 0x18;
        entity->ext.joint.pose = 2;
        break;
    case 22:
        CreakingSkull_InterpolateJointsInOrder(entity, parts);
        if ((s8)--entity->timer > 0)
            break;
        entity->subState++;
        entity->timer = 14;
        entity->ext.joint.pose = 3;
        break;
    case 23:
        CreakingSkull_InterpolateJointsInOrder(entity, parts);
        if ((s8)--entity->timer > 0)
            break;
        entity->subState++;
        entity->timer = 0x18;
        entity->ext.joint.pose = 0;
        break;
    case 24:
        CreakingSkull_InterpolateJointsInOrder(entity, parts);
        if ((s8)--entity->timer > 0)
            break;
        CreakingSkull_AttachJointSetAngle(entity, 12, 11, 0x14000, 0x1F0000);
        CreakingSkull_AttachJointSetAngle(entity, 1, 0, 0x17800, 0x1E0000);
        CreakingSkull_AttachJointSetAngle(entity, 2, 1, 0x1E000, 0x1C0000);
        entity->subState = 21;
        entity->timer = 14;
        entity->ext.joint.pose = 1;
        break;
    }
}

/* Interpolate a Creaking Skull joint toward a pose or its hanging angle. */
/**
 * @brief Interpolate one of thirteen joints toward pose coordinates or a hanging angle using the entity timer; also interpolate the pose rotation of joint 2.
 */
void CreakingSkull_InterpolateJoint(struct EwramData_EntityData *entity, s32 index)
{
    struct EntityJointPart *parts = entity->ext.joint.parts;
    const u8 *pose = sCreakingSkullPartPoses[(s8)entity->ext.joint.pose];
    const s32 *angles = sCreakingSkullPartAngles[(s8)entity->ext.joint.pose];
    s32 x, y, dx, dy, a;
    struct EntityJointPart *part;
    if (parts[index].targetSwingAngle == 0 && parts[index].swingAngle == 0)
    {
        if (entity->flipX) x = -((s8)pose[index * 2] * 0x10000);
        else x = (s8)pose[index * 2] * 0x10000;
        y = (s8)pose[index * 2 + 1] * 0x10000;
        part = (struct EntityJointPart *)(index * sizeof(struct EntityJointPart) + (u32)parts);
        dx = part->offsetX;
        x -= dx;
        dy = part->offsetY;
        y -= dy;
        dx += x / entity->timer;
        part->offsetX = dx;
        dy += y / entity->timer;
        part->offsetY = dy;
        if (index == 2)
        {
            x = angles[0];
            if (entity->flipX) x = 0x20000 - x;
            a = parts[2].rotationAngle;
            x -= a;
            a += x / entity->timer;
            parts[2].rotationAngle = a;
        }
    }
    else
    {
        struct EntityJointPart *second = (struct EntityJointPart *)(index * sizeof(struct EntityJointPart) + (u32)parts);
        x = second->targetSwingAngle;
        a = second->swingAngle;
        x -= a;
        a += x / entity->timer;
        second->swingAngle = a;
        a += 0x4000;
        x = Sin(a);
        y = Sin(second->swingAngle);
        x = FixedMultiply(x, second->linkLength);
        y = FixedMultiply(y, second->linkLength);
        if (second->anchorXOrSentinel == (s32)0x80000000)
        {
            second->offsetX = (parts[second->anchorYOrPartIndex].offsetX & 0xFFFF0000) + x;
            second->offsetY = (parts[second->anchorYOrPartIndex].offsetY & 0xFFFF0000) + y;
        }
        else
        {
            second->offsetX = second->anchorXOrSentinel + x;
            second->offsetY = second->anchorYOrPartIndex + y;
        }
        if (index == 2)
        {
            x = angles[0];
            if (entity->flipX) x = 0x20000 - x;
            a = parts[2].rotationAngle;
            x -= a;
            a += x / entity->timer;
            parts[2].rotationAngle = a;
        }
    }
}

/* Mirror all thirteen joints when the facing direction changes.
 * Angles of joints 0..2, 8 and 11 also reflect around 0x20000. */
/**
 * @brief When target-facing changes, flip and mirror all thirteen joint X positions; reflect rotations for joints 0..2, 8 and 11.
 */
void CreakingSkull_FaceTarget(struct EwramData_EntityData *entity)
{
    s32 facing = entity->flipX;
    if (facing != Entity_IsTargetRight(entity))
    {
        struct EntityJointPart *p;
        struct EntityJointPart *parts;
        s32 turn;
        entity->flipX ^= 1;
        turn = 0x20000;
        parts = entity->ext.joint.parts;
        p = parts;
        do
        {
            p->offsetX = -p->offsetX;
            if (p <= parts + 2 || p == parts + 8 || p == parts + 11)
                p->rotationAngle = turn - p->rotationAngle;
            p++;
        /* The original loop tests the addresses with a signed branch. */
        } while ((s32)p <= (s32)(parts + 12));
    }
}

/* CreakingSkull_AttachJointSetAngle (chain joint set-up, variant). */
/**
 * @brief Bind a joint with supplied or measured length; only values above 0xFFFF are absolute, and that branch also replaces the current angle with the mirrored target.
 */
void CreakingSkull_AttachJointSetAngle(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length)
{
    struct EntityJointPart *parts = entity->ext.joint.parts;
    // &parts[partIndex], written as integer arithmetic (offset + base) as in
    // the original
    struct EntityJointPart *part = (struct EntityJointPart *)(partIndex * sizeof(struct EntityJointPart) + (u32)parts);
    s32 dx;
    s32 dy;
    s32 baseAngle;
    s32 angle;
    struct EntityJointPart *anglePart;

    part->anchorXOrSentinel = 0x80000000;
    part->anchorYOrPartIndex = anchorIndex;
    if (length != 0)
    {
        part->linkLength = length;
    }
    else
    {
        dx = parts[anchorIndex].offsetX - part->offsetX;
        dy = parts[anchorIndex].offsetY - part->offsetY;
        dx = FixedMultiply(dx, dx);
        dy = FixedMultiply(dy, dy);
        part->linkLength = (u32)(Sqrt(dx + dy) << 16) >> 8;
    }
    dx = parts[partIndex].offsetX - parts[anchorIndex].offsetX;
    dy = parts[partIndex].offsetY - parts[anchorIndex].offsetY;
    angle = ArcTan2(dx < 0 ? -(-dx >> 16) : dx >> 16, dy < 0 ? -(-dy >> 16) : dy >> 16);
    anglePart = (struct EntityJointPart *)(partIndex * sizeof(struct EntityJointPart) + (u32)parts);
    anglePart->swingAngle = baseAngle = (u16)angle + 0x10000;
    if (angleSpec > 0xFFFF)
    {
        if (entity->flipX)
        {
            if (angleSpec > 0x18000)
                anglePart->targetSwingAngle = 0x38000 - angleSpec;
            else
                anglePart->targetSwingAngle = 0x28000 - angleSpec;
        }
        else
        {
            anglePart->targetSwingAngle = angleSpec;
        }
        {
            struct EntityJointPart *absolutePart = (struct EntityJointPart *)(partIndex * sizeof(struct EntityJointPart) + (u32)parts);

            absolutePart->swingAngle = absolutePart->targetSwingAngle;
        }
    }
    else if (entity->flipX)
    {
        anglePart->targetSwingAngle = baseAngle - angleSpec;
    }
    else
    {
        anglePart->targetSwingAngle = baseAngle + angleSpec;
    }
}

/* CreakingSkull_AttachJoint (chain joint set-up) and CreakingSkull_ApplyResolvedDamage. */

/**
 * @brief Bind a joint to its parent with supplied or measured length; mirror the target angle by facing, treating absolute values above 0xFFFF in magnitude as absolute angles.
 */
void CreakingSkull_AttachJoint(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length)
{
    struct EntityJointPart *parts = entity->ext.joint.parts;
    // &parts[partIndex], written as integer arithmetic (offset + base) as in
    // the original
    struct EntityJointPart *part = (struct EntityJointPart *)(partIndex * sizeof(struct EntityJointPart) + (u32)parts);
    s32 dx;
    s32 dy;
    s32 baseAngle;
    s32 angle;
    struct EntityJointPart *anglePart;

    part->anchorXOrSentinel = 0x80000000;
    part->anchorYOrPartIndex = anchorIndex;
    if (length != 0)
    {
        part->linkLength = length;
    }
    else
    {
        dx = parts[anchorIndex].offsetX - part->offsetX;
        dy = parts[anchorIndex].offsetY - part->offsetY;
        dx = FixedMultiply(dx, dx);
        dy = FixedMultiply(dy, dy);
        part->linkLength = (u32)(Sqrt(dx + dy) << 16) >> 8;
    }
    dx = parts[partIndex].offsetX - parts[anchorIndex].offsetX;
    dy = parts[partIndex].offsetY - parts[anchorIndex].offsetY;
    angle = ArcTan2(dx < 0 ? -(-dx >> 16) : dx >> 16, dy < 0 ? -(-dy >> 16) : dy >> 16);
    anglePart = (struct EntityJointPart *)(partIndex * sizeof(struct EntityJointPart) + (u32)parts);
    anglePart->swingAngle = baseAngle = (u16)angle + 0x10000;
    if ((angleSpec < 0 ? -angleSpec : angleSpec) > 0xFFFF)
    {
        if (entity->flipX)
        {
            if (angleSpec > 0x18000)
                anglePart->targetSwingAngle = 0x38000 - angleSpec;
            else
                anglePart->targetSwingAngle = 0x28000 - angleSpec;
        }
        else
        {
            anglePart->targetSwingAngle = angleSpec;
        }
    }
    else if (entity->flipX)
    {
        anglePart->targetSwingAngle = baseAngle - angleSpec;
    }
    else
    {
        anglePart->targetSwingAngle = baseAngle + angleSpec;
    }
}

/**
 * @brief Apply already-resolved damage and hit flash to the root. On
 * defeat drop rewards, enter death state 2 and optionally start boss
 * defeat effects and clear selected-target tracking.
 */
void CreakingSkull_ApplyResolvedDamage(struct EwramData_EntityData *entity, s32 unused1, u16 *unused2, s32 damage)
{
    entity->hitFlashTimer = 8;
    entity->hp -= damage;
    Link_MarkEnemyDamaged();
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x6B);
        entity->palette = entity->basePalette;
        entity->timer = 0x64;
        entity->state = 2;
        entity->subState = 0;
        if (entity->spawnParam0 != 0)
        {
            Player_SetSelectedTarget(NULL);
            BossDefeatEffect_Spawn(entity->posX.whole, entity->posY.whole - 0x200000);
            BossRecoveryPickup_SpawnWide((s16)entity->posX.part16.integer, 0x20, 0xB, 0x60, 0);
        }
    }
}

/* Creaking Skull part update and death motion. */
struct SkullPartBox { s8 x, y, w, h; };
#define JOINT(e) ((s8)e->ext.jointPart.index)
#define FIX_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)
#define HAS_SPRITE(e) (e->ext.jointPart.index <= 2 || JOINT(e) == 8 || JOINT(e) == 11)

/**
 * @brief Follow the selected root joint, mirror palette/facing and
 * maintain per-part hit boxes and affine orientation. On root death
 * enter the existing joint-dependent breakup motion.
 */
void CreakingSkullPart_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    u8 timers[13];
    struct SkullBox box;
    struct EwramData_EntityData *parent = entity->ext.jointPart.owner;
    struct EntityJointPart *parts;
    s32 x, y, size;
    u16 angle;

    memcpy(timers, sCreakingSkullPartTimers, sizeof(timers));
    if (entity->state == 0)
    {
        if (parent != NULL && parent->updateFunc != NULL
            && parent->enemyId == entity->enemyId)
        {
            parts = parent->ext.joint.parts;
            entity->palette = parent->palette;
            entity->flipX = parent->flipX;
            entity->timer++;
            entity->posX.whole = parent->posX.whole + parts[JOINT(entity)].offsetX;
            entity->posY.whole = parent->posY.whole + parts[JOINT(entity)].offsetY;
            if (entity->enemyId == 0x5C && JOINT(entity) == 5)
                entity->hidden = 1;
            if (entity->ext.jointPart.index <= 1 || JOINT(entity) == 11)
            {
                x = parts[(s8)entity->ext.jointPart.anchor].offsetX;
                x -= parts[JOINT(entity)].offsetX;
                y = parts[(s8)entity->ext.jointPart.anchor].offsetY - parts[JOINT(entity)].offsetY;
                angle = ArcTan2(FIX_INT(x), FIX_INT(y));
                if (entity->flipX)
                    entity->velX.whole = angle;
                else
                    entity->velX.whole = angle - 0x8000;
                parts[JOINT(entity)].rotationAngle = entity->velX.whole;
            }
            else if (JOINT(entity) == 2 || JOINT(entity) == 8)
                entity->velX.whole = parts[JOINT(entity)].rotationAngle;
            if (parent->state == 2)
            {
                entity->state = 1;
                if (JOINT(entity) == 8)
                {
                    entity->state = 2;
                    entity->ext.creakingSkullPart.floorY = parent->posY.whole - 0xA0000;
                }
                entity->subState = 0;
                entity->ext.creakingSkullPart.angle = entity->velX.whole;
                entity->ext.creakingSkullPart.spin = (RandomNumberGenerator() & 0xFF) - 0x80;
                entity->accelY.whole = 0x1800;
                entity->accelX.whole = 0;
                entity->velX.whole = (RandomNumberGenerator() & 0x3FFF) - 0x2000;
                entity->velY.whole = 0;
                entity->timer = timers[JOINT(entity)];
                return;
            }
            if ((parent->ext.joint.hurtMask >> JOINT(entity)) & 1)
            {
                if (JOINT(entity) == 8)
                {
                    if (entity->flipX) box.x = -18;
                    else box.x = -20;
                    box.y = -22;
                    box.w = 30;
                    box.h = 32;
                    HitBox_SetAttack(entity, &box);
                    HitBox_SetBody(entity, &box);
                    entity->hit.flags &= (u8)~HIT_NO_DAMAGE;
                    entity->hit.flags &= (u8)~HIT_NO_ATTACK;
                    Hit_CheckWithPlayerSharedHistory(&result, entity, parent);
                }
                else if (JOINT(entity) == 1)
                {
                    size = (entity->timer & 3) * 12;
                    if (entity->flipX)
                    {
                        x = Sin(-(parts[JOINT(entity)].rotationAngle + 0x8000) + 0x4000);
                        y = Sin(-(parts[JOINT(entity)].rotationAngle + 0x8000));
                    }
                    else
                    {
                        x = Sin(parts[JOINT(entity)].rotationAngle + 0xC000);
                        y = Sin(parts[JOINT(entity)].rotationAngle + 0x8000);
                    }
                    box.x = FIX_INT(x * size) - 3;
                    box.y = FIX_INT(y * size) - 3;
                    box.w = 6;
                    box.h = 6;
                    HitBox_SetAttack(entity, &box);
                    entity->hit.flags &= (u8)~HIT_NO_ATTACK;
                    entity->hit.flags &= (u8)~HIT_NO_DAMAGE;
                    Hit_CheckWithPlayerSharedHistory(&result, entity, parent);
                }
                else if (JOINT(entity) == 2)
                {
                    size = ((entity->timer & 3) + 1) * 13;
                    if (entity->flipX)
                    {
                        x = Sin(-(parts[JOINT(entity)].rotationAngle + 0x8000) + 0x4000);
                        y = Sin(-(parts[JOINT(entity)].rotationAngle + 0x8000));
                    }
                    else
                    {
                        x = Sin(parts[JOINT(entity)].rotationAngle + 0xC000);
                        y = Sin(parts[JOINT(entity)].rotationAngle + 0x8000);
                    }
                    box.x = FIX_INT(x * size) - 3;
                    box.y = FIX_INT(y * size) - 3;
                    box.w = 6;
                    box.h = 6;
                    HitBox_SetAttack(entity, &box);
                    entity->hit.flags &= (u8)~HIT_NO_ATTACK;
                    entity->hit.flags |= HIT_NO_DAMAGE;
                    Hit_CheckWithPlayerSharedHistory(&result, entity, parent);
                }
            }
            else
            {
                entity->hit.flags |= HIT_NO_DAMAGE;
                entity->hit.flags |= HIT_NO_ATTACK;
            }
            if (HAS_SPRITE(entity))
                ObjAffine_Set(entity->velX.whole, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
            return;
        }
        if (HAS_SPRITE(entity)) ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
    else if (entity->state == 1)
    {
        if (entity->subState == 0)
        {
            entity->subState++;
            entity->scrollWithBg = 1;
        }
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if (entity->enemyId == 0x5C && JOINT(entity) == 5)
            entity->hidden = 1;
        if (entity->timer <= 50)
        {
            EntityAccelerateAndMove(entity, 0x60000);
            entity->ext.creakingSkullPart.angle += entity->ext.creakingSkullPart.spin;
        }
        if (parent->spawnParam0 == 0
            && (u8)(entity->timer - 20) <= 20 && (entity->timer & 3) == 0)
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x40000, 0x100000, 0x100000, entity->spriteLayer - 1);
        if (entity->ext.jointPart.index <= 2 || JOINT(entity) == 11)
            ObjAffine_Set(entity->ext.creakingSkullPart.angle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
        if ((s8)--entity->timer <= 0)
        {
            if (entity->ext.jointPart.index <= 2 || JOINT(entity) == 11)
                ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
        }
    }
    else if (entity->state == 2)
    {
        if (entity->subState == 0)
        {
            entity->subState++;
            entity->scrollWithBg = 1;
        }
        entity->hidden = 0;
        if (entity->subState == 1 && entity->timer <= 50)
        {
            EntityAccelerateAndMove(entity, 0x60000);
            if (entity->posY.whole >= entity->ext.creakingSkullPart.floorY)
            {
                entity->posY.whole = entity->ext.creakingSkullPart.floorY;
                entity->velY.whole = 0;
                entity->accelY.whole = 0;
                entity->velX.whole = 0;
                entity->accelX.whole = 0;
                entity->subState++;
            }
        }
        if (parent->spawnParam0 == 0
            && (u8)(entity->timer - 20) <= 20 && (entity->timer & 3) == 0)
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x40000, 0x100000, 0x100000, entity->spriteLayer - 1);
        if ((s8)--entity->timer <= 0) entity->timer = 1;
        if (entity->subState == 2)
        {
            entity->subState++;
            entity->stage = 0;
            entity->animFrame = 23;
            if (entity->flipX) entity->posX.whole += 0x110000;
            else entity->posX.whole -= 0x110000;
            entity->posY.whole += 0x80000;
        }
        else if (entity->subState == 3)
        {
            if (entity->stage == 0)
            {
                entity->stage++;
                PlaySong(SE_HEAVY_STEP);
            }
            if (entity->flipX)
            {
                if ((s32)entity->ext.creakingSkullPart.angle > 0xE000) entity->ext.creakingSkullPart.angle -= 0x400;
            }
            else if ((s32)entity->ext.creakingSkullPart.angle < 0x12000) entity->ext.creakingSkullPart.angle += 0x400;
        }
        ObjAffine_Set(entity->ext.creakingSkullPart.angle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    }
}

/* CreakingSkullGuidedEffect_Update. */
/**
 * @brief Move toward the first stored room point while the timer exceeds
 * 7, then toward the second point using frame 0x35. Delete at zero;
 * retain the original countdown divisions and unreachable branch form.
 */
void CreakingSkullGuidedEffect_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 dx;
    s32 dy;
    s32 n;
    s32 timer;
    u8 t;

    timer = entity->timer - 1;
    if ((s8)(entity->timer = timer) <= 0)
    {
        EntityDelete(entity);
        return;
    }
    t = timer;
    if (t > 7)
    {
        dx = entity->posX.whole + bg->xPos.whole;
        dx = entity->ext.creakingSkullGuide.earlyX - dx;
        dy = entity->posY.whole + bg->yPos.whole;
        dy = entity->ext.creakingSkullGuide.earlyY - dy;
        if (t != 1)
        {
            n = entity->timer;
            n--;
            n *= 2;
            dx /= n;
            dy /= n;
        }
    }
    else
    {
        entity->animFrame = 0x35;
        dx = entity->posX.whole + bg->xPos.whole;
        dx = entity->ext.creakingSkullGuide.lateX - dx;
        dy = entity->posY.whole + bg->yPos.whole;
        dy = entity->ext.creakingSkullGuide.lateY - dy;
        if (entity->timer != 1)
        {
            n = entity->timer;
            n--;
            dx /= n;
            dy /= n;
        }
    }
    entity->posX.whole += dx;
    entity->posY.whole += dy;
}

/* Emit eight particles and a projectile from the owner's given offset. */
/**
 * @brief Emit eight particles and create the ranged projectile at the
 * supplied owner-relative offset and velocity. Configure contact/hit
 * callbacks; preserve the existing Anim_Update(owner) call.
 */
void CreakingSkull_SpawnProjectile(struct EwramData_EntityData *owner, s32 x, s32 y, s32 vx, s32 vy)
{
    u32 box = 0x0808FCFC;
    s32 i, particleX, particleY, graphics;
    struct EwramData_EntityData *entity;
    for (i = 0; i < 8; i++)
    {
        particleY = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
        if (owner->flipX) particleX = -(RandomNumberGenerator() & 0x7FFF) - i * 0x3000;
        else particleX = (RandomNumberGenerator() & 0x7FFF) + i * 0x3000;
        Effect_SpawnParticleWithDamping(owner->posX.whole + x, owner->posY.whole + y, particleX, particleY, owner->spriteLayer - 1, 0, 20, 15, 6);
    }
    entity = EntityCreateInRange(ENTITY_SLOT_OBJECT_FIRST, ENTITY_SLOT_OBJECT_LAST, CreakingSkullProjectile_Update);
    if (entity != NULL)
    {
        graphics = ObjGfx_Find(sCreakingSkullGfx);
        if (graphics < 0)
        {
            EntityDelete(entity);
            return;
        }
        Sprite_Attach(entity, sCreakingSkullSpriteSheet, graphics, owner->basePalette);
        Anim_StartIndexed(entity, 0, 0, 0);
        Anim_Update(owner);
        entity->drawFunc = Sprite_DrawEntity;
        Hit_Enable(entity, 10, CreakingSkullProjectile_DamagePlayerOnContact, CreakingSkullProjectile_OnHit);
        HitBox_SetAttack(entity, &box);
        HitBox_SetBody(entity, &box);
        entity->hit.flags &= (u8)~HIT_NO_DAMAGE;
        entity->hit.flags &= (u8)~HIT_NO_ATTACK;
        entity->scrollWithBg = 1;
        entity->spriteLayer = owner->spriteLayer - 1;
        ENTITY_OWNER(entity) = owner;
        entity->posX.whole = owner->posX.whole + x;
        entity->posY.whole = owner->posY.whole + y;
        entity->velX.whole = vx;
        entity->velY.whole = vy;
        entity->flipX = owner->flipX;
        entity->attack = owner->attack;
    }
}

/* Spinning projectile (CreakingSkullProjectile_Update): moves, leaves a trail every other
 * frame (Effect_SpawnParticleAnim11) in front of/behind its layer by angle; state 1
 * bursts into six random sparks and deletes itself. */

/**
 * @brief Move the projectile with its animated trail and contact checks;
 * on hit state 1 emit six effects and remove. Delete when outside
 * the screen during normal motion.
 */
void CreakingSkullProjectile_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    s32 vy, k;
    s32 r;
    s32 a, b;
    s32 i;

    switch (entity->state)
    {
    case 0:
        if (entity->animScriptIndex == 0 && entity->animDone)
            Anim_StartIndexed(entity, 1, 0, 1);
        EntityMove(entity);
        if (EntityIsOutsideScreen(entity, 1))
        {
            EntityDelete(entity);
            break;
        }
        Anim_Update(entity);
        Hit_CheckWithPlayer(&result, entity);
        entity->timer++;
        if (entity->timer & 1)
            break;
        vy = Sin(entity->ext.creakingSkullProjectile.orbit) << 1;
        b = Sin(entity->ext.creakingSkullProjectile.orbit) << 1;
        b += 0x20000;
        if (entity->flipX)
        {
            a = -Sin(entity->ext.creakingSkullProjectile.orbit + 0x4000) * 4;
            a += 0xC0000;
        }
        else
        {
            a = Sin(entity->ext.creakingSkullProjectile.orbit + 0x4000) * 4;
            a -= 0xC0000;
        }
        r = (u16)entity->ext.creakingSkullProjectile.orbit;
        if (r > 0x4000 && r < 0xC000)
            Effect_SpawnParticleAnim11(entity->posX.whole + a, entity->posY.whole + b, 0, vy, entity->spriteLayer + 1);
        else
            Effect_SpawnParticleAnim11(entity->posX.whole + a, entity->posY.whole + b, 0, vy, entity->spriteLayer - 1);
        entity->ext.creakingSkullProjectile.orbit += 0x2400;
        break;
    case 1:
        for (i = 0; i < 3; i++)
        {
            r = RandomNumberGenerator() & 0xFFFF;
            a = Sin(r + 0x4000);
            b = Sin(r);
            a = FixedMultiply(a, 0x10000);
            b = FixedMultiply(b, 0x10000);
            Effect_SpawnFadingParticle(entity->posX.whole, entity->posY.whole, a, b, entity->spriteLayer, 1);
            r = RandomNumberGenerator() & 0xFFFF;
            a = Sin(r + 0x4000);
            b = Sin(r);
            a = FixedMultiply(a, 0x10000);
            b = FixedMultiply(b, 0x10000);
            Effect_SpawnParticleAnim0(entity->posX.whole, entity->posY.whole, a, b, entity->spriteLayer);
        }
        EntityDelete(entity);
        break;
    }
}

/* Creaking Skull body-part helpers 0x08098BC0..0x08098C83 (13 parts of
 * struct EntityJointPart, same code as the Stolas helpers in enemy/stolas.c). */

#define CREAKING_SKULL_PART_COUNT 13

/**
 * @brief When timer is nonzero, interpolate joints 0..12 in numerical order.
 */
void CreakingSkull_InterpolateJoints(struct EwramData_EntityData *entity)
{
    s32 i;

    if (entity->timer != 0)
    {
        for (i = 0; i < CREAKING_SKULL_PART_COUNT; i++)
            CreakingSkull_InterpolateJoint(entity, i);
    }
}

/**
 * @brief When timer is nonzero, interpolate thirteen joints in the supplied signed-byte index order.
 */
void CreakingSkull_InterpolateJointsInOrder(struct EwramData_EntityData *entity, s8 *order)
{
    s32 i;

    if (entity->timer != 0)
    {
        for (i = 0; i < CREAKING_SKULL_PART_COUNT; i++)
            CreakingSkull_InterpolateJoint(entity, order[i]);
    }
}

/**
 * @brief Clear the selected joint target and current hanging-angle fields.
 */
void CreakingSkull_ClearJointAngles(struct EwramData_EntityData *entity, s32 partIndex)
{
    struct EntityJointPart *parts = entity->ext.joint.parts;

    parts[partIndex].targetSwingAngle = 0;
    parts[partIndex].swingAngle = 0;
}

/**
 * @brief Save X/Y positions of all thirteen joints into their backup fields.
 */
void CreakingSkull_SaveJointPositions(struct EwramData_EntityData *entity)
{
    struct EntityJointPart *parts = entity->ext.joint.parts;
    s32 i;

    for (i = CREAKING_SKULL_PART_COUNT - 1; i >= 0; parts++, i--)
    {
        parts->savedOffsetX = parts->offsetX;
        parts->savedOffsetY = parts->offsetY;
    }
}

/**
 * @brief Restore X/Y positions of all thirteen joints from their backup fields.
 */
void CreakingSkull_RestoreJointPositions(struct EwramData_EntityData *entity)
{
    struct EntityJointPart *parts = entity->ext.joint.parts;
    s32 i;

    for (i = CREAKING_SKULL_PART_COUNT - 1; i >= 0; parts++, i--)
    {
        parts->offsetX = parts->savedOffsetX;
        parts->offsetY = parts->savedOffsetY;
    }
}

/**
 * @brief Apply player contact damage using the entity attack value, zero effect fields and reaction field 1 at its current position.
 */
void CreakingSkull_OnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/* Creaking Skull helpers 0x08098C84..0x08098F03 (Giant Skeleton uses the
 * Creaking Skull functions). */

/**
 * @brief Resolve root damage at the struck part position while restoring
 * root coordinates, display damage on that part and pass it to the
 * root resolved-damage helper.
 */
void CreakingSkullPart_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    struct EwramData_EntityData *skull = entity->ext.jointPart.owner;
    s32 oldX = skull->posX.whole;
    s32 oldY = skull->posY.whole;
    s32 damage;

    skull->posX.whole = entity->posX.whole;
    skull->posY.whole = entity->posY.whole;
    damage = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(entity->ext.jointPart.owner, (struct EwramData_EntityData *)attacker, hit, damage);
    skull->posX.whole = oldX;
    skull->posY.whole = oldY;
    Entity_ShowDamage(entity, (s16)damage, 0);
    CreakingSkull_ApplyResolvedDamage(entity->ext.jointPart.owner, attacker, hit, (s16)damage);
}

/**
 * @brief Create a frame-0x66 effect at the supplied position with two
 * stored BG1-relative room targets, owner facing and countdown 20.
 * Exact visual identity remains unresolved.
 */
void CreakingSkull_SpawnGuidedEffect(struct EwramData_EntityData *entity, s32 lateTargetX, s32 lateTargetY, s32 earlyTargetX, s32 earlyTargetY, s32 x, s32 y)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    struct EwramData_EntityData *child = EntityCreateInRange(ENTITY_SLOT_EFFECT_FIRST, ENTITY_SLOT_EFFECT_LAST, CreakingSkullGuidedEffect_Update);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = x;
    child->posY.whole = y;
    child->spriteLayer = entity->spriteLayer - 1;
    child->ext.creakingSkullGuide.lateX = lateTargetX + bg->xPos.whole;
    child->ext.creakingSkullGuide.lateY = lateTargetY + bg->yPos.whole;
    child->ext.creakingSkullGuide.earlyX = bg->xPos.whole + earlyTargetX;
    child->ext.creakingSkullGuide.earlyY = earlyTargetY + bg->yPos.whole;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(child, sCommonObjectSpriteSheet, gfx, 7);
    child->animFrame = 0x66;
    child->flipX = entity->flipX;
    child->timer = 0x14;
    child->scrollWithBg = 1;
    child->drawFunc = Sprite_DrawEntity;
}

/**
 * @brief Apply stored damage with effect type 0 and flags 1, then
 * select burst state 1 and play sound 0x76.
 */
void CreakingSkullProjectile_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
    entity->state = 1;
    PlaySong(SE_PROJECTILE_IMPACT);
}

/**
 * @brief Select burst state 1 and play sound 0x76.
 */
void CreakingSkullProjectile_OnHit(struct EwramData_EntityData *entity)
{
    entity->state = 1;
    PlaySong(SE_PROJECTILE_IMPACT);
}

/**
 * @brief Load shared graphics and select palette variant 0 for enemy
 * 0x21 or 1 for 0x5C. Return 0 on graphics-slot failure, otherwise
 * attach the sprite, set its draw callback and return 1.
 */
u8 CreakingSkull_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sCreakingSkullGfx, 0);

    if (gfx < 0)
        return 0;
    if (entity->enemyId == 0x21)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sCreakingSkullPalette, 0);
    else if (entity->enemyId == 0x5C)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sCreakingSkullPalette, 1);
    Sprite_Attach(entity, sCreakingSkullSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief Apply root contact damage with effect type 0 and flags 1.
 * Distinct from the joint-part CreakingSkull_OnContact callback.
 */
void CreakingSkull_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief Resolve and display root body damage, then apply it through
 * the shared resolved-damage helper.
 */
void CreakingSkull_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s16 damage = Hit_ResolveDamage(entity, (struct EwramData_EntityData *)attacker, hit, Player_CalculateAttackPower(hit[2]));

    Entity_ShowDamage(entity, (s16)damage, 0);
    CreakingSkull_ApplyResolvedDamage(entity, attacker, hit, (s16)damage);
}

/**
 * @brief 98EEC | Giant Skeleton update: same as Creaking Skull.
 */
void EnemyGiantSkeletonUpdate(struct EwramData_EntityData *entity)
{
    EnemyCreakingSkullUpdate(entity);
}

/**
 * @brief 98EF8 | Giant Skeleton create: same as Creaking Skull.
 */
void EnemyGiantSkeletonCreate(struct EwramData_EntityData *entity)
{
    EnemyCreakingSkullCreate(entity);
}
