/* DeathFloatingWeapon_Update: matched (case 4 posY value pinned to r2 keeps r3 out of the spill
   register set; operand order of mul/and/or as in the target). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
#include "syscalls.h"
#include "entity_hit.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern u8 sDeathGfx[], sDeathPalette[], sDeathSpriteSheet[];
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Enemy_OnContactMask1(struct EwramData_EntityData *);
extern void DeathFloatingWeapon_OnHit(struct EwramData_EntityData *, s32, u16 *);
extern void DeathFloatingWeaponWindow_Update(struct EwramData_EntityData *);
extern void DeathFloatingWeaponHitBox_Update(struct EwramData_EntityData *);
extern void DeathWeaponTrail_Update(struct EwramData_EntityData *);
extern struct EwramData_EntityData *Entity_CreateObject(void *);
extern s32 Entity_AccelerateAlongAngle(struct EwramData_EntityData *entity, u16 angle, s32 speed, s32 target, s16 acceleration, u16 time);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);

#define MOVE_TO_A(x0, y0, tx, ty, shift, ANGLE)                                                         \
    {                                                                                    \
        u32 dist;                                                                        \
        u16 angle;                                                                       \
        x0 = entity->posX.whole;                                                         \
        y0 = entity->posY.whole;                                                         \
        dist = Sqrt(({ s32 d = (x0 - (tx)) >> 16; d * d; }) + ({ s32 d = (y0 - (ty)) >> 16; d * d; })) << 16; \
        angle = ANGLE(tx, ty); \
        dist >>= (shift);                                                                \
        Entity_AccelerateAlongAngle(entity, angle, dist, 0, 0, 0);                                      \
        entity->velX.whole = entity->posX.whole - x0;                                    \
        entity->velY.whole = entity->posY.whole - y0;                                    \
    }

#define MOVE_TO(x0, y0, tx, ty, shift) MOVE_TO_A(x0, y0, tx, ty, shift, ANGLE)
#define DIV16(x) ((x) < 0 ? -(-(x) >> 16) : (x) >> 16)
#define BOX(x, y) (((DIV16(x) - 8) & 0xFF) + (((DIV16(y) - 8) & 0xFF) << 8) + 0x10100000)

#define SPAWN(fn)                                                                        \
    {                                                                                    \
        struct EwramData_EntityData *child = Entity_CreateObject(fn);                           \
        if (child != NULL)                                                               \
        {                                                                                \
            child->ext.deathHelper.ownerIndex = ENTITY_INDEX(entity);        \
            child->scrollWithBg = 1;                                                        \
        }                                                                                \
    }

#define ANGLE(tx, ty) ArcTan2(DIV16(tx) - (s16)entity->posX.part16.integer, DIV16(ty) - ({ s32 py = (s16)entity->posY.part16.integer; py; }))
/* Case 4 copy: posY value pinned to r2, so dx takes r3 and the ldrsh scratch is r1; otherwise r3
   joins the function-wide spill register set and the reload rotation drifts. */
#define ANGLE3(tx, ty) ArcTan2(DIV16(tx) - (s16)entity->posX.part16.integer, DIV16(ty) - ({ register s32 py asm("r2") = (s16)entity->posY.part16.integer; /* FAKEMATCH: posY value in r2 */ asm("" : "+r"(py)); /* FAKEMATCH */ py; }))
/**
 * @brief Create the destructible affine weapon with an OBJ window and
 * shared-history hitbox. Follow the boss, spin and approach the player
 * from owner flags; on defeat notify the boss and remove with effects.
 * Preserve the pinned-register coordinate helpers and expression ordering.
 */
void DeathFloatingWeapon_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = &gEwramData->entityData[(u8)entity->ext.deathHelper.ownerIndex];
    struct HitResult res;
    s32 b;
    s32 sx, sy;

    switch (entity->state)
    {
    case 0:
    {
        u16 slot;
        if (!Entity_LoadGraphics(entity, sDeathGfx, sDeathPalette, 0, sDeathSpriteSheet))
            goto delete;
        slot = ObjAffine_Alloc(1);
        if ((s16)slot < 0)
            goto delete;
        entity->enemyId = parent->enemyId;
        Enemy_ResetFromTemplate(entity);
        entity->hp = 0x7D0;
        Hit_Enable(entity, 8, Enemy_OnContactMask1, (s32)DeathFloatingWeapon_OnHit);
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->spriteLayer = 0x12;
        Anim_StartIndexed(entity, 0xC, 2, 0);
        Anim_Update(entity);
        if (parent->spawnParam0 != 0)
        {
            struct BgLayerState *bg = &gEwramData->bgInfo[1];
            entity->posX.part16.integer = 0xE8 - bg->xPos.part16.integer;
            entity->posY.part16.integer = 0x64 - bg->yPos.part16.integer;
            entity->state = 1;
            entity->ext.deathWeapon.angle = 0x1000;
            SPAWN(DeathFloatingWeaponWindow_Update);
            entity->hit.flags |= 4;
        }
        else
        {
            entity->posX.whole = parent->posX.whole;
            entity->posY.whole = parent->posY.whole;
            entity->state = 4;
        }
        SPAWN(DeathFloatingWeaponHitBox_Update);
        break;
    }
    case 1:
        if (parent->ext.death.flags & 0x20)
        {
            entity->state = 2;
            entity->ext.deathFloatingWeapon.timer = 0x20;
            PlaySong(0x124);
        }
        break;
    case 2:
    {
        s32 x0, y0;
        s32 tx = parent->posX.whole + 0x500000;
        MOVE_TO(x0, y0, tx, 0x200000, 5);
        entity->ext.deathWeapon.angle -= 0x66;
        if (entity->ext.deathFloatingWeapon.timer != 0)
        {
            entity->ext.deathFloatingWeapon.timer--;
            break;
        }
        entity->state = 3;
        entity->ext.deathFloatingWeapon.timer = 0x60;
        break;
    }
    case 3:
    {
        s32 x0, y0;
        s32 tx = parent->posX.whole + 0x200000;
        MOVE_TO(x0, y0, tx, 0x300000, 4);
        entity->ext.deathWeapon.angle -= 0x616;
        if (entity->ext.deathFloatingWeapon.timer != 0)
        {
            entity->ext.deathFloatingWeapon.timer--;
            break;
        }
        entity->hit.flags &= ~4;
        entity->state = 4;
        break;
    }
    case 4:
    {

        s32 off, y0;
        s32 tx, ty;
        if (!(parent->ext.death.flags & 0x40) && (s16)entity->hp <= 0x4AF)
            parent->ext.death.flags |= 0x40;
        off = 0x200000;
        {
            s32 px = gEwramData->playerState.player->posX.whole;
            if (entity->posX.whole < px)
                off = -off;
        }
        tx = parent->posX.whole + off;
        ty = parent->posY.whole - 0x200000;
        MOVE_TO_A(off, y0, tx, ty, 4, ANGLE3);
        entity->ext.deathWeapon.angle += (entity->velX.whole + entity->velY.whole) / 256;
        if (parent->ext.death.flags & 4)
        {
            entity->state = 5;
            entity->ext.deathFloatingWeapon.timer = 0x40;
        }
        break;
    }
    case 5:
        entity->ext.deathWeapon.angle += 0x789;
        if (!(entity->timer & 0x1F))
            PlaySong(0x135);
        if (entity->ext.deathFloatingWeapon.timer != 0)
        {
            entity->ext.deathFloatingWeapon.timer--;
            break;
        }
        entity->ext.deathFloatingWeapon.timer = 0x80;
        entity->state = 6;
        break;
    case 6:
    {
        struct EwramData_EntityData *player = gEwramData->playerState.player;
        s32 tx = player->posX.whole;
        s32 ty = player->posY.whole;
        s32 x0, y0;
        MOVE_TO(x0, y0, tx, ty, 7);
        entity->ext.deathWeapon.angle += 0x789;
        if (!(entity->timer & 0x1F))
            PlaySong(0x135);
        if (entity->ext.deathFloatingWeapon.timer != 0)
        {
            entity->ext.deathFloatingWeapon.timer--;
            break;
        }
        entity->state = 4;
        break;
    }
    case 7:
    {
        u8 t = entity->timer;
        if (t & 1)
        {
            u16 a = entity->ext.deathWeapon.angle - 0x4000;
            if (({ s32 m_ = 2; m_ &= t; m_; }))
                a = (u32)((a << 16) + 0x80000000) >> 16;
            sx = (entity->timer >> 1) * Sin((s16)a + 0x4000);
            sy = (entity->timer >> 1) * Sin((s16)a);
            Effect_SpawnRandom(entity->posX.whole + sx, entity->posY.whole + sy, 0x40000, 0x40000, entity->spriteLayer);
        }
        if (entity->timer <= 0x2F)
        {
            entity->timer++;
            break;
        }
    }
    case 8:
        ObjAffine_Free(entity->affineIndex);
    delete:
        EntityDelete(entity);
        return;
    }

    if ((u8)(entity->state - 2) <= 4)
    {
        if (++entity->subState == 3)
        {
            struct EwramData_EntityData *child;
            entity->subState = 0;
            child = Entity_CreateObject(DeathWeaponTrail_Update);
            if (child != NULL)
            {
                child->ext.deathHelper.ownerIndex = ENTITY_INDEX(entity);
                child->scrollWithBg = 1;
                { register struct EwramData_EntityData *c asm("r1") = child; /* FAKEMATCH: copy of the new child in r1 */
                c->ext.deathWeapon.angle = entity->ext.deathWeapon.angle;
                c->posX.whole = entity->posX.whole;
                c->posY.whole = entity->posY.whole; }
            }
        }
    }
    ObjAffine_Set(entity->ext.deathWeapon.angle, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
    if (entity->state > 6)
        return;
    {
        s32 a = (s16)(entity->ext.deathWeapon.angle - 0x4000);
        sx = ((entity->timer & 3) * 16 - 0x20) * Sin(a + 0x4000);
        sy = ((entity->timer & 3) * 16 - 0x20) * Sin(a);
        HitBox_SetBodyRaw(entity, BOX(sx, sy));
    }
    {
        s32 a = (s16)(entity->ext.deathWeapon.angle - 0x4000);
        sx = Sin(a + 0x4000) * 24;
        sy = Sin(a) * 24;
        b = entity->ext.deathWeapon.angle;
        sx += (entity->timer & 3) * (Sin(b + 0x4000) << 3);
        sy += (entity->timer & 3) * (Sin(b) << 3);
        HitBox_SetAttackRaw(entity, BOX(sx, sy));
    }
    entity->timer++;
    if (!(entity->enemyFlags & 2))
    {
        Enemy_UpdateHitFlash(entity);
        Hit_CheckWithPlayer(&res, (u8 *)entity);
        return;
    }
    PlaySong(0x6E);
    parent->ext.death.flags |= 1;
    entity->state = 7;
    entity->timer = 0;
}
