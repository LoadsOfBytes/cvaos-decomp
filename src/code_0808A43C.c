/* Great Armor create: allocates the 14-joint skeleton and one part entity per joint. */
#include "code_08009A0.h"
#include "code_08013960.h"
#include "code_08033CAC.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

struct GreatArmorBox {
    s8 x;
    s8 y;
    s8 width;
    s8 height;
};

extern void *memcpy(void *dst, const void *src, unsigned long size);
extern u8 GreatArmor_LoadGraphics(struct EwramData_EntityData *entity);
extern void *Heap_Alloc(u16 words);
extern void Heap_Free(void *allocation);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *update);
extern void GreatArmorPart_Update(struct EwramData_EntityData *entity);
extern void GreatArmorPart_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void GreatArmorPart_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void GreatArmor_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void GreatArmor_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
extern void GreatArmor_AttachPartSetAngle(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length);
extern void GreatArmor_UpdateParts(struct EwramData_EntityData *entity, s8 *order);
extern void GreatArmor_ResetPartAngles(struct EwramData_EntityData *entity, s32 partIndex);
extern void GreatArmor_FaceTarget(struct EwramData_EntityData *entity);
extern u8 sGreatArmorGfx[], sGreatArmorSpriteSheet[];
extern const u8 sUnk_08118F78[14], sGreatArmorFrames67[14], sGreatArmorFrames69[14], sGreatArmorAnchors[14];
extern const s8 sGreatArmorChain[14];
extern const u8 *const sGreatArmorPartPoses[];
extern const s32 *const sGreatArmorPartAngles[];

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
        part = Entity_CreateObject((u32 *)GreatArmorPart_Update);
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
