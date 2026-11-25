/*
 * Legion (boss): body, corpse, core, fragments, shell parts and shots.
 * One translation unit for the consecutive ROM range of its code; the
 * functions keep their ROM order.
 */
#include "core.h"
#include "entity_util.h"
#include "player_core.h"
#include "music.h"
#include "player_level.h"
#include "gameplay.h"
#include "structs/ewram.h"
#include "sound.h"
#include "bg_effects.h"
#include "structs/entity_render_flags.h"
#include "boss_defeat.h"
#include "effects.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "palette.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "math_inline.h"
#include "damage_numbers.h"
#include "entity_hit.h"

extern s32 Entity_MoveWithBoxCollision(struct EwramData_EntityData *, s32, s32, s32, s32, s32, s32);

void Legion_UpdateCircle(struct EwramData_EntityData *);
void Legion_UpdateBounce(struct EwramData_EntityData *);
void Legion_UpdateDeath(struct EwramData_EntityData *);
void Legion_UpdatePreview(struct EwramData_EntityData *);
s32 Legion_LoadGraphics(struct EwramData_EntityData *entity);
struct EwramData_EntityData *Legion_CreateCorePart(struct EwramData_EntityData *entity, s32 x, s32 y, s32 kind);
struct EwramData_EntityData *Legion_CreateCorpse(struct EwramData_EntityData *entity, s32 x, s32 y, s32 index);
void Legion_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void Legion_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
s32 Legion_UpdateIntro(struct EwramData_EntityData *entity);
void Legion_SteerX(struct EwramData_EntityData *entity, s32 target);
void Legion_SteerY(struct EwramData_EntityData *entity, s32 target);
s32 Legion_GetPathEdge(s32 x, s32 y, s32 size);
struct EwramData_EntityData *Legion_CreateShellPart(struct EwramData_EntityData *entity, s32 center, s32 kind);
void Legion_SpawnDebris(struct EwramData_EntityData *owner, s32 x, s32 y, s32 vx, s32 vy, s32 frame);
void LegionCorpse_Update(struct EwramData_EntityData *entity);
struct EwramData_EntityData *LegionCorePart_CreateDetail(struct EwramData_EntityData *entity, s32 x, s32 y, s32 kind, s32 tiles);
void LegionCorePart_Update(struct EwramData_EntityData *entity);
void LegionCorePart_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
void Legion_SpawnFragment(struct EwramData_EntityData *owner, s32 x, s32 y, s32 vx, s32 vy, s32 graphics);
void Legion_SpawnDestructibleFragment(struct EwramData_EntityData *owner, s32 x, s32 y, s32 vx, s32 vy, s32 graphics);
void LegionCoreDetail_Update(struct EwramData_EntityData *entity);
void LegionFragment_Update(struct EwramData_EntityData *);
void LegionDestructibleFragment_Update(struct EwramData_EntityData *);
void LegionDestructibleFragment_DamagePlayerOnContact(struct EwramData_EntityData *);
void LegionDestructibleFragment_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
void LegionShellPart_Update(struct EwramData_EntityData *entity);
void LegionShellPart_FireShot(struct EwramData_EntityData *owner, s32 dx, s32 dy, s32 unused, s32 angle);
void LegionShot_Update(struct EwramData_EntityData *entity);
void LegionShot_DamagePlayerOnContact(struct EwramData_EntityData *entity);

struct ShotFrame {
    u16 frame;
    u16 flags;
};

struct ShotFrames {
    struct ShotFrame f[24];
};

struct ShotData {
    s32 graphics;
    u8 pad_4[0x24 - 4];
    s32 palette;
};

extern const struct ShotFrames sLegionShotFrames;

void EnemyLegionUpdate(struct EwramData_EntityData *entity)
{
    /* Original scratch area is 0x30 bytes. */
    struct HitResult results[3]; /* only results[0] is written; the original reserved 48 bytes */
    if (entity->state != 2)
    {
        if (Entity_IsInRect(entity, -16, 0, 0x110, 0xB0)) Player_TrackEntity(entity);
        if (entity->state != 2) Enemy_UpdateHitFlash(entity);
    }
    if (Entity_IsTimeStopped(entity) && entity->state != 2)
    {
        Hit_CheckWithPlayer(results, entity);
        return;
    }
    switch (entity->state)
    {
    case 0: Legion_UpdateCircle(entity); break;
    case 1: Legion_UpdateBounce(entity); break;
    case 2:
        Legion_UpdateDeath(entity);
        if (entity->deleteRequested && entity->ext.legion.data.ptr != NULL)
            Heap_Free(entity->ext.legion.data.ptr);
        break;
    case 3: Legion_UpdatePreview(entity); Anim_Update(entity); return;
    }
    Anim_Update(entity);
    if (entity->state != 2 && !(entity->ext.legion.flags & 1))
        Hit_CheckWithPlayer(results, entity);
}

struct LegionData {
    u32 unk_0;
    u32 rotationAngle;
    s32 angularVelocity;
    s32 roomX;
    s32 roomY;
    s32 bobPhase;
    s32 shellActionTimer;
};

/**
 * @brief 9C6BC | Legion create: with spawnParam0 set it is deleted once beaten
 * (bossEventFlags bit 5) unless in the boss rush mode; a 0x1C-byte state block in
 * unk_4F8, animation sLegionAnimScript0 with its own palette and identity
 * affine matrix, four core parts Legion_CreateCorePart (part 3 instead of 10 when
 * spawnParam0 is set and time runs); frozen time gives state 3, otherwise box
 * 0x2020F0F0 (boxes off) and song 0x10; without spawnParam0 the HP bar
 * (Player_SetSelectedTarget), with it five shell parts Legion_CreateCorpse; then unk_506 = 6
 * and the block starts at the room position with a random 120..247 timer.
 */
void EnemyLegionCreate(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    u32 box = 0x2020F0F0;
    struct LegionData *data;
    u16 palette;
    u8 frozen;

    if (entity->spawnParam0 != 0 && !gEwramData->gameState.bossRushMode
        && (gEwramData->gameState.bossEventFlags & 0x20))
        { EntityDelete(entity); return; }
    entity->ext.legion.data.value = (s32)Heap_Alloc(0x1C);
    if (entity->ext.legion.data.value == 0)
        { EntityDelete(entity); return; }
    if ((u8)Legion_LoadGraphics(entity) == 0)
    {
        if (entity->ext.legion.data.value != 0)
            Heap_Free((void *)entity->ext.legion.data.value);
        EntityDelete(entity);
        return;
    }
    Anim_Start(entity, (struct AnimScript *)sLegionAnimScript0, 2, 1);
    Anim_Update(entity);
    palette = ObjAffine_Alloc(1);
    if ((s16)palette < 0)
        { EntityDelete(entity); return; }
    entity->affineIndex = palette;
    entity->affine = 1;
    ObjAffine_Set(0, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    Enemy_ResetFromTemplate(entity);
    data = (struct LegionData *)entity->ext.legion.data.value;
    if (!Legion_CreateCorePart(entity, -0x180000, 0x180000, 2))
        { EntityDelete(entity); return; }
    if (gEwramData->gameState.bestiaryPreview || entity->spawnParam0 == 0)
    {
        if (!Legion_CreateCorePart(entity, 0x170000, 0x180000, 10))
        {
            EntityDelete(entity);
            return;
        }
    }
    else
    {
        if (!Legion_CreateCorePart(entity, 0x170000, 0x180000, 3))
            goto delete;
    }
    if (!Legion_CreateCorePart(entity, 0x180000, -0x180000, 1))
        { EntityDelete(entity); return; }
    if (!Legion_CreateCorePart(entity, -0x180000, -0x180000, 0))
        { EntityDelete(entity); return; }
    frozen = *((u8 *)gEwramData + 0x8E) & 0x40;
    if (frozen)
    {
        entity->state = 3;
        return;
    }
    Hit_Enable(entity, 8, Legion_DamagePlayerOnContact, Legion_OnHit);
    HitBox_SetBody(entity, &box);
    HitBox_SetAttack(entity, &box);
    entity->hit.flags &= 0xFB;
    entity->hit.flags &= 0xFD;
    Music_SetCurrent(0x10);
    if (entity->spawnParam0 == 0)
    {
        entity->ext.legion.flags = 6;
        data->roomX = entity->posX.whole + bg->xPos.whole;
        data->roomY = entity->posY.whole + bg->yPos.whole;
        data->bobPhase = 0;
        data->angularVelocity = 0x100;
        data->shellActionTimer = (RandomNumberGenerator() & 0x7F) + 0x78;
        entity->state = 0;
        entity->subState = 1;
        Player_SetSelectedTarget(entity);
        gEwramData->gameState.stateFlags |= 2;
        return;
    }
    gEwramData->gameState.stateFlags |= 2;
    if (!Legion_CreateCorpse(entity, 0xA80000, 0x1BF0000, 2))
        { EntityDelete(entity); return; }
    if (!Legion_CreateCorpse(entity, 0xD80000, 0x1BF0000, 0))
        { EntityDelete(entity); return; }
    if (!Legion_CreateCorpse(entity, 0x1400000, 0x1BF0000, 1))
        { EntityDelete(entity); return; }
    if (!Legion_CreateCorpse(entity, 0xBC0000, 0x1BF0000, 4))
        { EntityDelete(entity); return; }
    if (!Legion_CreateCorpse(entity, 0x1190000, 0x1BF0000, 3))
    {
    delete:
        EntityDelete(entity);
        return;
    }
    entity->ext.legion.flags = 6;
    data->roomX = entity->posX.whole + bg->xPos.whole;
    data->roomY = entity->posY.whole + bg->yPos.whole;
    data->bobPhase = 0;
    data->angularVelocity = 0x100;
    data->shellActionTimer = (RandomNumberGenerator() & 0x7F) + 0x78;
    entity->state = 0;
}

/* Legion (state 0): circling movement and shell part spawning. */
struct LegionShell {
    s32 state;
    s32 actionState;
    s32 radiusSet;
    s32 unk_C;
    s32 angleOffset;
    s32 timer;
};

struct LegionState {
    u32 unk_0;
    s32 rotationAngle;
    s32 angularVelocity;
    s32 roomX;
    s32 roomY;
    s32 bobPhase;
    s32 shellActionTimer;
    s32 unk_1C[3];
    struct LegionShell shells[3];
};

#define DIR (*(u8 *)&entity->ext.legion.circleSide)

void Legion_UpdateCircle(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    struct LegionState *data = (struct LegionState *)entity->ext.legion.data.value;
    struct EwramData_EntityData *part;
    struct LegionShell *shell;
    s32 i;
    s32 dir;

    switch (entity->subState)
    {
    case 0:
        if (Legion_UpdateIntro(entity))
        {
            entity->subState++;
            entity->ext.legion.flags = 4;
        }
        break;
    case 1:
        if (data->roomY <= 0x1900000)
        {
            data->roomY += 0x8000;
            break;
        }
        {
            u8 two = 2;
            entity->subState = two;
            entity->stage = 0;
            entity->scrollWithBg = 0;
            entity->velX.whole = 0;
            entity->velY.whole = 0;
            entity->accelY.whole = 0;
        }
        if (Entity_IsTargetRight(entity))
            entity->ext.legion.circleDir = 2;
        else
            entity->ext.legion.circleDir = 1;
        entity->ext.legion.retargetTimer = (RandomNumberGenerator() % 64) + 0x3C;
        break;
    case 2:
        if (entity->ext.legion.circleDir == 1)
        {
            switch (DIR)
            {
            case 0:
                entity->accelX.whole = -0x800;
                EntityAccelerateX(entity, 0x8000);
                data->roomX += entity->velX.whole;
                Legion_SteerY(entity, 0x1900000);
                if (data->roomX <= 0x700000)
                {
                    data->roomX = 0x700000;
                    DIR = 1;
                }
                break;
            case 1:
                entity->accelY.whole = -0x800;
                EntityAccelerateY(entity, 0x8000);
                data->roomY += entity->velY.whole;
                Legion_SteerX(entity, 0x700000);
                if (data->roomY <= 0x700000)
                {
                    data->roomY = 0x700000;
                    DIR = 2;
                }
                break;
            case 2:
                entity->accelX.whole = 0x800;
                EntityAccelerateX(entity, 0x8000);
                data->roomX += entity->velX.whole;
                Legion_SteerY(entity, 0x700000);
                if (data->roomX > 0x18FFFFF)
                {
                    data->roomX = 0x1900000;
                    DIR = 3;
                }
                break;
            case 3:
                entity->accelY.whole = 0x800;
                EntityAccelerateY(entity, 0x8000);
                data->roomY += entity->velY.whole;
                Legion_SteerX(entity, 0x1900000);
                if (data->roomY > 0x18FFFFF)
                {
                    data->roomY = 0x1900000;
                    DIR = 0;
                }
                break;
            }
        }
        else
        {
            switch (DIR)
            {
            case 0:
                entity->accelX.whole = 0x800;
                EntityAccelerateX(entity, 0x8000);
                data->roomX += entity->velX.whole;
                Legion_SteerY(entity, 0x1900000);
                if (data->roomX > 0x18FFFFF)
                {
                    data->roomX = 0x1900000;
                    DIR = 3;
                }
                break;
            case 1:
                entity->accelY.whole = 0x800;
                EntityAccelerateY(entity, 0x8000);
                data->roomY += entity->velY.whole;
                Legion_SteerX(entity, 0x700000);
                if (data->roomY > 0x18FFFFF)
                {
                    data->roomY = 0x1900000;
                    DIR = 0;
                }
                break;
            case 2:
                entity->accelX.whole = -0x800;
                EntityAccelerateX(entity, 0x8000);
                data->roomX += entity->velX.whole;
                Legion_SteerY(entity, 0x700000);
                if (data->roomX <= 0x700000)
                {
                    data->roomX = 0x700000;
                    DIR = 1;
                }
                break;
            case 3:
                entity->accelY.whole = -0x800;
                EntityAccelerateY(entity, 0x8000);
                data->roomY += entity->velY.whole;
                Legion_SteerX(entity, 0x1900000);
                if (data->roomY <= 0x700000)
                {
                    data->roomY = 0x700000;
                    DIR = 2;
                }
                break;
            }
        }
        if ((u8)entity->ext.legion.retargetTimer-- <= 1)
        {
            entity->ext.legion.retargetTimer = (RandomNumberGenerator() % 64) + 0x3C;
            dir = Legion_GetPathEdge(Target_GetX(entity) + bg->xPos.whole,
                               Target_GetY(entity) + bg->yPos.whole + -0x300000, 0x300000);
            switch (DIR)
            {
            case 0:
                if (dir == 1)
                    entity->ext.legion.circleDir = 1;
                else if (dir == 3)
                    entity->ext.legion.circleDir = 2;
                else if (dir == 0)
                {
                    if (Entity_IsTargetRight(entity))
                        entity->ext.legion.circleDir = 2;
                    else
                        entity->ext.legion.circleDir = 1;
                }
                break;
            case 1:
                if (dir == 2)
                    entity->ext.legion.circleDir = 1;
                else if (dir == 0)
                    entity->ext.legion.circleDir = 2;
                else if (dir == 1)
                {
                    if (Entity_IsTargetBelow(entity))
                        entity->ext.legion.circleDir = 2;
                    else
                        entity->ext.legion.circleDir = 1;
                }
                break;
            case 2:
                if (dir == 1)
                    entity->ext.legion.circleDir = 2;
                else if (dir == 3)
                    entity->ext.legion.circleDir = 1;
                else if (dir == 2)
                {
                    if (Entity_IsTargetRight(entity))
                        entity->ext.legion.circleDir = 1;
                    else
                        entity->ext.legion.circleDir = 2;
                }
                break;
            case 3:
                if (dir == 2)
                    entity->ext.legion.circleDir = 2;
                else if (dir == 0)
                    entity->ext.legion.circleDir = 1;
                else if (dir == 3)
                {
                    if (Entity_IsTargetBelow(entity))
                        entity->ext.legion.circleDir = 1;
                    else
                        entity->ext.legion.circleDir = 2;
                }
                break;
            }
        }
        if (!(entity->ext.legion.flags & 8) && (entity->ext.legion.brokenCores & 0xF))
        {
            part = entity;
            for (i = 0; i <= 5; i++)
            {
                part = Legion_CreateShellPart(entity, (s32)part, i);
                if (part != NULL)
                {
                    part->ext.legionShell.radius = 0;
                    part->ext.legionShell.angle = (i + 1) * 5000;
                    part->ext.legionShell.delay = (6 - i) * 6;
                    part->ext.legionShell.index = 0;
                    switch (i)
                    {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                        Anim_Start(part, (struct AnimScript *)(sUnk_08528340 + 0x8), 2, 1);
                        Anim_Update(part);
                        break;
                    case 5:
                        Anim_Start(part, (struct AnimScript *)(sUnk_08528340 + 0x1C), 2, 1);
                        Anim_Update(part);
                        break;
                    }
                }
                else
                {
                    data->shells[0].state = 1;
                    return;
                }
            }
            data->shells[0].radiusSet = 0;
            data->shells[0].state = 2;
            data->shells[0].actionState = 2;
            data->shells[0].timer = (RandomNumberGenerator() % 128) + 0x78;
            entity->ext.legion.flags |= 8;
        }
        if (!(entity->ext.legion.flags & 0x10) && (entity->ext.legion.brokenCores & 0xF) == 0xF)
        {
            part = entity;
            for (i = 0; i <= 5; i++)
            {
                part = Legion_CreateShellPart(entity, (s32)part, i);
                if (part != NULL)
                {
                    part->ext.legionShell.radius = 0;
                    part->ext.legionShell.angle = (i + 1) * 5000;
                    part->ext.legionShell.delay = (6 - i) * 6;
                    part->ext.legionShell.index = 1;
                    switch (i)
                    {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                        Anim_Start(part, (struct AnimScript *)(sUnk_08528340 + 0x8), 2, 1);
                        Anim_Update(part);
                        break;
                    case 5:
                        Anim_Start(part, (struct AnimScript *)(sUnk_08528340 + 0x1C), 2, 1);
                        Anim_Update(part);
                        break;
                    }
                }
                else
                {
                    data->shells[1].state = 1;
                    return;
                }
            }
            data->shells[1].radiusSet = 1;
            data->shells[1].state = 2;
            data->shells[1].actionState = 2;
            data->shells[1].timer = (RandomNumberGenerator() % 128) + 0x78;
            entity->ext.legion.flags |= 0x10;
        }
        if (!(entity->ext.legion.flags & 0x20) && (entity->ext.legion.brokenCores & 0xF) == 0xF)
        {
            part = entity;
            for (i = 0; i <= 5; i++)
            {
                part = Legion_CreateShellPart(entity, (s32)part, i);
                if (part != NULL)
                {
                    part->ext.legionShell.radius = 0;
                    part->ext.legionShell.angle = (i + 1) * 5000;
                    part->ext.legionShell.delay = (6 - i) * 6;
                    part->ext.legionShell.index = 2;
                    switch (i)
                    {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                        Anim_Start(part, (struct AnimScript *)(sUnk_08528340 + 0x8), 2, 1);
                        Anim_Update(part);
                        break;
                    case 5:
                        Anim_Start(part, (struct AnimScript *)(sUnk_08528340 + 0x1C), 2, 1);
                        Anim_Update(part);
                        break;
                    }
                }
                else
                {
                    data->shells[2].state = 1;
                    return;
                }
            }
            data->shells[0].radiusSet = 1;
            data->shells[2].radiusSet = 1;
            data->shells[2].state = 2;
            data->shells[2].actionState = 2;
            data->shells[2].timer = (RandomNumberGenerator() % 128) + 0x78;
            entity->ext.legion.flags |= 0x20;
        }
        if (data->shells[0].state == 2 && data->shells[0].actionState == 0)
            data->shells[0].state = 0;
        if (data->shells[1].state == 2 && data->shells[1].actionState == 0)
            data->shells[1].state = 0;
        if (data->shells[2].state == 2 && data->shells[2].actionState == 0)
            data->shells[2].state = 0;
        if (entity->stage == 0)
        {
            if ((entity->ext.legion.brokenCores & 0xF) == 0xF && data->shells[0].state == 0
                && data->shells[1].state == 0 && data->shells[2].state == 0)
            {
                if (data->shellActionTimer == 0)
                {
                    entity->stage = 1;
                    return;
                }
                data->shellActionTimer--;
            }
            for (i = 0, shell = data->shells; i <= 2; shell++, i++)
            {
                if (shell->state == 0)
                {
                    /* the spawned-flag test reuses dir (its own callee-saved register) */
                    if (i == 0)
                        dir = *(u8 *)&entity->ext.legion.flags & 8;
                    else if (i == 1)
                        dir = *(u8 *)&entity->ext.legion.flags & 0x10;
                    else
                        dir = *(u8 *)&entity->ext.legion.flags & 0x20;
                    if (dir && shell->timer != 0 && --shell->timer == 0)
                    {
                        shell->state = 4;
                        shell->actionState = 4;
                        shell->timer = 0x3C;
                    }
                }
                else if (shell->state == 4)
                {
                    if (shell->timer != 0)
                        shell->timer--;
                    else if (shell->actionState == 0)
                    {
                        shell->state = 0;
                        shell->timer = (RandomNumberGenerator() % 128) + 0x78;
                    }
                }
                if (shell->state != 4)
                {
                    switch (i)
                    {
                    case 0:
                        data->shells[0].angleOffset += 0x87;
                        break;
                    case 1:
                        data->shells[1].angleOffset -= 0x76;
                        break;
                    case 2:
                        data->shells[2].angleOffset += 0xC3;
                        break;
                    }
                }
            }
        }
        else
        {
            data->shells[0].state = 3;
            data->shells[1].state = 3;
            data->shells[2].state = 3;
            data->shells[0].actionState = 3;
            data->shells[1].actionState = 3;
            data->shells[2].actionState = 3;
            entity->state = 1;
            entity->subState = 0;
            entity->stage = 0;
            entity->timer = 0;
            data->angularVelocity = 0x100;
        }
        break;
    }
    if (entity->ext.legion.flags & 2)
    {
        data->bobPhase += 0x180;
        entity->posX.whole = data->roomX - bg->xPos.whole;
        entity->posY.whole = data->roomY - bg->yPos.whole + Sin(data->bobPhase) * 3;
    }
    else
    {
        entity->posX.whole = data->roomX - bg->xPos.whole;
        entity->posY.whole = data->roomY - bg->yPos.whole;
    }
    if (entity->ext.legion.flags & 4)
    {
        data->rotationAngle += data->angularVelocity;
        ObjAffine_Set(data->rotationAngle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    }
}

/* Legion (state == 1): rises, bounces around the room, sinks back. */
#define ANIM(P) Anim_Start(entity, (struct AnimScript *)(sLegionAnimScript0 + (P)), 2, 1)
#define BOUNCE(R) \
    if ((R) & 0xF00) \
    { \
        s32 t = entity->velX.whole; \
        entity->velX.whole = entity->velY.whole; \
        entity->velY.whole = t; \
        t = entity->accelX.whole; \
        entity->accelX.whole = entity->accelY.whole; \
        entity->accelY.whole = t; \
    } \
    else if ((R) & 3) \
    { \
        entity->velX.whole = -entity->velX.whole; \
        entity->accelX.whole = -entity->accelX.whole; \
    } \
    else if ((R) & 0xC) \
    { \
        entity->velY.whole = -entity->velY.whole; \
        entity->accelY.whole = -entity->accelY.whole; \
    }

void Legion_UpdateBounce(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    struct LegionState *data = (struct LegionState *)entity->ext.legion.data.value;
    s32 r;

    switch (entity->subState)
    {
    case 0:
        data->angularVelocity += 8;
        if (entity->timer < 200)
            entity->timer++;
        if (entity->timer == 30)
            ANIM(0x14);
        else if (entity->timer == 60)
            ANIM(0x28);
        else if (entity->timer == 90)
            ANIM(0x3C);
        else if (entity->timer == 120)
            ANIM(0x50);
        else if (entity->timer == 150)
            ANIM(0x64);
        if (data->angularVelocity < 0x800)
            break;
        data->angularVelocity = 0x800;
        if (data->shells[0].actionState != 0 || data->shells[1].actionState != 0 || data->shells[2].actionState != 0)
            break;
        data->shells[0].state = 0;
        data->shells[1].state = 0;
        data->shells[2].state = 0;
        entity->subState++;
        entity->timer = 250;
        entity->velX.whole = 0;
        entity->velY.whole = 0;
        if (Entity_IsTargetRight(entity))
            entity->accelX.whole = 0x800;
        else
            entity->accelX.whole = -0x800;
        if (Entity_IsTargetBelow(entity))
            entity->accelY.whole = 0x800;
        else
            entity->accelY.whole = -0x800;
        break;
    case 1:
        entity->posX.whole = data->roomX - bg->xPos.whole;
        entity->posY.whole = data->roomY - bg->yPos.whole;
        r = Entity_MoveWithFilteredBoxCollision(entity, 0, -16, -16, 32, 32, 0x20000);
        data->roomX = entity->posX.whole + bg->xPos.whole;
        data->roomY = entity->posY.whole + bg->yPos.whole;
        BOUNCE(r)
        if (--entity->timer < 2)
        {
            entity->subState++;
            entity->timer = 0;
            data->shells[0].state = 2;
            data->shells[1].state = 2;
            data->shells[2].state = 2;
            data->shells[0].actionState = 2;
            data->shells[1].actionState = 2;
            data->shells[2].actionState = 2;
            entity->accelX.whole = 0;
            entity->accelY.whole = 0;
        }
        break;
    case 2:
        EntityDampVelocityX(entity, 0x10);
        EntityDampVelocityY(entity, 0x10);
        entity->posX.whole = data->roomX - bg->xPos.whole;
        entity->posY.whole = data->roomY - bg->yPos.whole;
        r = Entity_MoveWithBoxCollision(entity, 0, -16, -16, 32, 32, 0x20000);
        data->roomX = entity->posX.whole + bg->xPos.whole;
        data->roomY = entity->posY.whole + bg->yPos.whole;
        BOUNCE(r)
        data->angularVelocity -= 8;
        if (entity->timer < 200)
            entity->timer++;
        if (entity->timer == 30)
            ANIM(0x50);
        else if (entity->timer == 60)
            ANIM(0x3C);
        else if (entity->timer == 90)
            ANIM(0x28);
        else if (entity->timer == 120)
            ANIM(0x14);
        else if (entity->timer == 150)
            ANIM(0);
        if (data->angularVelocity > 0x100)
            break;
        data->angularVelocity = 0x100;
        if (data->shells[0].actionState != 0 || data->shells[1].actionState != 0 || data->shells[2].actionState != 0)
            break;
        data->shells[0].state = 0;
        data->shells[1].state = 0;
        data->shells[2].state = 0;
        {
            u8 two = 2;
            u32 dir;
            entity->subState = two;
            entity->state = 0;
            entity->velX.whole = 0;
            entity->velY.whole = 0;
            entity->accelX.whole = 0;
            entity->accelY.whole = 0;
            entity->posX.whole = data->roomX - bg->xPos.whole;
            entity->posY.whole = data->roomY - bg->yPos.whole;
            r = Legion_GetPathEdge(entity->posX.whole + bg->xPos.whole, entity->posY.whole + bg->yPos.whole, 0x300000);
            DIR = r;
            dir = (u8)r;
            if (dir == 0)
            {
                if (Entity_IsTargetRight(entity))
                    entity->ext.legion.circleDir = two;
                else
                    entity->ext.legion.circleDir = 1;
            }
            else if (dir == 2)
            {
                if (Entity_IsTargetRight(entity))
                    entity->ext.legion.circleDir = 1;
                else
                    entity->ext.legion.circleDir = dir;
            }
            entity->ext.legion.retargetTimer = (RandomNumberGenerator() % 64) + 0x3C;
            data->shellActionTimer = (RandomNumberGenerator() % 128) + 0x78;
        }
        break;
    }
    data->shells[0].angleOffset += 0x87;
    data->shells[1].angleOffset -= 0x76;
    data->shells[2].angleOffset += 0xC3;
    data->rotationAngle += data->angularVelocity;
    ObjAffine_Set(data->rotationAngle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    if (entity->ext.legion.flags & 2)
    {
        data->bobPhase += 0x180;
        entity->posX.whole = data->roomX - bg->xPos.whole;
        entity->posY.whole = data->roomY - bg->yPos.whole + Sin(data->bobPhase) * 3;
    }
    else
    {
        entity->posX.whole = data->roomX - bg->xPos.whole;
        entity->posY.whole = data->roomY - bg->yPos.whole;
    }
}

/* Legion_UpdateDeath: matched (shared dx/dy/dvx for the four DEBRIS calls so the
   masks are not local-allocated; byte mask 0xFB, s16 coordinate arguments; one FAKEMATCH pin). */
/* Legion core (state == 2): stops, falls, breaks apart and dies. */

#undef ANIM
#define ANIM(P) Anim_Start(entity, (struct AnimScript *)(sLegionAnimScript0 + (P)), 2, 0)
#define DEBRIS(F) \
    { dx = entity->posX.whole + (RandomNumberGenerator() & 0x1FFFFF) - 0x100000; \
      dy = entity->posY.whole + (RandomNumberGenerator() & 0x1FFFFF) - 0x100000; \
      dvx = (RandomNumberGenerator() & 0xFFFF) - 0x8000; \
      Legion_SpawnDebris(entity, dx, dy, dvx, -0x10000 - (RandomNumberGenerator() & 0xFFFF), F); }
#define SPARK() \
    { \
        s32 x = entity->posX.whole + (RandomSigned(0x10) << 16); \
        s32 y = entity->posY.whole + (RandomSigned(0x10) << 16); \
        struct EwramData_EntityData *spark = Effect_CreateFromTable(x, y, RandomSigned(0x1000), 0, 4, 7); \
        if (spark != NULL) \
        { \
            spark->velY.whole = RandomSigned(0x4000) - 0x10000; \
            spark->accelY.whole = -spark->velY.whole / 32; \
            spark->spriteLayer = 10; \
        } \
    }

/**
 * @brief Death state (state 2): debris (Legion_SpawnDebris), sounds 0x6E / 0x6F,
 * then request deletion (the update frees the state block).
 */
void Legion_UpdateDeath(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    struct LegionState *data = (struct LegionState *)entity->ext.legion.data.value;
    s32 r;
    s32 dx, dy, dvx;

    switch (entity->subState)
    {
    case 0:
        if (entity->stage == 0 && entity->animStep == 0)
        {
            entity->stage = 1;
            ANIM(0x78);
        }
        if (data->angularVelocity > 0x7F)
            data->angularVelocity -= 3;
        if (({ register s32 v_ asm("r1") = data->rotationAngle; /* FAKEMATCH: value in r1, reload of data in r0 */ v_ &= 0x7FFF; v_; }) > 0x7FF)
            break;
        data->angularVelocity = 0;
        entity->subState++;
        break;
    case 1:
        if (entity->stage == 0 && entity->animStep == 0)
        {
            entity->stage = 1;
            ANIM(0x78);
        }
        entity->stage = 1;
        if (!entity->animDone)
            break;
        entity->subState++;
        entity->velX.whole = 0;
        entity->velY.whole = 0;
        entity->accelX.whole = 0;
        entity->accelY.whole = 0x1800;
        break;
    case 2:
        entity->posX.whole = data->roomX - bg->xPos.whole;
        entity->posY.whole = data->roomY - bg->yPos.whole;
        r = Entity_MoveWithFilteredBoxCollision(entity, 0, -16, -16, 32, 32, 0x40000);
        data->roomX = entity->posX.whole + bg->xPos.whole;
        data->roomY = entity->posY.whole + bg->yPos.whole;
        if (!(r & 0x304))
            break;
        if ((entity->ext.legion.brokenCores & 0xF) == 0xF)
            entity->subState++;
        else
        {
            entity->subState = 10;
            entity->timer = 60;
            PlaySong(0x6E);
        }
        ANIM(0xAC);
        *(u8 *)&entity->ext.legion.flags &= 0xFB;
        ObjAffine_Free(entity->affineIndex);
        entity->affine = 0;
        DEBRIS(0x33)
        DEBRIS(0x33)
        DEBRIS(0x32)
        DEBRIS(0x31)
        PlaySong(0x6F);
        break;
    case 3:
        if (!entity->animDone)
            break;
        entity->subState++;
        entity->timer = 0x40;
        entity->velX.whole = 0;
        entity->velY.whole = -0x8000;
        entity->accelX.whole = 0;
        entity->accelY.whole = 0;
        break;
    case 4:
        entity->posX.whole = data->roomX - bg->xPos.whole;
        entity->posY.whole = data->roomY - bg->yPos.whole;
        EntityMove(entity);
        data->roomX = entity->posX.whole + bg->xPos.whole;
        data->roomY = entity->posY.whole + bg->yPos.whole;
        if (!(entity->timer & 3))
            SPARK()
        if ((s8)--entity->timer > 0)
            break;
        entity->subState++;
        entity->timer = 0;
        ANIM(0xBC);
        break;
    case 5:
        if (!(entity->timer++ & 3))
            SPARK()
        if (!entity->animDone)
            break;
        entity->subState++;
        entity->timer = 0x40;
        SoulPickup_SpawnFromEnemyDefinition((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer, 0x6C, 5);
        BossRecoveryPickup_SpawnWide(0x100 - (s16)bg->xPos.part16.integer, 0x170 - (s16)bg->yPos.part16.integer, 5, 0x20, 1);
        break;
    case 10:
        if (!(entity->timer & 3))
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x60000, 0x100000, 0xC0000, entity->spriteLayer - 1);
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer > 0)
            break;
        entity->subState++;
        BossRecoveryPickup_SpawnWide(0x100 - (s16)bg->xPos.part16.integer, 0x170 - (s16)bg->yPos.part16.integer, 5, 0x20, 0);
        break;
    case 6:
    case 11:
        Enemy_DropRewards(entity);
        Player_SetSelectedTarget(NULL);
        entity->deleteRequested = 1;
        entity->subState++;
        break;
    case 12:
        break;
    }
    data->shells[0].angleOffset += 0x87;
    data->shells[1].angleOffset -= 0x76;
    data->shells[2].angleOffset += 0xC3;
    if (entity->ext.legion.flags & 4)
    {
        data->rotationAngle += data->angularVelocity;
        ObjAffine_Set(data->rotationAngle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    }
    if (entity->ext.legion.flags & 2)
    {
        data->bobPhase += 0x180;
        entity->posX.whole = data->roomX - bg->xPos.whole;
        entity->posY.whole = data->roomY - bg->yPos.whole + Sin(data->bobPhase) * 3;
    }
    else
    {
        entity->posX.whole = data->roomX - bg->xPos.whole;
        entity->posY.whole = data->roomY - bg->yPos.whole;
    }
}

#define STAGE entity->ext.legion.introStage
#define POSE entity->ext.legion.introStep
#define CYCLE entity->ext.legion.introArrived

/**
 * @brief Intro sequence run by Legion_UpdateCircle before the fight; returns
 * nonzero when it has finished.
 */
s32 Legion_UpdateIntro(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    u8 state = STAGE;
    struct EwramData **ew = &gEwramData;
    switch (state) {
    case 0:
        if ((u8)entity->timer++ > 7) {
            STAGE++;
            entity->timer = 0;
        }
        break;
    case 1: {
        u32 *flags = &(*ew)->gameState.stateFlags;
        u32 value = *flags;
        s32 one = 1;
        if (!(value & one)) {
            STAGE++;
            *flags |= one;
            (*ew)->startPauseEnabled = 0;
            (*ew)->gameState.stateFlags |= 2;
            Player_SetSelectedTarget(entity);
        }
        break;
    }
    case 2:
        if ((*ew)->playerState.player->posX.whole + bg->xPos.whole > 0xFFFFFF) {
            STAGE++;
            entity->timer = 0;
            SetPlayerInput(2, 0);
            return 0;
        }
        SetPlayerInput(2, 0x10);
        break;
    case 3:
        SetPlayerInput(2, 0);
        if ((u8)entity->timer++ > 9) {
            s32 zero = 0;
            entity->timer = zero;
            STAGE++;
            entity->stage = zero;
            gEwramData->gameState.cameraScrollMinPriority = 1;
            CYCLE = zero;
        }
        break;
    case 4:
        Camera_ScrollWithPriority(1, 0, -0x8000);
        SetPlayerInput(2, 0);
        if (entity->stage == 0 && (s16)bg->yPos.part16.integer <= 0x120) {
            entity->stage++;
            POSE = 1;
        }
        if (entity->stage == 1 && (s16)bg->yPos.part16.integer <= 0x118) {
            entity->stage++;
            POSE = 2;
        }
        if (entity->stage == 2 && (s16)bg->yPos.part16.integer <= 0x110) {
            entity->stage++;
            POSE = 3;
        }
        if (entity->stage == 3 && (s16)bg->yPos.part16.integer <= 0x10C) {
            entity->stage++;
            POSE = 4;
        }
        if (entity->stage == 4 && (s16)bg->yPos.part16.integer <= 0x108) {
            entity->stage++;
            POSE = 5;
        }
        if (entity->stage == 5 && (u8)CYCLE > 4) {
            PaletteAnim_Start(sLegionWalkingCorpsePalette, sLegionPaletteAnim0, (s8)entity->ext.legion.animPalette);
            entity->stage++;
            entity->timer = 0;
        }
        entity->timer++;
        if (bg->yPos.whole <= 0xC00000) {
            s32 zero;
            STAGE++;
            zero = 0;
            entity->stage = zero;
            POSE = 6;
            CYCLE = zero;
        }
        break;
    case 5:
        Camera_ScrollWithPriority(1, 0, 0);
        SetPlayerInput(2, 0);
        if ((u8)CYCLE != 0) {
            s32 zero;
            STAGE++;
            zero = 0;
            entity->timer = zero;
            entity->stage = zero;
            POSE = 7;
            CYCLE = zero;
        }
        break;
    case 6:
        Camera_ScrollWithPriority(1, 0, 0);
        SetPlayerInput(2, 0);
        if ((u8)entity->timer++ > 29) {
            STAGE++;
            entity->timer = 0;
        }
        break;
    case 7:
        Camera_ScrollWithPriority(1, 0, 0);
        SetPlayerInput(2, 0);
        if ((u8)CYCLE != 0) {
            STAGE++;
            entity->timer = 0;
            PaletteAnim_Start(sLegionWalkingCorpsePalette, sLegionPaletteAnim1, (s8)entity->ext.legion.animPalette);
        }
        break;
    case 8:
        Camera_ScrollWithPriority(1, 0, 0);
        SetPlayerInput(2, 0);
        if ((u8)entity->timer++ > 19) {
            STAGE++;
            entity->timer = 0;
        }
        break;
    case 9:
        SetPlayerInput(2, 0);
        if (bg->yPos.whole > 0x12FFFFF) {
            s32 zero;
            STAGE++;
            zero = 0;
            entity->timer = zero;
            gEwramData->gameState.stateFlags &= ~1;
            gEwramData->startPauseEnabled = 1;
            gEwramData->gameState.cameraScrollMinPriority = zero;
            return 1;
        }
        Camera_ScrollWithPriority(1, 0, 0x10000);
        break;
    }
    entity->velX.whole += 0x200;
    entity->posY.whole += Sin(entity->velX.whole) / 16;
    return 0;
}

/* Load the graphics 081190C0 / 082258FC with the two palette variants of
 * 0820AC4C (unk_504 / unk_505, the second also as basePalette) and record the
 * graphics slot and both variants in the shared data block at unk_4F8;
 * 0 when the graphics are not loaded. */

/**
 * @brief Load the Legion graphics and its two palettes (unk_504 / unk_505) and
 * store the tile and palette slots in the state block; return 0 when no
 * graphics slot is free.
 */
s32 Legion_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics;
    s32 *data = (s32 *)entity->ext.legion.data.ptr;
    graphics = ObjGfx_LoadResource(sLegionGfx, 0);
    if (graphics < 0)
        return 0;
    entity->ext.legion.animPalette = ObjPalette_FindOrLoadVariant(sLegionWalkingCorpsePalette, 0);
    entity->basePalette = entity->ext.legion.bodyPalette = ObjPalette_FindOrLoadVariant(sLegionWalkingCorpsePalette, 1);
    data[0] = graphics;
    data[8] = (s8)entity->ext.legion.animPalette;
    data[9] = (s8)entity->ext.legion.bodyPalette;
    Sprite_Attach(entity, sLegionSpriteSheet, graphics, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

void Legion_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
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
    if ((s16)entity->hp <= 0)
    {
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->state = 2;
        entity->subState = 0;
        entity->stage = 0;
        BossDefeatEffect_Spawn(entity->posX.whole, entity->posY.whole);
    }
}

/**
 * @brief 9DF7C | Create a Legion shell part LegionCorpse_Update at room position
 * (x, y) (screen position minus bgInfo[1], x also kept in unk_4F8 /
 * unk_4FC): index in unk_504, palette slot 3 below the owner, graphics
 * sLegionSpriteSheet with the owner's tiles (first word of its state block) and
 * palette (owner unk_504), animation sLegionAnimScript1, random phase animTimer
 * and facing (velX 0x4000 away), velY 0x40000, boxes off; owner in
 * unk_4F4; index > 9 starts in state 1.5 with animation sLegionAnimScript2.
 * Returns the part or NULL.
 */
struct EwramData_EntityData *Legion_CreateCorpse(struct EwramData_EntityData *entity, s32 x, s32 y, s32 index)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, LegionCorpse_Update);
    u32 *data = (u32 *)entity->ext.legion.data.value;
    u32 tiles, r;

    if (child != NULL)
    {
    child->posX.whole = x - bg->xPos.whole;
    child->posY.whole = y - bg->yPos.whole;
    child->ext.legionCorpse.targetX = x;
    child->ext.legionCorpse.targetY = x;
    child->ext.legionCorpse.frame = index;
    child->spriteLayer = entity->spriteLayer - 3;
    tiles = *data;
    child->basePalette = entity->ext.legion.animPalette;
    Sprite_Attach(child, sLegionSpriteSheet, tiles, child->basePalette);
    Anim_Start(child, (struct AnimScript *)sLegionAnimScript1, 2, 1);
    Anim_Update(child);
    r = RandomNumberGenerator() & 7;
    child->animTimer = r;
    child->enemyId = entity->enemyId;
    child->ext.legionPart.owner = entity;
    child->scrollWithBg = 1;
    r = RandomNumberGenerator() & 1;
    child->flipX = r;
    if (child->flipX)
        child->velX.whole = -0x4000;
    else
        child->velX.whole = 0x4000;
    child->velY.whole = 0x40000;
    child->accelY.whole = 0x2800;
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    child->drawFunc = Sprite_DrawEntity;
    if (index > 9)
    {
        child->state = 1;
        child->subState = 5;
        Anim_Start(child, (struct AnimScript *)sLegionAnimScript2, 2, 0);
    }
    }
    else
        return NULL;
    return child;
}

/* Legion corpse piece: wanders on the floor, then rises and swoops into a new shell part. */

#define TARGET_X entity->ext.legionCorpse.targetX
#define TARGET_Y entity->ext.legionCorpse.targetY
#define FRAME entity->ext.legionCorpse.frame
#define BOB() entity->posY.whole += Sin(entity->velX.whole += 0x400) / 4;
#define APPROACH(V, T) \
    if (V > T) \
    { \
        if (Abs(V - T) <= 0x3FFF) \
            V = T; \
        else \
            V -= 0x4000; \
    } \
    else \
    { \
        if (Abs(V - T) <= 0x3FFF) \
            V = T; \
        else \
            V += 0x4000; \
    }

void LegionCorpse_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.legionPart.owner;
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 dx, dy;

    if (owner == NULL || owner->updateFunc == NULL || owner->enemyId != entity->enemyId)
        return;
    if (entity->state == 0)
    {
        if (!entity->flipX)
        {
            if (entity->posX.whole + bg->xPos.whole >= (s32)TARGET_Y)
            {
                TARGET_Y = TARGET_X - (RandomNumberGenerator() & 0xFFFFF) - 0x80000;
                entity->velX.whole = -0x4000;
                entity->flipX = 1;
            }
        }
        else
        {
            if (entity->posX.whole + bg->xPos.whole <= (s32)TARGET_Y)
            {
                TARGET_Y = TARGET_X + (RandomNumberGenerator() & 0xFFFFF) + 0x80000;
                entity->velX.whole = 0x4000;
                entity->flipX = 0;
            }
        }
        Entity_MoveWithCollisionDefault(entity, 0x40000, -0x200000);
        Anim_Update(entity);
        if ((u8)owner->ext.legion.introStep == FRAME + 1)
        {
            entity->state++;
            entity->subState = 0;
            entity->stage = 0;
        }
        return;
    }
    if (entity->state != 1)
        return;
    switch (entity->subState)
    {
    case 0:
        entity->subState++;
        Anim_Start(entity, (struct AnimScript *)(sLegionAnimScript1 + 0x1C), 2, 1);
        { s32 k = 0x600000; TARGET_Y = entity->posY.whole + bg->yPos.whole - (RandomNumberGenerator() % 0x300000 + k); }
        entity->posY.whole -= 0xD0000;
        entity->accelY.whole = -0x1000;
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
        entity->velY.whole = 0;
        PlaySong(0x11E);
        break;
    case 1:
        EntityAccelerateAndMove(entity, 0x40000);
        if (entity->posY.whole + bg->yPos.whole > (s32)TARGET_Y)
            break;
        entity->accelY.whole = 0;
        entity->subState++;
        entity->timer = 0;
        break;
    case 2:
        EntityDampVelocityY(entity, 0xC);
        EntityAccelerateAndMove(entity, 0x40000);
        if (Abs(entity->velY.whole) > 0x8000)
            break;
        entity->subState++;
        owner->ext.legion.introArrived++;
        entity->velX.whole = 0;
        break;
    case 3:
        BOB()
        if ((u8)owner->ext.legion.introStep != 6)
            break;
        entity->subState++;
        entity->timer = FRAME * 20;
        break;
    case 4:
        BOB()
        if ((s8)--entity->timer <= 0)
        {
            Anim_Start(entity, (struct AnimScript *)sLegionAnimScript2, 2, 0);
            entity->subState++;
            PlaySong(0x11D);
        }
        if (entity->timer != 10)
            break;
        if (FRAME == 1)
            Legion_CreateCorpse(owner, 0x700000, 0x1000000, 10);
        if (FRAME == 3)
            Legion_CreateCorpse(owner, 0x1000000, 0x700000, 10);
        if (FRAME == 2)
            Legion_CreateCorpse(owner, 0x1900000, 0x1000000, 10);
        break;
    case 5:
        BOB()
        if (!entity->animDone)
            break;
        entity->subState++;
        TARGET_X = owner->posX.whole + (RandomNumberGenerator() & 0x7FFFFF) + ({ s32 b = bg->xPos.whole - 0x400000; b; });
        TARGET_Y = owner->posY.whole - (RandomNumberGenerator() & 0x3FFFFF) + bg->yPos.whole;
        entity->velX.whole = 0;
        entity->timer = 40;
        break;
    case 6:
        if (entity->timer > 19)
        {
            s32 t;
            dx = entity->posX.whole + bg->xPos.whole;
            dx = TARGET_X - dx;
            dy = entity->posY.whole + bg->yPos.whole;
            dy = TARGET_Y - dy;
            t = entity->timer;
            dx /= t - 1;
            dy /= t - 1;
            if (t == 20)
            {
                TARGET_X = owner->posX.whole + (RandomNumberGenerator() & 0xFFFFF) + ({ s32 b = bg->xPos.whole - 0x80000; b; });
                TARGET_Y = owner->posY.whole + (RandomNumberGenerator() & 0xFFFFF) + ({ s32 b = bg->yPos.whole - 0x80000; b; });
            }
        }
        else
        {
            dx = entity->posX.whole + bg->xPos.whole;
            dx = TARGET_X - dx;
            dy = entity->posY.whole + bg->yPos.whole;
            dy = TARGET_Y - dy;
            if (entity->timer != 1)
            {
                dx /= entity->timer - 1;
                dy /= entity->timer - 1;
            }
        }
        APPROACH(entity->velX.whole, dx)
        APPROACH(entity->velY.whole, dy)
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        if ((s8)--entity->timer > 0)
            break;
        if (FRAME == 0)
            owner->ext.legion.introArrived++;
        Effect_SpawnParticleAnim12(entity->posX.whole, entity->posY.whole, 0, 0, entity->spriteLayer, 0, 6);
        PlaySong(0x11C);
        EntityDelete(entity);
        return;
    }
    Anim_Update(entity);
}

/* Spawn a body part of a multi-part enemy (Legion_CreateCorePart). The do/while (0)
 * wrappers only weight references for global-alloc (kind r6, entity r7,
 * data r4); they do not change the code. */

#define CORE(k) LegionCorePart_CreateDetail(part, -part->ext.legionPart.offsetX, -part->ext.legionPart.offsetY, k, *data)

struct EwramData_EntityData *Legion_CreateCorePart(struct EwramData_EntityData *entity, s32 x, s32 y, s32 kind)
{
    struct EwramData_EntityData *part = Entity_CreateObject(LegionCorePart_Update);
    u32 box = 0x1010F0F0;
    s32 *data = (s32 *)entity->ext.legion.data.value;
    u16 palette;
    s32 tiles;

    if (part != NULL) {
        part->ext.legionPart.offsetX = x;
        part->ext.legionPart.offsetY = y;
        part->posX.whole = entity->posX.whole + x;
        part->posY.whole = entity->posY.whole + y;
        do { do { part->ext.legionPart.kind = kind; } while (0); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        if ((u8)kind == 3)
        {
            part->posX.whole = entity->posX.whole;
            part->posY.whole = entity->posY.whole;
        }
        part->spriteLayer = entity->spriteLayer - 2;
        do { do { tiles = *data; } while (0); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        part->ext.legionPart.tiles = tiles;
        part->basePalette = entity->ext.legion.animPalette;
        Sprite_Attach(part, sLegionSpriteSheet, tiles, part->basePalette);
        do { part->enemyId = entity->enemyId; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        switch (kind)
        {
        case 0:
            part->animFrame = 0;
            CORE(0);
            CORE(1);
            CORE(2);
            break;
        case 1:
            part->animFrame = 1;
            CORE(0);
            CORE(2);
            CORE(3);
            break;
        case 2:
            part->animFrame = 2;
            CORE(1);
            CORE(4);
            break;
        case 3:
            part->animFrame = 0x36;
            part->velX.whole = 0xC000;
            part->velY.whole = 0xC000;
            CORE(3);
            CORE(4);
            palette = ObjAffine_Alloc(1);
            if ((s16)palette < 0)
            {
                EntityDelete(part);
                return NULL;
            }
            part->affineIndex = palette;
            part->affine = 1;
            ObjAffine_SetBiased(0, part->velX.whole, part->velY.whole, part->flipX, part->flipY, part->affineIndex);
            break;
        case 10:
            part->animFrame = 3;
            CORE(3);
            CORE(4);
            break;
        }
        part->animMode = 2;

        do { part->ext.legionPart.owner = entity; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        part->scrollWithBg = 0;
        part->flipX = 0;
        do { part->hp = entity->hp; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        do { part->attack = entity->attack; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        if (!(*((u8 *)gEwramData + 0x8E) & 0x40))
        {
            Hit_Enable(part, 8, Legion_DamagePlayerOnContact, LegionCorePart_OnHit);
            HitBox_SetBody(part, &box);
            HitBox_SetAttack(part, &box);
            part->hit.flags &= 0xFB;
            part->hit.flags &= 0xFD;
        }
        part->drawFunc = Sprite_DrawEntity;
        return part;
    }
    return NULL;
}

/* LegionCorePart_Update: owner-attached effect - follow the owner, animate through a table of sprite parts and spawn particles on timer ticks. */

#undef FRAME
#define FRAME entity->ext.legionPart.kind
#define TICK (entity->ext.legionPart.tick & 0x1F)
#define FLAGS owner->ext.legion.brokenCores
#define SPAWN(vx, vy) Legion_SpawnFragment(entity, x, y, vx, vy, entity->ext.legionPart.tiles)

void LegionCorePart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.legionPart.owner;
    struct HitResult result;
    u32 b0 = 0x1934FFE4, b1 = 0x1524ECF5;
    u32 b4 = 0x1932E8E6, b5 = 0x1521FFF7;
    u32 b2 = 0x1522EBE8, b3 = 0x1932FFE8;
    u32 b6 = 0x1934E8E8, b7 = 0x1524FFE8;
    u32 *tbl[4][2] = { { &b0, &b1 }, { &b2, &b3 }, { &b4, &b5 }, { &b6, &b7 } };
    s32 x, y, vx;
    s32 counter;
    s32 vy = 0;

    u8 idx;

    counter = entity->ext.legionPart.tick + 1;
    entity->ext.legionPart.tick = counter;

    if (entity->state <= 9)
    {
        if (owner == NULL || owner->updateFunc == NULL || owner->enemyId != entity->enemyId)
            return;
        if (FRAME == 3)
        {
            if (entity->state == 0)
            {
                if ((u8)owner->ext.legion.introStep == 7)
                {
                    entity->state++;
                    entity->subState = 0;
                }
            }
            else if (entity->state == 1)
            {
                switch (entity->subState)
                {
                case 0:
                    entity->velX.whole += 0x80;
                    entity->velY.whole += 0x80;
                    if (entity->velX.whole > 0xCFFF)
                    {
                        entity->velX.whole = 0xD000;
                        entity->velY.whole = 0xD000;
                        entity->subState++;
                    }
                    break;
                case 1:
                    entity->velX.whole -= 0x80;
                    entity->velY.whole -= 0x80;
                    if (entity->velX.whole <= 0xCC00)
                    {
                        entity->velX.whole = 0xCC00;
                        entity->velY.whole = 0xCC00;
                        entity->subState++;
                    }
                    break;
                case 2:
                    entity->velX.whole += 0x80;
                    entity->velY.whole += 0x80;
                    if (entity->velX.whole > 0xDFFF)
                    {
                        entity->velX.whole = 0xE000;
                        entity->velY.whole = 0xE000;
                        entity->subState++;
                    }
                    break;
                case 3:
                    entity->velX.whole -= 0x80;
                    entity->velY.whole -= 0x80;
                    if (entity->velX.whole <= 0xDC00)
                    {
                        entity->velX.whole = 0xDC00;
                        entity->velY.whole = 0xDC00;
                        entity->subState++;
                    }
                    break;
                case 4:
                    entity->velX.whole += 0x80;
                    entity->velY.whole += 0x80;
                    if (entity->velX.whole > 0xEFFF)
                    {
                        entity->velX.whole = 0xF000;
                        entity->velY.whole = 0xF000;
                        entity->subState++;
                    }
                    break;
                case 5:
                    entity->velX.whole -= 0x80;
                    entity->velY.whole -= 0x80;
                    if (entity->velX.whole <= 0xEC00)
                    {
                        entity->velX.whole = 0xEC00;
                        entity->velY.whole = 0xEC00;
                        entity->subState++;
                    }
                    break;
                case 6:
                    entity->velX.whole += 0x80;
                    entity->velY.whole += 0x80;
                    if (entity->velX.whole > 0xFFFF)
                    {
                        entity->velX.whole = 0x10000;
                        entity->velY.whole = 0x10000;
                        entity->subState = 0;
                        FRAME = 0x14;
                        entity->state = 0;
                        owner->ext.legion.introArrived++;
                        entity->animFrame = 3;
                        entity->posX.whole = owner->posX.whole + entity->ext.legionPart.offsetX;
                        entity->posY.whole = owner->posY.whole + entity->ext.legionPart.offsetY;
                        ObjAffine_SetBiased(0, entity->velX.whole, entity->velY.whole, entity->flipX, entity->flipY, entity->affineIndex);
                        return;
                    }
                    break;
                case 7:
                    break;
                }
            }
            entity->posX.whole = owner->posX.whole;
            entity->posY.whole = owner->posY.whole;
            ObjAffine_SetBiased(0, entity->velX.whole, entity->velY.whole, entity->flipX, entity->flipY, entity->affineIndex);
            return;
        }
        if (entity->state != 0)
            return;
        idx = FRAME <= 4 ? FRAME : 3;
        if (!(*((u8 *)gEwramData + 0x8E) & 0x40))
        {
            if (counter & 1)
            {
                HitBox_SetBody(entity, tbl[idx][0]);
                HitBox_SetAttack(entity, tbl[idx][1]);
            }
            else
            {
                HitBox_SetBody(entity, tbl[idx][1]);
                HitBox_SetAttack(entity, tbl[idx][0]);
            }
        }
        entity->posX.whole = owner->posX.whole + entity->ext.legionPart.offsetX;
        entity->posY.whole = owner->posY.whole + entity->ext.legionPart.offsetY;
        if (!(*((u8 *)gEwramData + 0x8E) & 0x40))
            Hit_CheckWithPlayer(&result, entity);
        Enemy_UpdateHitFlash(entity);
        if (owner->state == 2)
        {
            entity->state = 0xB;
            entity->subState = 0;
            entity->hp = 0;
            entity->palette = entity->basePalette;
            entity->timer = 0x3C;
            entity->hitFlashTimer = 0;
        }
        if (FRAME == 0)
        {
            if ((FLAGS & 4) && (entity->ext.legionPart.tick & 0x1F) < 1)
            {
                x = RandomNumberGenerator() % 0x300000 + ({ s32 t = entity->posX.whole - 0x180000; t; });
                y = entity->posY.whole + 0x180000;
                vx = -(RandomNumberGenerator() & 0x7FFF);
                SPAWN(vx, vy);
            }
            if ((FLAGS & 2) && TICK == 4)
            {
                y = RandomNumberGenerator() % 0x300000 + ({ s32 t = entity->posY.whole - 0x180000; t; });
                x = entity->posX.whole + 0x180000;
                vx = (RandomNumberGenerator() & 0x3FFF) + 0x8000;
                SPAWN(vx, 0);
            }
        }
        else if (FRAME == 1)
        {
            if ((FLAGS & 8) && TICK == 8)
            {
                x = RandomNumberGenerator() % 0x300000 + ({ s32 t = entity->posX.whole - 0x180000; t; });
                y = entity->posY.whole + 0x180000;
                SPAWN(RandomNumberGenerator() & 0x7FFF, 0);
            }
            if ((FLAGS & 1) && TICK == 0xC)
            {
                y = RandomNumberGenerator() % 0x300000 + ({ s32 t = entity->posY.whole - 0x180000; t; });
                x = entity->posX.whole - 0x180000;
                vx = -0x8000 - (RandomNumberGenerator() & 0x3FFF);
                SPAWN(vx, 0);
            }
        }
        else if (FRAME == 2)
        {
            if ((FLAGS & 1) && TICK == 0x10)
            {
                x = RandomNumberGenerator() % 0x300000 + ({ s32 t = entity->posX.whole - 0x180000; t; });
                y = entity->posY.whole - 0x180000;
                vx = -(RandomNumberGenerator() & 0x7FFF);
                SPAWN(vx, -(RandomNumberGenerator() & 0x7FFF));
            }
            if ((FLAGS & 8) && TICK == 0x14)
            {
                y = RandomNumberGenerator() % 0x300000 + ({ s32 t = entity->posY.whole - 0x180000; t; });
                x = entity->posX.whole + 0x180000;
                vx = (RandomNumberGenerator() & 0x3FFF) + 0x8000;
                SPAWN(vx, 0);
            }
        }
        else
        {
            if ((FLAGS & 2) && TICK == 0x18)
            {
                x = RandomNumberGenerator() % 0x300000 + ({ s32 t = entity->posX.whole - 0x180000; t; });
                y = entity->posY.whole - 0x180000;
                vx = RandomNumberGenerator() & 0x7FFF;
                SPAWN(vx, -(RandomNumberGenerator() & 0x7FFF));
            }
            if ((FLAGS & 4) && TICK == 0x1C)
            {
                y = RandomNumberGenerator() % 0x300000 + ({ s32 t = entity->posY.whole - 0x180000; t; });
                x = entity->posX.whole - 0x180000;
                vx = -0x8000 - (RandomNumberGenerator() & 0x3FFF);
                SPAWN(vx, 0);
            }
        }
        return;
    }
    if (entity->subState == 0)
    {
        entity->subState++;
        if (FRAME == 0)
        {
            if (entity->state == 10)
                FLAGS |= 1;
            entity->velX.whole = -0x4000;
            entity->velY.whole = -0x10000;
        }
        else if (FRAME == 1)
        {
            if (entity->state == 10)
                FLAGS |= 2;
            entity->velX.whole = 0x4000;
            entity->velY.whole = -0x10000;
        }
        else if (FRAME == 2)
        {
            if (entity->state == 10)
                FLAGS |= 4;
            entity->velX.whole = -0x4000;
            entity->velY.whole = 0;
        }
        else
        {
            if (entity->state == 10)
                FLAGS |= 8;
            entity->velX.whole = 0x4000;
            entity->velY.whole = 0;
        }
        entity->posX.whole = owner->posX.whole + entity->ext.legionPart.offsetX;
        entity->posY.whole = owner->posY.whole + entity->ext.legionPart.offsetY;
        entity->scrollWithBg = 1;
        entity->accelX.whole = 0;
        entity->accelY.whole = 0x1800;
    }
    else if (entity->subState == 1)
    {
        EntityAccelerateAndMove(entity, 0x40000);
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0)
        {
            if (FRAME == 0x14)
                ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
        }
    }
}

/* Take damage; on death start the death state, otherwise occasionally emit
 * a piece (at most four pieces counted by the owner). */

/* The statement expressions keep (pos - 0x100000) as one sum: agbcc would
 * otherwise fold the constant into the random term. The 32-bit result of
 * Hit_ResolveDamage is narrowed only at its use. */
void LegionCorePart_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    struct EwramData_EntityData *base = entity->ext.legionPart.owner;
    s16 scale = Player_CalculateAttackPower(hit[2]);
    struct EwramData_EntityData *owner = ENTITY_OWNER(base);
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, scale);

    entity->hitFlashTimer = 8;
    {
        s32 hp = entity->hp;
        s32 applied = (s16)damage;
        entity->hp = hp - applied;
        Entity_ShowDamage(entity, (s16)applied, 0);
    }
    if ((s16)entity->hp <= 0)
    {
        entity->palette = entity->basePalette;
        entity->timer = 0x3C;
        entity->hitFlashTimer = 0;
        entity->state = 10;
        entity->subState = 0;
    }
    else if ((RandomNumberGenerator() & 0x3F) < 0x10 && owner->ext.raw.unk_500.whole <= 3)
    {
        s32 x, y, vx, vy;
        u8 type;

        owner->ext.raw.unk_500.whole++;
        type = entity->ext.legionPart.kind;
        if (type == 0)
            goto negative;
        if (type == 1)
        {
            x = (RandomNumberGenerator() & 0x1FFFFF) + ({ s32 t = entity->posX.whole - 0x100000; t; });
            y = (RandomNumberGenerator() & 0x1FFFFF) + ({ s32 t = entity->posY.whole - 0x100000; t; });
            vx = RandomNumberGenerator() & 0x7FFF;
            vy = -(RandomNumberGenerator() & 0x7FFF);
            Legion_SpawnDestructibleFragment(entity->ext.legionPart.owner, x, y, vx, vy, entity->ext.legionPart.tiles);
        }
        else if (type == 2)
        {
        negative:
            x = (RandomNumberGenerator() & 0x1FFFFF) + ({ s32 t = entity->posX.whole - 0x100000; t; });
            y = (RandomNumberGenerator() & 0x1FFFFF) + ({ s32 t = entity->posY.whole - 0x100000; t; });
            vx = -(RandomNumberGenerator() & 0x7FFF);
            vy = -(RandomNumberGenerator() & 0x7FFF);
            Legion_SpawnDestructibleFragment(entity->ext.legionPart.owner, x, y, vx, vy, entity->ext.legionPart.tiles);
        }
        else
        {
            x = (RandomNumberGenerator() & 0x1FFFFF) + ({ s32 t = entity->posX.whole - 0x100000; t; });
            y = (RandomNumberGenerator() & 0x1FFFFF) + ({ s32 t = entity->posY.whole - 0x100000; t; });
            vx = RandomNumberGenerator() & 0x7FFF;
            vy = -(RandomNumberGenerator() & 0x7FFF);
            Legion_SpawnDestructibleFragment(entity->ext.legionPart.owner, x, y, vx, vy, entity->ext.legionPart.tiles);
        }
    }
}

/**
 * @brief 9F0D0 | Create a Legion core part LegionCoreDetail_Update (file 0x0809E0C0)
 * at offset (x, y) from the owner, shifted per kind 0..4 (+1/-37, -22/+2,
 * -3/-21, +28/-1, -3/+20 px; kept in unk_4F8 / unk_4FC): kind in unk_504,
 * layer +4, owner's palette, graphics sLegionSpriteSheet with the given tiles,
 * frame kind + 4, animMode = 2, owner in unk_4F4, boxes off. Returns the
 * part or NULL.
 */
struct EwramData_EntityData *LegionCorePart_CreateDetail(struct EwramData_EntityData *entity, s32 x, s32 y, s32 kind, s32 tiles)
{
    struct EwramData_EntityData *part = Entity_CreateObject(LegionCoreDetail_Update);

    if (part != NULL)
    {
    part->ext.legionPart.offsetX = x;
    part->ext.legionPart.offsetY = y;
    switch (kind)
    {
    case 0:
        part->ext.legionPart.offsetX += 0x10000;
        part->ext.legionPart.offsetY += -0x250000;
        break;
    case 1:
        part->ext.legionPart.offsetX += -0x160000;
        part->ext.legionPart.offsetY += 0x20000;
        break;
    case 2:
        part->ext.legionPart.offsetX += -0x30000;
        part->ext.legionPart.offsetY += -0x150000;
        break;
    case 3:
        part->ext.legionPart.offsetX += 0x1C0000;
        part->ext.legionPart.offsetY += -0x10000;
        break;
    case 4:
        part->ext.legionPart.offsetX += -0x30000;
        part->ext.legionPart.offsetY += 0x140000;
        break;
    }
    part->posX.whole = entity->posX.whole + part->ext.legionPart.offsetX;
    part->posY.whole = entity->posY.whole + part->ext.legionPart.offsetY;
    part->ext.legionPart.kind = kind;
    part->spriteLayer = entity->spriteLayer + 4;
    part->basePalette = entity->basePalette;
    Sprite_Attach(part, sLegionSpriteSheet, tiles, part->basePalette);
    part->animFrame = kind + 4;
    part->animMode = 2;
    part->enemyId = entity->enemyId;
    part->ext.legionPart.owner = entity;
    part->scrollWithBg = 0;
    part->flipX = 0;
    part->hit.flags |= 4;
    part->hit.flags |= 2;
    part->drawFunc = Sprite_DrawEntity;
    return part;
    }
    return NULL;
}

/* Spawn a fragment at (x, y) with velocity (vx, vy), gravity 0x1000, a
 * random frame 0x2C-0x30 and random flips, in the owner palette (graphics
 * slot passed in, sprites 082258FC); lifetime 60, updated by LegionFragment_Update. */

void Legion_SpawnFragment(struct EwramData_EntityData *owner, s32 x, s32 y, s32 vx, s32 vy, s32 graphics)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, LegionFragment_Update);
    if (entity == NULL)
        return;
    entity->posX.whole = x;
    entity->posY.whole = y;
    entity->velX.whole = vx;
    entity->velY.whole = vy;
    entity->accelX.whole = 0;
    entity->accelY.whole = 0x1000;
    entity->spriteLayer = owner->spriteLayer + 1;
    entity->basePalette = owner->basePalette;
    Sprite_Attach(entity, sLegionSpriteSheet, graphics, entity->basePalette);
    entity->animFrame = RandomNumberGenerator() % 5 + 0x2C;
    entity->animMode = 2;
    entity->enemyId = owner->enemyId;
    entity->ext.legionPart.owner = owner;
    entity->scrollWithBg = 1;
    entity->flipX = RandomNumberGenerator() % 2;
    entity->flipY = RandomNumberGenerator() % 2;
    entity->timer = 0x3C;
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    entity->drawFunc = Sprite_DrawEntity;
}

/* Spawn a destructible fragment (HP 200, hit callbacks LegionDestructibleFragment_DamagePlayerOnContact /
 * LegionDestructibleFragment_OnHit) at (x, y) with velocity (vx, vy), animation 08528328,
 * palette from the owner unk_504, facing the direction of vx. */
void Legion_SpawnDestructibleFragment(struct EwramData_EntityData *owner, s32 x, s32 y, s32 vx, s32 vy, s32 graphics)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, LegionDestructibleFragment_Update);
    if (entity == NULL)
        return;
    entity->posX.whole = x;
    entity->posY.whole = y;
    entity->velX.whole = vx;
    entity->velY.whole = vy;
    entity->accelX.whole = 0;
    entity->accelY.whole = 0x1000;
    entity->spriteLayer = owner->spriteLayer - 3;
    entity->basePalette = owner->ext.legion.animPalette;
    Sprite_Attach(entity, sLegionSpriteSheet, graphics, entity->basePalette);
    Anim_Start(entity, sLegionAnimScript3, 2, 1);
    Anim_Update(entity);
    entity->enemyId = owner->enemyId;
    entity->ext.legionPart.owner = owner;
    entity->scrollWithBg = 1;
    if (vx < 0)
        entity->flipX = 1;
    else
        entity->flipX = 0;
    entity->timer = 0x3C;
    entity->attack = owner->attack;
    entity->hp = 200;
    entity->enemyFlags |= 1;
    Hit_Enable(entity, 8, LegionDestructibleFragment_DamagePlayerOnContact, LegionDestructibleFragment_OnHit);
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    entity->drawFunc = Sprite_DrawEntity;
}

/* Destructible fragment update, reconstructed from 0x0809F410. */

struct FragmentOwnerData {
    u8 pad[0x1C];
    s32 count;
};

void LegionDestructibleFragment_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner;
    struct BgLayerState *bg;
    s32 accelerationLimit;
    u32 box;
    struct FragmentOwnerData *ownerData;
    u32 state;
    u32 cur;
    u32 res;
    s32 x;
    struct HitResult result;
    owner = entity->ext.legionPart.owner;
    bg = &gEwramData->bgInfo[1];
    do { /* FAKEMATCH: keeps the constant in a register (no move to its use in local-alloc) */
        accelerationLimit = 0x60000;
    } while (0);
    box = 0x1C06E4FD;
    ownerData = (void *)owner->ext.legion.data.ptr;
    cur = entity->state;
    state = cur & 0xFF; /* not a plain copy: CSE must not learn cur from the state tests */
    if (state == 0) {
        s32 y;
        x = entity->posX.whole + bg->xPos.whole;
        y = entity->posY.whole + bg->yPos.whole;
        if ((u32)(x - 0xA00000) <= 0xC00000 && y > 0xCFFFFF && y <= 0x1600000) {
            EntityAccelerateAndMove(entity, accelerationLimit);
        } else if ((res = Entity_MoveWithCollisionDefault(entity, 0x40000, -0x200000)) & 4) {
            entity->state++;
            Anim_Start(entity, sLegionDestructibleFragmentAnimScript, 2, 1);
        }
    } else if (state == 1) {
        if (entity->animDone) {
            *(u8 *)&entity->state = cur + 1;
            Anim_Start(entity, (struct AnimScript *)sLegionAnimScript1, 2, 1);
            HitBox_SetBody(entity, &box);
            HitBox_SetAttack(entity, &box);
            *(u8 *)&entity->hit.flags &= 0xFB;
            *(u8 *)&entity->hit.flags &= 0xFD;
            entity->velY.whole = 0x40000;
            entity->accelY.whole = 0x1800;
            entity->accelX.whole = 0;
        }
    } else if (state == 2) {
        register u32 flags asm("r9"); /* FAKEMATCH: priority vs. the flipX byte pointer (sb/sl) */
        if (entity->flipX)
            entity->velX.whole = -0x4000;
        else
            entity->velX.whole = 0x4000;
        res = Entity_MoveWithCollisionDefault(entity, 0x40000, -0x200000);
        do { /* FAKEMATCH: raises the refs of x (global-alloc priority) */
            u32 b_ = *(ENTITY_RENDER_FLAGS_ADDRESS(entity)) & 0x40;
            x = 8;
            if (b_)
                x -= 16;
        } while (0);
        flags = 3;
        flags &= res;
        if (flags)
            entity->flipX ^= 1;
        if (!Collision_IsSlopeTile(bg->xPos.part16.integer + entity->posX.part16.integer,
                          bg->yPos.part16.integer + entity->posY.part16.integer + 1)) {
            do { /* FAKEMATCH: raises the refs of the posX/posY pointers */
                if (Tile_Classify(bg->xPos.part16.integer + entity->posX.part16.integer + x,
                                bg->yPos.part16.integer + entity->posY.part16.integer) || flags)
                    entity->flipX ^= 1;
            } while (0);
        }
        Enemy_UpdateHitFlash(entity);
        Hit_CheckWithPlayer(&result, entity);
        if (owner && owner->updateFunc && owner->enemyId == entity->enemyId
            && owner->state != 2) {
            Anim_Update(entity);
            return;
        }
        {
            s32 zero = 0;
            entity->state = 10;
            entity->subState = zero;
            entity->hp = zero;
            entity->palette = entity->basePalette;
            entity->timer = 40;
            entity->hitFlashTimer = zero;
            Anim_Start(entity, (struct AnimScript *)sUnk_08528340, 2, 1);
        }
    } else if (state == 10) {
        if (!(entity->timer & 3))
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - accelerationLimit, 0x100000, 0xC0000, entity->spriteLayer - 1);
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0) {
            ownerData->count--;
            EntityDelete(entity);
            return;
        }
    }
    Anim_Update(entity);
}

void LegionDestructibleFragment_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 damage = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(entity, attacker, hit, damage);
    entity->hitFlashTimer = 8;
    damage = (s16)damage;
    Entity_ShowDamage(entity, (s16)damage, 0);
    entity->hp -= damage;
    if ((s16)entity->hp <= 0)
    {
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->hitFlashTimer = 0;
        entity->state = 10;
        entity->subState = 0;
        Anim_Start(entity, (struct AnimScript *)sUnk_08528340, 2, 1);
    }
}

/**
 * @brief 9F764 | Create a Legion part LegionShellPart_Update at the owner: layer +1,
 * graphics sLegionSpriteSheet with the owner's tiles (first word of its state
 * block) and palette (owner unk_504); kind 5 gets its own palette,
 * blending and an affine matrix from unk_502; hidden, owner in unk_4F4,
 * param_1 in unk_504, kind in unk_501, owner's attack. Returns the part;
 * when the palette allocation fails the part is deleted and the original
 * returns whatever EntityDelete leaves in r0.
 */
struct EwramData_EntityData *Legion_CreateShellPart(struct EwramData_EntityData *entity, s32 center, s32 kind)
{
    u32 *data = (u32 *)entity->ext.legion.data.value;
    struct EwramData_EntityData *part = Entity_CreateObject(LegionShellPart_Update);
    u16 palette;
    u32 tiles;

    if (part != NULL)
    {
        part->posX.whole = entity->posX.whole;
        part->posY.whole = entity->posY.whole;
        part->spriteLayer = entity->spriteLayer + 1;
        tiles = *data;
        part->basePalette = entity->ext.legion.animPalette;
        Sprite_Attach(part, sLegionSpriteSheet, tiles, part->basePalette);
        if (kind == 5)
        {
            palette = ObjAffine_Alloc(1);
            if ((s16)palette < 0)
            {
                /* the original returns whatever EntityDelete leaves in r0 */
                return ((struct EwramData_EntityData *(*)(struct EwramData_EntityData *))EntityDelete)(part);
            }
            part->affineIndex = palette;
            part->affine = 1;
            ObjAffine_SetBiased(part->ext.legionShell.angle, 0x10000, 0x10000, part->flipX, part->flipY, part->affineIndex);
        }
        part->hidden = 1;
        part->ext.legionPart.owner = entity;
        part->ext.legionShell.center = (struct EwramData_EntityData *)center;
        part->ext.legionShell.kind = kind;
        part->attack = entity->attack;
        part->scrollWithBg = 0;
        part->drawFunc = Sprite_DrawEntity;
    }
    return part;
}

/* Legion shell part: orbits the core at a radius set by the shell state, fires, falls off. */

#define KIND ((s8)entity->ext.legionShell.kind)
#define ANGLE entity->ext.legionShell.angle
#define RADIUS entity->ext.legionShell.radius

void LegionShellPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.legionPart.owner;
    struct EwramData_EntityData *center = entity->ext.legionShell.center;
    u8 unused[0x10];
    struct ShellRadii radii = sLegionShellRadii;
    struct LegionState *data = (struct LegionState *)owner->ext.legion.data.value;
    s32 k = (s8)entity->ext.legionShell.index * 6 + 10;
#define shell ((struct LegionShell *)((s32 *)data + k))
    s32 limit;
    s32 a;

    if (shell->state == 1)
        { EntityDelete(entity); return; }
    if (entity->state == 0)
    {
        if (owner == NULL || owner->updateFunc == NULL || owner->enemyId != 0x6C)
            { EntityDelete(entity); return; }
        if (shell->state != 4)
        {
            switch (entity->subState)
            {
            case 0:
                entity->subState = 1;
                entity->timer = 60;
                break;
            case 1:
                ANGLE += 500;
                if (ANGLE > 0x7800)
                {
                    ANGLE = 0x7800;
                    entity->subState++;
                }
                break;
            case 2:
                ANGLE -= 500;
                if (ANGLE < 0x800)
                {
                    ANGLE = 0x800;
                    entity->subState--;
                }
                break;
            }
            { s32 sel; s32 i = shell->radiusSet; if (KIND == 5) sel = 2; else sel = KIND != 0; limit = radii.r[i][sel]; }
            if (shell->state == 2)
            {
                entity->hidden = 0;
                if (RADIUS < limit)
                {
                    shell->actionState = 2;
                    RADIUS += limit / 120;
                    if (RADIUS >= limit)
                    {
                        RADIUS = limit;
                        shell->actionState = 0;
                    }
                }
            }
            if (shell->state == 3 && RADIUS > 0)
            {
                shell->actionState = 3;
                RADIUS -= limit / 120;
                if (RADIUS <= 0)
                {
                    RADIUS = 0;
                    shell->actionState = 0;
                    entity->hidden = 1;
                }
            }
        }
        else if (KIND == 5)
        {
            shell->actionState = 4;
            if (shell->timer == 1)
            {
                entity->stage = 0;
                entity->palette = entity->basePalette;
            }
            else if (shell->timer == 0)
            {
                switch (entity->stage)
                {
                case 0:
                    entity->stage = 1;
                    entity->timer = 60;
                    PlaySong(0x11F);
                    break;
                case 1:
                    if (entity->timer > 0x1D && !(entity->timer & 1))
                        LegionShellPart_FireShot(entity, 0, 0, data->unk_0, ANGLE + shell->angleOffset);
                    if (--entity->timer == 0)
                        shell->actionState = 0;
                    break;
                }
            }
            else if (Entity_GetVisibilityPattern(shell->timer) == 1)
                entity->palette = 4;
            else
                entity->palette = entity->basePalette;
        }
        if (KIND == 0)
            a = ANGLE - (ANGLE - 0x4000) / 4;
        else if (KIND == 1)
            a = ANGLE;
        else
            a = ANGLE + (ANGLE - 0x4000) / 4;
        a += shell->angleOffset;
        {
            s32 x, y;
            x = Sin(a + 0x4000);
            x = FixedMultiply(x, RADIUS);
            y = Sin(a);
            y = FixedMultiply(y, RADIUS);
            entity->posX.whole = center->posX.whole + x;
            entity->posY.whole = center->posY.whole + y;
        }
        if (KIND == 5)
            ObjAffine_SetBiased(ANGLE + ({ s32 t = shell->angleOffset - 0x4000; t; }), 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
        if (entity->state == 0 && owner->state == 2 && --entity->ext.legionShell.delay == 0)
        {
            entity->state = 1;
            entity->subState = 0;
            entity->stage = 0;
            entity->hidden = 0;
            entity->palette = entity->basePalette;
        }
        Anim_Update(entity);
        return;
    }
    if (entity->subState == 0)
    {
        entity->subState++;
        entity->scrollWithBg = 1;
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
        entity->velY.whole = 0;
        entity->accelY.whole = 0x1800;
        entity->timer = 50;
    }
    else if (entity->subState == 1)
    {
        EntityAccelerateAndMove(entity, 0x40000);
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0)
        {
            if (KIND == 5)
                ObjAffine_Free(entity->affineIndex);
            { EntityDelete(entity); return; }
        }
    }
    Anim_Update(entity);
    return;

}

/* Spawn a shot (LegionShot_Update) at the owner + (dx, dy) moving at speed 8 in
 * the given angle; graphics and palette come from the owner's parent data,
 * the frame and flip from a 24-entry table indexed by the angle. */

void LegionShellPart_FireShot(struct EwramData_EntityData *owner, s32 dx, s32 dy, s32 unused, s32 angle)
{
    s32 speed = 0x80000;
    u8 zero = 0;
    struct SpriteBox box = {-4, -4, 8, 8};
    struct EwramData_EntityData *shot = EntityCreateInRange(0x49, 0xCF, LegionShot_Update);
    struct ShotFrames frames = sLegionShotFrames;
    struct EwramData_EntityData *parent = owner->ext.legionPart.owner;
    struct ShotData *data = (struct ShotData *)parent->ext.legion.data.ptr;
    s32 vx, vy, graphics, index;
    u16 *flags;

    if (shot == NULL)
        return;
    shot->posX.whole = owner->posX.whole + dx;
    shot->posY.whole = owner->posY.whole + dy;
    vx = FixedMultiply(Sin(angle + 0x4000), speed);
    vy = FixedMultiply(Sin(angle), speed);
    shot->velX.whole = vx;
    shot->velY.whole = vy;
    shot->spriteLayer = owner->spriteLayer + 1;
    graphics = data->graphics;
    shot->basePalette = data->palette;
    shot->palette = data->palette;
    Sprite_Attach(shot, sLegionSpriteSheet, graphics, shot->basePalette);
    angle &= 0xFFFF;
    index = angle / 0xAAA;
    shot->animFrame = frames.f[index].frame;
    shot->animMode = 2;
    {
        u16 *base = &frames.f[0].flags;
        flags = base + index * 2;
    }
    if (*flags & 1)
        shot->flipX = 1;
    if (*flags & 2)
        shot->flipY = 1;
    shot->enemyId = owner->enemyId;
    shot->ext.legionPart.owner = owner;
    shot->scrollWithBg = 1;
    shot->attack = parent->attack + (parent->attack >> 2);
    Hit_Enable(shot, 8, LegionShot_DamagePlayerOnContact, 0);
    HitBox_SetAttack(shot, &box);
    shot->hit.flags |= 4;
    shot->hit.flags |= zero;
    shot->hit.flags &= ~2;
    shot->drawFunc = Sprite_DrawEntity;
}

/* Spawn a falling fragment at (x, y) with velocity (vx, vy), gravity
 * 0x2000, lifetime 60 and the given frame; graphics slot and palette come
 * from the shared data block at owner->ext.raw.unk_4F8 (see Legion_LoadGraphics). */

void Legion_SpawnDebris(struct EwramData_EntityData *owner, s32 x, s32 y, s32 vx, s32 vy, s32 frame)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, Effect_UpdateTimedParticle);
    s32 *data = (s32 *)owner->ext.legion.data.ptr;
    s32 graphics;

    if (entity == NULL)
        return;
    entity->posX.whole = x;
    entity->posY.whole = y;
    entity->velX.whole = vx;
    entity->velY.whole = vy;
    entity->accelY.whole = 0x2000;
    entity->timer = 0x3C;
    entity->ext.particle.flags = 4;
    entity->spriteLayer = owner->spriteLayer - 1;
    graphics = data[0];
    entity->basePalette = data[9];
    entity->palette = data[9];
    Sprite_Attach(entity, sLegionSpriteSheet, graphics, entity->basePalette);
    entity->animFrame = frame;
    entity->animMode = 2;
    entity->scrollWithBg = 1;
    entity->drawFunc = Sprite_DrawEntity;
}

/* Entity helpers 0x0809FE4C..0x0809FF0F. */

/**
 * @brief 9FE4C | Player_ApplyContactDamage with (attack, 0, 0, 0) at the entity position
 * (same as Chaos2Part_DamagePlayerOnContact).
 */
void LegionDestructibleFragment_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief 9FE78 | Follow the parent (unk_4F4) at offset (unk_4F8, unk_4FC)
 * with its visibility; delete when the parent is gone or changed type
 * (enemyId).
 */
void LegionCoreDetail_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *core = entity->ext.legionPart.owner;

    if (core != NULL && core->updateFunc != NULL
        && core->enemyId == entity->enemyId)
    {
        entity->hidden = core->hidden;
        entity->posX.whole = core->posX.whole + entity->ext.legionPart.offsetX;
        entity->posY.whole = core->posY.whole + entity->ext.legionPart.offsetY;
    }
    else
    {
        EntityDelete(entity);
    }
}

/**
 * @brief 9FED0 | Fall (max 0x40000), blink, delete after timer.
 */
void LegionFragment_Update(struct EwramData_EntityData *entity)
{
    EntityAccelerateAndMove(entity, 0x40000);
    entity->hidden = Entity_GetVisibilityPattern(entity->timer);
    if ((s8)--entity->timer <= 0)
        EntityDelete(entity);
}

/* Legion steering uses a partial view of the state allocation reached through
 * entity->ext.raw.unk_4F8. Room-space position words are at 0x0C/0x10; retain this
 * padded view and the original pointer casts. */

/**
 * @brief Accelerate the path position x (parent word 0x0C) toward target.
 */
void Legion_SteerX(struct EwramData_EntityData *entity, s32 target)
{
    struct LegionData *parent = (struct LegionData *)entity->ext.legion.data.value;
    if (parent->roomX < target) entity->accelX.whole = 0x400;
    else entity->accelX.whole = -0x400;
    EntityAccelerateX(entity, 0x4000);
    parent->roomX += entity->velX.whole;
}

/**
 * @brief Accelerate the path position y (parent word 0x10) toward target.
 */
void Legion_SteerY(struct EwramData_EntityData *entity, s32 target)
{
    struct LegionData *parent = (struct LegionData *)entity->ext.legion.data.value;
    if (parent->roomY < target) entity->accelY.whole = 0x400;
    else entity->accelY.whole = -0x400;
    EntityAccelerateY(entity, 0x4000);
    parent->roomY += entity->velY.whole;
}

/* Kyoma Demon helpers 0x0809FF78..0x0809FFEB (Legion_SteerX / Legion_SteerY
 * before them read an s32 at offset 0x4F0 of the parent and stay asm). */

/**
 * @brief 9FF78 | Which of the lines y = 0x190, y = 0x70, x = 0x70,
 * x = 0x190 (16.16) lies within pos +- size: 0, 2, 1, 3, otherwise -1.
 */
s32 Legion_GetPathEdge(s32 x, s32 y, s32 size)
{
    if (y + size >= 0x1900000 && y - size <= 0x1900000)
        return 0;
    if (y + size >= 0x700000 && y - size <= 0x700000)
        return 2;
    if (x + size >= 0x700000 && x - size <= 0x700000)
        return 1;
    if (x + size >= 0x1900000 && x - size <= 0x1900000)
        return 3;
    return -1;
}

/**
 * @brief 9FFEC | Move with a hitbox; delete when off screen or after
 * 0xF0 frames.
 */
void LegionShot_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;

    EntityMove(entity);
    Hit_CheckWithPlayer(&result, entity);
    if (EntityIsOutsideScreen(entity, 1))
        EntityDelete(entity);
    else if (entity->timer++ >= 0xF0)
        EntityDelete(entity);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   LegionShot_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */

/**
 * @brief A0030 | Same as Enemy_DamagePlayerOnContact.
 */
void LegionShot_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/* Legion_UpdatePreview. */

/**
 * @brief A005C | Substate 0: PaletteAnim_Start with palette (s8)unk_504; then
 * advance the phase in velX by 0x200 and bob posY by sin(phase) / 16.
 */
void Legion_UpdatePreview(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        PaletteAnim_Start(sLegionWalkingCorpsePalette, sLegionPaletteAnim1, (s8)entity->ext.legion.animPalette);
        entity->subState++;
    }
    entity->velX.whole += 0x200;
    entity->posY.whole += Sin(entity->velX.whole) / 16;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Legion_DamagePlayerOnContact = PopUpTrapSmall_OnPlayerContact */

/**
 * @brief A00AC | Same as PopUpTrapSmall_OnPlayerContact.
 */
void Legion_DamagePlayerOnContact(struct EwramData_EntityData *entity)
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
