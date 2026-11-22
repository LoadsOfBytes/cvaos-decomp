/* Devil create: allocates the 8-joint skeleton and one part entity per joint. */
#include "code_08009A0.h"
#include "joint_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void *memcpy(void *dst, const void *src, unsigned long size);
extern u8 Devil_LoadGraphics(struct EwramData_EntityData *entity);
extern void *Heap_Alloc(u16 words);
extern void Heap_Free(void *allocation);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *update);
extern s32 Hit_EnableContact(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);
extern void DevilPart_Update(struct EwramData_EntityData *entity);
extern void DevilPart_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void DevilPart_OnHit(struct EwramData_EntityData *input, s32 source, u16 *collision);
extern void Devil_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void Devil_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void DevilPart_Attach(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length);
extern void Devil_UpdateAllParts(struct EwramData_EntityData *entity);
extern void Devil_ResetPartAngles(struct EwramData_EntityData *entity, s32 partIndex);
extern u8 sDevilGfx[], sDevilSpriteSheet[];
extern const u8 sDevilFrames30[8], sDevilFrames48[8], sDevilAnchors[8];
extern const s8 sDevilOrder[8], sUnk_0811902C[8];
extern const s8 *const sDevilPartPoses[];
extern const s32 *const sDevilPartAngles[];

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
    entity->unk_500.devilParts = Heap_Alloc(0x58);
    if (entity->unk_500.devilParts == NULL)
    {
        EntityDelete(entity);
        return;
    }
    entity->flipX = 0;
    stopped = *((u8 *)*ewram + 0x8E) & 0x40;
    if (stopped)
        JOINT_POSE(entity) = 2;
    else
        JOINT_POSE(entity) = stopped;
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 4;
    if (entity->unk_518.part8.enemyId == 0x30)
        frames = frames30;
    else if (entity->unk_518.part8.enemyId == 0x68)
        frames = frames68;
    else if (entity->unk_518.part8.enemyId == 0x48)
        frames = frames48;
    else if (entity->unk_518.part8.enemyId == 0x66)
        frames = frames66;
    for (i = 0; i <= 7; i++)
    {
        index = order[i];
        part = Entity_CreateObject((u32 *)DevilPart_Update);
        if (part != NULL)
        {
            if ((u32)index <= 1 || index == 4 || index == 5)
                part->spriteLayer = entity->spriteLayer + 1;
            else
                part->spriteLayer = entity->spriteLayer - 1;
            Sprite_Attach(part, sDevilSpriteSheet, ObjGfx_LoadResource(sDevilGfx, 0), entity->unk_510.part8.basePalette);
            part->unk_518.part8.enemyId = entity->unk_518.part8.enemyId;
            part->unk_510.part8.basePalette = entity->unk_510.part8.basePalette;
            part->scrollWithBg = 0;
            part->hidden = 0;
            part->attack = entity->attack;
            part->drawFunc = (u32 *)Sprite_DrawEntity;
            part->unk_500.unk_500_entity = entity;
            JOINT_PART_INDEX(part) = index;
            JOINT_PART_ANCHOR(part) = anchors[index];
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
                Hit_EnableContact(part, 8, DevilPart_DamagePlayerOnContact, (s32)DevilPart_OnHit);
        }
        else
        {
            Heap_Free(entity->unk_500.devilParts);
            EntityDelete(entity);
            return;
        }
    }
    pose = sDevilPartPoses[(s8)JOINT_POSE(entity)];
    angles = sDevilPartAngles[(s8)JOINT_POSE(entity)];
    parts = entity->unk_500.devilParts;
    for (index = 0; index <= 7; index++)
    {
        parts[index].offsetX = pose[index * 2] << 16;
        parts[index].offsetY = pose[index * 2 + 1] << 16;
        parts[index].rotationAngle = angles[index];
    }
    if (entity->unk_518.part8.enemyId == 0x30)
        Anim_StartIndexed(entity, 10, 0, 1);
    else if (entity->unk_518.part8.enemyId == 0x68)
        Anim_StartIndexed(entity, 10, 0, 1);
    else if (entity->unk_518.part8.enemyId == 0x48)
        Anim_StartIndexed(entity, 2, 0, 1);
    else if (entity->unk_518.part8.enemyId == 0x66)
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
    Hit_Enable(entity, 8, Devil_DamagePlayerOnContact, (s32)Devil_OnHit);
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
    JOINT_HURT_MASK(entity) = 0x88;
    entity->unk_508.part8.unk_50A = 1;
}
