/* Nightmare initialisation and state dispatch. */
#include "code_08009A0.h"
#include "nightmare_entity.h"
#include "code_08001194.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern s32 Hit_EnableContact(struct EwramData_EntityData *, s32, void *, s32);
extern void Nightmare_Draw(struct EwramData_EntityData *);
extern void Nightmare_OnHit(struct EwramData_EntityData *);
extern void Entity_HideAboveY(struct EwramData_EntityData *, s32);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern s32 Enemy_IsStunned(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern struct EwramData_EntityData *Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);
extern void Nightmare_UpdateRun(struct EwramData_EntityData *);
extern s32 Nightmare_UpdateDeath(struct EwramData_EntityData *);
extern u8 sNightmareGfx[], sNightmarePalette[], sNightmareSpriteSheet[];

void EnemyNightmareCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sNightmareGfx, sNightmarePalette, 0, sNightmareSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 9;
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->unk_514.part16.spawnParam0 = 1;
        Anim_StartIndexed(entity, 1, 0, 1);
        Anim_Update(entity);
        entity->state = 3;
        return;
    }
    Anim_StartIndexed(entity, (u16)entity->unk_514.part16.spawnParam0, 0, 1);
    Anim_Update(entity);
    entity->drawFunc = (u32 *)Nightmare_Draw;
    if (entity->unk_514.part16.spawnParam0 == 0)
    {
        Hit_Enable(entity, 8, Enemy_OnContact, (s32)Nightmare_OnHit);
        HitBox_SetBodyRaw(entity, 0x1838E0F0);
        HitBox_SetAttackRaw(entity, 0x1838E0F0);
    }
    else
    {
        Hit_EnableContact(entity, 8, Enemy_OnContact, (s32)Nightmare_OnHit);
        HitBox_SetBodyRaw(entity, 0x1838E0F0);
        HitBox_SetAttackRaw(entity, 0x2010C400);
    }
    Entity_SnapToGround(entity);
    entity->posY.part16.integer++;
    entity->scrollWithBg = 1;
    entity->state = 0;
    NIGHTMARE_RUN_TIMER(entity) = 0x28;
}

void EnemyNightmareUpdate(struct EwramData_EntityData *entity)
{
    u32 box[4];
    s32 enabled = 1;
    struct EwramData_EntityData *effect;
    NIGHTMARE_ROOM_X(entity) = GetEntityRoomXPositionInteger(entity);
    NIGHTMARE_ROOM_Y(entity) = GetEntityRoomYPositionInteger(entity);
    Entity_HideAboveY(entity, -0x20);
    if (!(entity->enemyFlags & 2))
    {
        if (!Entity_IsInRect(entity, -0x100, -0x60, 0x2F0, 0x160)) return;
        Player_TrackEntity(entity);
        if (entity->unk_514.part16.spawnParam0 == 0)
        {
            enabled = Entity_IsTimeStopped(entity) == 0;
        }
    }
    if (Enemy_IsStunned(entity))
    {
        if (entity->state == 2) return;
        Hit_CheckWithPlayer(box, entity);
        if (entity->unk_514.part16.spawnParam0 == 0) Player_QueueEntity(entity, 0);
        return;
    }
    if (entity->unk_514.part16.spawnParam0 == 0 && entity->state == 1) Player_QueueEntity(entity, enabled);
    switch (entity->state)
    {
    case 0:
        effect = Afterimage_Create(entity, 0, 0xC, 1);
        if (effect != NULL) NIGHTMARE_EFFECT_INDEX_0(entity) = ENTITY_INDEX(effect);
        effect = Afterimage_Create(entity, 0, 6, 0);
        if (effect != NULL) NIGHTMARE_EFFECT_INDEX_1(entity) = ENTITY_INDEX(effect);
        entity->state = 1;
        entity->stage = 0;
        entity->subState = 0;
        /* fall through */
    case 1:
        Nightmare_UpdateRun(entity);
        break;
    case 2:
        if (Nightmare_UpdateDeath(entity))
        {
            EntityDelete(entity);
            return;
        }
        break;
    case 3:
        Anim_Update(entity);
        return;
    }
    if (entity->state != 2) Hit_CheckWithPlayer(box, entity);
    Anim_Update(entity);
}

