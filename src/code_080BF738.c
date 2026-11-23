#include "code_08009A0.h"
extern s32 Collision_GetGroundOffset(s32 x, s32 y);
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
#include "entity_hit.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void Effect_SpawnRandom(s32 x, s32 y, s32 width, s32 height, s32 layer);
extern s32 RandomSigned(s32);
extern void Enemy_OnContactMask1(struct EwramData_EntityData *);
extern void EvilButcherProjectile_OnHit(struct EwramData_EntityData *);

/* Thrown projectile: flies sideways until it is destroyed (burst) or hits a
 * wall, then bounces off spinning and falls out of the screen. */
void EvilButcherProjectile_Update(struct EwramData_EntityData *entity)
{
    struct HitResult res;
    u16 slot;

    if (entity->state != 0 && (s16)entity->unk_518.part16.hp != 0 && Entity_IsTimeStopped(entity))
    {
        Hit_CheckWithPlayer(&res, (u8 *)entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, (void *)0x082052D0, (void *)0x0820C27C, (u8)entity->unk_514.part16.spawnParam1, (void *)0x08256720))
            goto delete;
        entity->animFrame = 8;
        entity->spriteLayer = 0x13;
        entity->scrollWithBg = 1;
    {
        s32 vx = -0x28000;
        s16 spin;
        entity->velX.whole = vx;
        entity->ext.evilButcherProjectile.probeX = -6;
        spin = 0x1234;
        entity->ext.evilButcherProjectile.spin = spin;
        if (entity->flipX)
        {
            entity->velX.whole = -vx;
            entity->ext.evilButcherProjectile.probeX = 6;
            entity->ext.evilButcherProjectile.spin = -spin;
        }
    }
        PlaySong(0x89);
        entity->state = 1;
        entity->unk_518.part16.hp = 1;
        Hit_Enable(entity, 10, Enemy_OnContactMask1, (s32)EvilButcherProjectile_OnHit);
        HitBox_SetAttackRaw(entity, 0x0810FCF8);
        HitBox_SetBodyRaw(entity, 0x0810FCF8);
        break;
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        Hit_CheckWithPlayer(&res, (u8 *)entity);
        if (EntityIsOutsideScreen(entity, 3))
            entity->state = 3;
        if ((s16)entity->unk_518.part16.hp == 0)
        {
            entity->state = 3;
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0x40000, 0x40000, entity->spriteLayer);
            break;
        }
    {
        struct BgLayerState *camera;
        if (!Collision_GetGroundOffset((s16)entity->posX.part16.integer + (s16)(camera = &gEwramData->bgInfo[1])->xPos.part16.integer + entity->ext.evilButcherProjectile.probeX,
                          (s16)entity->posY.part16.integer + (s16)camera->yPos.part16.integer))
            break;
    }
        slot = ObjAffine_Alloc(1);
        if ((s16)slot < 0)
        {
        delete:
            EntityDelete(entity);
            return;
        }
        PlaySong(0x13A);
        entity->unk_518.part16.hp = 0;
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->velX.whole = -(entity->velX.whole / 2 + (RandomNumberGenerator() & 0x7FFF));
        entity->velY.whole = RandomSigned(0x8000) - 0x18000;
        entity->accelY.whole = 0x2000;
        entity->state = 2;
    case 2:
        EntityAccelerateAndMove(entity, 0x80000);
        entity->ext.evilButcherProjectile.angle += entity->ext.evilButcherProjectile.spin;
        ObjAffine_Set(entity->ext.evilButcherProjectile.angle, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
        if (!EntityIsOutsideScreen(entity, 3))
            break;
    case 3:
        if (entity->affineIndex || entity->affine)
            ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
        break;
    }
}
