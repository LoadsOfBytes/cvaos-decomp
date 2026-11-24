/*
 * Golem: body, segments, thrown debris and idle effect. The last part of the range also holds the entry points of Iron Golem, Wooden Golem and Flesh Golem.
 */
#include "core.h"
#include "player_core.h"
#include "graphics.h"
#include "gameplay.h"
#include "entity_util.h"
#include "structs/ewram.h"
#include "sound.h"
#include "map_flags.h"
#include "syscalls.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "math_inline.h"
#include "damage_numbers.h"
#include "entity_hit.h"
#include "hit_shared_history.h"

void Golem_UpdateMovement(struct EwramData_EntityData *entity);
void Golem_UpdateAttack(struct EwramData_EntityData *entity);
void Golem_UpdateBestiaryPreview(struct EwramData_EntityData *entity);

struct GolemBox {
    s32 x:8;
    s32 y:8;
    u32 w:8;
    u32 h:8;
};

#define TO_PIXELS(d) ((d) < 0 ? -(-(d) >> 16) : (d) >> 16)

extern void *memcpy(void *dst, const void *src, unsigned long size);

u8 Golem_LoadGraphics(struct EwramData_EntityData *entity);
void GolemSegment_Update(struct EwramData_EntityData *entity);
void GolemSegment_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void GolemSegment_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
void Golem_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void Golem_OnDamaged(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
void Golem_FaceTarget(struct EwramData_EntityData *entity);
void Golem_UpdateAllParts(struct EwramData_EntityData *entity);
void Golem_SpawnIdleEffect(struct EwramData_EntityData *, s32, s32);
void Golem_AttachPart(struct EwramData_EntityData *, s32, s32, s32, s32);
void Golem_UpdateParts(struct EwramData_EntityData *, s8 *);
void Golem_ResetPartAngles(struct EwramData_EntityData *, s32);
void Golem_ThrowDebris(struct EwramData_EntityData *, s32, s32, s32, s32);
void GolemDebris_DamagePlayerOnContact(struct EwramData_EntityData *);
void GolemDebris_OnKilled(struct EwramData_EntityData *, s32, u16 *);
void GolemDebris_Update(struct EwramData_EntityData *entity);
void GolemIdleEffect_Update(struct EwramData_EntityData *entity);

/**
 * @brief 7E35C | Golem update (as EnemyGreatArmorUpdate): skipped off
 * screen unless dying; states 0 / 1 (Golem_UpdateMovement / Golem_UpdateAttack), 2
 * dying: dust every 4th frame within +-20 px (Effect_SpawnRisingParticle), frees the
 * parts (Heap_Free) when done, 3 Golem_UpdateBestiaryPreview; the 16 px hit box
 * follows part 4 (mirrored by flipX).
 */
void EnemyGolemUpdate(struct EwramData_EntityData *entity)
{
    u8 unused[48];
    struct GolemBox box;
    s32 *parts; /* GolemPart viewed as 8 words (needed for the addressing) */

    if (entity->state != 2)
    {
        if (EntityIsOutsideScreen(entity, 6))
            return;
        if (Entity_IsInRect(entity, -16, 0, 272, 176))
            Player_TrackEntity(entity);
        if (entity->state != 2)
            Enemy_UpdateHitFlash(entity);
    }
    if (Entity_IsTimeStopped(entity) && entity->state != 2)
        goto boxes;
    switch (entity->state)
    {
    case 0:
        Golem_UpdateMovement(entity);
        break;
    case 1:
        Golem_UpdateAttack(entity);
        break;
    case 2:
        if ((entity->timer & 3) == 0)
        {
            s32 x = RandomNumberGenerator() % 0x280000 - 0x140000;
            RandomNumberGenerator();
            Effect_SpawnRisingParticle(entity->posX.whole + x, entity->posY.whole, entity->spriteLayer - 1, 1);
        }
        if ((s8)--entity->timer <= 0)
            entity->deleteRequested = 1;
        if (entity->deleteRequested && entity->ext.golem.parts != NULL)
            Heap_Free(entity->ext.golem.parts);
        break;
    case 3:
        Golem_UpdateBestiaryPreview(entity);
        return;
    }
boxes:
    parts = (s32 *)entity->ext.golem.parts;
    if (entity->flipX)
        box.x = -TO_PIXELS(parts[4 * 8 + 0]) - 8;
    else
        box.x = TO_PIXELS(parts[4 * 8 + 0]) - 8;
    box.y = TO_PIXELS(parts[4 * 8 + 1]) - 8;
    box.w = 0x10;
    box.h = box.y < 0 ? -box.y : box.y;
    HitBox_SetAttack(entity, &box);
    HitBox_SetBody(entity, &box);
    Enemy_CheckHitPlayer(entity);
}

/* Golem create: allocates the 11-joint skeleton and one part entity per joint. */

void EnemyGolemCreate(struct EwramData_EntityData *entity)
{
    u8 frames[11];
    u8 frames65[11];
    u8 frames22[11];
    u8 frames22Rare[11];
    u8 frames50[11];
    u8 anchors[11];
    s8 order[11];
    s8 order50[11];
    s32 i;
    s32 index;
    s32 kindIndex;
    struct EwramData_EntityData *part;
    s32 *parts;
    const u8 *pose;
    const s32 *angles;
    u8 frozen;

    memcpy(frames, sGolemFrames, sizeof(frames));
    memcpy(frames65, sGolemFrames65, sizeof(frames65));
    memcpy(frames22, sGolemFrames22, sizeof(frames22));
    memcpy(frames22Rare, sGolemFrames22Rare, sizeof(frames22Rare));
    memcpy(frames50, sGolemFrames50, sizeof(frames50));
    memcpy(anchors, sGolemAnchors, sizeof(anchors));
    memcpy(order, sGolemOrder, sizeof(order));
    memcpy(order50, sGolemOrder50, sizeof(order50));

    if (!Golem_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    entity->ext.golem.parts = Heap_Alloc(0x58);
    if (entity->ext.golem.parts == NULL)
    {
        EntityDelete(entity);
        return;
    }
    entity->hidden = 1;
    entity->flipX = 0;
    if (entity->enemyId == 0x50)
        entity->ext.joint.pose = 0;
    else
        entity->ext.joint.pose = 7;
    Enemy_ResetFromTemplate(entity);
    for (i = 0; i <= 10; i++)
    {
        s32 graphics;

        if (entity->enemyId == 0x50)
            index = order50[i];
        else
            index = order[i];
        part = Entity_CreateObject(GolemSegment_Update);
        if (part != NULL)
        {
        part->spriteLayer = entity->spriteLayer;
        if (entity->enemyId == 0x50)
            graphics = ObjGfx_LoadResource(sGolemGfxEnemy50, 0);
        else
            graphics = ObjGfx_LoadResource(sGolemGfx, 0);
        Sprite_Attach(part, sGolemSpriteSheet, graphics, entity->basePalette);
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
        if (entity->enemyId == 0x50)
            part->animFrame = frames50[index];
        else if (entity->enemyId == 0x65)
            part->animFrame = frames65[index];
        else if (entity->enemyId == 0x22)
        {
            if ((RandomNumberGenerator() & 0xF) == 0)
                part->animFrame = frames22Rare[index];
            else
                part->animFrame = frames22[index];
        }
        else
            part->animFrame = frames[index];
        if ((u32)index <= 5 || index == 7 || index == 8 || index == 9 || index == 10)
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
            Hit_Enable(part, 8, GolemSegment_DamagePlayerOnContact, GolemSegment_OnHit);
        }
        else
        {
            Heap_Free(entity->ext.golem.parts);
            EntityDelete(entity);
            return;
        }
    }
    parts = (s32 *)entity->ext.golem.parts;
    pose = sGolemPartPoses[(s8)entity->ext.joint.pose];
    angles = sGolemPartAngles[(s8)entity->ext.joint.pose];
    for (index = 0; index <= 10; index++)
    {
        parts[index * 8] = (s8)pose[index * 2] << 16;
        parts[index * 8 + 1] = (s8)pose[index * 2 + 1] << 16;
        if (index == 1)
            parts[1 * 8 + 2] = angles[0];
        else if (index == 3)
            parts[3 * 8 + 2] = angles[1];
        else if (index == 4)
            parts[4 * 8 + 2] = angles[2];
        else if (index == 5)
            parts[5 * 8 + 2] = angles[3];
        else if (index == 8)
            parts[index * 8 + 2] = angles[4];
        else if (index == 10)
            parts[index * 8 + 2] = angles[5];
    }
    frozen = *((u8 *)gEwramData + 0x8E) & 0x40;
    if (frozen)
    {
        entity->state = 3;
        if (entity->enemyId == 0x50)
            entity->subState = 0x28;
        else
            entity->subState = 0x14;
        entity->timer = 0;
        return;
    }
    Hit_Enable(entity, 8, Golem_DamagePlayerOnContact, Golem_OnDamaged);
    Golem_FaceTarget(entity);
    Entity_SnapToGround(entity);
    entity->state = frozen;
    if (entity->enemyId == 0x50)
        entity->subState = 0x28;
    else
        entity->subState = 0x14;
    switch (entity->enemyId)
    {
    case 0x35:
        kindIndex = 0;
        break;
    case 0x65:
        kindIndex = 1;
        break;
    case 0x22:
        kindIndex = 2;
        break;
    case 0x50:
        kindIndex = 3;
        break;
    default:
        return;
    }
    entity->ext.golem.variant = kindIndex;
}

/* Golem main state machine: hops towards the player (turning at walls), landing dust, attacks. */

#define GOLEM_HOP_SPEED(entity)\
    if ((entity)->flipX)\
        (entity)->velX.whole = 0x100000 / ((entity)->timer * 2);\
    else\
        (entity)->velX.whole = -0x100000 / ((entity)->timer * 2);

/* Wait on the ground, then attack when the player is close or hop on (turning at walls). */
#define GOLEM_WAIT(NEXT, ANIM, TURN_STATE, TURN_ANIM)\
    if (entity->timer >= (s8)landTimes[entity->ext.golem.variant] - 10 && !(entity->timer & 3))\
        ScreenShake_RequestOffset(0, 0, 1);\
    if ((s8)--entity->timer > 0)\
        break;\
    r = RandomNumberGenerator() & 3;\
    if (r <= 1 && Abs((s16)Target_GetPixelX(entity) - (s16)entity->posX.part16.integer) <= 0x3B)\
    {\
        entity->state = 1;\
        entity->subState = 0;\
        break;\
    }\
    if (r == 2 && Abs((s16)Target_GetPixelX(entity) - (s16)entity->posX.part16.integer) <= 0x77)\
    {\
        entity->state = 1;\
        entity->subState = 0x14;\
        break;\
    }\
    entity->timer = hopTimes[entity->ext.golem.variant];\
    NEXT;\
    entity->ext.joint.pose = ANIM;\
    Golem_FaceTarget(entity);\
    GOLEM_HOP_SPEED(entity)\
    dx = entity->flipX ? 0x20 : -0x20;\
    if (Abs((s16)Target_GetPixelX(entity) - (s16)entity->posX.part16.integer) > 0x17\
        && !Tile_Classify(bg->xPos.part16.integer + entity->posX.part16.integer + dx,\
                         bg->yPos.part16.integer + entity->posY.part16.integer))\
        goto hop;\
    dx = entity->flipX ? -0x20 : 0x20;\
    if (Tile_Classify(bg->xPos.part16.integer + entity->posX.part16.integer + dx,\
                     bg->yPos.part16.integer + entity->posY.part16.integer))\
        goto hop;\
    entity->velX.whole = -entity->velX.whole;\
    entity->subState = TURN_STATE;\
    entity->ext.joint.pose = TURN_ANIM;\
    goto hop;

#define GOLEM_LAND(TIMES)\
    Golem_UpdateAllParts(entity);\
    Entity_MoveWithCollisionDefault(entity, 0x10000, -0x300000);\
    if ((s8)--entity->timer > 0)\
        break;\
    entity->timer = TIMES[entity->ext.golem.variant];\
    entity->subState++;

#define GOLEM_PAUSE()\
    Golem_FaceTarget(entity);\
    Golem_UpdateAllParts(entity);\
    if ((s8)--entity->timer > 0)\
        break;

/**
 * @brief Movement state 0 for the shared Golem family: wait, hop,
 * turn at obstacles and select nearby attacks. The Flesh Golem idle
 * branch may spawn its visual effect. Preserve macros and shared labels.
 */
void Golem_UpdateMovement(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    u8 hopTimes[4];
    u8 landTimes[4];
    u32 r;
    s32 dx; /* wall probe offset, also the dust offset */

    memcpy(hopTimes, sGolemHopTimes, sizeof(hopTimes));
    memcpy(landTimes, sGolemLandTimes, sizeof(landTimes));

    switch (entity->subState)
    {
    case 0:
        Golem_FaceTarget(entity);
        entity->velX.whole = 0;
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0x2800;
        Entity_MoveWithCollisionDefault(entity, 0x10000, -0x300000);
        if (!Target_IsInRect((s16)(entity->posX.part16.integer - 0x50), (s16)(entity->posY.part16.integer - 0x50), 0xA0, 0xA0)
            && !Enemy_IsDamaged(entity))
            break;
        entity->subState = 2;
        entity->timer = hopTimes[entity->ext.golem.variant];
        entity->ext.joint.pose = 2;
        GOLEM_HOP_SPEED(entity)
        goto hop;
    case 2:
        GOLEM_LAND(landTimes)
        PlaySong(0x7C);
        entity->ext.joint.pose = 2;
        if (entity->flipX)
            dx = 0x80000;
        else
            dx = -0x80000;
        Effect_SpawnRisingParticle(entity->posX.whole + dx, entity->posY.whole, entity->spriteLayer - 1, 1);
        ScreenShake_RequestOffset(0, 1, 2);
        break;
    case 3:
        GOLEM_WAIT(entity->subState++, 3, 10, 1)
    case 5:
        GOLEM_LAND(landTimes)
        PlaySong(0x7C);
        entity->ext.joint.pose = 4;
        if (entity->flipX)
            dx = 0x80000;
        else
            dx = -0x80000;
        Effect_SpawnRisingParticle(entity->posX.whole + dx, entity->posY.whole, entity->spriteLayer - 1, 1);
        ScreenShake_RequestOffset(0, 1, 2);
        break;
    case 6:
        GOLEM_WAIT(entity->subState = 1, 1, 7, 3)
    case 1:
    case 7:
        GOLEM_LAND(hopTimes)
        entity->ext.joint.pose = 2;
        break;
    case 8:
        GOLEM_LAND(landTimes)
        PlaySong(0x7C);
        entity->ext.joint.pose = 2;
        dx = entity->flipX ? -0x60000 : 0x60000;
        Effect_SpawnRisingParticle(entity->posX.whole + dx, entity->posY.whole, entity->spriteLayer - 1, 1);
        ScreenShake_RequestOffset(0, 1, 2);
        break;
    case 9:
        GOLEM_WAIT(entity->subState = 4, 3, 10, 1)
    case 4:
    case 10:
        GOLEM_LAND(hopTimes)
        entity->ext.joint.pose = 4;
        break;
    case 11:
        GOLEM_LAND(landTimes)
        PlaySong(0x7C);
        entity->ext.joint.pose = 4;
        dx = entity->flipX ? -0x60000 : 0x60000;
        Effect_SpawnRisingParticle(entity->posX.whole + dx, entity->posY.whole, entity->spriteLayer - 1, 1);
        ScreenShake_RequestOffset(0, 1, 2);
        break;
    case 12:
        GOLEM_WAIT(entity->subState = 1, 1, 7, 3)
    case 20:
        Golem_FaceTarget(entity);
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0x2800;
        Entity_MoveWithCollisionDefault(entity, 0x10000, -0x300000);
        if (!Target_IsInRect((s16)(entity->posX.part16.integer - 0x50), (s16)(entity->posY.part16.integer - 0x50), 0xA0, 0xA0)
            && !Enemy_IsDamaged(entity))
            break;
        entity->subState++;
        entity->timer = 0x14;
        entity->ext.joint.pose = 6;
        break;
    case 21:
        GOLEM_PAUSE()
        entity->timer = 4;
        entity->subState++;
        entity->ext.joint.pose = 8;
        PlaySong(0x7E);
        break;
    case 22:
        GOLEM_PAUSE()
        entity->timer = 4;
        entity->subState++;
        entity->ext.joint.pose = 6;
        break;
    case 24:
        GOLEM_PAUSE()
        entity->subState++;
        entity->timer = 0x14;
        entity->ext.joint.pose = 5;
        break;
    case 25:
        GOLEM_PAUSE()
        entity->timer = 4;
        entity->subState++;
        entity->ext.joint.pose = 9;
        PlaySong(0x7E);
        break;
    case 26:
        GOLEM_PAUSE()
        entity->timer = 4;
        entity->subState++;
        entity->ext.joint.pose = 5;
        break;
    case 28:
        GOLEM_PAUSE()
        entity->subState++;
        entity->timer = 0x14;
        entity->ext.joint.pose = 0;
        break;
    case 29:
        GOLEM_PAUSE()
        entity->timer = 4;
        entity->subState++;
        entity->ext.joint.pose = 10;
        PlaySong(0x7E);
        break;
    case 30:
        GOLEM_PAUSE()
        entity->timer = 4;
        entity->subState++;
        entity->ext.joint.pose = 0;
        break;
    case 23:
    case 27:
    case 31:
        GOLEM_PAUSE()
        entity->timer = 0x14;
        entity->subState++;
        break;
    case 32:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = hopTimes[entity->ext.golem.variant];
        entity->subState = 2;
        entity->ext.joint.pose = 2;
        GOLEM_HOP_SPEED(entity)
        goto hop;
    case 40:
        Golem_FaceTarget(entity);
        entity->velX.whole = 0;
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0x2800;
        Entity_MoveWithCollisionDefault(entity, 0x10000, -0x300000);
        if (Target_IsInRect((s16)(entity->posX.part16.integer - 0x50), (s16)(entity->posY.part16.integer - 0x50), 0xA0, 0xA0)
            || Enemy_IsDamaged(entity))
        {
            entity->subState = 2;
            entity->timer = hopTimes[entity->ext.golem.variant];
            entity->ext.joint.pose = 2;
            GOLEM_HOP_SPEED(entity)
        hop:
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
            break;
        }
        if (RandomNumberGenerator() % 60 != 0)
            break;
        entity->subState++;
        Golem_SpawnIdleEffect(entity, 0, 0);
        entity->timer = 0;
        break;
    case 41:
        Golem_FaceTarget(entity);
        entity->velX.whole = 0;
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0x2800;
        Entity_MoveWithCollisionDefault(entity, 0x10000, -0x300000);
        if (entity->timer++ > 0x13)
        {
            entity->subState = 0x28;
            entity->timer = 0;
        }
        break;
    }
}

/* Golem attack state machine (state == 1): overhead punches (states 0-17), ground smashes with rocks
   (20-35) and the repeated jab (40-56). */

#define STATE entity->subState
#define TIMER entity->timer
#define FRAME entity->ext.joint.pose
#define SWING entity->ext.joint.hurtMask
#define COUNT entity->ext.golem.attackCount
#define IDX entity->ext.golem.variant
#define UPDATE() Golem_UpdateParts(entity, chain);
#define COUNTDOWN() if ((s8)--TIMER > 0) break;
#define J(A, B, ANGLE, LEN) Golem_AttachPart(entity, A, B, ANGLE, LEN);
#define R(A) Golem_ResetPartAngles(entity, A);
#define DONE(S) STATE = S; entity->state = 0;
#define DEBRIS() \
    { \
        s32 vx = (RandomNumberGenerator() & 0xFFFF) - 0x8000; \
        s32 vy = -0x10000 - (RandomNumberGenerator() & 0x1FFFF); \
        Effect_SpawnFallingPiece(entity->posX.whole + dx, entity->posY.whole, vx, vy, entity->spriteLayer - 1, 0); \
    }
#define STOMP() \
    { ScreenShake_RequestOffset(0, 1, 2); \
    dx = entity->flipX ? 0x340000 : -0x340000; \
    Effect_SpawnRisingParticle(entity->posX.whole + dx, entity->posY.whole, entity->spriteLayer - 1, 1); \
    DEBRIS() DEBRIS() \
    PlaySong(0x80); }
#define ROCK() \
    { \
        s32 vx; \
        if (entity->flipX) { dx = 0x10000; vx = RandomNumberGenerator() & 0x1FFFF; } else { dx = -0x10000; vx = -(RandomNumberGenerator() & 0x1FFFF); } \
        Effect_SpawnFallingPiece(entity->posX.whole + dx, entity->posY.whole, vx + dx, -0x20000 - (RandomNumberGenerator() & 0x1FFFF), entity->spriteLayer - 1, 0); \
    }
#define SMASH() \
    if (TIMER == 6) \
    { \
        s32 v; \
        ScreenShake_RequestOffset(0, 1, 2); \
        dx = entity->flipX ? 0x10000 : -0x10000; \
        Effect_SpawnRisingParticle(entity->posX.whole + dx, entity->posY.whole, entity->spriteLayer - 1, 1); \
        ROCK() ROCK() \
        if (entity->flipX) { dx = 0x10000; v = 0x48000; } else { dx = -0x10000; v = -0x48000; } \
        Golem_ThrowDebris(entity, entity->posX.whole + dx, entity->posY.whole, v, -0x30000); \
        PlaySong(0x80); \
    }
#define P8(N) (*(s32 *)((u8 *)parts + (N) * 0x20 + 8))
#define SHAKE() if (!(TIMER & 3)) ScreenShake_RequestOffset(0, 0, 1);

/**
 * @brief Attack state 1: coordinate joint poses for overhead strikes,
 * ground smashes with debris and repeated jabs, then return to
 * movement. Preserve all joint macros and signed countdowns.
 */
void Golem_UpdateAttack(struct EwramData_EntityData *entity)
{
    s8 chain[11];
    u8 t0[4], t1[4], t2[4], t3[4], t4[4]; /* state timers, indexed by the difficulty variant */
    struct GolemPart *parts;
    s32 dx; /* ground offset of the fist, shared by all impacts */

    memcpy(chain, sGolemChain, 11);
    memcpy(t0, sUnk_08118EEB, 4);
    memcpy(t1, sUnk_08118EEB, 4);
    memcpy(t2, sUnk_08118EEF, 4);
    memcpy(t3, sUnk_08118EF3, 4);
    memcpy(t4, sUnk_08118EF7, 4);
    parts = entity->ext.golem.parts;
    switch (STATE)
    {
    case 0:
        if (entity->enemyId != 0x65) { STATE = 0x28; break; }
        Golem_FaceTarget(entity);
        if ((s8)FRAME == 2)
        {
            FRAME = 11; STATE = 1;
            J(3, 2, 0x1C00, 0) J(9, 5, 0, 0) J(10, 9, 0x4800, 0) J(4, 5, 0x400, 0)
            SWING = 0x400; entity->ext.golem.attackPhase = 0;
        }
        else
        {
            FRAME = 16; STATE = 10;
            J(3, 2, 0x4800, 0) J(10, 9, 0x1000, 0) J(4, 5, 0x400, 0)
            SWING = 8; entity->ext.golem.attackPhase = 0;
        }
        TIMER = t0[IDX];
        break;
    case 1:
        UPDATE() COUNTDOWN() TIMER = t1[IDX]; STATE++; FRAME = 12;
        J(3, 2, 0x1C00, 0) J(9, 5, 0, 0) J(10, 9, 0x4800, 0) J(4, 5, 0x400, 0)
        break;
    case 2:
        UPDATE() COUNTDOWN() TIMER = t2[IDX]; STATE++;
        break;
    case 3:
        UPDATE() COUNTDOWN() TIMER = 6; STATE++; FRAME = 13;
        J(3, 2, -0x3400, 0) J(4, 5, -0x1400, 0) J(2, 5, -0x1400, 0) J(9, 5, -0x2200, 0) J(10, 9, -0x4000, 0)
        PlaySong(0x87);
        break;
    case 4:
        UPDATE() COUNTDOWN() TIMER = 6; STATE++; FRAME = 14;
        J(3, 2, -0x3400, 0) J(4, 5, -0x1400, 0) J(2, 5, -0x1400, 0) J(9, 5, -0x2200, 0) J(10, 9, -0x4000, 0)
        break;
    case 5:
        UPDATE() COUNTDOWN() TIMER = 1; STATE++; FRAME = 15;
        J(3, 2, 0, 0) J(4, 5, 0, 0) J(2, 5, 0, 0) J(9, 5, 0, 0) J(10, 9, 0, 0)
        STOMP()
        break;
    case 6:
        UPDATE() COUNTDOWN() TIMER = 60; STATE++;
        R(3) R(4) R(2) J(9, 5, 0x2200, 0) J(10, 9, 0x4000, 0)
        break;
    case 7:
        if (TIMER > 0x27 && !(TIMER & 3)) ScreenShake_RequestOffset(0, 0, 1);
        COUNTDOWN() TIMER = 40; STATE++; FRAME = 2; SWING = 0;
        R(9) R(10)
        break;
    case 8:
        UPDATE() COUNTDOWN() TIMER = 20; DONE(3)
        break;
    case 10:
        UPDATE() COUNTDOWN() TIMER = t1[IDX]; STATE++; FRAME = 17;
        J(3, 2, 0x4800, 0) J(10, 9, 0x1000, 0) J(4, 5, 0x400, 0)
        break;
    case 11:
        UPDATE() COUNTDOWN() TIMER = t2[IDX]; STATE++; FRAME = 17;
        J(3, 2, 0, 0) J(10, 9, 0, 0)
        break;
    case 12:
        UPDATE() COUNTDOWN() TIMER = 6; STATE++; FRAME = 18;
        J(3, 2, -0x4000, 0) J(4, 5, -0x1400, 0) J(2, 5, -0x2400, 0) J(9, 5, -0x1400, 0) J(10, 9, -0x2C00, 0)
        PlaySong(0x87);
        break;
    case 13:
        UPDATE() COUNTDOWN() TIMER = 6; STATE++; FRAME = 19;
        J(3, 2, -0x4000, 0) J(4, 5, -0x1400, 0) J(2, 5, -0x2400, 0) J(9, 5, -0x1400, 0) J(10, 9, -0x2C00, 0)
        break;
    case 14:
        UPDATE() COUNTDOWN() TIMER = 1; STATE++; FRAME = 20;
        J(3, 2, 0, 0) J(4, 5, 0, 0) J(2, 5, 0, 0) J(9, 5, 0, 0) J(10, 9, 0, 0)
        STOMP()
        break;
    case 15:
        UPDATE() COUNTDOWN() TIMER = 60; STATE++;
        R(3) R(4) R(2) R(9) R(10)
        break;
    case 16:
        if (TIMER > 0x27 && !(TIMER & 3)) ScreenShake_RequestOffset(0, 0, 1);
        COUNTDOWN() TIMER = 40; STATE++; FRAME = 4; SWING = 0;
        break;
    case 20:
        Golem_FaceTarget(entity);
        if ((s8)FRAME == 2)
        {
            FRAME = 21; STATE = 31;
            J(8, 7, -0x1000, 0xD0000) J(7, 6, 0x14000, 0x50000) J(5, 6, -0x1000, 0) J(4, 5, -0x1000, 0)
            J(9, 6, -0x1000, 0) J(10, 9, -0x2000, 0) J(2, 6, -0x1000, 0) J(3, 2, -0x1000, 0)
            SWING = 0x100; entity->ext.golem.attackPhase = 0;
        }
        else
        {
            FRAME = 23; STATE = 21;
            J(1, 6, -0x1000, 0x120000) J(5, 6, -0x1000, 0) J(4, 5, -0x1000, 0) J(9, 6, -0x1000, 0) J(10, 9, -0x1000, 0)
            SWING = 2; entity->ext.golem.attackPhase = 0;
        }
        TIMER = t3[IDX];
        break;
    case 21:
        UPDATE() COUNTDOWN() TIMER = t4[IDX]; STATE++;
        J(1, 6, 0, 0) J(5, 6, 0, 0) J(4, 5, 0, 0) J(9, 6, 0, 0) J(10, 9, 0, 0)
        break;
    case 22:
        COUNTDOWN() TIMER = 10; STATE++;
        J(1, 6, 0x4000, 0x130000) J(5, 6, 0x1800, 0) J(4, 5, 0x1800, 0) J(9, 6, 0x1800, 0) J(10, 9, 0x1000, 0)
        FRAME = 24;
        break;
    case 23:
        UPDATE() SMASH() COUNTDOWN() TIMER = 12; STATE++; SWING = 0;
        J(1, 6, 0, 0) J(5, 6, 0, 0) J(4, 5, 0, 0) J(9, 6, 0, 0) J(10, 9, 0, 0)
        break;
    case 24:
        SHAKE() COUNTDOWN() TIMER = 30; STATE++; FRAME = 4;
        R(1) R(5) R(4) R(9) R(10)
        break;
    case 31:
        UPDATE() COUNTDOWN() TIMER = t4[IDX]; STATE++;
        J(8, 7, 0, 0) J(7, 6, 0x14000, 0x50000) J(5, 6, 0, 0) J(4, 5, 0, 0)
        J(9, 6, 0, 0) J(10, 9, 0, 0) J(2, 6, 0, 0) J(3, 2, 0, 0)
        break;
    case 32:
        COUNTDOWN() TIMER = 10; STATE++;
        J(8, 7, 0x4800, 0xD0000) J(7, 6, 0x14000, 0x50000) J(5, 6, 0x1800, 0) J(4, 5, 0x1800, 0)
        J(9, 6, 0x1800, 0) J(10, 9, 0x1000, 0) J(2, 6, 0x1800, 0) J(3, 2, 0x2000, 0)
        FRAME = 22;
        break;
    case 33:
        UPDATE() SMASH() COUNTDOWN() TIMER = 12; STATE++; SWING = 0;
        J(8, 7, 0, 0) J(7, 6, 0x14000, 0x50000) J(5, 6, 0, 0) J(4, 5, 0, 0)
        J(9, 6, 0, 0) J(10, 9, 0, 0) J(2, 6, 0, 0) J(3, 2, 0, 0)
        break;
    case 34:
        SHAKE() COUNTDOWN() TIMER = 30; STATE++; FRAME = 2;
        R(8) R(7) R(5) R(4) R(9) R(10) R(3) R(2)
        break;
    case 35:
        UPDATE() COUNTDOWN() TIMER = 20; DONE(3)
        break;
    case 40:
        Golem_FaceTarget(entity);
        COUNT = 0;
        if ((s8)FRAME == 2)
        {
            FRAME = 25; STATE = 41;
            J(2, 5, 0, 0) J(9, 5, 0, 0) J(10, 9, 0x8000, 0)
            SWING = 0x400;
        }
        else
        {
            FRAME = 29; STATE = 51;
            J(9, 5, 0, 0) J(2, 5, 0, 0) J(3, 2, 0x8000, 0)
            SWING = 8;
        }
        entity->ext.golem.attackPhase = 1; TIMER = 12;
        break;
    case 41:
        UPDATE() COUNTDOWN() TIMER = 12 - (s8)COUNT * 2; STATE++; FRAME = 26;
        J(10, 9, 0x8000, 0) PlaySong(0x87);
        break;
    case 42:
        UPDATE() COUNTDOWN()
        COUNT++; if ((s8)COUNT > 1)
        {
            if (entity->flipX) P8(10) += 0x10000; else P8(10) -= 0x10000;
            TIMER = 6; STATE++; FRAME = 27;
            J(3, 2, -0x2000, 0) J(10, 9, 0x4000, 0) J(9, 5, -0x1800, 0)
        }
        else
        {
            if (entity->flipX) P8(10) += 0x10000; else P8(10) -= 0x10000;
            TIMER = 12 - (s8)COUNT * 2; STATE--; FRAME = 25;
            J(10, 9, 0x8000, 0)
        }
        break;
    case 43:
        UPDATE() COUNTDOWN() TIMER = 3; STATE++; FRAME = 28;
        J(9, 5, 0, 0) J(10, 9, 0, 0) PlaySong(0x87);
        break;
    case 44:
        UPDATE() COUNTDOWN() TIMER = 30; STATE++;
        R(9) R(10)
        break;
    case 45:
        COUNTDOWN() TIMER = 40; STATE++; FRAME = 2; SWING = 0;
        R(2) R(3) R(9) R(10)
        break;
    case 46:
        UPDATE() COUNTDOWN() TIMER = 20; DONE(3)
        break;
    case 51:
        UPDATE() COUNTDOWN() TIMER = 12 - (s8)COUNT * 2; STATE++; FRAME = 30;
        J(3, 2, 0x8000, 0) PlaySong(0x87);
        break;
    case 52:
        UPDATE() COUNTDOWN()
        COUNT++; if ((s8)COUNT > 1)
        {
            if (entity->flipX) P8(3) += 0x10000; else P8(3) -= 0x10000;
            TIMER = 6; STATE++; FRAME = 31;
            J(10, 9, -0x2000, 0) J(3, 2, 0x4000, 0) J(2, 5, -0x1800, 0)
        }
        else
        {
            if (entity->flipX) P8(3) += 0x10000; else P8(3) -= 0x10000;
            TIMER = 12 - (s8)COUNT * 2; STATE--; FRAME = 29;
            J(3, 2, 0x8000, 0)
        }
        break;
    case 53:
        UPDATE() COUNTDOWN() TIMER = 3; STATE++; FRAME = 32;
        J(2, 5, 0, 0) J(3, 2, 0, 0) PlaySong(0x87);
        break;
    case 54:
        UPDATE() COUNTDOWN() TIMER = 30; STATE++;
        R(2) R(3)
        break;
    case 55:
        COUNTDOWN() TIMER = 40; STATE++; FRAME = 4; SWING = 0;
        R(2) R(3) R(9) R(10)
        break;
    case 17: case 25: case 56:
        UPDATE() COUNTDOWN() TIMER = 20; DONE(6)
        break;
    }
}

/* Separate cases preserve the original cross-jumps between timed phases. */
/**
 * @brief Animation-only state 3 selected by the bestiary flag at
 * EWRAM offset 0x8E bit 6. Cycle timed joint poses and the Flesh Golem
 * idle effect before returning to the shared pose loop.
 */
void Golem_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
{
    u8 delay[4];
    memcpy(delay, sGolemDelay, 4);
    switch (entity->subState) {
    case 0:
        if ((u8)entity->timer++ > 59) {
            entity->subState++;
            entity->timer = delay[entity->ext.golem.variant];
            entity->ext.joint.pose = 2;
        }
        break;
    case 1:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->timer = delay[entity->ext.golem.variant];
            entity->subState++;
            entity->ext.joint.pose = 2;
        }
        break;
    case 2:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->timer = 40;
            entity->subState++;
            entity->ext.joint.pose = 2;
        }
        break;
    case 3:
        if ((s8)--entity->timer <= 0) {
            entity->timer = delay[entity->ext.golem.variant];
            entity->subState++;
            entity->ext.joint.pose = 3;
        }
        break;
    case 4:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->timer = delay[entity->ext.golem.variant];
            entity->subState++;
            entity->ext.joint.pose = 4;
        }
        break;
    case 5:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->timer = 40;
            entity->subState++;
            entity->ext.joint.pose = 4;
        }
        break;
    case 6:
        if ((s8)--entity->timer <= 0) {
            entity->timer = delay[entity->ext.golem.variant];
            entity->subState = 1;
            entity->ext.joint.pose = 1;
        }
        break;
    case 20:
        if ((u8)entity->timer++ > 59) {
            entity->subState++;
            entity->timer = 20;
            entity->ext.joint.pose = 6;
        }
        break;
    case 21:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->timer = 4;
            entity->subState++;
            entity->ext.joint.pose = 8;
        }
        break;
    case 22:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->timer = 4;
            entity->subState++;
            entity->ext.joint.pose = 6;
        }
        break;
    case 24:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->subState++;
            entity->timer = 20;
            entity->ext.joint.pose = 5;
        }
        break;
    case 25:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->timer = 4;
            entity->subState++;
            entity->ext.joint.pose = 9;
        }
        break;
    case 26:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->timer = 4;
            entity->subState++;
            entity->ext.joint.pose = 5;
        }
        break;
    case 28:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->subState++;
            entity->timer = 20;
            entity->ext.joint.pose = 0;
        }
        break;
    case 29:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->timer = 4;
            entity->subState++;
            entity->ext.joint.pose = 10;
        }
        break;
    case 30:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->timer = 4;
            entity->subState++;
            entity->ext.joint.pose = 0;
        }
        break;
    case 23:
    case 27:
    case 31:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->timer = 20;
            entity->subState++;
        }
        break;
    case 32:
        Golem_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 0) {
            entity->timer = delay[entity->ext.golem.variant];
            entity->subState = 2;
            entity->ext.joint.pose = 2;
        }
        break;
    case 40:
        if ((s8)entity->ext.golemPreview.effects > 1) {
            entity->timer = delay[entity->ext.golem.variant];
            entity->subState = 2;
            entity->ext.joint.pose = 2;
        } else {
            s32 r = RandomNumberGenerator() % 60;
            if (r == 0) {
                entity->subState++;
                Golem_SpawnIdleEffect(entity, 0, 0);
                entity->timer = r;
                entity->ext.golemPreview.effects++;
            }
        }
        break;
    case 41:
        if ((u8)entity->timer++ > 59) {
            s32 zero = 0;
            entity->subState = 40;
            entity->timer = zero;
        }
        break;
    }
}

/* Move golem body part `index` 1/timer of the way to its pose: without a
 * swing (unk_C and unk_10 zero) toward the kind's offset (mirrored with the
 * entity), otherwise turn the swing angle unk_10 toward unk_C and hang the
 * part at distance unk_1C from its anchor (part unk_18, or the point
 * unk_14/unk_18 unless unk_14 is 0x80000000). Parts 1, 3, 4, 5, 8 and 10
 * also turn toward their resting angle (cf. Stolas Stolas_InterpolateJoint). */
void Golem_UpdatePart(struct EwramData_EntityData *entity, s32 index)
{
    struct GolemPart *parts = entity->ext.golem.parts;
    const u8 *pose = sGolemPartPoses[(s8)entity->ext.joint.pose];
    const s32 *angles = sGolemPartAngles[(s8)entity->ext.joint.pose];
    s32 x, y, dx, dy;
    if (parts[index].targetSwingAngle == 0 && parts[index].swingAngle == 0)
    {
        struct GolemPart *part;
        if (entity->flipX) x = -((s8)pose[index * 2] * 0x10000);
        else x = (s8)pose[index * 2] * 0x10000;
        y = (s8)pose[index * 2 + 1] * 0x10000;
        part = (struct GolemPart *)(index * sizeof(struct GolemPart) + (u32)parts);
        dx = part->offsetX;
        x -= dx;
        dy = part->offsetY;
        y -= dy;
        dx += x / entity->timer;
        part->offsetX = dx;
        dy += y / entity->timer;
        part->offsetY = dy;
        if (index == 1 || index == 3 || index == 4 || index == 5 || index == 8 || index == 10)
        {
            s32 angle;
            if (index == 1) x = angles[0];
            else if (index == 3) x = angles[1];
            else if (index == 4) x = angles[2];
            else if (index == 5) x = angles[3];
            else if (index == 8) x = angles[4];
            else if (index == 10) x = angles[5];
            if (entity->flipX) x = 0x20000 - x;
            angle = parts[index].rotationAngle;
            x -= angle;
            angle += x / entity->timer;
            parts[index].rotationAngle = angle;
        }
    }
    else
    {
        struct GolemPart *part = (struct GolemPart *)(index * sizeof(struct GolemPart) + (u32)parts);
        s32 swing;
        x = part->targetSwingAngle;
        swing = part->swingAngle;
        x -= swing;
        swing += x / entity->timer;
        part->swingAngle = swing;
        swing += 0x4000;
        x = Sin(swing);
        y = Sin(part->swingAngle);
        x = FixedMultiply(x, part->linkLength);
        y = FixedMultiply(y, part->linkLength);
        if (part->anchorXOrSentinel == (s32)0x80000000)
        {
            part->offsetX = parts[part->anchorYOrPartIndex].offsetX + x;
            part->offsetY = parts[part->anchorYOrPartIndex].offsetY + y;
        }
        else
        {
            part->offsetX = part->anchorXOrSentinel + x;
            part->offsetY = part->anchorYOrPartIndex + y;
        }
        if (index == 1 || index == 3 || index == 4 || index == 5 || index == 8 || index == 10)
        {
            s32 angle;
            if (index == 1) x = angles[0];
            else if (index == 3) x = angles[1];
            else if (index == 4) x = angles[2];
            else if (index == 5) x = angles[3];
            else if (index == 8) x = angles[4];
            else if (index == 10) x = angles[5];
            if (entity->flipX) x = 0x20000 - x;
            angle = parts[index].rotationAngle;
            x -= angle;
            angle += x / entity->timer;
            parts[index].rotationAngle = angle;
        }
    }
}

/* Mirror joint positions and selected joint angles when facing changes. */
/**
 * @brief Face the direction given by Entity_IsTargetRight; when turning around
 * mirror the x offsets of all 11 parts and the angle unk_8 of parts
 * 1, 3, 4, 5, 8 and 10.
 */
void Golem_FaceTarget(struct EwramData_EntityData *entity)
{
    s32 facing = entity->flipX;
    if (facing != Entity_IsTargetRight(entity))
    {
        struct GolemPart *base;
        s32 index;
        entity->flipX ^= 1;
        base = entity->ext.golem.parts;
        for (index = 0; index < 11; index++)
        {
            base[index].offsetX = -base[index].offsetX;
            if (index == 1 || index == 3 || index == 4 || index == 5 || index == 8 || index == 10)
                base[index].rotationAngle = 0x20000 - base[index].rotationAngle;
        }
    }
}

/* Golem_AttachPart (joint set-up, variant of CreakingSkull_AttachJointSetAngle on struct
 * GolemPart). */

/**
 * @brief 81018 | Hang part partIndex from part anchorIndex: store the parent,
 * the distance (length, or measured when 0), the angle to the parent + 0x10000
 * (unk_10) and the swing base unk_C from angleSpec (mirrored when facing
 * right); a angleSpec above 0xFFFF is an absolute angle, also
 * copied to unk_10.
 */
void Golem_AttachPart(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length)
{
    struct GolemPart *parts = entity->ext.golem.parts;
    // &parts[partIndex], written as integer arithmetic (offset + base) as in
    // the original
    struct GolemPart *part = (struct GolemPart *)(partIndex * sizeof(struct GolemPart) + (u32)parts);
    s32 dx;
    s32 dy;
    s32 baseAngle;
    s32 angle;
    struct GolemPart *anglePart;

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
    anglePart = (struct GolemPart *)(partIndex * sizeof(struct GolemPart) + (u32)parts);
    anglePart->swingAngle = baseAngle = (u16)angle + 0x10000;
    if (angleSpec > 0xFFFF)
    {
        if (entity->flipX)
            anglePart->targetSwingAngle = 0x28000 - angleSpec;
        else
            anglePart->targetSwingAngle = angleSpec;
        {
            struct GolemPart *absolutePart = (struct GolemPart *)(partIndex * sizeof(struct GolemPart) + (u32)parts);

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

/* Load the shared Golem graphics and the palette for its enemy variant. */
/**
 * @brief Load the golem graphics (enemy 0x50 uses its own set) and palette variant
 * 0..3 for enemy IDs 0x35, 0x65, 0x22 and 0x50; attach the sprite set.
 * Return 0 when no graphics slot is free, else 1.
 */
u8 Golem_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics;
    if (entity->enemyId == 0x50) graphics = ObjGfx_LoadResource(sGolemGfxEnemy50, 0);
    else graphics = ObjGfx_LoadResource(sGolemGfx, 0);
    if (graphics < 0) return 0;
    if (entity->enemyId == 0x35)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sGolemPalette, 0);
    else if (entity->enemyId == 0x65)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sGolemPalette, 1);
    else if (entity->enemyId == 0x22)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sGolemPalette, 2);
    else if (entity->enemyId == 0x50)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sGolemPalette, 3);
    Sprite_Attach(entity, sGolemSpriteSheet, graphics, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/* Golem_OnDamaged (hit with HP / death), generated with a template script from the
 * template of WingedSkeleton_OnHit and checked byte for byte. */

/**
 * @brief 811CC | Hit: flash (hitFlashTimer = 8), subtract the damage from the HP
 * and show it; at HP <= 0 die (Enemy_DropRewards, song 0x71, palette into
 * 0x53E bits 3-6, timer 0x4a, state 2). Same template as WingedSkeleton_OnHit.
 */
void Golem_OnDamaged(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, (struct EwramData_EntityData *)attacker, hit, Player_CalculateAttackPower(hit[2]));

    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;

        // (s16)damage, written so that agbcc sign-extends after the HP load
        damage <<= 16;
        damage >>= 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, (s16)damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x71);
        entity->palette = entity->basePalette;
        entity->timer = 0x4A;
        entity->state = 2;
        entity->subState = 0;
    }
}

/* Report contact effects and apply the parent's selected knockback response. */
/**
 * @brief Contact callback of a golem segment: Player_ApplyContactDamage with the
 * segment's attack attack at its position; reaction 1, or in the parent's
 * attack mode unk_4FC == 1 reaction 3 with a knockback velocity. The call
 * after the mode switch applies the damage again in every mode (kept).
 */
void GolemSegment_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.jointPart.owner;
    struct PlayerContactDamage effect;
    effect.attack = entity->attack;
    effect.attributes = 0;
    effect.unused = 0;
    effect.reaction = 1;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    switch (parent->ext.golem.attackPhase)
    {
    case 0:
        effect.reaction = 1;
        Player_ApplyContactDamage(&effect);
        break;
    case 1:
    {
        s32 vx;
        effect.reaction = 3;
        Player_ApplyContactDamage(&effect);
        vx = -0x30000;
        if (gEwramData->playerState.player->posX.whole > entity->posX.whole) vx = -vx;
        Player_SetKnockbackVelocity(vx, -0x60000);
        break;
    }
    }
    Player_ApplyContactDamage(&effect);
}

/* Golem part update: follows its skeleton joint, maintains collision,
 * then falls and spins after the parent enters state 2. */

struct SegmentBox {
    s8 x;
    s8 y;
    s8 w;
    s8 h;
};

#define PART(e) (*(s8 *)&e->ext.jointPart.index)   /* own part index */
#define NEXT_PART(e) (*(s8 *)&e->ext.jointPart.anchor)
#define ANGLE(e) ((e)->velX.whole)
#define SPIN(e) (e->ext.golemSegment.spin)
#define FALL_ANGLE(e) (e->ext.golemSegment.fallAngle)
/* parts 0-5 and 7-10 have an affine sprite */
#define HAS_AFFINE(i) (((i) >= 0 && (i) <= 5) || (i) == 7 || (i) == 8 || (i) == 9 || (i) == 10)

/* Body segment of a golem (the owner unk_500 holds 0x20-byte parts): follow
 * the owner's part PART, turn towards NEXT_PART (or take the part's angle),
 * hurt the player with a box at the rotated end when the owner's mask
 * unk_506 has the part's bit; when the owner dies, fall spinning and blink
 * out. */
void GolemSegment_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    struct SegmentBox box;
    struct EwramData_EntityData *owner = entity->ext.jointPart.owner;

    /* Share these temporaries with the collision calculation: agbcc register allocation. */
    s32 dx, dy;

    if (entity->state == 0)
    {
        struct GolemPart *parts;

        if (owner == NULL || owner->updateFunc == NULL
            || owner->enemyId != entity->enemyId)
            goto die;
        parts = owner->ext.golem.parts;
        entity->palette = owner->palette;
        entity->flipX = owner->flipX;
        entity->timer++;
        entity->posX.whole = owner->posX.whole + parts[PART(entity)].offsetX;
        entity->posY.whole = owner->posY.whole + parts[PART(entity)].offsetY;

        if (PART(entity) == 0 || PART(entity) == 2 || PART(entity) == 7 || PART(entity) == 9)
        {
            if (owner->state != 0 || owner->subState <= 19)
            {
                dx = parts[NEXT_PART(entity)].offsetX;
                dx -= parts[PART(entity)].offsetX;
                dy = parts[NEXT_PART(entity)].offsetY;
                dy -= parts[PART(entity)].offsetY;
                ANGLE(entity) = ArcTan2(TO_PIXELS(dx), TO_PIXELS(dy)) - 0x4000;
            }
            else
                /* This branch implies owner state 0; reuse that known zero. */
                ANGLE(entity) = 0;
        }
        else if (PART(entity) == 1 || PART(entity) == 3 || PART(entity) == 4 || PART(entity) == 5 || PART(entity) == 8 || PART(entity) == 10)
            ANGLE(entity) = parts[PART(entity)].rotationAngle;
        if (owner->state == 2)
        {
            s32 delay;
            entity->state = 1;
            entity->subState = 0;
            SPIN(entity) = (RandomNumberGenerator() & 0xFF) - 0x80;
            FALL_ANGLE(entity) = ANGLE(entity);
            entity->accelX.whole = 0;
            entity->accelY.whole = 0x1000;
            ANGLE(entity) = 0;
            entity->velY.whole = 0;
            delay = RandomNumberGenerator() & 3;
            entity->timer = delay + 70;
            if (PART(entity) == 3 || PART(entity) == 10)
                entity->timer = delay + 50;
            else if (PART(entity) == 2 || PART(entity) == 9)
                entity->timer = delay + 60;
            return;
        }
        if ((owner->ext.joint.hurtMask >> PART(entity)) & 1)
        {
            dx = Sin(parts[PART(entity)].rotationAngle + 0x8000);
            dy = Sin(parts[PART(entity)].rotationAngle + 0x4000);
            if (entity->flipX)
                box.x = -(TO_PIXELS(dx << 4) + 4);
            else
                box.x = TO_PIXELS(dx << 4) - 4;
            box.y = TO_PIXELS(dy << 4) - 4;
            box.w = 8;
            box.h = 8;
            HitBox_SetAttack(entity, &box);
            HitBox_SetBody(entity, &box);
            entity->hit.flags &= ~4;
            entity->hit.flags &= ~2;
            Hit_CheckWithPlayerSharedHistory(&result, entity, owner);
        }
        else
        {
            entity->hit.flags |= 4;
            entity->hit.flags |= 2;
        }
        if (HAS_AFFINE(PART(entity)))
            ObjAffine_Set(ANGLE(entity), 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
        return;
    die:
        if (HAS_AFFINE(PART(entity)))
            ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
        return;
    }
    if (entity->subState == 0)
    {
        entity->subState++;
        entity->scrollWithBg = 1;
    }
    if (entity->timer <= 49)
    {
        EntityAccelerateAndMove(entity, 0x70000);
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        FALL_ANGLE(entity) += SPIN(entity);
    }
    if (HAS_AFFINE(PART(entity)))
        ObjAffine_Set(FALL_ANGLE(entity), 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    if ((s8)--entity->timer <= 0)
    {
        if (HAS_AFFINE(PART(entity)))
            ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
}

/* Golem debris (816BC/817B0). */

/* Throw a piece of debris from the owner at (x, y) with (vx, vy): it hurts
 * the player (box 0x0808FCFC = x -4, y -4, w 8, h 8) and can be destroyed
 * (GolemDebris_DamagePlayerOnContact / GolemDebris_OnKilled). */
void Golem_ThrowDebris(struct EwramData_EntityData *owner, s32 x, s32 y, s32 vx, s32 vy)
{
    u32 box = 0x0808FCFC;
    struct EwramData_EntityData *debris = EntityCreateInRange(0x1B, 0x48, GolemDebris_Update);
    s32 graphics;

    if (debris == NULL)
        return;
    graphics = ObjGfx_Find(sCommonObjectGfx);
    if (graphics < 0)
    {
        EntityDelete(debris);
        return;
    }
    Sprite_Attach(debris, sCommonObjectSpriteSheet, graphics, 6);
    debris->animFrame = 0x42;
    debris->drawFunc = Sprite_DrawEntity;
    Hit_Enable(debris, 10, GolemDebris_DamagePlayerOnContact, GolemDebris_OnKilled);
    HitBox_SetAttack(debris, &box);
    HitBox_SetBody(debris, &box);
    debris->hit.flags &= ~4;
    debris->hit.flags &= ~2;
    debris->scrollWithBg = 1;
    debris->spriteLayer = owner->spriteLayer;
    ENTITY_OWNER(debris) = owner;
    debris->posX.whole = x;
    debris->posY.whole = y;
    debris->velX.whole = vx;
    debris->velY.whole = vy;
    debris->accelY.whole = 0x2000;
    debris->flipX = owner->flipX;
    debris->attack = owner->attack;
}

/* Debris: fall; on hitting a wall or ceiling/floor (bits 2/3) bounce up
 * and break into one piece, on the floor (bits 0/1) bounce back and break
 * into two; vanish off screen. */
void GolemDebris_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    s32 hit, vx, vy;

    if (entity->state != 0)
        return;
    hit = Entity_MoveWithCollisionDefault(entity, 0x10000, -0x40000);
    if (EntityIsOutsideScreen(entity, 1))
    {
        EntityDelete(entity);
        return;
    }
    if (hit & 0xC)
    {
        entity->velY.whole = -(entity->velY.whole < 0 ? -entity->velY.whole : entity->velY.whole);
        Effect_SpawnFallingPiece(entity->posX.whole, entity->posY.whole, entity->velX.whole, entity->velY.whole, entity->spriteLayer, 2);
        EntityDelete(entity);
        return;
    }
    if (hit & 3)
    {
        entity->velX.whole = (entity->velX.whole >> 2) - entity->velX.whole;
        vx = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
        vy = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
        Effect_SpawnFallingPiece(entity->posX.whole, entity->posY.whole, entity->velX.whole + vx, entity->velY.whole + vy, entity->spriteLayer, 1);
        vx = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
        vy = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
        Effect_SpawnFallingPiece(entity->posX.whole, entity->posY.whole, entity->velX.whole + vx, entity->velY.whole + vy, entity->spriteLayer, 0);
        EntityDelete(entity);
        return;
    }
    Hit_CheckWithPlayerSharedHistory(&result, entity, owner);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   GolemDebris_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */

/**
 * @brief 818A4 | Same as Enemy_DamagePlayerOnContact.
 */
void GolemDebris_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/* GolemDebris_OnKilled (death burst). */

/**
 * @brief 818D0 | Killed: HitEffect_Spawn at the position, two Effect_SpawnFallingPiece
 * pieces with the velocity +- 1 px random per axis, then delete.
 */
void GolemDebris_OnKilled(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 jitterX;
    s32 jitterY;

    Player_CalculateAttackPower(hit[2]);
    HitEffect_Spawn((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer, 0);
    jitterX = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
    jitterY = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
    Effect_SpawnFallingPiece(entity->posX.whole, entity->posY.whole, entity->velX.whole + jitterX,
                 entity->velY.whole + jitterY, entity->spriteLayer, 0);
    jitterX = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
    jitterY = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
    Effect_SpawnFallingPiece(entity->posX.whole, entity->posY.whole, entity->velX.whole + jitterX,
                 entity->velY.whole + jitterY, entity->spriteLayer, 0);
    EntityDelete(entity);
}

/* 8197C | Unless timer is 0, run Golem_UpdatePart on parts 0..10. */
void Golem_UpdateAllParts(struct EwramData_EntityData *entity)
{
    s32 i;

    if (entity->timer != 0)
    {
        for (i = 0; i <= 10; i++)
            Golem_UpdatePart(entity, i);
    }
}

/* Golem_UpdateParts: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */
#define GREAT_ARMOR_PART_COUNT 14

/**
 * @brief 8199C | Like GreatArmor_UpdateParts, but 11 parts.
 */
void Golem_UpdateParts(struct EwramData_EntityData *entity, s8 *order)
{
    s32 i;

    if (entity->timer != 0)
    {
        for (i = 0; i < 11; i++)
            Golem_UpdatePart(entity, order[i]);
    }
}

/* 819C4 | Clear unk_C and unk_10 of part param_1 (like GreatArmor_ResetPartAngles). */
void Golem_ResetPartAngles(struct EwramData_EntityData *entity, s32 partIndex)
{
    struct GolemPart *parts = entity->ext.golem.parts;

    parts[partIndex].targetSwingAngle = 0;
    parts[partIndex].swingAngle = 0;
}

/* 819D4 | Pass a hit on to the parent entity unk_500 (Golem_OnDamaged). */
void GolemSegment_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    Golem_OnDamaged(entity->ext.jointPart.owner, attacker, hit);
}

/* 819E0 | Spawn the debris GolemIdleEffect_Update at the owner + (x, y), one layer in
 * front, facing like the owner. */
/**
 * @brief Create the animation-0 effect at the owner-relative offset,
 * using Flesh Golem graphics and the owner palette/facing. Called
 * from the Flesh Golem idle and bestiary sequences.
 */
void Golem_SpawnIdleEffect(struct EwramData_EntityData *owner, s32 x, s32 y)
{
    struct EwramData_EntityData *debris = Entity_CreateObject(GolemIdleEffect_Update);

    if (debris != NULL)
    {
        s32 graphics;

        debris->posX.whole = owner->posX.whole + x;
        debris->posY.whole = owner->posY.whole + y;
        debris->spriteLayer = owner->spriteLayer - 1;
        graphics = ObjGfx_Find(sGolemGfxEnemy50);
        Sprite_Attach(debris, sGolemSpriteSheet, graphics, owner->basePalette);
        Anim_StartIndexed(debris, 0, 0, 0);
        Anim_Update(debris);
        debris->flipX = owner->flipX;
        debris->scrollWithBg = 1;
        debris->drawFunc = Sprite_DrawEntity;
    }
}

/* Golem helpers 0x08081A78..0x08081B0F (Iron, Wooden and Flesh Golem use
 * the Golem functions). */

/**
 * @brief Advance the idle effect animation and remove it on completion.
 */
void GolemIdleEffect_Update(struct EwramData_EntityData *entity)
{
    Anim_Update(entity);
    if (entity->animDone)
        EntityDelete(entity);
}

/**
 * @brief Apply stored root contact damage with effect type 0 and flags 0
 * through the existing static inline helper.
 */
void Golem_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief 81AC8 | Iron Golem update: same as Golem.
 */
void EnemyIronGolemUpdate(struct EwramData_EntityData *entity)
{
    EnemyGolemUpdate(entity);
}

/**
 * @brief 81AD4 | Iron Golem create: same as Golem.
 */
void EnemyIronGolemCreate(struct EwramData_EntityData *entity)
{
    EnemyGolemCreate(entity);
}

/**
 * @brief 81AE0 | Wooden Golem update: same as Golem.
 */
void EnemyWoodenGolemUpdate(struct EwramData_EntityData *entity)
{
    EnemyGolemUpdate(entity);
}

/**
 * @brief 81AEC | Wooden Golem create: same as Golem.
 */
void EnemyWoodenGolemCreate(struct EwramData_EntityData *entity)
{
    EnemyGolemCreate(entity);
}

/**
 * @brief 81AF8 | Flesh Golem update: same as Golem.
 */
void EnemyFleshGolemUpdate(struct EwramData_EntityData *entity)
{
    EnemyGolemUpdate(entity);
}

/**
 * @brief 81B04 | Flesh Golem create: same as Golem.
 */
void EnemyFleshGolemCreate(struct EwramData_EntityData *entity)
{
    EnemyGolemCreate(entity);
}
