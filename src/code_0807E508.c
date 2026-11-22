/* Golem create: allocates the 11-joint skeleton and one part entity per joint. */
#include "code_08009A0.h"
#include "golem_entity.h"
#include "joint_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void *memcpy(void *dst, const void *src, unsigned long size);
extern u8 Golem_LoadGraphics(struct EwramData_EntityData *entity);
extern void *Heap_Alloc(u16 words);
extern void Heap_Free(void *allocation);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *update);
extern void GolemSegment_Update(struct EwramData_EntityData *entity);
extern void GolemSegment_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void GolemSegment_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void Golem_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void Golem_OnDamaged(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void Golem_FaceTarget(struct EwramData_EntityData *entity);
extern u8 sGolemGfxEnemy50[], sGolemGfx[], sGolemSpriteSheet[];
extern const u8 sGolemFrames[11], sGolemFrames65[11], sGolemFrames22[11], sGolemFrames22Rare[11];
extern const u8 sGolemFrames50[11], sGolemAnchors[11];
extern const s8 sGolemOrder[11], sGolemOrder50[11];
extern const u8 *const sGolemPartPoses[];
extern const s32 *const sGolemPartAngles[];

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
    entity->unk_500.golemParts = Heap_Alloc(0x58);
    if (entity->unk_500.golemParts == NULL)
    {
        EntityDelete(entity);
        return;
    }
    entity->hidden = 1;
    entity->flipX = 0;
    if (entity->unk_518.part8.enemyId == 0x50)
        JOINT_POSE(entity) = 0;
    else
        JOINT_POSE(entity) = 7;
    Enemy_ResetFromTemplate(entity);
    for (i = 0; i <= 10; i++)
    {
        s32 graphics;

        if (entity->unk_518.part8.enemyId == 0x50)
            index = order50[i];
        else
            index = order[i];
        part = Entity_CreateObject((u32 *)GolemSegment_Update);
        if (part != NULL)
        {
        part->spriteLayer = entity->spriteLayer;
        if (entity->unk_518.part8.enemyId == 0x50)
            graphics = ObjGfx_LoadResource(sGolemGfxEnemy50, 0);
        else
            graphics = ObjGfx_LoadResource(sGolemGfx, 0);
        Sprite_Attach(part, sGolemSpriteSheet, graphics, entity->unk_510.part8.basePalette);
        part->unk_518.part8.enemyId = entity->unk_518.part8.enemyId;
        part->unk_510.part8.basePalette = entity->unk_510.part8.basePalette;
        part->scrollWithBg = 0;
        part->hidden = 0;
        part->attack = entity->attack;
        part->drawFunc = (u32 *)Sprite_DrawEntity;
        JOINT_PART_OWNER(part) = entity;
        JOINT_PART_INDEX(part) = index;
        JOINT_PART_ANCHOR(part) = anchors[index];
        part->posX.whole = entity->posX.whole;
        part->posY.whole = entity->posY.whole;
        if (entity->unk_518.part8.enemyId == 0x50)
            part->animFrame = frames50[index];
        else if (entity->unk_518.part8.enemyId == 0x65)
            part->animFrame = frames65[index];
        else if (entity->unk_518.part8.enemyId == 0x22)
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
            Hit_Enable(part, 8, GolemSegment_DamagePlayerOnContact, (s32)GolemSegment_OnHit);
        }
        else
        {
            Heap_Free(entity->unk_500.golemParts);
            EntityDelete(entity);
            return;
        }
    }
    parts = (s32 *)entity->unk_500.golemParts;
    pose = sGolemPartPoses[(s8)JOINT_POSE(entity)];
    angles = sGolemPartAngles[(s8)JOINT_POSE(entity)];
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
        if (entity->unk_518.part8.enemyId == 0x50)
            entity->subState = 0x28;
        else
            entity->subState = 0x14;
        entity->timer = 0;
        return;
    }
    Hit_Enable(entity, 8, Golem_DamagePlayerOnContact, (s32)Golem_OnDamaged);
    Golem_FaceTarget(entity);
    Entity_SnapToGround(entity);
    entity->state = frozen;
    if (entity->unk_518.part8.enemyId == 0x50)
        entity->subState = 0x28;
    else
        entity->subState = 0x14;
    switch (entity->unk_518.part8.enemyId)
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
    GOLEM_VARIANT(entity) = kindIndex;
}
