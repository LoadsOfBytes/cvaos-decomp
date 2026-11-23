/* Balore initialization, including persistent-state and frozen-state paths.
 * Local register bindings preserve agbcc's masks and EWRAM address lifetimes;
 * repeated masks must not be hoisted into an extra saved register.
 * This reconstruction does not establish original source spelling. */
#include "code_08009A0.h"
#include "balore_entity.h"
#include "code_08033CAC.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern s32 Balore_LoadGraphics(struct EwramData_EntityData *entity);
extern void Balore_CreateIntro(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *update);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void BaloreEye_Update(struct EwramData_EntityData *entity);
extern void BalorePart_Update(struct EwramData_EntityData *entity);
extern void BaloreBodyPart_Update(struct EwramData_EntityData *entity);
extern void Balore_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

void EnemyBaloreCreate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part;

    if (entity->spawnParam0 != 0 && !gEwramData->gameState.bossRushMode)
    {
        if (gEwramData->gameState.bossEventFlags & 0x40)
            goto delete;
        gEwramData->gameState.stateFlags |= 2;
        Balore_CreateIntro(entity);
        return;
    }
    if ((u8)Balore_LoadGraphics(entity) == 0)
    {
    delete:
        EntityDelete(entity);
        return;
    }
    {
        struct EwramData **ew = &gEwramData;
        u8 flags = *((u8 *)*ew + 0x8E);
        u32 mask = 0x40;
        if (!(mask &= flags))
            entity->posY.part16.integer -= 0x10;
    }
    Anim_StartIndexed(entity, 0, 2, 1);
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x1E;
    entity->ext.balore.eyeAnim = 4;
    entity->ext.balore.scale = 0x10000;
    part = Entity_CreateObject((u32 *)BaloreEye_Update);
    if (part != NULL)
    {
        part->posX.whole = entity->posX.whole;
        part->posY.whole = entity->posY.whole;
        part->scrollWithBg = 1;
        part->ext.balorePart.body = entity;
    }
    part = Entity_CreateObject((u32 *)BalorePart_Update);
    if (part != NULL)
    {
        part->posX.whole = entity->posX.whole;
        part->posY.whole = entity->posY.whole;
        part->scrollWithBg = 1;
        part->ext.balorePart.body = entity;
    }
    part = Entity_CreateObject((u32 *)BaloreBodyPart_Update);
    if (part != NULL)
    {
        part->posX.whole = entity->posX.whole;
        part->posY.whole = entity->posY.whole;
        part->scrollWithBg = 1;
        part->ext.balorePart.body = entity;
    }
    entity->flipX = 0;
    {
        u8 flags = *((u8 *)gEwramData + 0x8E);
        u32 mask = 0x40;
        if (mask &= flags)
        {
            entity->state = 5;
            return;
        }
    }
    Hit_Enable(entity, 8, NULL, (s32)Balore_OnHit);
    HitBox_SetBodyRaw(entity, 0x1010F010);
    {
        u32 flags = entity->hit.flags;
        u32 mask = 2;
        mask |= flags;
        {
            u32 next = 4;
            mask |= next;
        }
        entity->hit.flags = mask;
    }
    Player_SetSelectedTarget(entity);
    gEwramData->gameState.stateFlags |= 2;
}
