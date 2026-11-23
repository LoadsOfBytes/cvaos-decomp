/* Altair creation, update, graphics setup and hit response. */
#include "code_08009A0.h"
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

#include "code_080D73B8.h"
#include "structs/entity_template.h"
extern struct EwramData_EntityData *Altair_CreateFeather(struct EwramData_EntityData *);
extern void AltairFeather_SetPhase(struct EwramData_EntityData *, s32);
extern void Enemy_UpdateFragmentDeathWithSpread(struct EwramData_EntityData *, s32, s32, s32);
extern void EnemyAltairCreate(struct EwramData_EntityData *);

void EnemyAltairUpdate(struct EwramData_EntityData *entity)
{
    u32 box[4];
    struct EwramData_EntityData *child = entity->ext.altair.carried;
    struct EwramData_EntityData *parent;
    if (entity->state == 4)
    {
        Altair_UpdateBestiaryPreview(entity);
        return;
    }
    if (entity->state == 5)
    {
        Altair_UpdateSpawner(entity);
        return;
    }
    if (child->updateFunc == NULL) entity->ext.altair.carried = 0;
    if (entity->state != 2) Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity) && entity->state != 2) goto hit;
    if (entity->hitFlashTimer != 0 && entity->state != 3 && entity->state != 2)
    {
hit:
        Hit_CheckWithPlayer(box, entity);
        return;
    }
    switch (entity->state)
    {
    case 1: Altair_UpdateFlight(entity); break;
    case 2: Altair_UpdateDeath(entity); break;
    }
    Entity_AnimateWithBoxes(entity);
    if (entity->state != 2) Hit_CheckWithPlayer(box, entity);
    if (entity->deleteRequested)
    {
        parent = entity->ext.altair.spawner;
        if (parent != NULL && parent->updateFunc != NULL && parent->enemyId == 0x2C && parent->ext.altair.spawnCount != 0)
            parent->ext.altair.spawnCount--;
    }
}
