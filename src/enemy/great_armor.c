/*
 * Great Armor: body, parts, shockwave and slash effect. The last part of the range also holds the entry points of Final Guard and Shadow Knight.
 */
#include "core.h"
#include "player_core.h"
#include "graphics.h"
#include "gameplay.h"
#include "entity_util.h"
#include "structs/ewram.h"
#include "music.h"
#include "player_level.h"
#include "sound.h"
#include "map_flags.h"
#include "syscalls.h"
#include "boss_defeat.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"

/* Great Armor update. */
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_ClampXToRoom(struct EwramData_EntityData *, s32);
extern void GreatArmor_UpdateWalk(struct EwramData_EntityData *);
extern void GreatArmor_UpdateAttack(struct EwramData_EntityData *);

struct ArmorBox {
    s32 x:8;
    s32 y:8;
    u32 w:8;
    u32 h:8;
};

#define TO_PIXELS(d) ((d) < 0 ? -(-(d) >> 16) : (d) >> 16)
/* set by the AI when the shield arm (part 4) is up: makes box 2 harmless */
#define GUARD(e) (*(s8 *)&e->ext.greatArmor.guard)

/* Great Armor: dispatch AI (0) / attacks (1), crumble when killed (2,
 * dust, free the parts), frozen (3); the second box follows part 4. */
void EnemyGreatArmorUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[48];
    struct ArmorBox box;
    s32 *parts; /* GreatArmorPart viewed as 10 words (needed for the addressing) */

    GUARD(entity) = 0;
    if (entity->state != 2)
    {
        if (EntityIsOutsideScreen(entity, 6)) return;
        if (Entity_IsInRect(entity, -16, 0, 272, 176)) Player_TrackEntity(entity);
        if (entity->state != 2) Enemy_UpdateHitFlash(entity);
    }
    if (Entity_IsTimeStopped(entity) && entity->state != 2)
        goto boxes;
    switch (entity->state)
    {
    case 0:
        GreatArmor_UpdateWalk(entity);
        break;
    case 1:
        GreatArmor_UpdateAttack(entity);
        break;
    case 2:
        if (entity->timer >= 121 && entity->timer <= 214 && (entity->timer & 3) == 0)
        {
            s32 x = RandomNumberGenerator() % 0x280000 - 0x140000;
            RandomNumberGenerator();
            Effect_SpawnRisingParticle(entity->posX.whole + x, entity->posY.whole, entity->spriteLayer - 1, 1);
        }
        if (--entity->timer <= 1) entity->deleteRequested = 1;
        if (entity->deleteRequested && entity->ext.greatArmor.parts != NULL)
            Heap_Free(entity->ext.greatArmor.parts);
        return;
    case 3:
        return;
    }
boxes:
    parts = (s32 *)entity->ext.greatArmor.parts;
    if (entity->flipX)
        box.x = -TO_PIXELS(parts[4 * 10 + 0]) - 8;
    else
        box.x = TO_PIXELS(parts[4 * 10 + 0]) - 8;
    box.y = TO_PIXELS(parts[4 * 10 + 1]) - 8;
    box.w = 0x18;
    box.h = box.y < 0 ? -box.y : box.y;
    if (!gEwramData->gameState.bestiaryPreview)
    {
        HitBox_SetAttack(entity, &box);
        HitBox_SetBody(entity, &box);
    }
    if (entity->state != 2)
    {
        if (GUARD(entity)) entity->hit.flags |= 4;
        else entity->hit.flags &= ~4;
        Hit_CheckWithPlayer(collision, entity);
    }
    Entity_ClampXToRoom(entity, 1);
    if ((RandomNumberGenerator() & 0x7F) <= 3)
        entity->ext.greatArmor.hit = 0;
}

/* Great Armor create: allocates the 14-joint skeleton and one part entity per joint. */
struct GreatArmorBox {
    s8 x;
    s8 y;
    s8 width;
    s8 height;
};

extern void *memcpy(void *dst, const void *src, unsigned long size);
extern u8 GreatArmor_LoadGraphics(struct EwramData_EntityData *entity);
extern void GreatArmorPart_Update(struct EwramData_EntityData *entity);
extern void GreatArmorPart_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void GreatArmorPart_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
extern void GreatArmor_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void GreatArmor_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
extern void GreatArmor_AttachPartSetAngle(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length);
extern void GreatArmor_UpdateParts(struct EwramData_EntityData *entity, s8 *order);
extern void GreatArmor_ResetPartAngles(struct EwramData_EntityData *entity, s32 partIndex);
extern void GreatArmor_FaceTarget(struct EwramData_EntityData *entity);

void EnemyGreatArmorCreate(struct EwramData_EntityData *entity)
{
    u8 frames3C[14];
    u8 frames67[14];
    u8 frames69[14];
    u8 anchors[14];
    s8 order[14];
    s8 chain[14];
    struct GreatArmorBox box;
    s32 i;
    s32 index;
    struct EwramData_EntityData *part;
    s32 *parts;
    const u8 *pose;
    const s32 *angles;
    u8 *frames;
    u8 frozen;
    struct EwramData **ewram = &gEwramData;

    memcpy(frames3C, sUnk_08118F78, sizeof(frames3C));
    memcpy(frames67, sGreatArmorFrames67, sizeof(frames67));
    memcpy(frames69, sGreatArmorFrames69, sizeof(frames69));
    memcpy(anchors, sGreatArmorAnchors, sizeof(anchors));
    memcpy(order, sUnk_08118F78, sizeof(order));
    memcpy(chain, sGreatArmorChain, sizeof(chain));

    if (entity->spawnParam0 != 0 && !(*ewram)->gameState.bossRushMode
        && (((*ewram)->gameState.bossEventFlags >> entity->spawnParam0) & 1))
    {
        EntityDelete(entity);
        return;
    }
    if (!GreatArmor_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    entity->ext.greatArmor.parts = Heap_Alloc(0x8C);
    if (entity->ext.greatArmor.parts == NULL)
    {
        EntityDelete(entity);
        return;
    }
    entity->hidden = 1;
    entity->flipX = 0;
    if (entity->enemyId == 0x3C)
        entity->ext.joint.pose = 0;
    else
        entity->ext.joint.pose = 0;
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bossRushMode)
    {
        entity->hp += 1000;
        entity->attack += 20;
    }
    if (entity->enemyId == 0x3C)
        frames = frames3C;
    else if (entity->enemyId == 0x67)
        frames = frames67;
    else if (entity->enemyId == 0x69)
        frames = frames69;
    for (i = 0; i <= 13; i++)
    {
        index = order[i];
        part = Entity_CreateObject(GreatArmorPart_Update);
        if (part != NULL)
        {
            part->spriteLayer = entity->spriteLayer;
            Sprite_Attach(part, sGreatArmorSpriteSheet, ObjGfx_LoadResource(sGreatArmorGfx, 0), entity->basePalette);
            part->enemyId = entity->enemyId;
            part->basePalette = entity->basePalette;
            part->scrollWithBg = 0;
            part->hidden = 0;
            part->attack = entity->attack;
            part->drawFunc = (u32 *)Sprite_DrawEntity;
            part->ext.jointPart.owner = entity;
            part->ext.jointPart.index = index;
            part->ext.jointPart.anchor = anchors[index];
            part->posX.whole = entity->posX.whole;
            part->posY.whole = entity->posY.whole;
            part->animFrame = frames[index];
            if (index == 2 || index == 5 || index == 8 || index == 11 || index == 12 || index == 13)
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
            if (!(*((u8 *)gEwramData + 0x8E) & 0x40))
                Hit_Enable(part, 8, GreatArmorPart_DamagePlayerOnContact, (s32)GreatArmorPart_OnHit);
            if (index == 0 && !(*((u8 *)gEwramData + 0x8E) & 0x40))
            {
                box.x = -16;
                box.y = -21;
                box.width = 8;
                box.height = 54;
                HitBox_SetAttack(part, &box);
                HitBox_SetBody(part, &box);
                part->hit.flags &= 0xFB;
                part->hit.flags &= 0xFD;
            }
        }
        else
        {
            Heap_Free(entity->ext.greatArmor.parts);
            EntityDelete(entity);
            return;
        }
    }
    parts = (s32 *)entity->ext.greatArmor.parts;
    pose = sGreatArmorPartPoses[(s8)entity->ext.joint.pose];
    angles = sGreatArmorPartAngles[(s8)entity->ext.joint.pose];
    for (index = 0; index <= 13; index++)
    {
        parts[index * 10] = (s8)pose[index * 2] << 16;
        parts[index * 10 + 1] = (s8)pose[index * 2 + 1] << 16;
        if (index == 2)
            parts[2 * 10 + 2] = angles[0];
        else if (index == 5)
            parts[5 * 10 + 2] = angles[1];
        else if (index == 8)
            parts[index * 10 + 2] = angles[2];
        else if (index == 11)
            parts[index * 10 + 2] = angles[3];
        else if (index == 12)
            parts[index * 10 + 2] = angles[4];
        else if (index == 13)
            parts[index * 10 + 2] = angles[5];
    }
    entity->timer = 1;
    GreatArmor_AttachPartSetAngle(entity, 12, 13, 0x4D00, 0x130000);
    GreatArmor_AttachPartSetAngle(entity, 13, 11, 0xF00, 0xC0000);
    GreatArmor_AttachPartSetAngle(entity, 10, 11, 0xF00, 0x80000);
    GreatArmor_UpdateParts(entity, chain);
    GreatArmor_ResetPartAngles(entity, 12);
    GreatArmor_ResetPartAngles(entity, 13);
    GreatArmor_ResetPartAngles(entity, 10);
    frozen = *((u8 *)gEwramData + 0x8E) & 0x40;
    if (frozen)
    {
        entity->state = 3;
        entity->subState = 0;
        entity->timer = 0;
        return;
    }
    Hit_Enable(entity, 8, GreatArmor_DamagePlayerOnContact, (s32)GreatArmor_OnHit);
    GreatArmor_FaceTarget(entity);
    Entity_SnapToGround(entity);
    entity->state = frozen;
    entity->subState = 0x14;
    entity->ext.joint.hurtMask = 0x1001;
    if (entity->spawnParam0 != 0)
    {
        Music_SetCurrent(0x10);
        Player_SetSelectedTarget(entity);
        gEwramData->gameState.stateFlags |= 2;
    }
    switch (entity->enemyId)
    {
    case 0x3C:
        entity->ext.greatArmor.variant = 0;
        break;
    case 0x67:
        entity->ext.greatArmor.variant = 1;
        break;
    case 0x69:
        entity->ext.greatArmor.variant = 2;
        break;
    }
}

/* Great Armor walk state machine (state == 0): steps with dust and shake, turns at ledges and walls, starts attacks. */
extern void GreatArmor_SaveParts(struct EwramData_EntityData *);
extern void GreatArmor_RestoreParts(struct EwramData_EntityData *);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);
extern s32 Target_GetPixelX(struct EwramData_EntityData *);

static inline s32 ABS(s32 x)
{
    if (x < 0)
        x = -x;
    return x;
}

#define STATE entity->subState
#define TIMER entity->timer
#define FRAME entity->ext.joint.pose
#define VARIANT ((s8)entity->ext.greatArmor.variant)
#define WAS_HIT (*(s8 *)&entity->ext.greatArmor.hit)
#define HIT_COUNT ((s8)entity->ext.greatArmor.hitCount)
#define UPDATE() GreatArmor_UpdateParts(entity, chain);
#define GRAVITY() Entity_MoveWithCollisionDefault(entity, 0x10000, -0x300000);
#define COUNTDOWN() if ((s8)--TIMER > 0) break;
#define POSE(A,B) \
    GreatArmor_AttachPartSetAngle(entity, 12, 13, A, 0x130000); \
    GreatArmor_AttachPartSetAngle(entity, 13, 11, B, 0xC0000); \
    GreatArmor_AttachPartSetAngle(entity, 10, 11, B, 0x80000);
#define POSE3(A) POSE(A,A)
#define VELX() if (entity->flipX) entity->velX.whole = 0x140000 / (TIMER * 2); else entity->velX.whole = -0x140000 / (TIMER * 2);
#define LAND() entity->velY.whole = 0x10000; entity->accelY.whole = 0x2800;
#define PROBE(D) Tile_Classify(camera->xPos.part16.integer + entity->posX.part16.integer + (D), camera->yPos.part16.integer + entity->posY.part16.integer)
#define NEAR(N) (ABS((s16)Target_GetPixelX(entity) - (s16)entity->posX.part16.integer) <= (N))
#define SHAKE() if (TIMER >= t1[VARIANT] - 10 && !(TIMER & 3)) ScreenShake_RequestOffset(0, 0, 1);
#define ATTACK(S) { entity->state = 1; STATE = S; break; }
#define RECOVER(S) TIMER = 1; STATE = S; entity->state = 0; GreatArmor_RestoreParts(entity);
#define STOMP(DX) \
    dx = DX; \
    Effect_SpawnFadingParticle(entity->posX.whole + dx, entity->posY.whole, (RandomNumberGenerator() & 0xFFFF) + 0x10000, -0x4000, entity->spriteLayer - 1, 1); \
    Effect_SpawnFadingParticle(entity->posX.whole + dx, entity->posY.whole, -0x10000 - (RandomNumberGenerator() & 0xFFFF), -0x4000, entity->spriteLayer - 1, 1); \
    ScreenShake_RequestOffset(0, 1, 2);
/* turn around at a ledge */
#define EDGE_TURN(S, F, A) \
    if (WAS_HIT != 0 && HIT_COUNT > 3) \
    { \
        dx = entity->flipX ? -30 : 30; \
        if (PROBE(dx) == 0) \
        { \
            entity->ext.greatArmor.hopTurns = 0; entity->ext.greatArmor.hit = 0; entity->ext.greatArmor.hitCount = 0; \
            GreatArmor_FaceTarget(entity); \
            TIMER = t0[VARIANT]; VELX() entity->velX.whole = -entity->velX.whole; \
            STATE = S; FRAME = F; POSE3(A) LAND() \
            break; \
        } \
    }
#define SPECIAL() \
    if (entity->enemyId != 0x3C && (RandomNumberGenerator2() & 0x7F) < 0x20) ATTACK(0x28) \
    if (entity->enemyId == 0x67 && (Player_GetFocusedActionFlags() & 0x360)) { STATE = 0x1E; break; }
#define BACK_TURN(S, F, A) \
    { \
        dx = entity->flipX ? -30 : 30; \
        if (PROBE(dx) == 0) \
        { \
            entity->velX.whole = -entity->velX.whole; \
            STATE = S; FRAME = F; POSE3(A) \
        } \
    }
/* walk states 3 and 6 */
#define WALK_A(ES, EF, A, NEXT, NF, TS, TF) \
    SHAKE() COUNTDOWN() \
    EDGE_TURN(ES, EF, A) \
    if (NEAR(0x63)) \
    { \
        if ((RandomNumberGenerator2() & 0x7F) < 0x40) ATTACK(0) \
        if ((RandomNumberGenerator2() & 0x7F) < 0x40) \
        { \
            GreatArmor_FaceTarget(entity); \
            dx = entity->flipX ? -30 : 30; \
            if (PROBE(dx) == 0) \
            { \
                TIMER = t0[VARIANT]; VELX() entity->velX.whole = -entity->velX.whole; \
                STATE = TS; FRAME = TF; POSE3(A) LAND() \
                break; \
            } \
        } \
    } \
    SPECIAL() \
    GreatArmor_FaceTarget(entity); \
    TIMER = t0[VARIANT]; NEXT; FRAME = NF; POSE3(A) VELX() \
    dx = entity->flipX ? 40 : -40; \
    if (!NEAR(0x1D) && PROBE(dx) == 0 \
        && !((!entity->flipX && Entity_GetHorizontalRoomEdge(entity, 0x10) == 1) || (entity->flipX && Entity_GetHorizontalRoomEdge(entity, 0x10) == 2))) \
        ; \
    else \
        BACK_TURN(TS, TF, A) \
    LAND() \
    break;
/* walk states 9 and 12 */
#define WALK_B(ES, EF, A, NS, NF, TS, TF) \
    SHAKE() COUNTDOWN() \
    EDGE_TURN(ES, EF, A) \
    if ((RandomNumberGenerator2() & 0x7F) < 0x40 && NEAR(0x4F)) ATTACK(0) \
    SPECIAL() \
    GreatArmor_FaceTarget(entity); \
    TIMER = t0[VARIANT]; STATE = NS; FRAME = NF; POSE3(A) VELX() \
    dx = entity->flipX ? 30 : -30; \
    if (!NEAR(0x1D) && PROBE(dx) == 0) \
        ; \
    else \
        BACK_TURN(TS, TF, A) \
    LAND() \
    break;
/* last walk step of a jump sequence */
#define HOP_END(S, F, A, NS, NF) \
    SHAKE() COUNTDOWN() \
    GreatArmor_FaceTarget(entity); \
    if (++*(s8 *)&entity->ext.greatArmor.hopTurns <= 1) \
    { \
        dx = entity->flipX ? -30 : 30; \
        if (PROBE(dx) == 0) \
        { \
            TIMER = 20; VELX() entity->velX.whole = -entity->velX.whole; \
            STATE = S; FRAME = F; POSE3(A) LAND() \
            break; \
        } \
    } \
    TIMER = t0[VARIANT]; STATE = NS; FRAME = NF; POSE3(A) VELX() LAND() \
    entity->ext.greatArmor.hopTurns = 0; entity->ext.greatArmor.hit = 0; entity->ext.greatArmor.hitCount = 0; \
    break;

/**
 * @brief Walk state machine (state 0): steps with dust and screen shake, turns at
 * ledges and walls and starts the attacks of state 1.
 */
void GreatArmor_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 dx;
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    s8 t0[3], t1[3];
    s8 chain[14];
    s8 t2[3];
    memcpy(t0, sUnk_08118FBE, 3);
    memcpy(t1, sUnk_08118FC1, 3);
    memcpy(chain, sGreatArmorChain, 14);
    memcpy(t2, sUnk_08118FC4, 3);
    switch (STATE)
    {
    case 0:
        GreatArmor_FaceTarget(entity);
        if (!Target_IsNearEntity(entity, 0xA0, 0xA0) && !Enemy_IsDamaged(entity))
            break;
        STATE = 2; entity->stage = 0; TIMER = t0[VARIANT]; FRAME = 3;
        VELX() LAND()
        break;
    case 1:
        UPDATE() GRAVITY() COUNTDOWN() TIMER = t0[VARIANT]; STATE++; FRAME = 3; POSE3(0x1000)
        break;
    case 2:
        UPDATE() GRAVITY() COUNTDOWN() TIMER = t1[VARIANT]; STATE++; FRAME = 3; POSE3(0)
        STOMP(entity->flipX ? 0xA0000 : -0xA0000) PlaySong(0x7C);
        break;
    case 3:
        WALK_A(0x32, 2, -0x1000, STATE++, 4, 10, 2)
    case 4:
        UPDATE() GRAVITY() COUNTDOWN() TIMER = t0[VARIANT]; STATE++; FRAME = 5; POSE3(-0x1000)
        break;
    case 5:
        UPDATE() GRAVITY() COUNTDOWN() TIMER = t1[VARIANT]; STATE++; FRAME = 5; POSE3(0)
        STOMP(entity->flipX ? 0xA0000 : -0xA0000) PlaySong(0x7C);
        break;
    case 6:
        WALK_A(0x2F, 4, 0x1000, STATE = 1, 2, 7, 4)
    case 7:
        UPDATE() GRAVITY() COUNTDOWN() TIMER = t0[VARIANT]; STATE++; FRAME = 3; POSE3(0x1000)
        break;
    case 8:
        UPDATE() GRAVITY() COUNTDOWN() TIMER = t1[VARIANT]; STATE++; FRAME = 3;
        STOMP(entity->flipX ? -0x80000 : 0x80000) POSE3(0) PlaySong(0x7C);
        break;
    case 9:
        WALK_B(0x32, 2, -0x1000, 4, 4, 10, 2)
    case 10:
        UPDATE() GRAVITY() COUNTDOWN() TIMER = t0[VARIANT]; STATE++; FRAME = 5; POSE3(-0x1000)
        break;
    case 11:
        UPDATE() GRAVITY() COUNTDOWN() TIMER = t1[VARIANT]; STATE++; FRAME = 5;
        STOMP(entity->flipX ? -0x80000 : 0x80000) POSE3(0) PlaySong(0x7C);
        break;
    case 12:
        WALK_B(0x2F, 4, 0x1000, 1, 2, 7, 4)
    case 20:
        GreatArmor_FaceTarget(entity);
        if (!Target_IsNearEntity(entity, 0xC8, 0xA0) && !Enemy_IsDamaged(entity))
            break;
        STATE++; entity->stage = 0; TIMER = 20; FRAME = 1;
        POSE(-0x3300, -0x100) PlaySong(0x64);
        break;
    case 21:
        UPDATE() COUNTDOWN() STATE++; TIMER = 40; POSE3(0)
        ((s32 *)entity->ext.greatArmor.parts)[0x7C] = 0x160000;
        break;
    case 22:
        COUNTDOWN() STATE = 2; TIMER = t0[VARIANT]; FRAME = 3; POSE3(0x1000) VELX() LAND()
        break;
    case 30:
        GreatArmor_FaceTarget(entity); GreatArmor_SaveParts(entity);
        entity->ext.greatArmor.guard = 1;
        if ((s8)FRAME == 3)
        {
            FRAME = 14; STATE = 31; POSE(-0x800, -0x2000)
            entity->ext.jointAttack.phase = 0; TIMER = t2[0];
        }
        else
        {
            FRAME = 15; STATE = 36; POSE(0x1800, 0)
            entity->ext.jointAttack.phase = 0; TIMER = t2[0] / 2 + (u8)t2[0];
        }
        break;
    case 31:
        entity->ext.greatArmor.guard = 1; UPDATE() COUNTDOWN() STATE++; TIMER = 120; POSE3(0)
        break;
    case 32:
        entity->ext.greatArmor.guard = 1; UPDATE() COUNTDOWN() FRAME = 3; STATE++; TIMER = 20; POSE(0x800, 0x2000)
        break;
    case 34:
        entity->ext.greatArmor.guard = 1; UPDATE() COUNTDOWN() RECOVER(3)
        break;
    case 36:
        entity->ext.greatArmor.guard = 1; UPDATE() COUNTDOWN() STATE++; TIMER = 120; POSE3(0)
        break;
    case 37:
        entity->ext.greatArmor.guard = 1; UPDATE() COUNTDOWN() FRAME = 5; STATE++; TIMER = 20; POSE(-0x1800, 0)
        break;
    case 33: case 38:
        entity->ext.greatArmor.guard = 1; UPDATE() if ((s8)--TIMER > 4) break;
        TIMER = 10; STATE++;
        break;
    case 39:
        entity->ext.greatArmor.guard = 1; UPDATE() COUNTDOWN() RECOVER(6)
        break;
    case 47:
        UPDATE() GRAVITY() COUNTDOWN() TIMER = 20; STATE++; FRAME = 0x21; POSE3(0x1000)
        break;
    case 48:
        UPDATE() GRAVITY() COUNTDOWN() if ((s8)entity->ext.greatArmor.hopTurns == 0) TIMER = 14; else TIMER = 30; STATE++; FRAME = 0x21;
        STOMP(entity->flipX ? -0x80000 : 0x80000) POSE3(0) PlaySong(0x7C);
        break;
    case 49:
        HOP_END(0x32, 0x20, -0x1000, 4, 4)
    case 50:
        UPDATE() GRAVITY() COUNTDOWN() TIMER = 20; STATE++; FRAME = 0x23; POSE3(-0x1000)
        break;
    case 51:
        UPDATE() GRAVITY() COUNTDOWN() if ((s8)entity->ext.greatArmor.hopTurns == 0) TIMER = 14; else TIMER = 30; STATE++; FRAME = 0x23;
        STOMP(entity->flipX ? -0x80000 : 0x80000) POSE3(0) PlaySong(0x7C);
        break;
    case 52:
        HOP_END(0x2F, 0x22, 0x1000, 1, 2)
    }
}

/* Great Armor attack state machine. */
extern void GreatArmor_AttachPart(struct EwramData_EntityData *, s32, s32, s32, s32);
extern void GreatArmor_SpawnShockwave(struct EwramData_EntityData *, s32, s32);
extern void GreatArmorSlashEffect_Spawn(struct EwramData_EntityData *, s32, s32, s32, s32, s32);
#define HIT entity->ext.greatArmor.attackHit
#define STEP(T,F) TIMER = T; STATE++; FRAME = F;
#define ARMS(A) \
    GreatArmor_AttachPartSetAngle(entity, 13, 11, A, 0xC0000); \
    GreatArmor_AttachPartSetAngle(entity, 10, 11, A, 0x80000);
#undef POSE
#define POSE(A,B) GreatArmor_AttachPartSetAngle(entity, 12, 13, A, 0x130000); ARMS(B)
#define SLOW_ARMS(A) \
    GreatArmor_AttachPart(entity, 13, 11, A, 0xC0000); \
    GreatArmor_AttachPart(entity, 10, 11, A, 0x80000);
#define SLOW_POSE(A,B) GreatArmor_AttachPart(entity, 12, 13, A, 0x130000); SLOW_ARMS(B)
#undef SHAKE
#define SHAKE() if ((s8)HIT != 0 && !(TIMER & 3)) ScreenShake_RequestOffset(0, 0, 1);
#define SPARKS() \
    if ((TIMER & 3) == 2) \
    { \
        s32 magnitude = RandomNumberGenerator() % 40 + 16; \
        s32 dy; register s32 y asm("r0"); /* FAKEMATCH: pinned register */ \
        if (entity->flipX) { dx = Sin(parts[0x7A] + 0x4000); dy = Sin(parts[0x7A]); } \
        else { dx = Sin(parts[0x7A] + 0xC000); dy = Sin(parts[0x7A] + 0x8000); } \
        dx *= magnitude; dy *= magnitude; \
        magnitude = entity->posX.whole + parts[0x78]; y = entity->posY.whole + parts[0x79]; \
        GreatArmorSlashEffect_Spawn(entity, magnitude + dx, y + dy, dx, dy, entity->spriteLayer - 1); \
    }
#define SMASH() \
    if (entity->flipX) dx = 0x440000; else dx = -0x440000; \
    { \
        s32 x = (s16)camera->xPos.part16.integer + entity->posX.part16.integer; s32 probeX; \
        if (dx < 0) probeX = x - ((-dx) >> 16); else probeX = x + (dx >> 16); \
        if (Tile_Classify(probeX, camera->yPos.part16.integer + entity->posY.part16.integer) != 1) HIT = 1; else HIT = 0; \
    } \
    if ((s8)HIT != 0) \
    { \
        \
        Effect_SpawnParticleWithDamping(entity->posX.whole + dx, entity->posY.whole, (RandomNumberGenerator() & 0xFFFF) + 0x10000, -0x4000, entity->spriteLayer - 1, 0, 8, 15, 6); \
        Effect_SpawnParticleWithDamping(entity->posX.whole + dx, entity->posY.whole, 0x10000 - (RandomNumberGenerator() & 0xFFFF), -0x4000, entity->spriteLayer - 1, 0, 8, 15, 6); \
        { s32 vx = (RandomNumberGenerator() & 0xFFFF) - 0x8000; s32 vy = -(RandomNumberGenerator() & 0x1FFFF) - 0x10000; \
        Effect_SpawnFallingPiece(entity->posX.whole + dx, entity->posY.whole, vx, vy, entity->spriteLayer - 1, 0); } \
        { s32 vx = (RandomNumberGenerator() & 0xFFFF) - 0x8000; s32 vy = -(RandomNumberGenerator() & 0x1FFFF) - 0x10000; \
        Effect_SpawnFallingPiece(entity->posX.whole + dx, entity->posY.whole, vx, vy, entity->spriteLayer - 1, 1); } \
        ScreenShake_RequestOffset(0, 1, 2); PlaySong(0x7F); \
        for (i = 0; i <= 2;) \
        { \
            s32 step, vx, vy, rx, ry, delta; s32 next; \
            if (entity->flipX) { dx = 0x440000; step = 0x20000; vx = ({ s32 k = 0x10000; u32 r = RandomNumberGenerator() & 0x7FFF; r + k; }); } \
            else { dx = -0x440000; step = -0x20000; vx = -(RandomNumberGenerator() & 0x7FFF) - 0x10000; } \
            vy = -0x8000 - (RandomNumberGenerator() & 0x7FFF); \
            Effect_SpawnParticleWithDamping(entity->posX.whole + dx + (delta = i * step), entity->posY.whole, vx, vy, entity->spriteLayer - 1, 0, 8, 15, 6); \
            rx = (RandomNumberGenerator() & 0xFFFF) - 0x8000 + vx; \
            ry = -(RandomNumberGenerator() & 0xFFFF); next = i + 1; ry -= next << 16; \
            Effect_SpawnFallingPiece(entity->posX.whole + dx + delta, entity->posY.whole, rx, ry, entity->spriteLayer - 1, RandomNumberGenerator() & 1); i = next; \
        } \
    }
#define SHOCKWAVE() \
    ScreenShake_RequestOffset(0, 1, 2); \
    if (entity->flipX) dx = 0x2A0000; else dx = -0x2A0000; \
    GreatArmor_SpawnShockwave(entity, dx, 0); \
    GreatArmorSlashEffect_Spawn(entity, entity->posX.whole + dx, entity->posY.whole, dx, 0x300000, entity->spriteLayer - 1); \
    PlaySong(0x7F);

/**
 * @brief Attack state machine (state 1): weapon swings posed through the arm chain
 * (parts 12, 13, 10 on 11), slash effects along part 12 and the ground
 * shockwave; returns to the walk state.
 */
void GreatArmor_UpdateAttack(struct EwramData_EntityData *entity)
{
    s32 dx;
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    s8 t0[3], t1[3], t2[3], t3[3], t4[3], t5[3], t6[3], t7[3], t8[3];
    s8 chain[14];
    s32 *parts;
    s32 i;
    memcpy(t0, sUnk_08118FC7, 3);
    memcpy(t1, sUnk_08118FCA, 3);
    memcpy(t2, sUnk_08118FC7, 3);
    memcpy(t3, sUnk_08118FCD, 3);
    memcpy(t4, sUnk_08118FD0, 3);
    memcpy(t5, sUnk_08118FD3, 3);
    memcpy(t6, sUnk_08118FD6, 3);
    memcpy(t7, sUnk_08118FD9, 3);
    memcpy(t8, sUnk_08118FDC, 3);
    memcpy(chain, sGreatArmorChain, 14);
    parts = (s32 *)entity->ext.greatArmor.parts;
    switch (STATE)
    {
    case 0:
        GreatArmor_FaceTarget(entity); GreatArmor_SaveParts(entity);
        if ((RandomNumberGenerator() & 0x7F) <= 0x3F)
        {
            if ((s8)FRAME == 3)
            {
                FRAME = 6; STATE = 1; POSE(0x5800, 0x4800)
                entity->ext.jointAttack.phase = 0; TIMER = t0[VARIANT];
            }
            else
            {
                FRAME = 10; STATE = 11; POSE(0x7800, 0x6800)
                entity->ext.jointAttack.phase = 0; TIMER = t0[VARIANT] / 2 + (u8)t0[VARIANT];
            }
        }
        else
        {
            if ((s8)FRAME == 3)
            {
                FRAME = 16; STATE = 21; SLOW_POSE(0x16800, 0x12000)
                entity->ext.jointAttack.phase = 0; TIMER = t3[VARIANT];
            }
            else
            {
                FRAME = 20; STATE = 31; SLOW_POSE(0x16800, 0x12000)
                entity->ext.jointAttack.phase = 0; TIMER = t3[VARIANT];
            }
        }
        break;
    case 1:
        UPDATE() COUNTDOWN() FRAME = 7; STATE++; TIMER = t1[VARIANT]; POSE(0x400,0x200)
        break;

    case 2:
        UPDATE() COUNTDOWN() TIMER = 4; STATE++; POSE(0,0)
        break;

    case 3:
        UPDATE() COUNTDOWN() STEP(14,8) POSE(-0x6000,-0x6200) PlaySong(0x86);
        break;

    case 4:
        UPDATE() COUNTDOWN() STEP(6,9) POSE(0x400,0) SMASH()
        break;
    case 5:
        UPDATE() SHAKE() COUNTDOWN() STEP(6,8) POSE(-0x400,0)
        break;

    case 6:
        UPDATE() COUNTDOWN() STEP(t2[VARIANT],8) POSE(0,0)
        break;

    case 7:
        UPDATE() COUNTDOWN() STEP(30,3) SLOW_POSE(0x16C00,0x15000)
        break;

    case 11:
        UPDATE() COUNTDOWN() FRAME = 11; STATE++; TIMER = t1[VARIANT]; POSE(0x400,0x200)
        break;

    case 12:
        UPDATE() COUNTDOWN() TIMER = 2; STATE++; POSE(0,0)
        break;

    case 13:
        UPDATE() COUNTDOWN() STEP(14,12) POSE(-0x6000,-0x6200) PlaySong(0x86);
        break;

    case 14:
        UPDATE() COUNTDOWN() STEP(6,13) POSE(0x400,0) SMASH()
        break;
    case 15:
        UPDATE() SHAKE() COUNTDOWN() STEP(6,12) POSE(-0x400,0)
        break;

    case 16:
        UPDATE() COUNTDOWN() STEP(t2[VARIANT],12) POSE(0,0)
        break;

    case 19:
        UPDATE() COUNTDOWN() RECOVER(6)
        break;

    case 21:
        UPDATE() COUNTDOWN() FRAME = 17; STATE++; TIMER = t4[VARIANT]; GreatArmor_AttachPart(entity,12,13,0x16C00,0x130000); ARMS(-0x200)
        break;

    case 22:
        UPDATE() COUNTDOWN() TIMER = 8; STATE++; POSE(0,0)
        break;

    case 23:
        UPDATE() COUNTDOWN() STEP(10,18) SLOW_POSE(0x16800,0x16000) PlaySong(0x86);
        break;

    case 24:
        UPDATE() COUNTDOWN() STEP(6,19) POSE(0,0)
        break;

    case 25:
        UPDATE() COUNTDOWN() STEP(6,18) POSE(0,0)
        break;

    case 26:
        UPDATE() COUNTDOWN() STEP(t5[VARIANT],18) POSE(0,0)
        break;

    case 27:
        UPDATE() COUNTDOWN() STEP(30,3) SLOW_POSE(0x16C00,0x15000)
        break;

    case 31:
        UPDATE() COUNTDOWN() FRAME = 21; STATE++; TIMER = t4[VARIANT]; GreatArmor_AttachPart(entity,12,13,0x16C00,0x130000); ARMS(-0x200)
        break;

    case 32:
        UPDATE() COUNTDOWN() TIMER = 8; STATE++; POSE(0,0)
        break;

    case 33:
        UPDATE() COUNTDOWN() STEP(10,22) SLOW_POSE(0x16800,0x16000) PlaySong(0x86);
        break;

    case 34:
        UPDATE() COUNTDOWN() STEP(6,23) POSE(0,0)
        break;

    case 35:
        UPDATE() COUNTDOWN() STEP(6,22) POSE(0,0)
        break;

    case 36:
        UPDATE() COUNTDOWN() STEP(t5[VARIANT],22) POSE(0,0)
        break;

    case 39:
        UPDATE() COUNTDOWN() RECOVER(6)
        break;

    case 40:
    {
        s32 zero;
        GreatArmor_FaceTarget(entity); GreatArmor_SaveParts(entity);
        if ((s8)FRAME == 3) { zero = 0; FRAME = 24; STATE = 41; }
        else { zero = 0; FRAME = 28; STATE = 51; }
        SLOW_POSE(0x16800, 0x11000)
        entity->ext.jointAttack.phase = zero; TIMER = t6[VARIANT];
        break;
    }
    case 41:
        UPDATE() COUNTDOWN() FRAME = 25; STATE++; TIMER = t7[VARIANT]; GreatArmor_AttachPart(entity,12,13,0x16800,0x130000); ARMS(-0x200) PlaySong(0x75);
        break;

    case 42:
        UPDATE() SPARKS() COUNTDOWN() TIMER = 4; STATE++; POSE(0,0)
        break;
    case 43:
        UPDATE() SPARKS() COUNTDOWN() STEP(10,26) SLOW_POSE(0x16800,0x14000) PlaySong(0x85);
        break;
    case 44:
        UPDATE() COUNTDOWN() STEP(6,27) POSE(0,0) SHOCKWAVE()
        break;
    case 45:
        UPDATE() SHAKE() COUNTDOWN() STEP(6,26) POSE(0,0)
        break;

    case 46:
        UPDATE() COUNTDOWN() STEP(t8[VARIANT],26) POSE(0,0)
        break;

    case 47:
        UPDATE() COUNTDOWN() STEP(30,3) SLOW_POSE(0x16C00,0x15000)
        break;

    case 9: case 29: case 49:
        UPDATE() COUNTDOWN() RECOVER(3)
        break;
    case 51:
        UPDATE() COUNTDOWN() FRAME = 29; STATE++; TIMER = t7[VARIANT]; GreatArmor_AttachPart(entity,12,13,0x16800,0x130000); ARMS(-0x200) PlaySong(0x75);
        break;

    case 52:
        UPDATE() SPARKS() COUNTDOWN() TIMER = 4; STATE++; POSE(0,0)
        break;
    case 53:
        UPDATE() SPARKS() COUNTDOWN() STEP(10,30) SLOW_POSE(0x16800,0x14000) PlaySong(0x85);
        break;
    case 54:
        UPDATE() COUNTDOWN() STEP(6,31) POSE(0,0) SHOCKWAVE()
        break;
    case 55:
        UPDATE() SHAKE() COUNTDOWN() STEP(6,30) POSE(0,0)
        break;

    case 56:
        UPDATE() COUNTDOWN() STEP(t8[VARIANT],30) POSE(0,0)
        break;

    case 17: case 37: case 57:
        UPDATE() COUNTDOWN() STEP(30,5) SLOW_POSE(0x14D00,0x13000)
        break;
    case 8: case 18: case 28: case 38: case 48: case 58:
        UPDATE() if ((s8)--TIMER > 4) break;
        TIMER = 10; STATE++;
        break;
    case 59:
        UPDATE() COUNTDOWN() RECOVER(6)
        break;
    }
}

/* Move Great Armor body part `index` 1/timer of the way to its pose: without a
 * swing (unk_C and unk_10 zero) toward the kind's offset (mirrored with the
 * entity), otherwise turn the swing angle unk_10 toward unk_C and hang the
 * part at distance unk_24 from its anchor (part unk_18, or the point
 * unk_14/unk_18 unless unk_14 is 0x80000000). Parts 2, 5, 8, 11, 12 and 13
 * also turn toward their resting angle (clone of the golem Golem_UpdatePart). */
void GreatArmor_UpdatePart(struct EwramData_EntityData *entity, s32 index)
{
    struct GreatArmorPart *parts = entity->ext.greatArmor.parts;
    const u8 *pose = sGreatArmorPartPoses[(s8)entity->ext.joint.pose];
    const s32 *angles = sGreatArmorPartAngles[(s8)entity->ext.joint.pose];
    s32 x, y, dx, dy;
    if (parts[index].targetSwingAngle == 0 && parts[index].swingAngle == 0)
    {
        struct GreatArmorPart *part;
        if (entity->flipX) x = -((s8)pose[index * 2] * 0x10000);
        else x = (s8)pose[index * 2] * 0x10000;
        y = (s8)pose[index * 2 + 1] * 0x10000;
        part = (struct GreatArmorPart *)(index * sizeof(struct GreatArmorPart) + (u32)parts);
        dx = part->offsetX;
        x -= dx;
        dy = part->offsetY;
        y -= dy;
        dx += x / entity->timer;
        part->offsetX = dx;
        dy += y / entity->timer;
        part->offsetY = dy;
        if (index == 2 || index == 5 || index == 8 || index == 11 || index == 12 || index == 13)
        {
            s32 angle;
            if (index == 2) x = angles[0];
            else if (index == 5) x = angles[1];
            else if (index == 8) x = angles[2];
            else if (index == 11) x = angles[3];
            else if (index == 12) x = angles[4];
            else if (index == 13) x = angles[5];
            if (entity->flipX) x = 0x20000 - x;
            angle = parts[index].rotationAngle;
            x -= angle;
            angle += x / entity->timer;
            parts[index].rotationAngle = angle;
        }
    }
    else
    {
        struct GreatArmorPart *part = (struct GreatArmorPart *)(index * sizeof(struct GreatArmorPart) + (u32)parts);
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
        if (index == 2 || index == 5 || index == 8 || index == 11 || index == 12 || index == 13)
        {
            s32 angle;
            if (index == 2) x = angles[0];
            else if (index == 5) x = angles[1];
            else if (index == 8) x = angles[2];
            else if (index == 11) x = angles[3];
            else if (index == 12) x = angles[4];
            else if (index == 13) x = angles[5];
            if (entity->flipX) x = 0x20000 - x;
            angle = parts[index].rotationAngle;
            x -= angle;
            angle += x / entity->timer;
            parts[index].rotationAngle = angle;
        }
    }
}

/* GreatArmor_FaceTarget (Great Armor turning around). */

/**
 * @brief 8DD38 | Face the direction given by Entity_IsTargetRight; when turning
 * around mirror the x offsets of all 14 parts and the angle unk_8 of parts
 * 2, 5, 8, 11, 12 and 13.
 */
void GreatArmor_FaceTarget(struct EwramData_EntityData *entity)
{
    u32 flipX = entity->flipX;
    struct GreatArmorPart *parts;
    s32 i;

    if (flipX != Entity_IsTargetRight(entity))
    {
        entity->flipX ^= 1;
        parts = entity->ext.greatArmor.parts;
        for (i = 0; i < 14; i++)
        {
            parts[i].offsetX = -parts[i].offsetX;
            if (i == 2 || i == 5 || i == 8 || i == 11 || i == 12 || i == 13)
                parts[i].rotationAngle = 0x20000 - parts[i].rotationAngle;
        }
    }
}

/* GreatArmor_AttachPartSetAngle (Great Armor joint set-up, variant of CreakingSkull_AttachJointSetAngle on
 * struct GreatArmorPart). */

/**
 * @brief 8DDB4 | Hang part partIndex from part anchorIndex: store the parent,
 * the distance (length, or measured when 0), the angle to the parent + 0x10000
 * (unk_10) and the swing base unk_C from angleSpec (mirrored when facing
 * right); a angleSpec with |angleSpec| > 0xFFFF is an absolute angle, also
 * copied to unk_10.
 */
void GreatArmor_AttachPartSetAngle(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length)
{
    struct GreatArmorPart *parts = entity->ext.greatArmor.parts;
    // &parts[partIndex], written as integer arithmetic (offset + base) as in
    // the original
    struct GreatArmorPart *part = (struct GreatArmorPart *)(partIndex * sizeof(struct GreatArmorPart) + (u32)parts);
    s32 dx;
    s32 dy;
    s32 baseAngle;
    s32 angle;
    struct GreatArmorPart *anglePart;

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
    anglePart = (struct GreatArmorPart *)(partIndex * sizeof(struct GreatArmorPart) + (u32)parts);
    anglePart->swingAngle = baseAngle = (u16)angle + 0x10000;
    if ((angleSpec < 0 ? -angleSpec : angleSpec) > 0xFFFF)
    {
        if (entity->flipX)
            anglePart->targetSwingAngle = 0x28000 - angleSpec;
        else
            anglePart->targetSwingAngle = angleSpec;
        {
            struct GreatArmorPart *absolutePart = (struct GreatArmorPart *)(partIndex * sizeof(struct GreatArmorPart) + (u32)parts);

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

/* GreatArmor_AttachPart (Great Armor joint set-up, same code as CreakingSkull_AttachJoint on
 * struct GreatArmorPart). */

/**
 * @brief 8DEEC | Hang part partIndex from part anchorIndex: store the parent,
 * the distance (length, or measured when 0), the angle to the parent + 0x10000
 * (unk_10) and the swing base unk_C from angleSpec (mirrored when facing
 * right).
 */
void GreatArmor_AttachPart(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length)
{
    struct GreatArmorPart *parts = entity->ext.greatArmor.parts;
    // &parts[partIndex], written as integer arithmetic (offset + base) as in
    // the original
    struct GreatArmorPart *part = (struct GreatArmorPart *)(partIndex * sizeof(struct GreatArmorPart) + (u32)parts);
    s32 dx;
    s32 dy;
    s32 baseAngle;
    s32 angle;
    struct GreatArmorPart *anglePart;

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
    anglePart = (struct GreatArmorPart *)(partIndex * sizeof(struct GreatArmorPart) + (u32)parts);
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

/* Load the graphics 081CC004 / 0821F240 with the palette variant of
 * 0820A8F8 chosen by the type byte (0x3C, 0x67, 0x69 -> 0, 1, 2; other
 * types keep basePalette); 0 when the graphics are not loaded. */
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

u8 GreatArmor_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics = ObjGfx_LoadResource(sGreatArmorGfx, 0);
    if (graphics < 0)
        return 0;
    if (entity->enemyId == 0x3C)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sGreatArmorPalette, 0);
    else if (entity->enemyId == 0x67)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sGreatArmorPalette, 1);
    else if (entity->enemyId == 0x69)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sGreatArmorPalette, 2);
    Sprite_Attach(entity, sGreatArmorSpriteSheet, graphics, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

extern s16 Hit_ResolveDamage(struct EwramData_EntityData *entity, struct EwramData_EntityData *other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s16, s32);

extern void BossRecoveryPickup_Spawn(s32, s32, s32, s32, s32);

/**
 * @brief Hit: resolve the damage, flash (hitFlashTimer = 8), subtract it from the HP and
 * show it, count the hit (unk_4FD, unk_4FE = 1); at HP <= 0 drop the
 * rewards and crumble (state 2, timer 0xFF), running the boss-defeat effects
 * BossDefeatEffect_Spawn / BossRecoveryPickup_Spawn when spawnParam0 is set.
 */
void GreatArmor_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 damage = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(entity, attacker, hit, (s16)damage);
    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;
        damage <<= 16;
        damage >>= 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    entity->ext.greatArmor.hitCount++;
    entity->ext.greatArmor.hit = 1;
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->timer = 0xFF;
        entity->state = 2;
        entity->subState = 0;
        if (entity->spawnParam0 != 0)
        {
            BossDefeatEffect_Spawn(entity->posX.whole, entity->posY.whole - 0x200000);
            BossRecoveryPickup_Spawn((s16)entity->posX.part16.integer, 0x20, entity->spawnParam0, 0x60, 0);
            Player_SetSelectedTarget(NULL);
        }
    }
}

/* Great Armor part update: joint following, weapon collision and death motion. */
struct ArmorPartBox { s8 x, y, w, h; };
extern void Hit_CheckWithPlayerSharedHistory(void *, struct EwramData_EntityData *, struct EwramData_EntityData *);
#define JOINT(e) ((s8)e->ext.jointPart.index)
#define FIX_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)
#define HAS_SPRITE(e) (JOINT(e) == 2 || JOINT(e) == 5 || JOINT(e) == 8 || JOINT(e) == 11 || JOINT(e) == 12 || JOINT(e) == 13)

/**
 * @brief Part entity update: follow its joint of the parent skeleton, weapon
 * collision and the death motion.
 */
void GreatArmorPart_Update(struct EwramData_EntityData *entity)
{
    u8 hit[16];
    u8 timers[14];
    s16 offsets[14];
    struct ArmorPartBox box;
    struct EwramData_EntityData *parent = entity->ext.jointPart.owner;
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    struct GreatArmorPart *parts;
    s32 x, y, size;

    memcpy(timers, sGreatArmorPartTimers, sizeof(timers));
    memcpy(offsets, sGreatArmorPartOffsets, sizeof(offsets));
    if (entity->state == 0)
    {
        if (parent != NULL && parent->updateFunc != NULL
            && parent->enemyId == entity->enemyId)
        {
            parts = parent->ext.greatArmor.parts;
            entity->palette = parent->palette;
            entity->flipX = parent->flipX;
            entity->timer++;
            entity->posX.whole = parent->posX.whole + parts[JOINT(entity)].offsetX;
            entity->posY.whole = parent->posY.whole + parts[JOINT(entity)].offsetY;
            if (JOINT(entity) == 2 || JOINT(entity) == 8 || JOINT(entity) == 11)
            {
                x = parts[(s8)entity->ext.jointPart.anchor].offsetX;
                x -= parts[JOINT(entity)].offsetX;
                /* Keep both coordinate loads separate for matching agbcc allocation. */
                y = parts[(s8)entity->ext.jointPart.anchor].offsetY;
                y -= parts[JOINT(entity)].offsetY;
                entity->velX.whole = ArcTan2(FIX_INT(x), FIX_INT(y)) - 0x4000;
            }
            else if (JOINT(entity) == 5 || JOINT(entity) == 12 || JOINT(entity) == 13)
                entity->velX.whole = parts[JOINT(entity)].rotationAngle;
            if (parent->state == 2)
            {
                entity->state = 1;
                entity->subState = 0;
                entity->ext.greatArmorPart.angle = entity->velX.whole;
                entity->ext.greatArmorPart.spin = (RandomNumberGenerator() & 0xFF) - 0x80;
                entity->ext.greatArmorPart.floorY = parent->posY.part16.integer + (s16)camera->yPos.part16.integer + offsets[JOINT(entity)];
                entity->accelY.whole = 0x1800;
                entity->accelX.whole = 0;
                entity->velX.whole = (RandomNumberGenerator() & 0x3FFF) - 0x2000;
                entity->velY.whole = 0;
                entity->timer = timers[JOINT(entity)];
                return;
            }
            if ((parent->ext.joint.hurtMask >> JOINT(entity)) & 1)
            {
                if (JOINT(entity) == 0)
                    Hit_CheckWithPlayer(hit, entity);
                else if (JOINT(entity) == 12)
                {
                    size = ((entity->timer & 3) + 1) * 14;
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
                    if ((entity->timer & 3) == 0)
                    {
                        box.x = FIX_INT(size * x) - 6;
                        box.y = FIX_INT(size * y) - 6;
                        box.w = 12;
                        box.h = 16;
                    }
                    else
                    {
                        box.x = FIX_INT(size * x) - 4;
                        box.y = FIX_INT(size * y) - 4;
                        box.w = 8;
                        box.h = 8;
                    }
                    HitBox_SetAttack(entity, &box);
                    entity->hit.flags &= 0xFD;
                    entity->hit.flags |= 4;
                    Hit_CheckWithPlayerSharedHistory(hit, entity, parent);
                }
            }
            else
            {
                entity->hit.flags |= 4;
                entity->hit.flags |= 2;
            }
            if (HAS_SPRITE(entity))
                ObjAffine_Set(entity->velX.whole, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
            if (entity->posY.part16.integer < -24) entity->hidden = 1;
            else if (entity->posY.part16.integer > 192) entity->hidden = 1;
            else entity->hidden = 0;
            return;
        }
        if (HAS_SPRITE(entity)) ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
    else
    {
        if (entity->subState == 0)
        {
            entity->subState++;
            entity->scrollWithBg = 1;
            if (JOINT(entity) == 12) entity->subState = 10;
        }
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if (entity->timer <= 60)
        {
            EntityAccelerateAndMove(entity, 0x60000);
            entity->ext.greatArmorPart.angle += entity->ext.greatArmorPart.spin;
            if (entity->subState == 1
                && entity->posY.part16.integer + (s16)camera->yPos.part16.integer >= entity->ext.greatArmorPart.floorY)
            {
                entity->velX.whole = 0;
                entity->accelX.whole = 0;
                entity->velY.whole = 0;
                entity->accelY.whole = 0;
                entity->ext.greatArmorPart.spin = 0;
                entity->subState++;
                if ((u8)(entity->ext.jointPart.index - 1) > 2
                    && entity->ext.jointPart.index != 7)
                    PlaySong(0x7F);
            }
        }
        if (entity->subState == 2 && parent->spawnParam0 == 0 && (entity->timer & 7) == 0)
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x40000, 0x100000, 0x100000, entity->spriteLayer - 1);
        if (HAS_SPRITE(entity))
            ObjAffine_Set(entity->ext.greatArmorPart.angle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
        if (--entity->timer <= 1)
        {
            if (HAS_SPRITE(entity)) ObjAffine_Free(entity->affineIndex);
            if (JOINT(entity) == 4) PlaySong(0x6E);
            EntityDelete(entity);
        }
        else if (!entity->hidden)
        {
            if (entity->posY.part16.integer < -24) entity->hidden = 1;
            else if (entity->posY.part16.integer > 240) entity->hidden = 1;
        }
    }
}

extern void GreatArmorShockwave_Update(struct EwramData_EntityData *);
extern void GreatArmorShockwave_DamagePlayerOnContact(struct EwramData_EntityData *);
/**
 * @brief Spawn a ground shockwave at offset (dx, dy) from owner, snapped to the
 * ground and moving 3.5 px per frame in the facing direction; contact damage
 * GreatArmorShockwave_DamagePlayerOnContact, update
 * GreatArmorShockwave_Update.
 */
void GreatArmor_SpawnShockwave(struct EwramData_EntityData *owner, s32 dx, s32 dy)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)GreatArmorShockwave_Update);
    u32 box = 0x1008F0FC;
    if (entity != NULL)
    {
        s32 graphics;
        entity->posX.whole = owner->posX.whole + dx;
        entity->posY.whole = owner->posY.whole + dy;
        entity->spriteLayer = owner->spriteLayer - 1;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, 7);
        Anim_StartIndexed(entity, 0, 0, 1);
        Anim_Update(entity);
        entity->hidden = 0;
        Hit_Enable(entity, 8, GreatArmorShockwave_DamagePlayerOnContact, 0);
        HitBox_SetAttack(entity, &box);
        entity->hit.flags |= 4;
        entity->hit.flags &= ~2;
        Entity_SnapToGround(entity);
        entity->attack = owner->attack;
        entity->scrollWithBg = 1;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->flipX = owner->flipX;
        if (entity->flipX) entity->velX.whole = 0x38000;
        else entity->velX.whole = -0x38000;
        entity->velY.whole = 0x40000;
        entity->accelY.whole = 0x2800;
    }
}

extern void GreatArmor_SpawnParticle(s32, s32, s32, s32, s32);
/**
 * @brief Shockwave update: move along the ground and delete when it leaves the
 * ground, stops or leaves the screen; contact test, debris pieces with sound
 * 0x80 every 8 frames and particles (GreatArmor_SpawnParticle) every 4.
 */
void GreatArmorShockwave_Update(struct EwramData_EntityData *entity)
{
    s32 buffer[4];
    s32 vx, vy;
    s32 oldX = (s16)entity->posX.part16.integer;
    if (!(Entity_MoveWithCollisionDefault(entity, 0x40000, -0x200000) & 4) || oldX == (s16)entity->posX.part16.integer || EntityIsOutsideScreen(entity, 6))
    {
        EntityDelete(entity);
        return;
    }
    Hit_CheckWithPlayer(buffer, entity);
    if ((entity->timer++ & 7) == 0)
    {
        s32 variant;
        vx = (RandomNumberGenerator() & 0xFFFF) - 0x8000;
        vy = -0x10000 - RandomNumberGenerator() % 0x30000;
        variant = RandomNumberGenerator() & 1;
        Effect_SpawnFallingPiece(entity->posX.whole, entity->posY.whole, vx, vy, entity->spriteLayer, variant);
        PlaySong(0x80);
    }
    if ((entity->timer & 3) == 2)
    {
        vx = (RandomNumberGenerator() & 0xFFFF) - 0x8000;
        vy = -0x10000 - RandomNumberGenerator() % 0x30000;
        GreatArmor_SpawnParticle(entity->posX.whole, entity->posY.whole, vx, vy, 9);
    }
}

/* Great Armor body-part helpers 0x0808E94C..0x0808EB3B (same code as the Big Golem helpers in
 * enemy/big_golem.c with 0x28-byte parts). */

#define GREAT_ARMOR_PART_COUNT 14

extern void GreatArmorSlashEffect_Update(struct EwramData_EntityData *entity);

/**
 * @brief 8E94C | Unless timer is 0, run GreatArmor_UpdatePart on parts 0..13.
 */
void GreatArmor_UpdateAllParts(struct EwramData_EntityData *entity)
{
    s32 i;

    if (entity->timer != 0)
    {
        for (i = 0; i < GREAT_ARMOR_PART_COUNT; i++)
            GreatArmor_UpdatePart(entity, i);
    }
}

/**
 * @brief 8E96C | Unless timer is 0, run GreatArmor_UpdatePart on the 14 parts
 * listed in order.
 */
void GreatArmor_UpdateParts(struct EwramData_EntityData *entity, s8 *order)
{
    s32 i;

    if (entity->timer != 0)
    {
        for (i = 0; i < GREAT_ARMOR_PART_COUNT; i++)
            GreatArmor_UpdatePart(entity, order[i]);
    }
}

/**
 * @brief 8E994 | Clear unk_C and unk_10 of part partIndex.
 */
void GreatArmor_ResetPartAngles(struct EwramData_EntityData *entity, s32 partIndex)
{
    struct GreatArmorPart *parts = entity->ext.greatArmor.parts;

    parts[partIndex].targetSwingAngle = 0;
    parts[partIndex].swingAngle = 0;
}

/**
 * @brief 8E9A8 | Save the offsets of all parts.
 */
void GreatArmor_SaveParts(struct EwramData_EntityData *entity)
{
    struct GreatArmorPart *parts = entity->ext.greatArmor.parts;
    s32 i;

    for (i = GREAT_ARMOR_PART_COUNT - 1; i >= 0; parts++, i--)
    {
        parts->savedOffsetX = parts->offsetX;
        parts->savedOffsetY = parts->offsetY;
    }
}

/**
 * @brief 8E9C4 | Restore the offsets saved by GreatArmor_SaveParts.
 */
void GreatArmor_RestoreParts(struct EwramData_EntityData *entity)
{
    struct GreatArmorPart *parts = entity->ext.greatArmor.parts;
    s32 i;

    for (i = GREAT_ARMOR_PART_COUNT - 1; i >= 0; parts++, i--)
    {
        parts->offsetX = parts->savedOffsetX;
        parts->offsetY = parts->savedOffsetY;
    }
}

/**
 * @brief 8E9E0 | Player_ApplyContactDamage with (attack, 2, 0, 0) at the entity position.
 */
void GreatArmorShockwave_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 2;
    contactDamage.unused = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 8EA10 | Spawn a Effect_UpdateParticle effect (animation 0xF) at (x,
 * y) moving with (velocityX, velocityY) on layer layer.
 */
void GreatArmor_SpawnParticle(s32 x, s32 y, s32 velocityX, s32 velocityY, s32 layer)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateParticle);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = x;
    child->posY.whole = y;
    child->velX.whole = velocityX;
    child->velY.whole = velocityY;
    child->spriteLayer = layer;
    child->timer = 0;
    child->ext.particle.flags = 3;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(child, sCommonObjectSpriteSheet, gfx, 7);
    Anim_StartIndexed(child, 0xF, 0, 0);
    Anim_Update(child);
    child->scrollWithBg = 1;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
}

/**
 * @brief 8EA98 | Spawn a GreatArmorSlashEffect_Update effect (animation 0x10) following
 * entity at the offset of its part 12 (saved in accelX/accelY).
 */
void GreatArmorSlashEffect_Spawn(struct EwramData_EntityData *entity, s32 x, s32 y, s32 velocityX, s32 velocityY, s32 layer)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)GreatArmorSlashEffect_Update);
    struct GreatArmorPart *parts = entity->ext.greatArmor.parts;
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = x;
    child->posY.whole = y;
    child->velX.whole = velocityX;
    child->velY.whole = velocityY;
    // "*&" keeps the original address arithmetic (base + 0x1E0, base + 0x1E4)
    child->accelX.whole = *&parts[12].offsetX;
    child->accelY.whole = *&parts[12].offsetY;
    child->spriteLayer = layer;
    child->timer = 0;
    child->ext.greatArmorSlash.unusedKind = 3;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(child, sCommonObjectSpriteSheet, gfx, 7);
    Anim_StartIndexed(child, 0x10, 0, 0);
    Anim_Update(child);
    child->ext.greatArmorSlash.owner = entity;
    child->scrollWithBg = 0;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
}

/* Great Armor helpers 0x0808EB3C..0x0808EC63 (Final Guard and Shadow Knight
 * use the Great Armor functions). */

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
 * @brief 8EB3C | Follow the parent (unk_500) at the offset of its part 12 plus
 * velX/velY; delete when 0x53D bit 0 is set or the parent is in state 2.
 */
void GreatArmorSlashEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *armor = entity->ext.greatArmorSlash.owner;
    struct GreatArmorPart *parts = armor->ext.greatArmor.parts;
    s32 offsetX = parts[12].offsetX;
    s32 *offsetY = &parts[12].offsetY;

    entity->posX.whole = armor->posX.whole + offsetX + entity->velX.whole;
    entity->posY.whole = armor->posY.whole + *offsetY + entity->velY.whole;
    if (entity->animDone || armor->state == 2)
        EntityDelete(entity);
    else
        Anim_Update(entity);
}

/**
 * @brief 8EB90 | Player_ApplyContactDamage at the entity position with (attack, 1, 0, 1)
 * for attack 12 (unk_504), else (attack, 0, 0, 0).
 */
void GreatArmorPart_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    if ((s8)entity->ext.jointPart.index == 12)
    {
        contactDamage.attributes = 1;
        contactDamage.reaction = 1;
    }
    else
    {
        contactDamage.attributes = 0;
    }
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 8EBD8 | Hit without damage: hit effect, "0" damage and sound 0x69.
 */
void GreatArmorPart_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    Player_CalculateAttackPower(hit[2]);
    Hit_ResolveDamage(entity, attacker, hit, 0);
    Entity_ShowDamage(entity, 0, 0);
    PlaySong(0x69);
}

/**
 * @brief 8EC08 | Player_ApplyContactDamage with (attack, 0, 0, 0) at the entity position
 * (same as Chaos2Part_DamagePlayerOnContact).
 */
void GreatArmor_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief 8EC34 | Final Guard update: same as Great Armor.
 */
void EnemyFinalGuardUpdate(struct EwramData_EntityData *entity)
{
    EnemyGreatArmorUpdate(entity);
}

/**
 * @brief 8EC40 | Final Guard create: same as Great Armor.
 */
void EnemyFinalGuardCreate(struct EwramData_EntityData *entity)
{
    EnemyGreatArmorCreate(entity);
}

/**
 * @brief 8EC4C | Shadow Knight update: same as Great Armor.
 */
void EnemyShadowKnightUpdate(struct EwramData_EntityData *entity)
{
    EnemyGreatArmorUpdate(entity);
}

/**
 * @brief 8EC58 | Shadow Knight create: same as Great Armor.
 */
void EnemyShadowKnightCreate(struct EwramData_EntityData *entity)
{
    EnemyGreatArmorCreate(entity);
}
