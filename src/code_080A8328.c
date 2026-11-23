/* Big Golem create: allocates the 14-joint skeleton and one part entity per joint. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void *memcpy(void *dst, const void *src, unsigned long size);
extern void *memset(void *dst, int value, unsigned long size);
extern u8 BigGolem_LoadGraphics(struct EwramData_EntityData *entity);
extern void *Heap_Alloc(u16 words);
extern void Heap_Free(void *allocation);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *update);
extern void BigGolemPart_Update(struct EwramData_EntityData *entity);
extern void BigGolemPart_OnContact(struct EwramData_EntityData *entity);
extern void BigGolemPart_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void BigGolem_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void BigGolem_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void BigGolem_AttachPart(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length, s32 duration);
extern void BigGolem_UpdateParts(struct EwramData_EntityData *entity, s8 *order);
extern void BigGolem_ResetPartAngles(struct EwramData_EntityData *entity, s32 partIndex);
extern void BigGolem_FaceTarget(struct EwramData_EntityData *entity);
extern void Player_SetSelectedTarget(struct EwramData_EntityData *entity);
extern s32 Music_SetCurrent(s32 song);
extern u8 sBigGolemGfx[], sBigGolemGfx2[], sBigGolemSpriteSheet[], sBigGolemSpriteSheet2[];
extern const u8 sBigGolemFrames[14], sBigGolemAnchors[14];
extern const s8 sBigGolemOrder[14], sBigGolemChain[14];
extern const s16 *const sBigGolemPartPoses[];
extern const s32 *const sBigGolemPartAngles[];

static inline s32 Angle(s32 angle)
{
    return angle;
}

void EnemyBigGolemCreate(struct EwramData_EntityData *entity)
{
    u8 frames[14];
    u8 anchors[14];
    s8 order[14];
    s8 chain[14];
    s8 altGraphics[14];
    s8 altPalette[14];
    s32 i;
    s32 index;
    struct EwramData_EntityData *part;
    s32 *parts; /* struct BigGolemPart viewed as 14 words */
    const s16 *pose;
    const s32 *angles;
    s32 frozen;
    struct EwramData **ewram = &gEwramData;

    memcpy(frames, sBigGolemFrames, sizeof(frames));
    memcpy(anchors, sBigGolemAnchors, sizeof(anchors));
    memcpy(order, sBigGolemOrder, sizeof(order));
    memcpy(chain, sBigGolemChain, sizeof(chain));
    memset(altGraphics, 0, sizeof(altGraphics));
    altGraphics[2] = 1;
    altGraphics[8] = 1;
    altGraphics[13] = 1;
    memset(altPalette, 0, sizeof(altPalette));
    altPalette[0] = 1;
    altPalette[1] = 1;
    altPalette[2] = 1;

    if (entity->spawnParam0 != 0 && !(*ewram)->gameState.bossRushMode && ((*ewram)->gameState.bossEventFlags & 0x1000))
    {
        EntityDelete(entity);
        return;
    }
    if (!BigGolem_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    entity->ext.bigGolem.parts = Heap_Alloc(0xC4);
    if (entity->ext.bigGolem.parts == NULL)
    {
        EntityDelete(entity);
        return;
    }
    entity->flipX = 0;
    entity->ext.joint.pose = 0;
    Enemy_ResetFromTemplate(entity);
    if (entity->spawnParam0 != 0 && !gEwramData->gameState.bossRushMode)
        entity->attack = entity->attack - (entity->attack >> 2) - (entity->attack >> 3);
    if (gEwramData->gameState.bossRushMode)
        entity->hp += 1000;
    for (i = 0; i <= 13; i++)
    {
        index = order[i];
        part = Entity_CreateObject((u32 *)BigGolemPart_Update);
        if (part != NULL)
        {
            part->spriteLayer = entity->spriteLayer;
            if (altGraphics[index])
                Sprite_Attach(part, sBigGolemSpriteSheet2, ObjGfx_LoadResource(sBigGolemGfx2, 0), entity->basePalette);
            else
                Sprite_Attach(part, sBigGolemSpriteSheet, ObjGfx_LoadResource(sBigGolemGfx, 0), entity->basePalette);
            if (altPalette[index])
                part->basePalette = entity->ext.bigGolem.palette1;
            else
                part->basePalette = entity->ext.bigGolem.palette0;
            part->enemyId = entity->enemyId;
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
            if (index == 2 || index == 4 || index == 5 || index == 8 || index == 12 || index == 13)
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
                Hit_Enable(part, 8, BigGolemPart_OnContact, (s32)BigGolemPart_OnHit);
        }
        else
        {
            Heap_Free(entity->ext.bigGolem.parts);
            EntityDelete(entity);
            return;
        }
    }
    parts = (s32 *)entity->ext.bigGolem.parts;
    pose = sBigGolemPartPoses[(s8)entity->ext.joint.pose];
    angles = sBigGolemPartAngles[(s8)entity->ext.joint.pose];
    for (index = 0; index <= 13; index++)
    {
        parts[index * 14] = pose[index * 2] << 16;
        parts[index * 14 + 1] = pose[index * 2 + 1] << 16;
        if (index == 5)
            parts[5 * 14 + 2] = angles[0];
        else if (index == 12)
            parts[12 * 14 + 2] = angles[1];
    }
    entity->hidden = 1;
    entity->animFrame = 0;
    frozen = (u8)(*((u8 *)gEwramData + 0x8E) & 0x40);
    if (frozen)
    {
        /* same joint set-up as below; the ROM has it twice */
        entity->timer = 1;
        BigGolem_AttachPart(entity, 1, 3, Angle(0x14000), 0x50000, entity->timer);
        BigGolem_AttachPart(entity, 2, 1, Angle(0x16000), 0x110000, entity->timer);
        BigGolem_AttachPart(entity, 0, 2, Angle(0x13000), 0x300000, entity->timer);
        BigGolem_AttachPart(entity, 7, 3, Angle(0x14000), 0x50000, entity->timer);
        BigGolem_AttachPart(entity, 8, 7, Angle(0x16000), 0x110000, entity->timer);
        BigGolem_AttachPart(entity, 6, 8, Angle(0x13000), 0x300000, entity->timer);
        BigGolem_AttachPart(entity, 10, 9, Angle(0x14000), 0x160000, entity->timer);
        BigGolem_AttachPart(entity, 11, 10, Angle(0x14000), 0xE0000, entity->timer);
        BigGolem_AttachPart(entity, 13, 11, Angle(0x14000), 0x80000, entity->timer);
        BigGolem_AttachPart(entity, 12, 13, Angle(0x15000), 0x2C0000, entity->timer);
        BigGolem_UpdateParts(entity, chain);
        BigGolem_ResetPartAngles(entity, 0);
        BigGolem_ResetPartAngles(entity, 1);
        BigGolem_ResetPartAngles(entity, 2);
        BigGolem_ResetPartAngles(entity, 3);
        BigGolem_ResetPartAngles(entity, 4);
        BigGolem_ResetPartAngles(entity, 5);
        BigGolem_ResetPartAngles(entity, 6);
        BigGolem_ResetPartAngles(entity, 7);
        BigGolem_ResetPartAngles(entity, 8);
        BigGolem_ResetPartAngles(entity, 9);
        BigGolem_ResetPartAngles(entity, 10);
        BigGolem_ResetPartAngles(entity, 11);
        BigGolem_ResetPartAngles(entity, 12);
        BigGolem_ResetPartAngles(entity, 13);
        entity->state = 3;
        entity->subState = 0;
        entity->timer = 0;
        return;
    }
    entity->timer = 1;
    BigGolem_AttachPart(entity, 1, 3, Angle(0x14000), 0x50000, entity->timer);
    BigGolem_AttachPart(entity, 2, 1, Angle(0x16000), 0x110000, entity->timer);
    BigGolem_AttachPart(entity, 0, 2, Angle(0x13000), 0x300000, entity->timer);
    BigGolem_AttachPart(entity, 7, 3, Angle(0x14000), 0x50000, entity->timer);
    BigGolem_AttachPart(entity, 8, 7, Angle(0x16000), 0x110000, entity->timer);
    BigGolem_AttachPart(entity, 6, 8, Angle(0x13000), 0x300000, entity->timer);
    BigGolem_AttachPart(entity, 10, 9, Angle(0x14000), 0x160000, entity->timer);
    BigGolem_AttachPart(entity, 11, 10, Angle(0x14000), 0xE0000, entity->timer);
    BigGolem_AttachPart(entity, 13, 11, Angle(0x14000), 0x80000, entity->timer);
    BigGolem_AttachPart(entity, 12, 13, Angle(0x15000), 0x2C0000, entity->timer);
    BigGolem_UpdateParts(entity, chain);
    BigGolem_ResetPartAngles(entity, 0);
    BigGolem_ResetPartAngles(entity, 1);
    BigGolem_ResetPartAngles(entity, 2);
    BigGolem_ResetPartAngles(entity, 3);
    BigGolem_ResetPartAngles(entity, 4);
    BigGolem_ResetPartAngles(entity, 5);
    BigGolem_ResetPartAngles(entity, 6);
    BigGolem_ResetPartAngles(entity, 7);
    BigGolem_ResetPartAngles(entity, 8);
    BigGolem_ResetPartAngles(entity, 9);
    BigGolem_ResetPartAngles(entity, 10);
    BigGolem_ResetPartAngles(entity, 11);
    BigGolem_ResetPartAngles(entity, 12);
    BigGolem_ResetPartAngles(entity, 13);
    Hit_Enable(entity, 8, BigGolem_DamagePlayerOnContact, (s32)BigGolem_OnHit);
    BigGolem_FaceTarget(entity);
    entity->ext.bigGolem.heading = entity->flipX;
    Entity_SnapToGround(entity);
    entity->state = frozen;
    entity->subState = frozen;
    entity->ext.joint.hurtMask = 0x312C;
    if (entity->spawnParam0 != 0)
    {
        Music_SetCurrent(0x10);
        Player_SetSelectedTarget(entity);
        gEwramData->gameState.stateFlags |= 2;
    }
}
