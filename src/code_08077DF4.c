/* Quetzalcoatl: allocate the displacement history and create fifteen following body segments. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern u8 Quezlcoatl_LoadGraphics(struct EwramData_EntityData *);
extern void *Heap_Alloc(u16);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern void QuezlcoatlSegment_Update(struct EwramData_EntityData *);
extern void QuezlcoatlSegment_EnableHitBoxes(struct EwramData_EntityData *);
extern void Quezlcoatl_DamagePlayerOnContact(void);
extern void Quezlcoatl_OnHit(void);
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern u8 sQuezlcoatlWhiteDragonGfx[], sQuezlcoatlWhiteDragonSpriteSheet[];

void EnemyQuezlcoatlCreate(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    u32 box = 0x0C10F8F8;
    s32 index;
    u8 demo;

    if (!Quezlcoatl_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    entity->animFrame = 12;
    entity->unk_504.whole = (s32)Heap_Alloc(60);
    if (!entity->unk_504.whole)
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    if (!gEwramData->gameState.bestiaryPreview)
        Entity_SnapToGround(entity);
    entity->scrollWithBg = 1;
    entity->unk_4FC.part16.unk_4FC = camera->xPos.part16.integer;
    entity->unk_4FC.part16.unk_4FE[0] = camera->yPos.part16.integer;
    index = 14;
    do
    {
        struct EwramData_EntityData *part = Entity_CreateObject((u32 *)QuezlcoatlSegment_Update);
        s32 graphics;

        if (!part)
            goto fail;
        part->spriteLayer = entity->spriteLayer + 1;
        graphics = ObjGfx_Find(sQuezlcoatlWhiteDragonGfx);
        Sprite_Attach(part, sQuezlcoatlWhiteDragonSpriteSheet, graphics, entity->unk_510.part8.basePalette);
        if (!gEwramData->gameState.bestiaryPreview)
            QuezlcoatlSegment_EnableHitBoxes(part);
        part->unk_518.part8.enemyId = entity->unk_518.part8.enemyId;
        part->unk_510.part8.basePalette = entity->unk_510.part8.basePalette;
        part->scrollWithBg = 0;
        part->drawFunc = (u32 *)Sprite_DrawEntity;
        part->unk_4F8.unk_4F8_entity = entity;
        part->unk_500.part8.unk_500 = index;
        part->timer = index;
        part->attack = entity->attack;
        part->unk_4FC.part16.unk_4FC = entity->unk_4FC.part16.unk_4FC;
        part->unk_4FC.part16.unk_4FE[0] = entity->unk_4FC.part16.unk_4FE[0];
        part->posX.whole = entity->posX.whole;
        part->posY.whole = entity->posY.whole - 0x40000;
        if (index <= 5)
            part->animFrame = 3;
        else if (index <= 9)
            part->animFrame = 15;
        else if (index <= 13)
            part->animFrame = 16;
        else if (index <= 14)
            part->animFrame = 17;
        continue;
    fail:
        /* the two failures above are separate copies; jump2 cross-jumps them into this one */
        EntityDelete(entity);
        return;
    } while (--index >= 0);
    demo = *(u8 *)((u8 *)gEwramData + 0x8E) & 0x40;
    if (demo)
    {
        entity->state = 3;
        return;
    }
    Hit_Enable(entity, 8, Quezlcoatl_DamagePlayerOnContact, (s32)Quezlcoatl_OnHit);
    HitBox_SetBody(entity, &box);
    HitBox_SetAttack(entity, &box);
    entity->flipX = Entity_IsTargetRight(entity);
    entity->state = demo;
}
