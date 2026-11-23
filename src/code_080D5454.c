/* GrahamSpiralProjectile_Update: matched (index/sin/cos/radius shared at function level,
   SCALE <= 0, angle loaded before adding 0x4000). */
#include "code_08009A0.h"
#include "graham_projectile.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
extern void Effect_SpawnAt(s32, s32, s32);
extern void Effect_SpawnRisingParticle(s32, s32, s32, s32);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
#define SCALE(e) (e->ext.grahamSpiral.scale)
#define RADIUS(e) (e->ext.grahamSpiral.radius)
#define ANGLE(e) (e->ext.grahamSpiral.angle)
#define STATE(e, value) { (e)->state = value; (e)->stage = 0; (e)->subState = 0; }

/**
 * @brief Grow an affine projectile, then move in an expanding spiral
 * relative to the owner with trail effects. Shrink on owner transition
 * or free the slot on removal. Preserve the nonzero initialization
 * test, parent test ordering and shared temporaries needed for matching.
 */
void GrahamSpiralProjectile_Update(struct EwramData_EntityData *entity)
{
    s32 index, sin, cos, radius;
    struct EwramData_EntityData *owner = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    if (entity->state != 0xF0 && entity->state != 4 && owner->state == 5 && owner != NULL)
    {
        entity->hit.flags |= 2;
        STATE(entity, 4);
    }
    {
        s32 state = entity->state;
        switch (state)
        {
        case 0:
            *(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(entity)) &= 0xDF;
            entity->timer = 0x1A;
            entity->state = 2;
            entity->stage = state;
            entity->subState = state;
        case 1:
            if (--entity->timer == 0xFF)
            {
                STATE(entity, 2);
                entity->hidden = 0;
            }
            break;
        case 2:
            /* The original tests for nonzero, so the first ordinary update
             * skips this block; do not reverse this condition. */
            if (entity->subState != 0)
            {

                RADIUS(entity) = 0x200000;
                ANGLE(entity) = 0;
                index = (u8)entity->ext.grahamSpiral.index;
                sin = Sin(index * (0x10000 / entity->ext.grahamSpiral.count));
                cos = Sin(index * (0x10000 / entity->ext.grahamSpiral.count) + ({ s32 a_ = ANGLE(entity); a_ + 0x4000; }));
                radius = GRAHAM_SPIRAL_RADIUS_PIXELS(entity);
                sin *= radius;
                cos *= radius;
                entity->posX.whole = owner->posX.whole + cos;
                entity->posY.whole = owner->posY.whole + ({ s32 t = sin - 0x400000; t; }) + 0x200000;
                entity->subState++;
            }
            if (SCALE(entity) <= 0xFFFF) SCALE(entity) += 0x800;
            ObjAffine_Set(entity->ext.grahamSpiral.rotation, SCALE(entity), SCALE(entity), entity->flipX, 0, entity->affineIndex);
            if (SCALE(entity) == 0x10000)
            {
                STATE(entity, 3);
                PlaySong(0x10D);
                entity->timer = 0;
            }
            break;
        case 3:
            {

                index = (u8)entity->ext.grahamSpiral.index;
                sin = Sin(index * (0x10000 / entity->ext.grahamSpiral.count) + ANGLE(entity));
                cos = Sin(index * (0x10000 / entity->ext.grahamSpiral.count) + ({ s32 a_ = ANGLE(entity); a_ + 0x4000; }));
                radius = RADIUS(entity) >> 16;
                sin *= radius;
                cos *= radius;
                entity->posX.whole = owner->posX.whole + cos;
                entity->posY.whole = sin + owner->posY.whole - 0x200000;
                RADIUS(entity) += 0x8000;
                ANGLE(entity) += GRAHAM_SPIRAL_SPIN(entity);
                if (SCALE(entity) <= 0xFFFF) SCALE(entity) += 0x800;
                entity->ext.grahamSpiral.rotation += entity->ext.grahamSpiral.rotationSpeed;
                ObjAffine_Set(entity->ext.grahamSpiral.rotation, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
                if (EntityIsOutsideScreen(entity, 4)) STATE(entity, 0xF0);
                if (!(entity->timer & 0xF) && entity->timer > 0x20)
                {
                    s32 offset = (10 - RandomNumberGenerator() % 20) << 16;
                    Effect_SpawnAt(entity->posX.whole, entity->posY.whole - offset, entity->spriteLayer - 1);
                    Effect_SpawnRisingParticle(entity->posX.whole, entity->posY.whole - offset, entity->spriteLayer - 1, 0);
                }
                entity->timer++;
            }
            break;
        case 4:
            if (entity->timer++ & 2) entity->palette = 4;
            else entity->palette = entity->basePalette;
            SCALE(entity) -= 0x800;
            if (SCALE(entity) >= 0)
            {
                if (SCALE(entity) <= 0) STATE(entity, 0xF0);
                ObjAffine_Set(entity->ext.grahamSpiral.rotation, SCALE(entity), SCALE(entity), entity->flipX, 0, entity->affineIndex);
            }
            break;
        case 0xF0:
            ObjAffine_Free(entity->affineIndex);
            entity->deleteRequested = 1;
            return;
        }
    }
    Anim_Update(entity);
    if (!entity->hidden && entity->state > 2 && entity->state != 4)
        Enemy_CheckHitPlayer(entity);
}
