/* Slime setup, dispatch and shrinking death animation. */
#include "code_08009A0.h"
#include "code_08001194.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern void Slime_UpdateCrawl(struct EwramData_EntityData *);
extern void Slime_UpdateDetach(struct EwramData_EntityData *);
extern void Slime_SpawnParticles(struct EwramData_EntityData *, s32, s32, s32);
extern void Slime_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void Slime_OnHit(struct EwramData_EntityData *);
extern u8 sSlimeGfx[], sSlimePalette[], sSlimeSpriteSheet[], sSlimeWalkProfile[];

/**
 * @brief Death state (2): land if detached, shrink while moving along the
 * surface normal and emit particles. Return nonzero when the scale
 * is exhausted; the dispatcher owns affine-slot cleanup.
 */
s32 Slime_UpdateDeath(struct EwramData_EntityData *entity)
{
    s32 angle = entity->unk_504.part16.unk_504 + 0x4000;
    s32 timer;
    s32 dx;
    s32 dy;
    s32 scale;
    switch (entity->subState)
    {
    case 0:
        PlaySong(0x70);
        Anim_StartIndexed(entity, 0, 0, 1);
        if (entity->unk_4F4.part8.unk_4F5 & 4)
        {
            entity->accelY.whole = 0x2000;
            entity->unk_504.part16.unk_504 = 0;
        }
        entity->subState = 1;
        /* fall through */
    case 1:
        if (entity->unk_4F4.part8.unk_4F5 & 4)
        {
            if (Entity_Walk(entity, entity->unk_4F8.whole, sSlimeWalkProfile, 0x22) & 8)
            {
                entity->unk_4F4.part8.unk_4F5 &= ~4;
                entity->unk_4F8.whole = 0;
                {
                    s32 correction = (0x10000 - entity->unk_500.whole) >> 8;
                    s32 factor = 0x500;
                    factor *= correction;
                    entity->posY.whole += factor;
                }
            }
        }
        if (entity->unk_500.whole > 0)
        {
            {
                s32 value = Sin(angle + 0x4000) >> 8;
                scale = 0x12;
                dx = value;
                dx *= scale;
                value = Sin(angle) >> 8;
                dy = value;
                dy *= scale;
                {
                    s32 x = entity->posX.whole;
                    x += dx;
                    entity->posX.whole = x;
                }
                {
                    s32 y = entity->posY.whole;
                    y += dy;
                    entity->posY.whole = y;
                }
            }
            timer = entity->timer;
            timer++;
            entity->timer = timer;
            if (!(timer & 3)) Slime_SpawnParticles(entity, 1, 0, -0x20000);
            if (!(entity->timer & 0xF)) PlaySong(0x165);
            entity->unk_500.whole -= 0x400;
        }
        else return 1;
        break;
    }
    return 0;
}

void EnemySlimeCreate(struct EwramData_EntityData *entity)
{
    u16 affine;
    s32 x;
    s32 height;
    if (!Entity_LoadGraphics(entity, sSlimeGfx, sSlimePalette, (u8)entity->unk_514.part16.spawnParam1, sSlimeSpriteSheet)) goto remove;
    affine = ObjAffine_Alloc(1);
    if ((s16)affine < 0)
    {
remove:
        EntityDelete(entity);
        return;
    }
    entity->affineIndex = affine;
    entity->affine = 1;
    entity->unk_504.part16.unk_504 = 0;
    entity->unk_4FC.whole = 0x10000;
    entity->unk_500.whole = 0x10000;
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x11;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        ObjAffine_Set(entity->unk_504.part16.unk_504, entity->unk_4FC.whole, entity->unk_500.whole, entity->flipX, 0, entity->affineIndex);
        entity->state = 3;
        return;
    }
    Hit_Enable(entity, 8, Slime_DamagePlayerOnContact, (s32)Slime_OnHit);
    HitBox_SetBodyRaw(entity, 0x1010F8F8);
    HitBox_SetAttackRaw(entity, 0x1010F8F8);
    if (entity->unk_514.part16.spawnParam0 == 0) Entity_SnapToGround(entity);
    else
    {
        for (;;)
        {
            x = (s16)GetEntityRoomXPositionInteger(entity);
            height = (s16)Collision_GetPushDown(x, (s16)GetEntityRoomYPositionInteger(entity));
            if (height == 0)
            {
                entity->posY.part16.integer = (u16)entity->posY.part16.integer - 8;
            }
            else break;
        }
        entity->posY.part16.integer = height + ((u16)entity->posY.part16.integer + 5);
        entity->stage = 2;
        entity->unk_504.part16.unk_504 = 0x8000;
    }
    ObjAffine_Set(entity->unk_504.part16.unk_504, entity->unk_4FC.whole, entity->unk_500.whole, entity->flipX, 0, entity->affineIndex);
    entity->scrollWithBg = 1;
    entity->state = 0;
}

void EnemySlimeUpdate(struct EwramData_EntityData *entity)
{
    if (Enemy_PreUpdate(entity)) return;
    switch (entity->state)
    {
    case 0: Slime_UpdateCrawl(entity); break;
    case 1: Slime_UpdateDetach(entity); break;
    case 2:
        if (Slime_UpdateDeath(entity))
        {
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
        }
        break;
    case 3:
        Anim_Update(entity);
        return;
    }
    ObjAffine_Set(entity->unk_504.part16.unk_504, entity->unk_4FC.whole, entity->unk_500.whole, entity->flipX, 0, entity->affineIndex);
    Enemy_CheckHitPlayer(entity);
    Anim_Update(entity);
}
