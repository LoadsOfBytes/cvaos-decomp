/* Object19 init: motion table entry, flip and launch angle (Object19Create).
 * Needs -fno-cse-skip-blocks (Makefile); the volatile read keeps the reload of unk_4FC. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Object19_Draw(struct EwramData_EntityData *);
extern void Object19_OnPlayerContact(struct EwramData_EntityData *);
extern void Object19_OnHit(struct EwramData_EntityData *);
extern u8 sObject19Gfx[], sObject19Palette[], sObject19SpriteSheet[];
struct Object19Motion { s32 angle, vx, vy; };
extern struct Object19Motion sObject19Steps[];
void Object19Create(struct EwramData_EntityData *entity)
{
    u16 slot;
    struct Object19Motion *motion;
    s32 angle;
    if (!Entity_LoadGraphics(entity, sObject19Gfx, sObject19Palette, 0, sObject19SpriteSheet)) goto fail;
    slot = ObjAffine_Alloc(1);
    if ((s16)slot < 0)
    {
fail:
        EntityDelete(entity);
        return;
    }
    entity->drawFunc = (u32 *)Object19_Draw;
    entity->affineIndex = slot;
    entity->affine = 1;
    entity->spriteLayer = 10;
    Hit_Enable(entity, 8, Object19_OnPlayerContact, (s32)Object19_OnHit);
    HitBox_SetAttackRaw(entity, 0x0C0CFAFA);
    HitBox_SetBodyRaw(entity, 0x0C0CFAFA);
    entity->unk_4F8.part16.unk_4FA = 0x400;
    entity->flipX = entity->unk_514.part16.spawnParam1;
    entity->unk_514.part16.spawnParam1 = entity->flipX;
    if (entity->unk_514.part16.spawnParam1 != 0) entity->unk_4F8.part16.unk_4FA = -entity->unk_4F8.part16.unk_4FA;
    entity->unk_514.part16.spawnParam0 += 14;
    motion = &sObject19Steps[entity->state];
    entity->velX.whole = motion->vx;
    entity->velY.whole = motion->vy;
    if (entity->unk_514.part16.spawnParam1 != 0) entity->velX.whole = -entity->velX.whole;
    angle = motion->angle;
    if (angle == 0) angle = entity->unk_514.part16.spawnParam0 << 16;
    entity->unk_4FC.whole = angle;
    entity->unk_4FC.whole = *(volatile s32 *)&entity->unk_4FC.whole >> 1;
    entity->scrollWithBg = 1;
}
