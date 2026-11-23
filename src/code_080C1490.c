/* Altair creation, update, graphics setup and hit response. */
#include "code_08009A0.h"
#include "altair_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, void *);
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Altair_UpdateFlight(struct EwramData_EntityData *);
extern void Altair_UpdateDeath(struct EwramData_EntityData *);
extern void Altair_UpdateSpawner(struct EwramData_EntityData *);
extern s32 Altair_CreateCarriedEnemy(struct EwramData_EntityData *, s32, s32, s32);
extern void Altair_UpdateBestiaryPreview(struct EwramData_EntityData *);
extern void Altair_DamagePlayerOnContact(struct EwramData_EntityData *);
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s16, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern u8 sAltairPalette[], sAltairSpriteSheet[], sAltairGfx[];
u8 Altair_LoadGraphics(struct EwramData_EntityData *);
void Altair_OnHit(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *);

void EnemyAltairCreate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent;
    if (!Altair_LoadGraphics(entity)) return;
    entity->spriteLayer = 3;
    entity->state = 1;
    entity->stage = 0;
    entity->subState = 0;
    Enemy_ResetFromTemplate(entity);
    ALTAIR_CARRIED(entity) = 0;
    ALTAIR_DROP_READY(entity) = 0;
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        entity->stage = 0;
        entity->subState = 0;
        return;
    }
    if (entity->unk_514.part16.spawnParam0 == 0)
    {
        entity->state = 5;
        entity->hidden = 1;
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        return;
    }
    Entity_EnableBoxes(entity, Altair_DamagePlayerOnContact, Altair_OnHit);
    if (entity->unk_514.part16.spawnParam0 == 2)
    {
        s32 x = -(u16)entity->velX.part16.integer;
        x -= 0x10;
        entity->posX.part16.integer = x;
    }
    else if (entity->unk_514.part16.spawnParam0 == 3)
        entity->posX.part16.integer = 0x100 + (u16)entity->velX.part16.integer;
    {
        u32 random = RandomNumberGenerator();
        u16 *position = (u16 *)&entity->posY.part16.integer;
        s32 offset = 0x1F;
        offset &= random;
        *position = offset + 0xFFF0 + *position;
    }
    if ((s16)Entity_GetTargetDistanceX(entity) > 0) entity->flipX = 0;
    else entity->flipX = 1;
    parent = ALTAIR_SPAWNER(entity);
    if (ALTAIR_DROP_COUNT(parent) <= 1 && parent != NULL)
    {
        if (entity->unk_514.part16.spawnParam1 == 1)
        {
            if (Altair_CreateCarriedEnemy(entity, 0x18, 0xFF, 0xFF)) return;
        }
        else if (entity->unk_514.part16.spawnParam1 == 2)
        {
            if (Altair_CreateCarriedEnemy(entity, 0x4A, 0xFF, 0xFF)) return;
        }
        else return;
    }
    entity->deleteRequested = 1;
}

