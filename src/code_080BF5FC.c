#include "code_08009A0.h"
#include "code/code_08039340.h"
extern void Death_UpdateIntro(struct EwramData_EntityData *);
extern void Death_UpdateFirstPhase(struct EwramData_EntityData *);
extern void Death_UpdateWeaponTransition(struct EwramData_EntityData *);
extern void Death_UpdateHover(struct EwramData_EntityData *);
extern void Death_UpdateDashAttack(struct EwramData_EntityData *);
extern void Death_UpdateThrowAttack(struct EwramData_EntityData *);
extern void Death_UpdateWallAttack(struct EwramData_EntityData *);
extern s32 Death_UpdateDeath(struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);

void EnemyDeathUpdate(struct EwramData_EntityData *entity)
{
    u32 box[4];
    switch (entity->state)
    {
    case 0: Death_UpdateIntro(entity); break;
    case 1: Death_UpdateFirstPhase(entity); break;
    case 2: Death_UpdateWeaponTransition(entity); break;
    case 3: Death_UpdateHover(entity); break;
    case 4: Death_UpdateDashAttack(entity); break;
    case 5: Death_UpdateThrowAttack(entity); break;
    case 6: Death_UpdateWallAttack(entity); break;
    case 7:
        if (Death_UpdateDeath(entity))
        {
            EntityDelete(entity);
            return;
        }
        break;
    case 8: Anim_Update(entity); return;
    }
    if (!(entity->enemyFlags & 2))
    {
        Enemy_UpdateHitFlash(entity);
        Hit_CheckWithPlayer(box, entity);
    }
    Anim_Update(entity);
}


extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, u8, void *);
extern u8 sEvilButcherGfx[], sEvilButcherPalette[], sEvilButcherSpriteSheet[];

/**
 * @brief Display frame 9 using the selected palette, enable drawSkip at
 * countdown 4 and request deletion when the countdown expires.
 */
void EvilButcherDeathParticle_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sEvilButcherGfx, sEvilButcherPalette,
                         (u8)entity->spawnParam1, sEvilButcherSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        entity->animFrame = 9;
        entity->spriteLayer = 0x13;
        entity->state++;
    case 1:
        if (entity->timer != 0)
        {
            if (entity->timer == 4) entity->drawSkip = 1;
            entity->timer--;
        }
        else entity->deleteRequested = 1;
        break;
    }
}
