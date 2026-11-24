/*
 * Devil: body, parts, projectiles and charge effect. The last part of the range also holds the entry points of Flame Demon, Arc Demon and Demon Lord.
 */
#include "core.h"
#include "player_core.h"
#include "graphics.h"
#include "entity_util.h"
#include "structs/ewram.h"
#include "gameplay.h"
#include "sound.h"
#include "syscalls.h"
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
#include "damage_numbers.h"
#include "effects.h"
#include "entity_hit.h"
#include "hit_shared_history.h"

extern void *memcpy(void *dst, const void *src, unsigned long size);
extern s32 Target_IsInRect(s16, s16, s16, s16);
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);

void Devil_UpdateApproach(struct EwramData_EntityData *entity);
void Devil_UpdateAttack(struct EwramData_EntityData *entity);
void Devil_UpdateHover(struct EwramData_EntityData *entity);
u8 Devil_LoadGraphics(struct EwramData_EntityData *entity);
void DevilPart_Update(struct EwramData_EntityData *entity);
void DevilPart_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void DevilPart_OnHit(struct EwramData_EntityData *input, s32 source, u16 *collision);
void Devil_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void Devil_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
void DevilPart_Attach(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length);
void Devil_UpdateAllParts(struct EwramData_EntityData *entity);
void Devil_ResetPartAngles(struct EwramData_EntityData *entity, s32 partIndex);
void Devil_SaveParts(struct EwramData_EntityData *);
void Devil_RestoreParts(struct EwramData_EntityData *);
void DevilChargeEffect_Spawn(struct EwramData_EntityData *owner, s32 dx, s32 dy, u8 timer);
void DevilProjectile_Spawn(struct EwramData_EntityData *owner, s32 x, s32 y, s32 vx, s32 vy, u8 id);
void Devil_Draw(struct EwramData_EntityData *entity);
void DevilProjectile_Update(struct EwramData_EntityData *entity);
void DevilProjectile_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void DevilChargeEffect_Update(struct EwramData_EntityData *);
void Devil_OnDamaged(struct EwramData_EntityData *entity, s32 attacker, u16 *hit, s32 damage);

/**
 * @brief 8F964 | Devil update (pattern of EnemyKillerFishUpdate): wake when
 * on screen, states 0 / 1 (Devil_UpdateApproach / Devil_UpdateAttack), 2 dying, 3
 * (Devil_UpdateHover + animation); contact test unless dying or unk_50A is set.
 * The original compares the frame against an uninitialised "previous
 * frame" register before calling Entity_UpdateBoxes.
 */
void EnemyDevilUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    u8 previous;

    if (entity->state != 2 && Entity_IsInRect(entity, -16, 0, 272, 176))
        Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 2 && (s8)entity->ext.devil.intangible == 0)
            Hit_CheckWithPlayer(&result, entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        Devil_UpdateApproach(entity);
        break;
    case 1:
        Devil_UpdateAttack(entity);
        break;
    case 2:
        Enemy_UpdateFragmentDeath(entity);
        break;
    case 3:
        Devil_UpdateHover(entity);
        Anim_Update(entity);
        return;
    }
    Anim_Update(entity);
    if (previous != entity->animFrame)
        Entity_UpdateBoxes(entity);
    if (entity->state != 2 && (s8)entity->ext.devil.intangible == 0)
        Hit_CheckWithPlayer(&result, entity);
}

/* Devil create: allocates the 8-joint skeleton and one part entity per joint. */

void EnemyDevilCreate(struct EwramData_EntityData *entity)
{
    u8 frames30[8];
    u8 frames68[8];
    u8 frames48[8];
    u8 frames66[8];
    u8 anchors[8];
    s8 order[8];
    s8 unused[8];
    s32 i;
    s32 index;
    struct EwramData_EntityData *part;
    struct DevilPart *parts;
    const s8 *pose;
    const s32 *angles;
    u8 *frames;
    u8 frozen;
    u8 stopped;
    struct EwramData **ewram = &gEwramData;

    memcpy(frames30, sDevilFrames30, sizeof(frames30));
    memcpy(frames68, sDevilFrames30, sizeof(frames68));
    memcpy(frames48, sDevilFrames48, sizeof(frames48));
    memcpy(frames66, sDevilFrames48, sizeof(frames66));
    memcpy(anchors, sDevilAnchors, sizeof(anchors));
    memcpy(order, sDevilOrder, sizeof(order));
    memcpy(unused, sUnk_0811902C, sizeof(unused));

    if (!Devil_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    entity->ext.devil.parts = Heap_Alloc(0x58);
    if (entity->ext.devil.parts == NULL)
    {
        EntityDelete(entity);
        return;
    }
    entity->flipX = 0;
    stopped = *((u8 *)*ewram + 0x8E) & 0x40;
    if (stopped)
        entity->ext.joint.pose = 2;
    else
        entity->ext.joint.pose = stopped;
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 4;
    if (entity->enemyId == 0x30)
        frames = frames30;
    else if (entity->enemyId == 0x68)
        frames = frames68;
    else if (entity->enemyId == 0x48)
        frames = frames48;
    else if (entity->enemyId == 0x66)
        frames = frames66;
    for (i = 0; i <= 7; i++)
    {
        index = order[i];
        part = Entity_CreateObject(DevilPart_Update);
        if (part != NULL)
        {
            if ((u32)index <= 1 || index == 4 || index == 5)
                part->spriteLayer = entity->spriteLayer + 1;
            else
                part->spriteLayer = entity->spriteLayer - 1;
            Sprite_Attach(part, sDevilSpriteSheet, ObjGfx_LoadResource(sDevilGfx, 0), entity->basePalette);
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
            if (index > 3)
                part->flipX = 1;
            else
                part->flipX = 0;
            if (index == 0 || index == 2 || index == 3 || index == 4 || index == 6 || index == 7)
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
                Hit_EnableContact(part, 8, DevilPart_DamagePlayerOnContact, DevilPart_OnHit);
        }
        else
        {
            Heap_Free(entity->ext.devil.parts);
            EntityDelete(entity);
            return;
        }
    }
    pose = sDevilPartPoses[(s8)entity->ext.joint.pose];
    angles = sDevilPartAngles[(s8)entity->ext.joint.pose];
    parts = entity->ext.devil.parts;
    for (index = 0; index <= 7; index++)
    {
        parts[index].offsetX = pose[index * 2] << 16;
        parts[index].offsetY = pose[index * 2 + 1] << 16;
        parts[index].rotationAngle = angles[index];
    }
    if (entity->enemyId == 0x30)
        Anim_StartIndexed(entity, 10, 0, 1);
    else if (entity->enemyId == 0x68)
        Anim_StartIndexed(entity, 10, 0, 1);
    else if (entity->enemyId == 0x48)
        Anim_StartIndexed(entity, 2, 0, 1);
    else if (entity->enemyId == 0x66)
        Anim_StartIndexed(entity, 6, 0, 1);
    Anim_Update(entity);
    frozen = *((u8 *)gEwramData + 0x8E) & 0x40;
    if (frozen)
    {
        entity->state = 3;
        entity->subState = 0;
        entity->timer = 0;
        return;
    }
    Hit_Enable(entity, 8, Devil_DamagePlayerOnContact, Devil_OnHit);
    entity->timer = 1;
    DevilPart_Attach(entity, 3, 2, 0x5000, 0xB0000);
    DevilPart_Attach(entity, 7, 6, -0x3000, 0xB0000);
    Devil_UpdateAllParts(entity);
    Devil_ResetPartAngles(entity, 3);
    Devil_ResetPartAngles(entity, 7);
    Entity_UpdateBoxes(entity);
    entity->state = frozen;
    entity->subState = 10;
    entity->hidden = 1;
    entity->ext.joint.hurtMask = 0x88;
    entity->ext.devil.intangible = 1;
}

/* Devil flight state machine; DEVIL_WINGS is a macro (an inline function loses constant CSE and gets cross-jumped). */

#define DEVIL_WINGS(entity) \
{ \
    switch (entity->stage) \
    { \
    case 0: \
        Devil_UpdateAllParts(entity); \
        if ((s8)--entity->timer <= 2) \
        { \
            entity->timer = 8; \
            entity->stage++; \
        } \
        break; \
    case 1: \
        Devil_UpdateAllParts(entity); \
        if ((s8)--entity->timer > 0) \
            break; \
        entity->timer = 0x16; \
        entity->stage++; \
        entity->ext.joint.pose = 2; \
        DevilPart_Attach(entity, 1, 0, 0xA00, 0xA0000); \
        DevilPart_Attach(entity, 2, 0, 0xA00, 0xD0000); \
        DevilPart_Attach(entity, 3, 2, 0x800, 0xB0000); \
        DevilPart_Attach(entity, 5, 4, -0xA00, 0xA0000); \
        DevilPart_Attach(entity, 6, 4, -0xA00, 0xD0000); \
        DevilPart_Attach(entity, 7, 6, -0x800, 0xB0000); \
        break; \
    case 2: \
        Devil_UpdateAllParts(entity); \
        if ((s8)--entity->timer <= 2) \
        { \
            entity->timer = 8; \
            entity->stage++; \
        } \
        break; \
    case 3: \
        Devil_UpdateAllParts(entity); \
        if ((s8)--entity->timer > 0) \
            break; \
        entity->timer = 0x16; \
        entity->stage = 0; \
        Devil_RestoreParts(entity); \
        entity->ext.joint.pose = 1; \
        DevilPart_Attach(entity, 1, 0, -0xA00, 0xA0000); \
        DevilPart_Attach(entity, 2, 0, -0xA00, 0xD0000); \
        DevilPart_Attach(entity, 3, 2, -0x800, 0xB0000); \
        DevilPart_Attach(entity, 5, 4, 0xA00, 0xA0000); \
        DevilPart_Attach(entity, 6, 4, 0xA00, 0xD0000); \
        DevilPart_Attach(entity, 7, 6, 0x800, 0xB0000); \
        break; \
    } \
}

/**
 * @brief Flight state machine (state 0): face the target and wait until it is
 * within 0x50 px or the Devil is damaged, then fly with rising speed
 * (Entity_MoveTowardPoint) to a point 0x28 px beside the target and 0..7 px
 * above it; on arrival, or once the target is close (|dx| < 0x2C,
 * |dy| < 0x10 px), switch to the attack state. Further sub-states drift and
 * retreat. Hidden (unk_50B) while above the screen top; DEVIL_WINGS flaps.
 */
void Devil_UpdateApproach(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];

    if (entity->posY.part16.integer < -0x18)
        entity->ext.devilParts.hidden = 1;
    else
        entity->ext.devilParts.hidden = 0;

    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        if (!Target_IsInRect(entity->posX.part16.integer - 0x50, entity->posY.part16.integer - 0x50, 0xA0, 0xA0)
            && !Enemy_IsDamaged(entity))
            break;
        entity->subState++;
        entity->stage = 0;
        if (entity->flipX)
            entity->ext.devil.targetX = Target_GetX(entity) + ({ s32 t = bg->xPos.whole + -0x280000; t; });
        else
            entity->ext.devil.targetX = Target_GetX(entity) + ({ s32 t = bg->xPos.whole + 0x280000; t; });
        entity->ext.devil.targetY = Target_GetY(entity) - (RandomNumberGenerator() & 0x7FFFF) + bg->yPos.whole;
        Anim_StartIndexed(entity, entity->animScriptIndex + 1, 0, 1);
        entity->timer = 0x16;
        entity->ext.joint.pose = 1;
        DevilPart_Attach(entity, 1, 0, -0xA00, 0xA0000);
        DevilPart_Attach(entity, 2, 0, -0xA00, 0xD0000);
        DevilPart_Attach(entity, 3, 2, -0x800, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0xA00, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0xA00, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x800, 0xB0000);
        break;
    case 1:
    {
        s32 done;
        s32 d;

        entity->flipX = Entity_IsTargetRight(entity);
        entity->ext.devil.speed += 0x1000;
        if ((s32)entity->ext.devil.speed > 0x1FFFF)
            entity->ext.devil.speed = 0x20000;
        done = Entity_MoveTowardPoint(entity, entity->ext.devil.targetX - bg->xPos.whole,
                            entity->ext.devil.targetY - bg->yPos.whole, entity->ext.devil.speed);
        if (done)
        {
            entity->state = 1;
            entity->subState = 0;
        }
        else
        {
            d = Target_GetX(entity) - entity->posX.whole;
            if (d < 0)
                d = -d;
            if (d <= 0x2BFFFF)
            {
                d = Target_GetY(entity) - entity->posY.whole;
                if (d < 0)
                    d = -d;
                if (d <= 0xFFFFF)
                {
                    entity->state = 1;
                    entity->subState = done;
                }
            }
        }
        DEVIL_WINGS(entity)
        break;
    }
    case 2:
        entity->flipX = Entity_IsTargetRight(entity);
        if (entity->flipX)
            entity->accelX.whole = -0x800;
        else
            entity->accelX.whole = 0x800;
        entity->accelY.whole = -0x800;
        entity->subState++;
        entity->ext.devil.timer = 0;
    case 3:
        entity->flipX = Entity_IsTargetRight(entity);
        EntityAccelerateAndMove(entity, 0x20000);
        if (entity->ext.devil.timer++ > 0x1D)
        {
            entity->subState++;
            entity->ext.devil.timer = 0;
        }
        DEVIL_WINGS(entity)
        break;
    case 4:
        EntityDampVelocityX(entity, 0x18);
        EntityDampVelocityY(entity, 0x18);
        EntityMove(entity);
        if (entity->ext.devil.timer++ > 0x1D)
        {
            entity->subState = 1;
            if (entity->flipX)
                entity->ext.devil.targetX = Target_GetX(entity) + ({ s32 t = bg->xPos.whole + -0x280000; t; });
            else
                entity->ext.devil.targetX = Target_GetX(entity) + ({ s32 t = bg->xPos.whole + 0x280000; t; });
            entity->ext.devil.targetY = Target_GetY(entity) - (RandomNumberGenerator() & 0x7FFFF) + bg->yPos.whole;
            entity->ext.devil.speed = 0;
        }
        DEVIL_WINGS(entity)
        break;
    case 10:
        entity->flipX = Entity_IsTargetRight(entity);
        entity->ext.devil.intangible = 1;
        if (Target_IsInRect(entity->posX.part16.integer - 0x50, entity->posY.part16.integer - 0x50, 0xA0, 0xA0))
        {
            entity->subState++;
            entity->timer = 0x28;
            PlaySong(0x6E);
        }
        break;
    case 11:
    {
        s32 r = Entity_GetVisibilityPattern(entity->timer);

        entity->hidden = r ^ 1;
        entity->ext.devil.intangible = 1;
    }
        if ((s8)--entity->timer > 0)
            break;
        entity->subState++;
        entity->timer = 8;
        entity->hidden = 0;
        entity->ext.devil.intangible = 0;
        entity->ext.joint.pose = 3;
        Anim_StartIndexed(entity, entity->animScriptIndex + 1, 0, 1);
        DevilPart_Attach(entity, 1, 0, 0x10000, 0xA0000);
        DevilPart_Attach(entity, 2, 0, 0x10000, 0xD0000);
        DevilPart_Attach(entity, 3, 2, 0x10000, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0x18000, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0x18000, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x18000, 0xB0000);
        break;
    case 12:
        entity->flipX = Entity_IsTargetRight(entity);
        Devil_UpdateAllParts(entity);
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = 0x14;
        entity->subState++;
        DevilPart_Attach(entity, 1, 0, 0, 0xA0000);
        DevilPart_Attach(entity, 2, 0, 0, 0xD0000);
        DevilPart_Attach(entity, 3, 2, 0, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0, 0xB0000);
        break;
    case 13:
        entity->flipX = Entity_IsTargetRight(entity);
        Devil_UpdateAllParts(entity);
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = 0x16;
        entity->subState++;
        entity->ext.joint.pose = 2;
        DevilPart_Attach(entity, 1, 0, 0x12A00, 0xA0000);
        DevilPart_Attach(entity, 2, 0, 0x12A00, 0xD0000);
        DevilPart_Attach(entity, 3, 2, 0x14800, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0x15600, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0x15600, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x13800, 0xB0000);
        break;
    case 14:
        entity->flipX = Entity_IsTargetRight(entity);
        Devil_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 2)
        {
            entity->timer = 8;
            entity->subState++;
        }
        break;
    case 15:
        entity->flipX = Entity_IsTargetRight(entity);
        Devil_UpdateAllParts(entity);
        if ((s8)--entity->timer > 0)
            break;
        Devil_SaveParts(entity);
        entity->timer = 0x16;
        entity->subState = 1;
        if (entity->flipX)
            entity->ext.devil.targetX = Target_GetX(entity) + ({ s32 t = bg->xPos.whole + -0x280000; t; });
        else
            entity->ext.devil.targetX = Target_GetX(entity) + ({ s32 t = bg->xPos.whole + 0x280000; t; });
        entity->ext.devil.targetY = Target_GetY(entity) - (RandomNumberGenerator() & 0x7FFFF) + bg->yPos.whole;
        entity->ext.joint.pose = 1;
        DevilPart_Attach(entity, 1, 0, -0xA00, 0xA0000);
        DevilPart_Attach(entity, 2, 0, -0xA00, 0xD0000);
        DevilPart_Attach(entity, 3, 2, -0x800, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0xA00, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0xA00, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x800, 0xB0000);
        break;
    }
}

/* Devil (08220A38 skeleton enemy) main state machine. */

static inline void DevilStep(struct EwramData_EntityData *entity)
{
    entity->flipX = Entity_IsTargetRight(entity);
    EntityDampVelocityX(entity, 0x18);
    EntityDampVelocityY(entity, 0x10);
    EntityMove(entity);
    Devil_UpdateAllParts(entity);
}

/**
 * @brief Attack state machine (state 1): types other than 0x30 randomly raise the
 * wings and throw a projectile at the target (DevilProjectile_Spawn, sound
 * 0x78; types 0x48 / 0x66 first show DevilChargeEffect_Spawn above the
 * head); otherwise a wing-beat lunge. Both end in state 0 aimed at a new
 * point beside the target.
 */
void Devil_UpdateAttack(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];

    switch (entity->subState)
    {
    case 0:
        if (entity->enemyId != 0x30 && (RandomNumberGenerator() & 0x7F) > 0x40)
        {
            entity->ext.joint.pose = 4;
            entity->timer = 0xE;
            Anim_StartIndexed(entity, entity->animScriptIndex + 1, 0, 0);
            DevilPart_Attach(entity, 1, 0, 0x10000, 0xA0000);
            DevilPart_Attach(entity, 2, 0, 0x10000, 0xD0000);
            DevilPart_Attach(entity, 3, 2, 0x10000, 0xB0000);
            DevilPart_Attach(entity, 5, 4, 0x18000, 0xA0000);
            DevilPart_Attach(entity, 6, 4, 0x18000, 0xD0000);
            DevilPart_Attach(entity, 7, 6, 0x18000, 0xB0000);
            entity->subState = 10;
            EntityDampVelocityX(entity, 0x18);
            EntityDampVelocityY(entity, 0x10);
            EntityMove(entity);
            break;
        }
        entity->animFlags |= 2;
        entity->ext.joint.pose = 0;
        entity->timer = 0xE;
        DevilPart_Attach(entity, 1, 0, 0x12000, 0xA0000);
        DevilPart_Attach(entity, 2, 0, 0x12000, 0xD0000);
        DevilPart_Attach(entity, 3, 2, 0x19000, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0x16000, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0x16000, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x11000, 0xB0000);
        entity->subState++;
    case 1:
        DevilStep(entity);
        if ((s8)--entity->timer <= 2)
        {
            entity->timer = 0x12;
            entity->subState++;
        }
        break;
    case 2:
        DevilStep(entity);
        if ((s8)--entity->timer > 0)
            break;
        entity->subState++;
        entity->timer = 8;
        entity->ext.joint.pose = 3;
        PlaySong(0x86);
        DevilPart_Attach(entity, 1, 0, 0x10000, 0xA0000);
        DevilPart_Attach(entity, 2, 0, 0x10000, 0xD0000);
        DevilPart_Attach(entity, 3, 2, 0x10000, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0x18000, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0x18000, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x18000, 0xB0000);
        break;
    case 3:
        DevilStep(entity);
        if ((s8)--entity->timer <= 0)
        {
            entity->timer = 0x10;
            entity->subState++;
        }
        break;
    case 4:
        DevilStep(entity);
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = 0x16;
        entity->subState++;
        entity->ext.joint.pose = 2;
        Anim_StartIndexed(entity, entity->animScriptIndex, 0, 1);
        DevilPart_Attach(entity, 1, 0, 0x12A00, 0xA0000);
        DevilPart_Attach(entity, 2, 0, 0x12A00, 0xD0000);
        DevilPart_Attach(entity, 3, 2, 0x14800, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0x15600, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0x15600, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x13800, 0xB0000);
        break;
    case 6:
        DevilStep(entity);
        if ((s8)--entity->timer > 0)
            break;
        Devil_SaveParts(entity);
        entity->timer = 0x16;
        entity->state = 0;
        entity->subState = 2;
        entity->stage = 0;
        entity->ext.joint.pose = 1;
        entity->ext.devil.speed = 0;
        if (entity->flipX)
            entity->ext.devil.targetX = Target_GetX(entity) + ({ s32 t = bg->xPos.whole + -0x280000; t; });
        else
            entity->ext.devil.targetX = Target_GetX(entity) + ({ s32 t = bg->xPos.whole + 0x280000; t; });
        entity->ext.devil.targetY = Target_GetY(entity) - (RandomNumberGenerator() & 0x7FFFF) + bg->yPos.whole;
        DevilPart_Attach(entity, 1, 0, -0xA00, 0xA0000);
        DevilPart_Attach(entity, 2, 0, -0xA00, 0xD0000);
        DevilPart_Attach(entity, 3, 2, -0x800, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0xA00, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0xA00, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x800, 0xB0000);
        break;
    case 10:
        DevilStep(entity);
        if ((s8)--entity->timer <= 2)
        {
            entity->timer = 0x3C;
            entity->subState++;
            entity->stage = 0;
        }
        break;
    case 11:
        DevilStep(entity);
        if (entity->stage == 0 && entity->animDone)
        {
            entity->stage++;
            Anim_StartIndexed(entity, entity->animScriptIndex + 1, 0, 1);
        }
        if (entity->timer == 0x28
            && (entity->enemyId == 0x48 || entity->enemyId == 0x66))
            DevilChargeEffect_Spawn(entity, 0, -0x300000, entity->timer);
        if (entity->timer == 0x1E)
        {
            s32 dx = Target_GetX(entity) - entity->posX.whole;
            s32 dy = Target_GetY(entity) - ({ s32 t = entity->posY.whole + -0x200000; t; });
            u16 angle = ArcTan2(dx < 0 ? -(-dx >> 16) : dx >> 16, dy < 0 ? -(-dy >> 16) : dy >> 16);
            s32 speed;

            dx = Sin(angle + 0x4000);
            dy = Sin(angle);
            speed = 0x20000;
            dx = FixedMultiply(dx, speed);
            dy = FixedMultiply(dy, speed);
            DevilProjectile_Spawn(entity, entity->flipX ? speed : -0x20000, -0x360000, dx, dy, entity->timer);
            PlaySong(0x78);
        }
        if ((s8)--entity->timer > 0)
            break;
        entity->subState++;
        entity->timer = 1;
        entity->ext.joint.pose = 3;
        DevilPart_Attach(entity, 1, 0, 0x10000, 0xA0000);
        DevilPart_Attach(entity, 2, 0, 0x10000, 0xD0000);
        DevilPart_Attach(entity, 3, 2, 0x10000, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0x18000, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0x18000, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x18000, 0xB0000);
        break;
    case 12:
        DevilStep(entity);
        if ((s8)--entity->timer <= 0)
        {
            entity->timer = 0x30;
            entity->subState++;
        }
        break;
    case 13:
        DevilStep(entity);
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = 0x16;
        entity->subState++;
        entity->ext.joint.pose = 2;
        Anim_StartIndexed(entity, entity->animScriptIndex - 2, 0, 1);
        DevilPart_Attach(entity, 1, 0, 0x12A00, 0xA0000);
        DevilPart_Attach(entity, 2, 0, 0x12A00, 0xD0000);
        DevilPart_Attach(entity, 3, 2, 0x14800, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0x15600, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0x15600, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x13800, 0xB0000);
        break;
    case 5:
    case 14:
        DevilStep(entity);
        if ((s8)--entity->timer <= 2)
        {
            entity->timer = 8;
            entity->subState++;
        }
        break;
    case 15:
        DevilStep(entity);
        if ((s8)--entity->timer > 0)
            break;
        Devil_SaveParts(entity);
        entity->timer = 0x16;
        entity->state = 0;
        entity->subState = 1;
        entity->ext.joint.pose = 1;
        entity->ext.devil.speed = 0;
        if (entity->flipX)
            entity->ext.devil.targetX = Target_GetX(entity) + ({ s32 t = bg->xPos.whole + -0x280000; t; });
        else
            entity->ext.devil.targetX = Target_GetX(entity) + ({ s32 t = bg->xPos.whole + 0x280000; t; });
        entity->ext.devil.targetY = Target_GetY(entity) - (RandomNumberGenerator() & 0x7FFFF) + bg->yPos.whole;
        DevilPart_Attach(entity, 1, 0, -0xA00, 0xA0000);
        DevilPart_Attach(entity, 2, 0, -0xA00, 0xD0000);
        DevilPart_Attach(entity, 3, 2, -0x800, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0xA00, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0xA00, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x800, 0xB0000);
        break;
    }
}

/**
 * @brief 91204 | Swinging body-part controller: every 22 frames (timer)
 * flips the swing direction (unk_504 = 1 / 2) and re-launches the six parts
 * via DevilPart_Attach (index, parent, angular speed +-0xA00 / +-0x800, radius
 * 0xA0000 / 0xD0000 / 0xB0000); states 1 and 3 pause until the counter
 * drops to 2. Devil_UpdateAllParts updates the parts every frame.
 */
void Devil_UpdateHover(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        entity->subState++;
        Anim_StartIndexed(entity, entity->animScriptIndex + 1, 0, 1);
        entity->timer = 0x16;
        entity->ext.joint.pose = 1;
        Devil_SaveParts(entity);
        DevilPart_Attach(entity, 1, 0, -0xA00, 0xA0000);
        DevilPart_Attach(entity, 2, 0, -0xA00, 0xD0000);
        DevilPart_Attach(entity, 3, 2, -0x800, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0xA00, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0xA00, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x800, 0xB0000);
        break;
    case 2:
        Devil_UpdateAllParts(entity);
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = 0x16;
        entity->subState++;
        entity->ext.joint.pose = 2;
        DevilPart_Attach(entity, 1, 0, 0xA00, 0xA0000);
        DevilPart_Attach(entity, 2, 0, 0xA00, 0xD0000);
        DevilPart_Attach(entity, 3, 2, 0x800, 0xB0000);
        DevilPart_Attach(entity, 5, 4, -0xA00, 0xA0000);
        DevilPart_Attach(entity, 6, 4, -0xA00, 0xD0000);
        DevilPart_Attach(entity, 7, 6, -0x800, 0xB0000);
        break;
    case 1:
    case 3:
        Devil_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 2)
        {
            entity->timer = 8;
            entity->subState++;
        }
        break;
    case 4:
        Devil_UpdateAllParts(entity);
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = 0x16;
        entity->subState = 1;
        entity->ext.joint.pose = 1;
        Devil_RestoreParts(entity);
        DevilPart_Attach(entity, 1, 0, -0xA00, 0xA0000);
        DevilPart_Attach(entity, 2, 0, -0xA00, 0xD0000);
        DevilPart_Attach(entity, 3, 2, -0x800, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0xA00, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0xA00, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x800, 0xB0000);
        break;
    }
}

/* Load the graphics 081CC014 / 08220A38 with the palette variant of
 * 0820A9A0 chosen by the type byte (0x30, 0x68, 0x48, 0x66 -> 2, 3, 0, 1;
 * other types keep basePalette) and draw with Devil_Draw; 0 when the graphics
 * are not loaded. */

/**
 * @brief Load the Devil graphics and the palette variant of its enemy ID (0x30,
 * 0x68, ...) and attach the sprite set; return 0 when no graphics slot is
 * free, else 1.
 */
u8 Devil_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics = ObjGfx_LoadResource(sDevilGfx, 0);
    if (graphics < 0)
        return 0;
    if (entity->enemyId == 0x30)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sDevilPalette, 2);
    else if (entity->enemyId == 0x68)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sDevilPalette, 3);
    else if (entity->enemyId == 0x48)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sDevilPalette, 0);
    else if (entity->enemyId == 0x66)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sDevilPalette, 1);
    Sprite_Attach(entity, sDevilSpriteSheet, graphics, entity->basePalette);
    entity->drawFunc = Devil_Draw;
    return 1;
}

/* Interpolate a Devil joint toward its pose or its hanging angle. */
void Devil_UpdatePart(struct EwramData_EntityData *entity, s32 index)
{
    struct DevilPart *parts = entity->ext.devil.parts;
    const s8 *pose = sDevilPartPoses[(s8)entity->ext.joint.pose];
    const s32 *angles = sDevilPartAngles[(s8)entity->ext.joint.pose];
    struct DevilPart *part = (struct DevilPart *)(index * sizeof(struct DevilPart) + (u32)parts);
    s32 x, y, dx, dy, a;
    if (part->targetSwingAngle == 0 && part->swingAngle == 0)
    {
        x = pose[index * 2] * 0x10000;
        y = pose[index * 2 + 1] * 0x10000;
        dx = part->offsetX;
        x -= dx;
        dy = part->offsetY;
        y -= dy;
        dx += x / entity->timer;
        part->offsetX = dx;
        dy += y / entity->timer;
        part->offsetY = dy;
        x = angles[index];
        a = part->rotationAngle;
        x -= a;
        a += x / entity->timer;
        part->rotationAngle = a;
    }
    else
    {
        struct DevilPart *second = (struct DevilPart *)(index * sizeof(struct DevilPart) + (u32)parts);
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
            second->offsetX = parts[second->anchorYOrPartIndex].offsetX + x;
            second->offsetY = parts[second->anchorYOrPartIndex].offsetY + y;
        }
        else
        {
            second->offsetX = second->anchorXOrSentinel + x;
            second->offsetY = second->anchorYOrPartIndex + y;
        }
        x = angles[index];
        second = (struct DevilPart *)(index * sizeof(struct DevilPart));
        second = (struct DevilPart *)((u32)second + (u32)parts);
        a = second->rotationAngle;
        x -= a;
        a += x / entity->timer;
        second->rotationAngle = a;
    }
}

/* DevilPart_Attach (Devil joint set-up, like CreakingSkull_AttachJoint on struct DevilPart
 * without mirroring). */

/**
 * @brief Bind a DevilPart to its parent with supplied or measured length; set measured angle and absolute-or-relative target angle without facing mirroring.
 */
void DevilPart_Attach(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length)
{
    struct DevilPart *parts = entity->ext.devil.parts;
    // &parts[partIndex], written as integer arithmetic (offset + base) as in
    // the original
    struct DevilPart *part = (struct DevilPart *)(partIndex * sizeof(struct DevilPart) + (u32)parts);
    s32 dx;
    s32 dy;
    s32 baseAngle;
    s32 angle;
    struct DevilPart *anglePart;

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
    anglePart = (struct DevilPart *)(partIndex * sizeof(struct DevilPart) + (u32)parts);
    anglePart->swingAngle = baseAngle = (u16)angle + 0x10000;
    if ((angleSpec < 0 ? -angleSpec : angleSpec) > 0xFFFF)
        anglePart->targetSwingAngle = angleSpec;
    else
        anglePart->targetSwingAngle = baseAngle + angleSpec;
}

/* Apply a hit to the Devil's owner at the struck part's position. */
void DevilPart_OnHit(struct EwramData_EntityData *input, s32 source, u16 *collision)
{
    struct EwramData_EntityData *entity = input;
    s32 attacker = source;
    u16 *hit = collision;
    register struct EwramData_EntityData *owner asm("r5") = entity->ext.jointPart.owner; /* FAKEMATCH: pinned register */
    s32 oldX = owner->posX.whole;
    s32 oldY = owner->posY.whole;
    s16 damage;
    s16 power;
    owner->posX.whole = entity->posX.whole;
    owner->posY.whole = entity->posY.whole;
    power = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(entity->ext.jointPart.owner, attacker, hit, power);
    owner->posX.whole = oldX;
    owner->posY.whole = oldY;
    Entity_ShowDamage(entity, (s16)damage, 0);
    owner = entity->ext.jointPart.owner;
    owner->hitFlashTimer = 8;
    {
    s32 hp = owner->hp - damage;
    owner->hp = hp;
    if ((s16)hp <= 0)
    {
        Enemy_DropRewards(owner);
        PlaySong(0x72);
        owner->palette = owner->basePalette;
        owner->timer = 40;
        owner->state = 2;
        owner->subState = 0;
    }
}
}

/* Devil part update: follows its joint of the parent's skeleton, aims the
 * sprite along the bone, maintains the hit box and blinks away on death. */

struct DevilBox {
    s8 x;
    s8 y;
    s8 width;
    s8 height;
};

#define JOINT(e) ((s8)e->ext.jointPart.index)
#define FIX_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)
#define HAS_SPRITE(e) (JOINT(e) == 0 || JOINT(e) == 2 || JOINT(e) == 3 || JOINT(e) == 4 || JOINT(e) == 6 || JOINT(e) == 7)

/**
 * @brief Part entity update: follow its joint of the parent's skeleton, aim the
 * sprite along the bone, maintain the hit box and blink away on death.
 */
void DevilPart_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    u8 timers[8];
    struct DevilBox box;
    struct EwramData_EntityData *parent = entity->ext.jointPart.owner;
    struct DevilPart *parts;
    s32 size;
    s32 x;
    s32 y;

    memcpy(timers, sDevilPartTimers, sizeof(timers));
    if (entity->state == 0)
    {
        if (parent != NULL && parent->updateFunc != NULL
            && parent->enemyId == entity->enemyId)
        {
            parts = parent->ext.devil.parts;
            entity->palette = parent->palette;
            if (!parent->hidden && (s8)parent->ext.devilParts.hidden == 0)
                entity->hidden = 0;
            else
                entity->hidden = 1;
            entity->posX.whole = parent->posX.whole + parts[JOINT(entity)].offsetX;
            entity->posY.whole = parent->posY.whole + parts[JOINT(entity)].offsetY;
            if (JOINT(entity) == 2 || JOINT(entity) == 6)
            {
                x = parts[(s8)entity->ext.jointPart.anchor].offsetX;
                x -= parts[JOINT(entity)].offsetX;
                y = parts[(s8)entity->ext.jointPart.anchor].offsetY;
                y -= parts[JOINT(entity)].offsetY;
                entity->velX.whole = ArcTan2(FIX_INT(x), FIX_INT(y)) - 0x4000;
            }
            else if (JOINT(entity) == 0 || JOINT(entity) == 3 || JOINT(entity) == 4 || JOINT(entity) == 7)
            {
                entity->velX.whole = parts[JOINT(entity)].rotationAngle;
            }
            if (parent->state == 2)
            {
                entity->state = 1;
                entity->subState = 0;
                entity->ext.devilPart.angle = entity->velX.whole;
                entity->timer = timers[JOINT(entity)];
                return;
            }
            if ((parent->ext.joint.hurtMask >> JOINT(entity)) & 1)
            {
                size = 8;
                x = Sin(parts[JOINT(entity)].rotationAngle + 0x8000);
                y = Sin(parts[JOINT(entity)].rotationAngle + 0x4000);
                if (entity->flipX)
                    box.x = -(FIX_INT(x * size) + 4);
                else
                    box.x = FIX_INT(x * size) - 4;
                box.y = FIX_INT(y * size) - 4;
                box.width = 8;
                box.height = 8;
                HitBox_SetBody(entity, &box);
                if (entity->animFrame == 3)
                {
                    size = 0x14;
                    x = Sin(parts[JOINT(entity)].rotationAngle + 0x8000);
                    y = Sin(parts[JOINT(entity)].rotationAngle + 0x4000);
                    if (entity->flipX)
                        box.x = -(FIX_INT(x * size) + 4);
                    else
                        box.x = FIX_INT(x * size) - 4;
                    box.y = FIX_INT(y * size) - 4;
                    box.width = 12;
                    box.height = 12;
                }
                HitBox_SetAttack(entity, &box);
                entity->hit.flags &= 0xFD;
                entity->hit.flags &= 0xFB;
                if ((s8)parent->ext.devil.intangible != 1)
                    Hit_CheckWithPlayerSharedHistory(&result, entity, parent);
            }
            else
            {
                entity->hit.flags |= 4;
                entity->hit.flags |= 2;
            }
            if (HAS_SPRITE(entity))
                ObjAffine_Set(entity->velX.whole, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
            return;
        }
        if (HAS_SPRITE(entity))
            ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
    else
    {
        if (entity->subState == 0)
        {
            entity->subState++;
            entity->scrollWithBg = 1;
        }
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((u8)(entity->timer - 0x14) <= 0x14 && (entity->timer & 3) == 0)
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0x100000, 0x100000, entity->spriteLayer - 1);
        if (HAS_SPRITE(entity))
            ObjAffine_Set(entity->ext.devilPart.angle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
        if ((s8)--entity->timer <= 0)
        {
            if (HAS_SPRITE(entity))
                ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
        }
    }
}

/* Create a falling body part relative to its owner. */

/**
 * @brief Spawn a projectile at offset (x, y) from owner with launch velocity
 * (vx, vy) in the owner palette and type; it rides on the owner for timer
 * frames (DevilProjectile_Update) and damages on contact through
 * DevilProjectile_DamagePlayerOnContact.
 */
void DevilProjectile_Spawn(struct EwramData_EntityData *owner, s32 x, s32 y, s32 vx, s32 vy, u8 timer)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, DevilProjectile_Update);
    struct SpriteBox box = {-5, -5, 10, 10};
    if (part)
    {
        s32 graphics;
        part->posX.whole = owner->posX.whole + x;
        part->posY.whole = owner->posY.whole + y;
        part->velX.whole = vx;
        part->velY.whole = vy;
        part->ext.devilProjectile.offsetX = x;
        part->ext.devilProjectile.offsetY = y;
        part->spriteLayer = owner->spriteLayer - 2;
        graphics = ObjGfx_Find(sDevilGfx);
        part->basePalette = owner->basePalette;
        Sprite_Attach(part, sDevilSpriteSheet, graphics, part->basePalette);
        Anim_StartIndexed(part, 1, 0, 1);
        Anim_Update(part);
        Hit_Enable(part, 8, DevilProjectile_DamagePlayerOnContact, 0);
        HitBox_SetAttack(part, &box);
        part->hit.flags |= 4;
        part->hit.flags &= ~2;
        part->timer = timer;
        part->ext.devilProjectile.release = 0;
        part->enemyId = owner->enemyId;
        part->attack = owner->attack;
        part->ext.devilProjectile.owner = owner;
        part->scrollWithBg = 0;
        part->drawFunc = Sprite_DrawEntity;
    }
}

/* Parts of the 081CC014 / 08220A38 enemy attached to their owner. */

/* Ride on the owner at (unk_4F8, unk_4FC) while the timer timer runs
 * (sound 0x12A and two flashes when it expires; for types 0x48 / 0x66 the
 * hurt box 0x1818F4F4 = x -12, y -12, w 24, h 24 switches on at frame
 * unk_504), then fall; delete off screen. */
void DevilProjectile_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    struct EwramData_EntityData *owner = entity->ext.devilProjectile.owner;
    u32 box = 0x1818F4F4;

    if (entity->timer == 0)
    {
        entity->scrollWithBg = 1;
        EntityAccelerateAndMove(entity, 0x40000);
    }
    else
    {
        entity->timer--;
        entity->posX.whole = owner->posX.whole + entity->ext.devilProjectile.offsetX;
        entity->posY.whole = owner->posY.whole + entity->ext.devilProjectile.offsetY;
        if (entity->timer == 0)
        {
            PlaySong(0x12A);
            Afterimage_Create(entity, 0x7FFF, 12, 16);
            Afterimage_Create(entity, 0x7FFF, 6, 16);
        }
        if ((entity->enemyId == 0x48 || entity->enemyId == 0x66)
            && entity->ext.devilProjectile.release == entity->timer)
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            HitBox_SetAttack(entity, &box);
        }
    }
    if (EntityIsOutsideScreen(entity, 2))
        EntityDelete(entity);
    else
    {
        Anim_Update(entity);
        Hit_CheckWithPlayer(&result, entity);
    }
}

/* Spawn a part attached to owner at offset (dx, dy) (kept in unk_4F8 /
 * unk_4FC) in the owner palette, timer timer, the owner type byte and
 * attack; updated by DevilChargeEffect_Update. */
void DevilChargeEffect_Spawn(struct EwramData_EntityData *owner, s32 dx, s32 dy, u8 timer)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, DevilChargeEffect_Update);
    s32 graphics;

    if (entity == NULL)
        return;
    entity->posX.whole = owner->posX.whole + dx;
    entity->posY.whole = owner->posY.whole + dy;
    entity->ext.devilCharge.offsetX = dx;
    entity->ext.devilCharge.offsetY = dy;
    entity->spriteLayer = owner->spriteLayer - 1;
    graphics = ObjGfx_Find(sDevilGfx);
    entity->basePalette = owner->basePalette;
    Sprite_Attach(entity, sDevilSpriteSheet, graphics, entity->basePalette);
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    entity->timer = timer;
    entity->enemyId = owner->enemyId;
    entity->attack = owner->attack;
    entity->ext.devilCharge.owner = owner;
    entity->scrollWithBg = 0;
    entity->drawFunc = Sprite_DrawEntity;
}

/* Devil helpers 0x08091ED8..0x08092173 (also used by Flame Demon, Arc Demon
 * and Demon Lord). */

#define DEVIL_PART_COUNT 8

/**
 * @brief 91ED8 | Unless timer is 0, run Devil_UpdatePart on parts 0..7.
 */
void Devil_UpdateAllParts(struct EwramData_EntityData *entity)
{
    s32 i;

    if (entity->timer != 0)
    {
        for (i = 0; i < DEVIL_PART_COUNT; i++)
            Devil_UpdatePart(entity, i);
    }
}

/**
 * @brief 91EF8 | Unless timer is 0, run Devil_UpdatePart on the 8 parts
 * listed in order.
 */
void Devil_UpdateParts(struct EwramData_EntityData *entity, s8 *order)
{
    s32 i;

    if (entity->timer != 0)
    {
        for (i = 0; i < DEVIL_PART_COUNT; i++)
            Devil_UpdatePart(entity, order[i]);
    }
}

/**
 * @brief 91F20 | Face the direction given by Entity_IsTargetRight; when turning
 * around mirror all parts (x offset and unk_8).
 */
void Devil_FaceTarget(struct EwramData_EntityData *entity)
{
    u32 flipX = entity->flipX;
    struct DevilPart *parts;
    s32 i;

    if (flipX != Entity_IsTargetRight(entity))
    {
        entity->flipX ^= 1;
        parts = entity->ext.devil.parts;
        for (i = 0; i < DEVIL_PART_COUNT; i++)
        {
            parts[i].offsetX = -parts[i].offsetX;
            parts[i].rotationAngle = 0x20000 - parts[i].rotationAngle;
        }
    }
}

/**
 * @brief 91F70 | Clear unk_10 and unk_14 of part partIndex.
 */
void Devil_ResetPartAngles(struct EwramData_EntityData *entity, s32 partIndex)
{
    struct DevilPart *parts = entity->ext.devil.parts;

    parts[partIndex].targetSwingAngle = 0;
    parts[partIndex].swingAngle = 0;
}

/**
 * @brief 91F80 | Save the offsets of all parts.
 */
void Devil_SaveParts(struct EwramData_EntityData *entity)
{
    struct DevilPart *parts = entity->ext.devil.parts;
    s32 i;

    for (i = 0; i < DEVIL_PART_COUNT; i++)
    {
        parts[i].savedOffsetX = parts[i].offsetX;
        parts[i].savedOffsetY = parts[i].offsetY;
    }
}

/**
 * @brief 91F9C | Restore the offsets saved by Devil_SaveParts.
 */
void Devil_RestoreParts(struct EwramData_EntityData *entity)
{
    struct DevilPart *parts = entity->ext.devil.parts;
    s32 i;

    for (i = 0; i < DEVIL_PART_COUNT; i++)
    {
        parts[i].offsetX = parts[i].savedOffsetX;
        parts[i].offsetY = parts[i].savedOffsetY;
    }
}

/**
 * @brief 91FB8 | Player_ApplyContactDamage with (attack, 1, 0, 1) at the entity position.
 */
void DevilPart_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 1;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 91FE8 | Player_ApplyContactDamage with (attack, 2, 0, 0) at the entity position.
 */
void DevilProjectile_DamagePlayerOnContact(struct EwramData_EntityData *entity)
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
 * @brief 92018 | Follow the entity ext.devilCharge.owner at offset (unk_4F8, unk_4FC);
 * delete when the timer timer ends.
 */
void DevilChargeEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *devil = entity->ext.devilCharge.owner;
    u32 collision[4];

    entity->posX.whole = devil->posX.whole + entity->ext.devilCharge.offsetX;
    entity->posY.whole = devil->posY.whole + entity->ext.devilCharge.offsetY;
    if ((s8)--entity->timer <= 0)
        EntityDelete(entity);
}

/**
 * @brief Draw the Devil unless it is hidden above the screen top (unk_50B).
 */
void Devil_Draw(struct EwramData_EntityData *entity)
{
    if ((s8)entity->ext.devilParts.hidden == 0)
        Sprite_DrawEntity(entity);
}

/**
 * @brief 92064 | Player_ApplyContactDamage with (attack, 0, 0, 0) at the entity position
 * (same as Chaos2Part_DamagePlayerOnContact).
 */
void Devil_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief 92090 | Hit: damage from hit, damage number, then Devil_OnDamaged.
 */
void Devil_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s16 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    Entity_ShowDamage(entity, (s16)damage, 0);
    Devil_OnDamaged(entity, attacker, hit, damage);
}

/**
 * @brief 920D8 | Subtract damage from the HP in hp; at 0 die (state 2,
 * sound 0x72, 0x28 frames).
 */
void Devil_OnDamaged(struct EwramData_EntityData *entity, s32 attacker, u16 *hit, s32 damage)
{
    entity->hitFlashTimer = 8;
    entity->hp -= damage;
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x72);
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->state = 2;
        entity->subState = 0;
    }
}

/**
 * @brief 9212C | Flame Demon update: same as Devil.
 */
void EnemyFlameDemonUpdate(struct EwramData_EntityData *entity)
{
    EnemyDevilUpdate(entity);
}

/**
 * @brief 92138 | Flame Demon create: same as Devil.
 */
void EnemyFlameDemonCreate(struct EwramData_EntityData *entity)
{
    EnemyDevilCreate(entity);
}

/**
 * @brief 92144 | Arc Demon update: same as Devil.
 */
void EnemyArcDemonUpdate(struct EwramData_EntityData *entity)
{
    EnemyDevilUpdate(entity);
}

/**
 * @brief 92150 | Arc Demon create: same as Devil.
 */
void EnemyArcDemonCreate(struct EwramData_EntityData *entity)
{
    EnemyDevilCreate(entity);
}

/**
 * @brief 9215C | Demon Lord update: same as Devil.
 */
void EnemyDemonLordUpdate(struct EwramData_EntityData *entity)
{
    EnemyDevilUpdate(entity);
}

/**
 * @brief 92168 | Demon Lord create: same as Devil.
 */
void EnemyDemonLordCreate(struct EwramData_EntityData *entity)
{
    EnemyDevilCreate(entity);
}
