/* SpinningBlade init: motion table entry, flip and the time of the first step (SpinningBladeCreate).
 * Needs -fno-cse-skip-blocks (Makefile); the volatile read keeps the reload of the step time. */
#include "object_19_create.h"
#include "core.h"
#include "gameplay.h"
#include "structs/ewram.h"
#include "hitbox.h"
#include "oam.h"
#include "objects_15_18.h"
#include "objects_19_1c.h"
#include "data/data_118B48.h"
#include "entity_common.h"
struct SpinningBladeMotion { s32 time, vx, vy; };

extern struct SpinningBladeMotion sSpinningBladeSteps[];

void SpinningBladeCreate(struct EwramData_EntityData *entity)
{
    u16 slot;
    struct SpinningBladeMotion *motion;
    s32 time;
    if (!Entity_LoadGraphics(entity, sSpinningBladeGfx, sSpinningBladePalette, 0, sSpinningBladeSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    slot = ObjAffine_Alloc(1);
    if ((s16)slot < 0)
    {
        EntityDelete(entity);
        return;
    }
    entity->drawFunc = SpinningBlade_Draw;
    entity->affineIndex = slot;
    entity->affine = 1;
    entity->spriteLayer = 10;
    Hit_Enable(entity, 8, SpinningBlade_OnPlayerContact, SpinningBlade_OnHit);
    HitBox_SetAttackRaw(entity, 0x0C0CFAFA);
    HitBox_SetBodyRaw(entity, 0x0C0CFAFA);
    entity->ext.spinningBlade.spin = 0x400;
    entity->flipX = entity->spawnParam1;
    entity->spawnParam1 = entity->flipX;
    if (entity->spawnParam1 != 0) entity->ext.spinningBlade.spin = -entity->ext.spinningBlade.spin;
    entity->spawnParam0 += 14;
    motion = &sSpinningBladeSteps[entity->state];
    entity->velX.whole = motion->vx;
    entity->velY.whole = motion->vy;
    if (entity->spawnParam1 != 0) entity->velX.whole = -entity->velX.whole;
    time = motion->time;
    if (time == 0) time = entity->spawnParam0 << 16;
    entity->ext.spinningBlade.stepTime = time;
    entity->ext.spinningBlade.stepTime = *(volatile s32 *)&entity->ext.spinningBlade.stepTime >> 1; /* FAKEMATCH: volatile read forces the reload of the original */
    entity->scrollWithBg = 1;
}
