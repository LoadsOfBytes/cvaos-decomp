/* Chronomage affine casting effect. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"

/*
 * Chronomage cast effect (ChronomageCastEffect_Update): affine scale (grows to
 * 1.0, then shrinks). Token-identical expansion.
 */
#define CHRONOMAGE_CAST_SCALE(e) e->unk_4FC.whole
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Chronomage_UpdateWait(struct EwramData_EntityData *);
extern void Chronomage_UpdateCast(struct EwramData_EntityData *);
extern void Chronomage_UpdateEscape(struct EwramData_EntityData *);
extern void Chronomage_UpdateBestiaryPreview(struct EwramData_EntityData *);
extern void Chronomage_OnHit(struct EwramData_EntityData *);
extern u8 sChronomageGfx[], sChronomagePalette[], sChronomageSpriteSheet[];

#include "code_0800B700.h"
#include "code_080109F4.h"
#include "code_08011DD0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
extern u8 sChronomageDestinationRoom[];

/**
 * @brief Allocate an affine sprite, grow its scale, hold for 32 updates, then
 * shrink and release it. Allocation failure removes the effect.
 */
void ChronomageCastEffect_Update(struct EwramData_EntityData *entity)
{
    u16 affine;
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sChronomageGfx, sChronomagePalette, 0, sChronomageSpriteSheet)) goto remove;
        affine = ObjAffine_Alloc(1);
        if ((s16)affine < 0) goto remove;
        entity->affineIndex = affine;
        entity->affine = 1;
        entity->spriteLayer = 0x12;
        Anim_StartIndexed(entity, 6, 0, 0);
        Anim_Update(entity);
        entity->state = 1;
        /* fall through */
    case 1:
        if ((s32)CHRONOMAGE_CAST_SCALE(entity) <= 0xFFFF) CHRONOMAGE_CAST_SCALE(entity) += 0x1000;
        else
        {
            entity->state = 2;
            entity->timer = 0x20;
        }
        break;
    case 2:
        if (entity->timer != 0) entity->timer--;
        else entity->state = 3;
        break;
    case 3:
        if ((s32)CHRONOMAGE_CAST_SCALE(entity) > 0)
        {
            CHRONOMAGE_CAST_SCALE(entity) -= 0x2000;
            break;
        }
        /* fall through */
    case 4:
        if (entity->affine) ObjAffine_Free(entity->affineIndex);
remove:
        EntityDelete(entity);
        return;
    }
    ObjAffine_Set(0, CHRONOMAGE_CAST_SCALE(entity), CHRONOMAGE_CAST_SCALE(entity), 0, 0, entity->affineIndex);
}

