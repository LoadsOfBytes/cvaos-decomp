/* Rotating one-shot effect. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern u8 sDeathGfx[], sDeathPalette[], sDeathSpriteSheet[];

/**
 * @brief BCECC | Affine variant of DeathBlinkEffect_Update's effect (same graphics,
 * palette 0): allocates a matrix (affineIndex, affine) or is deleted,
 * priority 0x14, animation 0xC and a 10-frame counter; at 8 objMode = 1,
 * at 4 drawSkip = 1, at 0 (or state 2) the matrix is freed and the entity
 * deleted.  While alive the matrix rotates by (copied from the weapon; scale
 * 1.0, mirrored with flipX)->ext.deathWeapon.angle.
 */
void DeathWeaponTrail_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        if (Entity_LoadGraphics(entity, sDeathGfx, sDeathPalette, 0, sDeathSpriteSheet))
        {
            u16 matrix = ObjAffine_Alloc(1);

            if ((s16)matrix >= 0)
            {
                entity->affineIndex = matrix;
                entity->affine = 1;
                entity->spriteLayer = 0x14;
                Anim_StartIndexed(entity, 0xC, 2, 0);
                Anim_Update(entity);
                entity->timer = 10;
                entity->state = 1;
                goto active;
            }
        }
        goto delete;
    case 1:
    active:
        if (entity->timer == 8)
            entity->objMode = 1;
        if (entity->timer == 4)
            entity->drawSkip = 1;
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
    case 2:
        ObjAffine_Free(entity->affineIndex);
    delete:
        EntityDelete(entity);
        return;
    }
    ObjAffine_Set(entity->ext.deathWeapon.angle, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
}
