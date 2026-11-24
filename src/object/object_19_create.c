/* Object19 init: motion table entry, flip and the time of the first step (Object19Create).
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
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
struct Object19Motion { s32 time, vx, vy; };

extern struct Object19Motion sObject19Steps[];

void Object19Create(struct EwramData_EntityData *entity)
{
    u16 slot;
    struct Object19Motion *motion;
    s32 time;
    if (!Entity_LoadGraphics(entity, sObject19Gfx, sObject19Palette, 0, sObject19SpriteSheet)) goto fail;
    slot = ObjAffine_Alloc(1);
    if ((s16)slot < 0)
    {
fail:
        EntityDelete(entity);
        return;
    }
    entity->drawFunc = Object19_Draw;
    entity->affineIndex = slot;
    entity->affine = 1;
    entity->spriteLayer = 10;
    Hit_Enable(entity, 8, Object19_OnPlayerContact, Object19_OnHit);
    HitBox_SetAttackRaw(entity, 0x0C0CFAFA);
    HitBox_SetBodyRaw(entity, 0x0C0CFAFA);
    entity->ext.object19.spin = 0x400;
    entity->flipX = entity->spawnParam1;
    entity->spawnParam1 = entity->flipX;
    if (entity->spawnParam1 != 0) entity->ext.object19.spin = -entity->ext.object19.spin;
    entity->spawnParam0 += 14;
    motion = &sObject19Steps[entity->state];
    entity->velX.whole = motion->vx;
    entity->velY.whole = motion->vy;
    if (entity->spawnParam1 != 0) entity->velX.whole = -entity->velX.whole;
    time = motion->time;
    if (time == 0) time = entity->spawnParam0 << 16;
    entity->ext.object19.stepTime = time;
    entity->ext.object19.stepTime = *(volatile s32 *)&entity->ext.object19.stepTime >> 1; /* FAKEMATCH: volatile read forces the reload of the original */
    entity->scrollWithBg = 1;
}
