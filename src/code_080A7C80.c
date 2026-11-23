#include "code_08009A0.h"
#include "code/code_08039340.h"
extern u8 SkyFish_LoadGraphics(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *);
extern void Hit_EnableContact(struct EwramData_EntityData *, s32, void *, void *);
extern void Hit_Enable(struct EwramData_EntityData *, s32, void *, void *);
extern void SkyFish_DamagePlayerOnContact(void);
extern void SkyFish_OnHit(void);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);

extern struct AnimScript sSkyFishFlyAnimScript;

void EnemySkyFishCreate(struct EwramData_EntityData *entity)
{
    s32 one;
    if (!SkyFish_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_Start(entity, &sSkyFishFlyAnimScript, 0, 0);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 2;
        Anim_StartIndexed(entity, 0, 0, 1);
        Anim_Update(entity);
        return;
    }
    entity->spriteLayer = 3;
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, SkyFish_DamagePlayerOnContact, SkyFish_OnHit);
    else
        Hit_Enable(entity, 8, SkyFish_DamagePlayerOnContact, SkyFish_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->state = 0;
    one = 1;
    entity->ext.skyFish.darting = one;
    {
        u32 random = RandomNumberGenerator();
        s32 mask = 0x3F;
        mask &= random;
        entity->ext.skyFish.delay = mask + 0x40;
    }
    entity->hidden = 1;
    entity->ext.skyFish.fromRight = RandomNumberGenerator() & one;
}
