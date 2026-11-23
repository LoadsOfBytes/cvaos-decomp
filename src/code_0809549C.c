/* Creaking Skull create: allocates the 13-joint skeleton and one part entity per joint. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void *memcpy(void *dst, const void *src, unsigned long size);
extern u8 CreakingSkull_LoadGraphics(struct EwramData_EntityData *entity);
extern void *Heap_Alloc(u16 words);
extern void Heap_Free(void *allocation);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *update);
extern void CreakingSkullPart_Update(struct EwramData_EntityData *entity);
extern void CreakingSkull_OnContact(struct EwramData_EntityData *entity);
extern void CreakingSkullPart_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void CreakingSkull_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void CreakingSkull_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void CreakingSkull_AttachJointSetAngle(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length);
extern void CreakingSkull_InterpolateJointsInOrder(struct EwramData_EntityData *entity, s8 *order);
extern void CreakingSkull_FaceTarget(struct EwramData_EntityData *entity);
extern void Player_SetSelectedTarget(struct EwramData_EntityData *entity);
extern s32 Music_SetCurrent(s32 song);
extern u8 sCreakingSkullGfx[], sCreakingSkullSpriteSheet[];
extern const u8 sCreakingSkullFrames0[13], sCreakingSkullAnchors[13];
extern const s8 sCreakingSkullOrder[13], sUnk_08119063[13];
extern const u8 *const sCreakingSkullPartPoses[];
extern const s32 *const sCreakingSkullPartAngles[];

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

    if (entity->unk_514.part16.spawnParam0 != 0 && !(*ewram)->gameState.bossRushMode && ((*ewram)->gameState.bossEventFlags & 0x800))
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
        entity->unk_518.part16.hp += 1000;
        entity->attack += 20;
    }
    frames = frames0;
    parts = entity->ext.joint.parts;
    for (i = 0; i <= 12; i++)
    {
        index = order[i];
        part = Entity_CreateObject((u32 *)CreakingSkullPart_Update);
        if (part != NULL)
        {
            part->spriteLayer = entity->spriteLayer;
            Sprite_Attach(part, sCreakingSkullSpriteSheet, ObjGfx_LoadResource(sCreakingSkullGfx, 0), entity->unk_510.part8.basePalette);
            part->unk_518.part8.enemyId = entity->unk_518.part8.enemyId;
            part->unk_510.part8.basePalette = entity->unk_510.part8.basePalette;
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
                Hit_Enable(part, 8, CreakingSkull_OnContact, (s32)CreakingSkullPart_OnHit);
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
        entity->state = 3;
        entity->subState = 0x14;
        entity->timer = 0;
        return;
    }
    Hit_Enable(entity, 8, CreakingSkull_DamagePlayerOnContact, (s32)CreakingSkull_OnHit);
    CreakingSkull_FaceTarget(entity);
    Entity_SnapToGround(entity);
    entity->state = frozen;
    entity->subState = 0x14;
    entity->ext.joint.hurtMask = 0x106;
    switch (entity->unk_518.part8.enemyId)
    {
    case 0x21:
        entity->ext.creakingSkull.variant = 0;
        break;
    case 0x5C:
        entity->ext.creakingSkull.variant = 1;
        break;
    }
    if (entity->unk_514.part16.spawnParam0 != 0)
    {
        Player_SetSelectedTarget(entity);
        gEwramData->gameState.stateFlags |= 2;
        Music_SetCurrent(0x11);
    }
}
