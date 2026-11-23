/*
 * Stolas (enemy): body, joints, parts, cast effect, beam and debris.
 * One translation unit for the consecutive ROM range of its code; the
 * functions keep their ROM order.
 */
#include "core.h"
#include "player_core.h"
#include "graphics.h"
#include "gameplay.h"
#include "entity_util.h"
#include "structs/ewram.h"
#include <string.h>
#include "sound.h"
#include "structs/entity_template.h"
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

extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *entity);
extern void Stolas_UpdateMove(struct EwramData_EntityData *entity);
extern void Stolas_UpdateAttack(struct EwramData_EntityData *entity);
extern void Stolas_SpawnDebris(struct EwramData_EntityData *entity, s32 x, s32 y, s32 vx, s32 vy);
extern void Stolas_UpdatePreview(struct EwramData_EntityData *entity);

struct StolasPreviewAnimation {
    u16 stepCount;
    u16 type;
    u8 frame;
    u8 duration;
    u16 unk_6;
};

struct StolasBox {
    s32 x:8;
    s32 y:8;
    u32 w:8;
    u32 h:8;
};

extern const struct StolasPreviewAnimation sStolasPreviewAnimation;

void EnemyStolasUpdate(struct EwramData_EntityData *entity)
{
    u8 unused[48];
    struct StolasBox box;
    struct StolasPreviewAnimation anim = sStolasPreviewAnimation;
    s32 *parts;
    /* The signed address comparison below preserves the original ble. */
    s32 *cursor;
    struct EwramData_EntityData **slot;
    struct EwramData_EntityData *child;

    entity->ext.stolas.flags = 0;
    parts = (s32 *)entity->ext.joint.parts;
    entity->ext.stolas.freeSlots = 3;
    cursor = parts;
    slot = (struct EwramData_EntityData **)(parts + 0x48);
    do {
        child = *slot;
        if (child != NULL && child->updateFunc != NULL && child->enemyId == cursor[0x4B]
            && (s16)entity->hp > 0)
            entity->ext.stolas.freeSlots--;
        else
            *slot = NULL;
        cursor++;
        slot++;
    } while ((s32)cursor <= (s32)(parts + 2));
    if (entity->state != 2)
    {
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
        Stolas_UpdateMove(entity);
        break;
    case 1:
        Stolas_UpdateAttack(entity);
        break;
    case 2:
        if ((entity->timer & 3) == 0)
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x260000, 0x100000, 0xC0000, entity->spriteLayer - 1);
        if ((entity->timer & 7) == 0)
        {
            s32 vx = RandomNumberGenerator() % 0x60000 - 0x30000;
            Stolas_SpawnDebris(entity, 0, -0x1D0000, vx, RandomNumberGenerator() % 0x60000 - 0x30000);
        }
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0)
            entity->deleteRequested = 1;
        if (entity->deleteRequested && entity->ext.joint.parts != NULL)
        {
            /* Reuse the allocation variable after the loop. In agbcc this
             * also preserves the distinct base and cursor registers. */
            parts = (s32 *)entity->ext.joint.parts;
            Heap_Free(parts);
        }
        Anim_Update(entity);
        return;
    case 3:
        Stolas_UpdatePreview(entity);
        Anim_Update(entity);
        return;
    }
boxes:
    box.x = 0;
    box.y = -0x48;
    box.w = 0x0A;
    box.h = 0x48;
    HitBox_SetAttack(entity, &box);
    HitBox_SetBody(entity, &box);
    if (entity->ext.stolas.flags & 1)
    {
        Anim_Start(entity, (struct AnimScript *)&anim, 0, 1);
        anim.frame = entity->ext.stolas.heldFrame;
        Anim_Update(entity);
    }
    else
        Anim_Update(entity);
    Enemy_CheckHitPlayer(entity);
}

/* Stolas create: allocates the 6-joint skeleton and one part entity per joint. */
extern void *memcpy(void *dst, const void *src, unsigned long size);
extern u8 Stolas_LoadGraphics(struct EwramData_EntityData *entity);
extern void StolasPart_Update(struct EwramData_EntityData *entity);
extern void Stolas_OnContact(struct EwramData_EntityData *entity);
extern void StolasPart_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void Stolas_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void Stolas_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void Stolas_AttachJoint(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length);
extern void Stolas_AttachJointSetAngle(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length);
extern void Stolas_InterpolateJointsInOrder(struct EwramData_EntityData *entity, s8 *order);
extern void Stolas_FaceTarget(struct EwramData_EntityData *entity);

void EnemyStolasCreate(struct EwramData_EntityData *entity)
{
    u8 frames0[6];
    u8 *frames;
    u8 anchors[6];
    s8 order[6];
    s8 chain[6];
    s32 i;
    s32 index;
    struct EwramData_EntityData *part;
    struct EntityJointPart *parts;
    const u8 *pose;
    const s32 *angles;
    u8 frozen;

    memcpy(frames0, sStolasFrames0, sizeof(frames0));
    memcpy(anchors, sStolasAnchors, sizeof(anchors));
    memcpy(order, sUnk_081190AC, sizeof(order));
    memcpy(chain, sUnk_081190AC, sizeof(chain));

    if (!Stolas_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    entity->ext.joint.parts = Heap_Alloc(0x4E);
    if (entity->ext.joint.parts == NULL)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    entity->flipX = 0;
    entity->ext.joint.pose = 5;
    Enemy_ResetFromTemplate(entity);
    frames = frames0;
    parts = entity->ext.joint.parts;
    for (i = 0; i <= 5; i++)
    {
        index = order[i];
        part = Entity_CreateObject(StolasPart_Update);
        if (part != NULL)
        {
            u16 palette;

            part->spriteLayer = entity->spriteLayer + 1;
            Sprite_Attach(part, sStolasSpriteSheet, ObjGfx_LoadResource(sStolasGfx, 0), entity->basePalette);
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
            palette = ObjAffine_Alloc(1);
            if ((s16)palette < 0)
            {
                EntityDelete(part);
                EntityDelete(entity);
                return;
            }
            part->affineIndex = palette;
            part->affine = 1;
            ObjAffine_Set(0, 0x10000, 0x10000, part->flipX, part->flipY, part->affineIndex);
            if (!(*((u8 *)gEwramData + 0x8E) & 0x40))
                Hit_Enable(part, 8, Stolas_OnContact, (s32)StolasPart_OnHit);
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
    pose = sStolasPartPoses[(s8)entity->ext.joint.pose];
    angles = sStolasPartAngles[(s8)entity->ext.joint.pose];
    for (index = 0; index <= 5; index++)
    {
        parts[index].offsetX = (s8)pose[index * 2] << 16;
        parts[index].offsetY = (s8)pose[index * 2 + 1] << 16;
        if (index == 2)
            parts[2].rotationAngle = angles[0];
        else if (index == 5)
            ((s32 *)parts)[5 * 12 + 2] = angles[1];
        else
            parts[index].rotationAngle = 0x10000;
    }
    entity->timer = 1;
    Stolas_AttachJoint(entity, 1, 0, 0x12800, 0x80000);
    Stolas_AttachJoint(entity, 2, 1, 0x13000, 0xD0000);
    Stolas_AttachJoint(entity, 4, 3, 0x14000, 0x80000);
    Stolas_AttachJoint(entity, 5, 4, 0x15000, 0xD0000);
    Stolas_InterpolateJointsInOrder(entity, chain);
    Stolas_AttachJointSetAngle(entity, 1, 0, 0, 0x80000);
    Stolas_AttachJointSetAngle(entity, 2, 1, 0, 0xD0000);
    Stolas_AttachJointSetAngle(entity, 4, 3, 0, 0x80000);
    Stolas_AttachJointSetAngle(entity, 5, 4, 0, 0xD0000);
    entity->timer = 1;
    Stolas_InterpolateJointsInOrder(entity, chain);
    frozen = *((u8 *)gEwramData + 0x8E) & 0x40;
    if (frozen)
    {
        entity->state = 3;
        entity->timer = 0;
        entity->ext.stolas.drawOffsetX = 0;
        entity->ext.stolas.drawOffsetY = -29;
        return;
    }
    Hit_Enable(entity, 8, Stolas_DamagePlayerOnContact, (s32)Stolas_OnHit);
    Stolas_FaceTarget(entity);
    Entity_SnapToGround(entity);
    entity->ext.stolas.drawOffsetX = frozen;
    entity->ext.stolas.drawOffsetY = -29;
    entity->ext.joint.hurtMask = 1;
    if (entity->spawnParam0 == 0)
        entity->spawnParam0 = 13;
    if (entity->spawnParam1 == 0)
        entity->spawnParam1 = 0x52;
    entity->state = 0;
    entity->subState = 0;
}

/* Stolas movement and transitions to attack states. */
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);
extern void Stolas_TurnAround(struct EwramData_EntityData *);
extern s32 Target_GetPixelX(struct EwramData_EntityData *);
#define POSE(e) (e->ext.joint.pose)
/**
 * @brief Movement state machine (state 0): face the target, wake when it is within
 * 160 px or Stolas is damaged, then fly with the joint poses and start the
 * attack (state 1).
 */
void Stolas_UpdateMove(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    s8 order[6];
    memcpy(order, sUnk_081190AC, sizeof(order));
    switch (entity->subState)
    {
    case 0:
        Stolas_FaceTarget(entity);
        if (!(Target_IsNearEntity(entity, 160, 160) || Enemy_IsDamaged(entity))) break;
        entity->subState = 1;
        entity->stage = 0;
        entity->timer = 36;
        Anim_StartIndexed(entity, 1, 0, 0);
        POSE(entity) = 1;
        Stolas_AttachJoint(entity, 1, 0, 0x14800, 0x80000);
        Stolas_AttachJoint(entity, 2, 1, 0x16800, 0xd0000);
        Stolas_AttachJoint(entity, 4, 3, 0x13000, 0x80000);
        Stolas_AttachJoint(entity, 5, 4, 0x13800, 0xd0000);
        if (entity->flipX) entity->velX.whole = 0xD0000 / entity->timer;
        else entity->velX.whole = -0xD0000 / entity->timer;
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0x2800;
        break;
    case 1:
        Stolas_InterpolateJointsInOrder(entity, order);
        Entity_MoveWithCollisionDefault(entity, 0x10000, -0x300000);
        if ((s8)--entity->timer > 0) break;
        entity->timer = 16;
        entity->subState++;
        POSE(entity) = 2;
        Stolas_AttachJoint(entity, 1, 0, 0x14000, 0x80000);
        Stolas_AttachJoint(entity, 2, 1, 0x15000, 0xd0000);
        Stolas_AttachJoint(entity, 4, 3, 0x12800, 0x80000);
        Stolas_AttachJoint(entity, 5, 4, 0x13000, 0xd0000);
        if (entity->flipX) entity->velX.whole = 0x40000 / entity->timer;
        else entity->velX.whole = -0x40000 / entity->timer;
        break;
    case 2:
        Stolas_InterpolateJointsInOrder(entity, order);
        Entity_MoveWithCollisionDefault(entity, 0x10000, -0x300000);
        if ((s8)--entity->timer <= 4)
        {
            entity->timer = 10;
            entity->subState++;
            POSE(entity) = 3;
        }
        break;
    case 3:
        Stolas_InterpolateJointsInOrder(entity, order);
        if ((s8)--entity->timer > 0) break;
        entity->timer = 36;
        entity->subState++;
        Anim_StartIndexed(entity, 1, 0, 0);
        POSE(entity) = 4;
        Stolas_AttachJoint(entity, 1, 0, 0x13000, 0x80000);
        Stolas_AttachJoint(entity, 2, 1, 0x13800, 0xd0000);
        Stolas_AttachJoint(entity, 4, 3, 0x14800, 0x80000);
        Stolas_AttachJoint(entity, 5, 4, 0x16800, 0xd0000);
        if (entity->flipX) entity->velX.whole = 0xD0000 / entity->timer;
        else entity->velX.whole = -0xD0000 / entity->timer;
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0x2800;
        break;
    case 4:
        Stolas_InterpolateJointsInOrder(entity, order);
        Entity_MoveWithCollisionDefault(entity, 0x10000, -0x300000);
        if ((s8)--entity->timer > 0) break;
        entity->timer = 16;
        entity->subState++;
        POSE(entity) = 5;
        Stolas_AttachJoint(entity, 1, 0, 0x12800, 0x80000);
        Stolas_AttachJoint(entity, 2, 1, 0x13000, 0xd0000);
        Stolas_AttachJoint(entity, 4, 3, 0x14000, 0x80000);
        Stolas_AttachJoint(entity, 5, 4, 0x15000, 0xd0000);
        if (entity->flipX) entity->velX.whole = 0x40000 / entity->timer;
        else entity->velX.whole = -0x40000 / entity->timer;
        break;
    case 5:
        Stolas_InterpolateJointsInOrder(entity, order);
        Entity_MoveWithCollisionDefault(entity, 0x10000, -0x300000);
        if ((s8)--entity->timer <= 4)
        {
            entity->timer = 10;
            entity->subState++;
            POSE(entity) = 6;
        }
        break;
    case 6:
        Stolas_InterpolateJointsInOrder(entity, order);
        if ((s8)--entity->timer <= 0)
        {
            s32 changed;
            Stolas_FaceTarget(entity);
            changed = 0;
            if (entity->flipX)
            {
                if (Entity_GetHorizontalRoomEdge(entity, 20) == 2)
                {
                    Stolas_TurnAround(entity);
                    goto reset;
                }
            }
            else if (Entity_GetHorizontalRoomEdge(entity, 20) == 1)
            {
                Stolas_TurnAround(entity);
                changed = 1;
            }
            if (!changed)
            {
                if ((RandomNumberGenerator() & 127) <= 63)
                {
                    s32 distance = 48;
                    /* Keep the X pointer in the original callee-saved register;
                     * automatic allocation swaps camera/X/Y storage. */
                    register u16 *entityX asm("r8") = &entity->posX.part16.integer; /* FAKEMATCH: pinned register */
                    s16 *entityY = &entity->posY.part16.integer;
                    for (; distance > 0; distance -= 8)
                    {
                        s32 x;
                        if (entity->flipX)
                            x = camera->xPos.part16.integer + *entityX + distance;
                        else
                            x = camera->xPos.part16.integer + *entityX - distance;
                        /* The statement expression delays Y evaluation until after
                         * X narrowing; unsigned component loads narrow only at the end. */
                        if (!Tile_Classify((s16)x, ({
                            s32 y = (u16)camera->yPos.part16.integer;
                            y += (u16)*entityY;
                            (s16)y;
                        }))) break;
                    }
                    entity->ext.stolas.beamLength = distance;
                    if (distance >= 0 && (s16)entity->ext.stolas.freeSlots > 0)
                    {
                        entity->state = 1;
                        entity->subState = 0;
                        break;
                    }
                }
                else
                {
                    s32 distance = (s16)Target_GetPixelX(entity) - (s16)entity->posX.part16.integer;
                    if (distance < 0) distance = -distance;
                    if (distance <= 31)
                    {
                        entity->state = 1;
                        entity->subState = 10;
                        break;
                    }
                }
            }
reset:
            entity->timer = 36;
            entity->subState = 1;
            Anim_StartIndexed(entity, 1, 0, 0);
            POSE(entity) = 1;
            Stolas_AttachJoint(entity, 1, 0, 0x14800, 0x80000);
            Stolas_AttachJoint(entity, 2, 1, 0x16800, 0xd0000);
            Stolas_AttachJoint(entity, 4, 3, 0x13000, 0x80000);
            Stolas_AttachJoint(entity, 5, 4, 0x13800, 0xd0000);
            if (entity->flipX) entity->velX.whole = 0xD0000 / entity->timer;
            else entity->velX.whole = -0xD0000 / entity->timer;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
        }
        break;
    }
}

/* Stolas attack substates and return to the movement pose. */
extern void Stolas_SpawnCastEffect(struct EwramData_EntityData *, s32, s32);
extern void Stolas_SpawnBeamSegment(struct EwramData_EntityData *, s32, s32);
extern void Stolas_SpawnBeamImpact(struct EwramData_EntityData *, s32, s32);
/* Six 0x30-byte joints, then three child slots and their template IDs. */
struct StolasParts {
    struct EntityJointPart joints[6];
    struct EwramData_EntityData *children[3];
    s32 ids[3];
};
#define FLAGS(e) (e->ext.stolas.flags)
#define FRAME(e) (e->ext.stolas.heldFrame)
#define DISTANCE(e) (e->ext.stolas.beamLength)
#define TYPE_A(e) ((e)->spawnParam0)
#define TYPE_B(e) ((e)->spawnParam1)
/**
 * @brief Attack state machine (state 1): cast effect (Stolas_SpawnCastEffect),
 * a column of five beam segments in front (Stolas_SpawnBeamSegment) and at
 * frame 73 the impact (Stolas_SpawnBeamImpact); then back to state 0.
 */
void Stolas_UpdateAttack(struct EwramData_EntityData *entity)
{
    s8 order[6];
    /* Function scope preserves the original order of spilled pointer slots. */
    struct EwramData_EntityData *child;
    memcpy(order, sUnk_081190AC, sizeof(order));
    switch (entity->subState)
    {
    case 0:
        entity->subState++;
        entity->timer = 0;
        Stolas_SpawnCastEffect(entity, entity->flipX ? 0xC0000 : -0xC0000, -0x2E0000);
        /* fall through */
    case 1:
        FLAGS(entity) |= 1;
        FRAME(entity) = 13;
        if (entity->timer++ > 59)
        {
            entity->timer = 0;
            entity->subState++;
            FLAGS(entity) = (u8)FLAGS(entity) & 0xFE;
            Anim_StartIndexed(entity, 2, 0, 0);
            Stolas_SpawnBeamSegment(entity, DISTANCE(entity), 1);
            Stolas_SpawnBeamSegment(entity, DISTANCE(entity), -31);
            Stolas_SpawnBeamSegment(entity, DISTANCE(entity), -63);
            Stolas_SpawnBeamSegment(entity, DISTANCE(entity), -95);
            Stolas_SpawnBeamSegment(entity, DISTANCE(entity), -127);
            PlaySong(0x11F);
        }
        break;
    case 2:
        if (++entity->timer == 73) Stolas_SpawnBeamImpact(entity, DISTANCE(entity), 1);
        if (entity->timer == 112)
        {
            child = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
            if (child != NULL)
            {
                struct StolasParts *parts = (struct StolasParts *)entity->ext.joint.parts;
                s32 type;
                s32 index;
                if ((RandomNumberGenerator() & 127) <= 31)
                {
                    if (TYPE_B(entity) >= 0) type = TYPE_B(entity);
                    else type = TYPE_A(entity);
                }
                else type = TYPE_A(entity);
                /* Add the scaled index before each array-field offset. Rewriting
                 * these as direct indexed fields changes the address arithmetic. */
                for (index = 0; index < 3; index++)
                {
                    if (((struct StolasParts *)((u8 *)parts + index * 4))->children[0] == NULL)
                    {
                        ((struct StolasParts *)((u8 *)parts + index * 4))->children[0] = child;
                        ((struct StolasParts *)((u8 *)parts + index * 4))->ids[0] = type;
                        break;
                    }
                }
                if (entity->flipX)
                    child->posX.part16.integer = DISTANCE(entity) + entity->posX.part16.integer;
                else
                {
                    u16 distance = DISTANCE(entity);
                    child->posX.part16.integer = entity->posX.part16.integer - distance;
                }
                child->posY.part16.integer = entity->posY.part16.integer;
                child->flipX = entity->flipX;
                child->scrollWithBg = 1;
                child->spawnIndex = 0;
                child->enemyId = type;
                child->drawFunc = (u32 *)Sprite_DrawEntity;
                {
                    struct EntityTemplate *template = &sEnemyData[type];
                    child->updateFunc = template->updateFunc;
                    template->createFunc(child);
                }
            }
        }
        if (entity->animDone)
        {
            entity->state = 0;
            entity->timer = 36;
            entity->subState = 1;
            FLAGS(entity) = (u8)FLAGS(entity) & 0xFE;
            Anim_StartIndexed(entity, 1, 0, 0);
            POSE(entity) = 1;
            Stolas_AttachJoint(entity, 1, 0, 0x14800, 0x80000);
            Stolas_AttachJoint(entity, 2, 1, 0x16800, 0xd0000);
            Stolas_AttachJoint(entity, 4, 3, 0x13000, 0x80000);
            Stolas_AttachJoint(entity, 5, 4, 0x13800, 0xd0000);
            if (entity->flipX) entity->velX.whole = 0xD0000 / entity->timer;
            else entity->velX.whole = -0xD0000 / entity->timer;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
        }
        break;
    case 10:
        entity->subState++;
            POSE(entity) = 7;
            Stolas_AttachJoint(entity, 1, 0, 0x17000, 0x80000);
            Stolas_AttachJoint(entity, 2, 1, 0x17000, 0xd0000);
            Stolas_AttachJoint(entity, 4, 3, 0x13000, 0x80000);
            Stolas_AttachJoint(entity, 5, 4, 0x13000, 0xd0000);
        entity->timer = 12;
        if (entity->flipX) entity->velX.whole = (0x100000 / entity->timer);
        else entity->velX.whole = (-0x100000 / entity->timer);
        entity->ext.joint.hurtMask |= 4;
        break;
    case 11:
        Stolas_InterpolateJointsInOrder(entity, order);
        EntityMoveX(entity);
        if ((s8)--entity->timer <= 0)
        {
            entity->timer = 30;
            entity->subState++;
        }
        break;
    case 12:
        Stolas_InterpolateJointsInOrder(entity, order);
        if ((s8)--entity->timer <= 0)
        {
            entity->timer = 30;
            entity->subState++;
            POSE(entity) = 5;
            Stolas_AttachJoint(entity, 1, 0, 0x12800, 0x80000);
            Stolas_AttachJoint(entity, 2, 1, 0x13000, 0xd0000);
            Stolas_AttachJoint(entity, 4, 3, 0x14000, 0x80000);
            Stolas_AttachJoint(entity, 5, 4, 0x15000, 0xd0000);
            if (entity->flipX) entity->velX.whole = -0xE0000 / entity->timer;
            else entity->velX.whole = 0xE0000 / entity->timer;
        }
        break;
    case 13:
        Stolas_InterpolateJointsInOrder(entity, order);
        EntityMoveX(entity);
        if ((s8)--entity->timer <= 6)
        {
            entity->timer = 12;
            entity->subState++;
            if (entity->flipX) entity->velX.whole = (-0x20000 / entity->timer);
            else entity->velX.whole = (0x20000 / entity->timer);
        }
        break;
    case 14:
        Stolas_InterpolateJointsInOrder(entity, order);
        EntityMoveX(entity);
        if ((s8)--entity->timer <= 0)
        {
            s32 one;
            entity->state = 0;
            /* Retain this local to order the state/timer stores as in ROM. */
            one = 1;
            entity->timer = 36;
            entity->subState = one;
            POSE(entity) = one;
            Stolas_AttachJoint(entity, 1, 0, 0x14800, 0x80000);
            Stolas_AttachJoint(entity, 2, 1, 0x16800, 0xd0000);
            Stolas_AttachJoint(entity, 4, 3, 0x13000, 0x80000);
            Stolas_AttachJoint(entity, 5, 4, 0x13800, 0xd0000);
            if (entity->flipX) entity->velX.whole = 0xD0000 / entity->timer;
            else entity->velX.whole = -0xD0000 / entity->timer;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
            entity->ext.joint.hurtMask &= ~4;
        }
        break;
    }
}

/* Interpolate a Stolas joint toward a pose or its hanging angle; parts 2
 * and 5 also turn toward the pose angles (cf. Creaking Skull CreakingSkull_InterpolateJoint). */
/**
 * @brief Interpolate one of six joints toward pose coordinates or a hanging angle using the entity timer; also interpolate pose rotations of joints 2 and 5.
 */
void Stolas_InterpolateJoint(struct EwramData_EntityData *entity, s32 index)
{
    struct EntityJointPart *parts = entity->ext.joint.parts;
    const u8 *pose = sStolasPartPoses[(s8)entity->ext.joint.pose];
    const s32 *angles = sStolasPartAngles[(s8)entity->ext.joint.pose];
    s32 x, y, dx, dy, a;
    struct EntityJointPart *part;
    s32 *p5;
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
        else if (index == 5)
        {
            x = angles[1];
            if (entity->flipX) x = 0x20000 - x;
            p5 = &parts[5].rotationAngle;
            a = *p5;
            x -= a;
            a += x / entity->timer;
            *p5 = a;
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
        else if (index == 5)
        {
            x = angles[1];
            if (entity->flipX) x = 0x20000 - x;
            p5 = &parts[5].rotationAngle;
            a = *p5;
            x -= a;
            a += x / entity->timer;
            *p5 = a;
        }
    }
}

/**
 * @brief When target-facing changes, flip and mirror all six joint X positions and reflect their rotations around 0x20000.
 */
void Stolas_FaceTarget(struct EwramData_EntityData *entity)
{
    u32 facing = entity->flipX;
    if (facing != Entity_IsTargetRight(entity))
    {
        struct EntityJointPart *part;
        s32 i;
        s32 halfTurn;
        entity->flipX ^= 1;
        halfTurn = 0x20000;
        part = entity->ext.joint.parts;
        for (i = 5; i >= 0; i--)
        {
            part->offsetX = -part->offsetX;
            part->rotationAngle = halfTurn - part->rotationAngle;
            part++;
        }
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Stolas_AttachJointSetAngle = CreakingSkull_AttachJointSetAngle */

/**
 * @brief Bind a joint with supplied or measured length; only values above 0xFFFF are absolute, and that branch also replaces the current angle with the mirrored target.
 */
void Stolas_AttachJointSetAngle(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length)
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

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Stolas_AttachJoint = CreakingSkull_AttachJoint */

/**
 * @brief Bind a joint to its parent with supplied or measured length; mirror the target angle by facing, treating absolute values above 0xFFFF in magnitude as absolute angles.
 */
void Stolas_AttachJoint(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length)
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

/* Stolas hit callback: on death, emit six projectiles with random velocities. */
void Stolas_OnDamaged(struct EwramData_EntityData *entity, s32 attacker, u16 *hit, s32 damage)
{
    entity->hitFlashTimer = 8;
    entity->hp -= damage;
    if ((s16)entity->hp <= 0)
    {
        s32 i;
        u32 range;
        s32 offset;
        Enemy_DropRewards(entity);
        Anim_StartIndexed(entity, 3, 0, 1);
        PlaySong(0x6E);
        entity->palette = entity->basePalette;
        entity->timer = 60;
        entity->state = 2;
        entity->subState = 0;
        range = 0x60000;
        offset = -0x30000;
        for (i = 5; i >= 0; i--)
        {
            s32 vx = RandomNumberGenerator() % range + offset;
            s32 vy = RandomNumberGenerator() % range + offset;
            Stolas_SpawnDebris(entity, 0, -0x1D0000, vx, vy);
        }
    }
}

/* Stolas part update: follows its joint of the parent's skeleton, aims the
 * sprite along the bone and maintains the hit boxes of joints 0 and 2. */

struct StolasPartBox {
    s8 x;
    s8 y;
    s8 width;
    s8 height;
};

extern void Hit_CheckWithPlayerSharedHistory(void *, struct EwramData_EntityData *, struct EwramData_EntityData *);

#define JOINT(e) ((s8)e->ext.jointPart.index)
#define FIX_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)

void StolasPart_Update(struct EwramData_EntityData *entity)
{
    u8 hit[16];
    u8 unused[13];
    struct StolasPartBox box;
    struct EwramData_EntityData *parent = entity->ext.jointPart.owner;
    struct EntityJointPart *parts;
    s32 size = 8;
    s32 x;
    s32 y;
    u16 angle;
    s32 state;

    memcpy(unused, sUnk_081190B2, sizeof(unused));
    parts = parent->ext.joint.parts;
    state = entity->state;
    if (state == 0)
    {
        if (parent != NULL && parent->updateFunc != NULL
            && parent->enemyId == entity->enemyId)
        {
            entity->palette = parent->palette;
            entity->flipX = parent->flipX;
            entity->timer++;
            entity->posX.whole = parent->posX.whole + parts[JOINT(entity)].offsetX;
            entity->posY.whole = parent->posY.whole + parts[JOINT(entity)].offsetY;
            if (entity->ext.jointPart.index <= 1 || JOINT(entity) == 3 || JOINT(entity) == 4)
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
            else if (JOINT(entity) == 2 || JOINT(entity) == 5)
            {
                entity->velX.whole = parts[JOINT(entity)].rotationAngle;
            }
            if (parent->state == 2)
            {
                entity->state = 1;
                entity->subState = 0;
                entity->timer = 0x3C;
                return;
            }
            if ((parent->ext.joint.hurtMask >> JOINT(entity)) & 1)
            {
                if (JOINT(entity) == 0)
                {
                    box.x = -16;
                    box.y = -24;
                    box.width = 32;
                    box.height = 24;
                    HitBox_SetAttack(entity, &box);
                    HitBox_SetBody(entity, &box);
                    entity->hit.flags &= 0xFB;
                    entity->hit.flags &= 0xFD;
                    Hit_CheckWithPlayerSharedHistory(hit, entity, parent);
                }
                else if (JOINT(entity) == 2)
                {

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
                    { s32 px = x * size; box.x = FIX_INT(px) - 3; }
                    { s32 py = y * size; box.y = FIX_INT(py) - 3; }
                    box.width = 8;
                    box.height = 8;
                    HitBox_SetAttack(entity, &box);
                    entity->hit.flags &= 0xFD;
                    entity->hit.flags &= 0xFB;
                    Hit_CheckWithPlayerSharedHistory(hit, entity, parent);
                }
            }
            else
            {
                entity->hit.flags |= 4;
                entity->hit.flags |= 2;
            }
            ObjAffine_Set(entity->velX.whole, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
            return;
        }
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
    else if (state == 1)
    {
        entity->posX.whole = parent->posX.whole + parts[JOINT(entity)].offsetX;
        entity->posY.whole = parent->posY.whole + parts[JOINT(entity)].offsetY;
        state = parent->hidden;
        entity->hidden = state;
        if ((s8)--entity->timer <= 0)
        {
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
        }
    }
}

extern void StolasCastEffect_Update(struct EwramData_EntityData *entity);

/**
 * @brief 9BD5C | Create an effect StolasCastEffect_Update of the owner (file
 * 0x0809B9D4) at (dx, dy) from it: graphics sStolasSpriteSheet / sStolasGfx
 * with the owner's palette, frame 0xC, boxes off, layer one below the
 * owner, owner in unk_500, its facing; own palette with blending and an
 * affine matrix with angle unk_4F8 - 0x8000. Deleted when the graphics or
 * the palette cannot be allocated.
 */
void Stolas_SpawnCastEffect(struct EwramData_EntityData *entity, s32 dx, s32 dy)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, (u32 *)StolasCastEffect_Update);
    s32 tiles;
    u16 palette;

    if (child == NULL)
        return;
    tiles = ObjGfx_Find(sStolasGfx);
    if (tiles < 0)
        goto delete;
    Sprite_Attach(child, sStolasSpriteSheet, tiles, entity->basePalette);
    child->animFrame = 0xC;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    child->scrollWithBg = 1;
    child->spriteLayer = entity->spriteLayer - 1;
    child->basePalette = entity->basePalette;
    child->enemyId = entity->enemyId;
    child->ext.stolasEffect.owner = entity;
    child->flipX = entity->flipX;
    child->posX.whole = entity->posX.whole + dx;
    child->posY.whole = entity->posY.whole + dy;
    palette = ObjAffine_Alloc(1);
    if ((s16)palette < 0)
    {
    delete:
        EntityDelete(child);
        return;
    }
    child->affineIndex = palette;
    child->affine = 1;
    ObjAffine_Set(child->ext.stolasEffect.castAngle - 0x8000, 0x10000, 0x10000, child->flipX, child->flipY, child->affineIndex);
}

/**
 * @brief 9BE78 | Effect created by Stolas_SpawnCastEffect: freed (palette and
 * entity) when its owner is gone or no longer entity 0x61; copies the
 * owner's blending 4 or uses its own palette index; after 15 frames the
 * affine angle unk_4F8 turns by 0x800 per frame up to +-0x10000 depending
 * on the facing; freed after 60 frames or when the owner dies; affine
 * matrix from the angle.
 */
void StolasCastEffect_Update(struct EwramData_EntityData *entity)
{
    u8 unused[16];
    struct EwramData_EntityData *owner = entity->ext.stolasEffect.owner;

    if (owner == NULL || owner->updateFunc == NULL || owner->enemyId != 0x61)
        goto free;
    if (entity->state == 0)
    {
        if (owner->palette == 4)
            entity->palette = 4;
        else
            entity->palette = entity->basePalette;
        if (entity->timer > 0xE)
        {
            if (entity->flipX)
            {
                if (entity->ext.stolasEffect.castAngle <= 0xFFFF)
                    entity->ext.stolasEffect.castAngle += 0x800;
            }
            else if (entity->ext.stolasEffect.castAngle > -0x10000)
                entity->ext.stolasEffect.castAngle += -0x800;
        }
        if (entity->timer++ > 0x3B || owner->state == 2)
        {
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
        }
    }
    ObjAffine_Set(entity->ext.stolasEffect.castAngle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    return;
free:
    ObjAffine_Free(entity->affineIndex);
    EntityDelete(entity);
}

/* Stolas_SpawnBeamSegment (Stolas hit box child). */
extern void StolasBeamSegment_Update(struct EwramData_EntityData *entity);

/**
 * @brief 9BF7C | Spawn a StolasBeamSegment_Update child (slots 0x1B..0x48) at
 * (offsetX, offsetY) px from entity (x mirrored by its facing) with the
 * already loaded graphics sStolasGfx (deleted when they are not loaded),
 * frame 0x1F, layer 9, entity's palette, type enemyId and facing; the
 * parent is kept in ext.stolasEffect.owner.
 */
void Stolas_SpawnBeamSegment(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, (u32 *)StolasBeamSegment_Update);
    s32 gfx;

    if (child == NULL)
        return;
    gfx = ObjGfx_Find(sStolasGfx);
    if (gfx < 0)
    {
        EntityDelete(child);
        return;
    }
    Sprite_Attach(child, sStolasSpriteSheet, gfx, entity->basePalette);
    child->animFrame = 0x1F;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    child->scrollWithBg = 1;
    child->spriteLayer = 9;
    child->basePalette = entity->basePalette;
    child->enemyId = entity->enemyId;
    child->ext.stolasEffect.owner = entity;
    child->flipX = entity->flipX;
    if (entity->flipX)
        child->posX.whole = entity->posX.whole + (offsetX << 16);
    else
        child->posX.whole = entity->posX.whole - (offsetX << 16);
    child->posY.whole = entity->posY.whole + (offsetY << 16);
}

/* StolasBeamSegment_Update (Stolas hit box child update). */
/**
 * @brief 9C054 | Deleted when the parent (ext.stolasEffect.owner) is gone or no longer of
 * type 0x61; in state 0 switch the frame at 8 / 16 / 82 frames (0x20 ..
 * 0x22) and delete after 90 frames or when the parent is in state 2.
 */
void StolasBeamSegment_Update(struct EwramData_EntityData *entity)
{
    u8 collision[0x10];
    struct EwramData_EntityData *stolas = entity->ext.stolasEffect.owner;

    if (stolas != NULL && stolas->updateFunc != NULL && stolas->enemyId == 0x61)
    {
        if (entity->state == 0)
        {
            if (entity->timer == 8)
                entity->animFrame = 0x20;
            if (entity->timer == 0x10)
                entity->animFrame = 0x21;
            if (entity->timer == 0x52)
                entity->animFrame = 0x22;
            if (entity->timer++ > 0x59 || stolas->state == 2)
                EntityDelete(entity);
        }
    }
    else
    {
        EntityDelete(entity);
    }
}

/* Stolas_SpawnBeamImpact (Stolas hit box child). */
extern void StolasBeamImpact_Update(struct EwramData_EntityData *entity);

/**
 * @brief 9C0C8 | Spawn a StolasBeamImpact_Update child (slots 0x1B..0x48) at
 * (offsetX, offsetY) px from entity (x mirrored by its facing) with the
 * already loaded graphics sStolasGfx (deleted when they are not loaded),
 * frame 0x23, layer 0xA, entity's palette, type enemyId and facing; the
 * parent is kept in ext.stolasEffect.owner.
 */
void Stolas_SpawnBeamImpact(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, (u32 *)StolasBeamImpact_Update);
    s32 gfx;

    if (child == NULL)
        return;
    gfx = ObjGfx_Find(sStolasGfx);
    if (gfx < 0)
    {
        EntityDelete(child);
        return;
    }
    Sprite_Attach(child, sStolasSpriteSheet, gfx, entity->basePalette);
    child->animFrame = 0x23;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    child->scrollWithBg = 1;
    child->spriteLayer = 0xA;
    child->basePalette = entity->basePalette;
    child->enemyId = entity->enemyId;
    child->ext.stolasEffect.owner = entity;
    child->flipX = entity->flipX;
    if (entity->flipX)
        child->posX.whole = entity->posX.whole + (offsetX << 16);
    else
        child->posX.whole = entity->posX.whole - (offsetX << 16);
    child->posY.whole = entity->posY.whole + (offsetY << 16);
}

/* StolasBeamImpact_Update (Stolas hit box child update, second kind). */
/**
 * @brief 9C1A0 | Deleted when the parent (unk_500) is gone or no longer of
 * type 0x61; in state 0 switch the frame at 8 / 16 / 24 / 40 frames (0x24
 * .. 0x27) and delete after 48 frames or when the parent is in state 2.
 */
void StolasBeamImpact_Update(struct EwramData_EntityData *entity)
{
    u8 collision[0x10];
    struct EwramData_EntityData *stolas = entity->ext.stolasEffect.owner;

    if (stolas != NULL && stolas->updateFunc != NULL && stolas->enemyId == 0x61)
    {
        if (entity->state == 0)
        {
            if (entity->timer == 8)
                entity->animFrame = 0x24;
            if (entity->timer == 0x10)
                entity->animFrame = 0x25;
            if (entity->timer == 0x18)
                entity->animFrame = 0x26;
            if (entity->timer == 0x28)
                entity->animFrame = 0x27;
            if (entity->timer++ > 0x2F || stolas->state == 2)
                EntityDelete(entity);
        }
    }
    else
    {
        EntityDelete(entity);
    }
}

/* Stolas_SpawnDebris (Stolas projectile). */
extern void StolasDebris_Update(struct EwramData_EntityData *entity);

/**
 * @brief 9C224 | Spawn a StolasDebris_Update projectile (slots 0x1B..0x48) at
 * entity's position + (offsetX, offsetY) with velocity (velocityX, velocityY),
 * gravity 0x800, timer 40, the already loaded graphics sStolasGfx (deleted
 * when they are not loaded), frame 0x1E, one layer above, entity's palette,
 * type enemyId and facing; the parent is kept in unk_500.
 */
void Stolas_SpawnDebris(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY, s32 velocityX, s32 velocityY)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, (u32 *)StolasDebris_Update);
    s32 gfx;

    if (child == NULL)
        return;
    gfx = ObjGfx_Find(sStolasGfx);
    if (gfx < 0)
    {
        EntityDelete(child);
        return;
    }
    Sprite_Attach(child, sStolasSpriteSheet, gfx, entity->basePalette);
    child->animFrame = 0x1E;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    child->scrollWithBg = 1;
    child->spriteLayer = entity->spriteLayer + 1;
    child->basePalette = entity->basePalette;
    child->enemyId = entity->enemyId;
    child->ext.stolasEffect.owner = entity;
    child->flipX = entity->flipX;
    child->posX.whole = entity->posX.whole + offsetX;
    child->posY.whole = entity->posY.whole + offsetY;
    child->velX.whole = velocityX;
    child->velY.whole = velocityY;
    child->accelY.whole = 0x800;
    child->timer = 0x28;
}

/* Stolas body-part helpers 0x0809C304..0x0809C41B. */
#define STOLAS_PART_COUNT 6

/**
 * @brief When timer is nonzero, interpolate joints 0..5 in numerical order.
 */
void Stolas_InterpolateJoints(struct EwramData_EntityData *entity)
{
    s32 i;

    if (entity->timer != 0)
    {
        for (i = 0; i < STOLAS_PART_COUNT; i++)
            Stolas_InterpolateJoint(entity, i);
    }
}

/**
 * @brief When timer is nonzero, interpolate six joints in the supplied signed-byte index order.
 */
void Stolas_InterpolateJointsInOrder(struct EwramData_EntityData *entity, s8 *order)
{
    s32 i;

    if (entity->timer != 0)
    {
        for (i = 0; i < STOLAS_PART_COUNT; i++)
            Stolas_InterpolateJoint(entity, order[i]);
    }
}

/**
 * @brief Unconditionally flip facing and mirror all six joint X positions and rotations.
 */
void Stolas_TurnAround(struct EwramData_EntityData *entity)
{
    struct EntityJointPart *parts;
    s32 halfTurn;
    s32 i;

    entity->flipX ^= 1;
    halfTurn = 0x20000;
    parts = entity->ext.joint.parts;
    for (i = STOLAS_PART_COUNT - 1; i >= 0; parts++, i--)
    {
        parts->offsetX = -parts->offsetX;
        parts->rotationAngle = halfTurn - parts->rotationAngle;
    }
}

/**
 * @brief Clear the selected joint target and current hanging-angle fields.
 */
void Stolas_ClearJointAngles(struct EwramData_EntityData *entity, s32 partIndex)
{
    struct EntityJointPart *parts = entity->ext.joint.parts;

    parts[partIndex].targetSwingAngle = 0;
    parts[partIndex].swingAngle = 0;
}

/**
 * @brief Save X/Y positions of all six joints into their backup fields.
 */
void Stolas_SaveJointPositions(struct EwramData_EntityData *entity)
{
    struct EntityJointPart *parts = entity->ext.joint.parts;
    s32 i;

    for (i = STOLAS_PART_COUNT - 1; i >= 0; parts++, i--)
    {
        parts->savedOffsetX = parts->offsetX;
        parts->savedOffsetY = parts->offsetY;
    }
}

/**
 * @brief Restore X/Y positions of all six joints from their backup fields.
 */
void Stolas_RestoreJointPositions(struct EwramData_EntityData *entity)
{
    struct EntityJointPart *parts = entity->ext.joint.parts;
    s32 i;

    for (i = STOLAS_PART_COUNT - 1; i >= 0; parts++, i--)
    {
        parts->offsetX = parts->savedOffsetX;
        parts->offsetY = parts->savedOffsetY;
    }
}

/**
 * @brief Apply player contact damage at the entity position; effect field is 1 for attack index 2 and zero otherwise, with reaction field 1.
 */
void Stolas_OnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    if ((s8)entity->ext.jointPart.index == 2)
        contactDamage.attributes = 1;
    else
        contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/* Stolas helpers 0x0809C41C..0x0809C5E7. */
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);

void Stolas_Draw(struct EwramData_EntityData *entity);

/**
 * @brief 9C41C | Hit on a part: compute the damage at the part's position
 * for the body (unk_500), show the number and pass it to Stolas_OnDamaged.
 */
void StolasPart_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    struct EwramData_EntityData *stolas = entity->ext.jointPart.owner;
    s32 oldX = stolas->posX.whole;
    s32 oldY = stolas->posY.whole;
    s32 damage;

    stolas->posX.whole = entity->posX.whole;
    stolas->posY.whole = entity->posY.whole;
    damage = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(entity->ext.jointPart.owner, attacker, hit, (s16)damage);
    stolas->posX.whole = oldX;
    stolas->posY.whole = oldY;
    Entity_ShowDamage(entity, damage, 0);
    Stolas_OnDamaged(entity->ext.jointPart.owner, attacker, hit, (s16)damage);
}

/**
 * @brief 9C488 | Debris: slow down, fall, blink and delete after timer.
 */
void StolasDebris_Update(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (entity->state == 0)
    {
        EntityDampVelocityX(entity, 0x18);
        EntityDampVelocityY(entity, 0x18);
        EntityAccelerateAndMove(entity, 0x80000);
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if (--entity->timer == 0)
            EntityDelete(entity);
    }
}

/**
 * @brief 9C4E4 | Load the graphics of Stolas (drawn by Stolas_Draw); 0 when
 * no slot is free.
 */
u8 Stolas_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sStolasGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sStolasPalette, 0);
    Sprite_Attach(entity, sStolasSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Stolas_Draw;
    return 1;
}

/**
 * @brief Bestiary preview state (state 3): nothing.
 */
void Stolas_UpdatePreview(struct EwramData_EntityData *entity)
{
}

/**
 * @brief 9C534 | Player_ApplyContactDamage with (attack, 0, 0, 1) at the entity position.
 */
void Stolas_DamagePlayerOnContact(struct EwramData_EntityData *entity)
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
 * @brief 9C564 | Hit on the body: damage from hit, damage number, then
 * Stolas_OnDamaged.
 */
void Stolas_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s16 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    Entity_ShowDamage(entity, damage, 0);
    Stolas_OnDamaged(entity, attacker, hit, (s16)damage);
}

/**
 * @brief 9C5AC | Draw at the whole-pixel position offset by (unk_4F8,
 * unk_4FA), then restore posX / posY.
 */
void Stolas_Draw(struct EwramData_EntityData *entity)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;

    entity->posX.part16.integer += entity->ext.stolas.drawOffsetX;
    entity->posX.part16.fraction = 0;
    entity->posY.part16.integer += entity->ext.stolas.drawOffsetY;
    entity->posY.part16.fraction = 0;
    Sprite_DrawEntity(entity);
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}
