/* Beam Skeleton hit callback and Needles create/idle behavior. */
#include "code_08009A0.h"
#include "needles_entity.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity);
extern void BeamSkeleton_UpdateDeath(struct EwramData_EntityData *entity);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity,u8 *graphics,void *palette,u8 slot,void *sprite);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Enemy_OnContact(struct EwramData_EntityData *entity);
extern void Needles_OnHit(struct EwramData_EntityData *entity);
extern void Hit_Enable(struct EwramData_EntityData *entity,s32 mode,void *callback,s32 flags);
extern void HitBox_SetAttackRaw(struct EwramData_EntityData *entity,s32 flags);
extern s32 HitBox_SetBodyRaw(struct EwramData_EntityData *entity,s32 flags);
extern u8 sNeedlesPalette[];
extern u8 sNeedlesSpriteSheet[];
extern u8 sNeedlesGfx[];

/**
 * @brief Apply enemy damage; on defeat enter state 0xFF, immediately run
 * the death helper and play the death sound.
 */
void BeamSkeleton_OnHit(struct EwramData_EntityData *entity)
{
    if (Enemy_TakeDamage(entity))
    {
        entity->state = 0xFF;
        entity->stage = 0;
        entity->subState = 0;
        BeamSkeleton_UpdateDeath(entity);
        PlaySong(0x6B);
    }
}

void EnemyNeedlesCreate(struct EwramData_EntityData *entity)
{
    void *palette = sNeedlesPalette;
    void *sprite = sNeedlesSpriteSheet;
    u8 *graphics = sNeedlesGfx;
    s32 zero;
    if (!Entity_LoadGraphics(entity,graphics,palette,0,sprite))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity,0,0,1);
    entity->spriteLayer = 17;
    zero = 0;
    Enemy_ResetFromTemplate(entity);
    entity->flipX = 0;
    entity->state = 1;
    entity->stage = zero;
    entity->subState = zero;
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        return;
    }
    Hit_Enable(entity,8,Enemy_OnContact,(s32)Needles_OnHit);
    HitBox_SetAttackRaw(entity,0x1010F8F8);
    HitBox_SetBodyRaw(entity,0x1010F8F8);
    if (entity->unk_514.part16.spawnParam0 != 0)
        NEEDLES_BOB_PHASE(entity) = RandomNumberGenerator() % 0xFFFF;
}

/**
 * @brief Alive state: pause and resume animation with random countdowns.
 * The configured moving variant also bobs vertically and updates X motion.
 */
void Needles_UpdateIdle(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            if (entity->animScriptIndex != 0)
                Anim_StartIndexed(entity,0,0,1);
            entity->animDone = 1;
            entity->subState = 1;
            entity->stage = 0;
        case 1:
            switch (entity->stage)
            {
                case 0:
                    if (entity->animDone)
                    {
                        entity->stage = 1;
                        entity->timer = RandomNumberGenerator() % 128;
                        entity->animDone = 0;
                        entity->animFlags |= 2;
                    }
                    break;
                case 1:
                    if (--entity->timer == 0xFF)
                    {
                        entity->stage = 0;
                        entity->animFlags &= 0xFD;
                    }
                    break;
            }
            break;
        default:
            return;
    }
    if (entity->unk_514.part16.spawnParam0 != 0)
    {
        s32 vy = Sin((u16)NEEDLES_BOB_PHASE(entity) + 0x4000) / 4;
        NEEDLES_BOB_PHASE(entity) += 0x400;
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += vy;
        entity->velX.whole += entity->accelX.whole;
        entity->velY.whole = vy + entity->accelY.whole;
    }
}
