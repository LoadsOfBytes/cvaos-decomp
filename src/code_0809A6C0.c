/* Stolas create: allocates the 6-joint skeleton and one part entity per joint. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void *memcpy(void *dst, const void *src, unsigned long size);
extern u8 Stolas_LoadGraphics(struct EwramData_EntityData *entity);
extern void *Heap_Alloc(u16 words);
extern void Heap_Free(void *allocation);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *update);
extern void StolasPart_Update(struct EwramData_EntityData *entity);
extern void Stolas_OnContact(struct EwramData_EntityData *entity);
extern void StolasPart_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void Stolas_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void Stolas_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void Stolas_AttachJoint(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length);
extern void Stolas_AttachJointSetAngle(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length);
extern void Stolas_InterpolateJointsInOrder(struct EwramData_EntityData *entity, s8 *order);
extern void Stolas_FaceTarget(struct EwramData_EntityData *entity);
extern u8 sStolasGfx[], sStolasSpriteSheet[];
extern const u8 sStolasFrames0[6], sStolasAnchors[6];
extern const s8 sUnk_081190AC[6];
extern const u8 *const sStolasPartPoses[];
extern const s32 *const sStolasPartAngles[];

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
        part = Entity_CreateObject((u32 *)StolasPart_Update);
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
