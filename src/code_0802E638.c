/* SkillBonePillarProjectileUpdate (blue soul projectile). */
#include "code_08009A0.h"
#include "blue_soul_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern u8 sSkillBonePillarProjectileSpriteSheet[];

/* Same as BlueSoul_DeleteEntity: end a blue soul entity. */
static inline void BlueSoulProjectile_Delete(struct EwramData_EntityData *entity)
{
    if (entity->affineIndex != 0 || entity->affine)
        ObjAffine_Free(entity->affineIndex);
    gEwramData->playerState.activeBlueSoulEntities[BLUE_SOUL_SLOT(entity)] = NULL;
    EntityDelete(entity);
}

/**
 * @brief Child of SkillBonePillarUse: blue-soul projectile with the soul
 * graphics, box 0x1414F6F6 (slot 6) and a 26-update timer; flies with
 * the layer following the timer and ends when the timer runs out.
 */
void SkillBonePillarProjectileUpdate(struct EwramData_EntityData *entity)
{
    entity->roomActive = 1;
    switch (entity->state)
    {
        case 0:
            Sprite_Attach(entity, sSkillBonePillarProjectileSpriteSheet, gEwramData->playerState.blueSoulGfx, 3);
            Anim_StartIndexed(entity, 0, 0, 0);
            entity->spriteLayer = 0xD;
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
            Hit_Enable(entity, 6, NULL, 0);
            HitBox_SetAttackRaw(entity, 0x1414F6F6);
            entity->hit.flags |= 4;
            entity->timer = 0x1A;
            entity->scrollWithBg = 1;
            entity->state++;
        case 1:
            entity->spriteLayer = (entity->timer >> 2) + 7;
            EntityAccelerateAndMove(entity, 0x80000);
            if (--entity->timer == 0)
            {
                BlueSoulProjectile_Delete(entity);
                return;
            }
            break;
    }
    Anim_Update(entity);
}
