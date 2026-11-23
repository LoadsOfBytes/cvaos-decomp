/* Boss arm segment: hangs from its owner, swings, is thrown and flies off. */
#include "code_08009A0.h"
#include "code_08013960.h"
#include "code_08033CAC.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"

struct ArmPose {
    u16 angle;
    u16 dx;
    u16 dy;
    u8 layer;
    u8 flag;
};



extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern s32 DeathWeapon_UpdateWallTraversal(struct EwramData_EntityData *entity, struct EwramData_EntityData *target);
extern void DeathWeapon_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern struct ArmPose sDeathWeaponArmPoses[];
extern u8 sDeathGfx[], sDeathPalette[], sDeathSpriteSheet[];

#define SPIN(e) (e->ext.deathWeapon.angle)
#define SPEED(e) (e->ext.deathWeapon.spin)
#define FLAGS(e) (e->ext.death.flags)
#define TO_PIXELS(d) ((d) < 0 ? -(-(d) >> 16) : (d) >> 16)

static inline s32 Id(s32 v) { return v; }
static inline s32 SparkOffset(s32 a, s32 k)
{
    return Sin(a) * 48 + RandomSigned(Id(k));
}

/**
 * @brief Affine weapon controller: follow owner poses, perform throw/return
 * or wall traversal from owner flags, and fly away on owner defeat.
 * A secondary instance mirrors the primary weapon and shares its lifetime.
 */
void DeathWeapon_Update(struct EwramData_EntityData *entity)
{
    u8 box[16];
    struct EwramData_EntityData *owner = &gEwramData->entityData[(u8)entity->ext.deathHelper.ownerIndex];
    struct ArmPose *pose = &sDeathWeaponArmPoses[(u8)owner->ext.death.dashStep];
    s32 dx, dy, angle;
    s32 zero;
    u8 t;

    if ((u8)(entity->state - 3) <= 4 && (owner->enemyFlags & 2))
    {
        Sound_StopTracked(0x105);
        entity->attack = 0x28;
        entity->state = 9;
        entity->velX.whole = 0x18000;
        entity->accelX.whole = 0;
        entity->velY.whole = -0x40000;
        entity->accelY.whole = 0x2000;
        entity->hidden = 0;
    }
    switch (entity->state)
    {
    case 0:
    {
        u16 slot;
        if (!Entity_LoadGraphics(entity, sDeathGfx, sDeathPalette, 1, sDeathSpriteSheet))
            goto remove;
        slot = ObjAffine_Alloc(1);
        if ((s16)slot < 0)
            goto remove;
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->attack = owner->attack;
        entity->unk_518.part8.enemyId = owner->unk_518.part8.enemyId;
        entity->spriteLayer = owner->spriteLayer;
        Anim_StartIndexed(entity, 0x11, 2, 0);
        Anim_Update(entity);
        Hit_Enable(entity, 8, DeathWeapon_DamagePlayerOnContact, 0);
        entity->hit.flags |= 4;
        entity->attack = 0x3C;
        SPIN(entity) = 0x4000;
        if (entity->ext.deathWeapon.mirror)
        {
            entity->state = 8;
            goto tail;
        }
        entity->state = 1;
        entity->timer = 8;
    }
    case 1:
        entity->posX.whole = owner->posX.whole;
        entity->posY.whole = owner->posY.whole - 0x280000;
        if (entity->timer)
        {
            if ((u8)--entity->timer > 3)
                goto tail;
            if (entity->timer & 1)
                entity->palette = owner->palette;
            else
                entity->palette = entity->unk_510.part8.basePalette;
            goto tail;
        }
        entity->timer = 4;
        entity->palette = owner->palette;
        entity->state = 2;
    case 2:
        SPIN(entity) = (entity->timer << 10) + pose->angle;
        entity->posX.part16.integer = owner->posX.part16.integer + pose->dx;
        { u16 v_ = owner->posY.part16.integer + pose->dy - entity->timer; register u16 *p_ asm("r3") = &entity->posY.part16.integer; *p_ = v_; } /* FAKEMATCH: pinned register */
        if ((t = entity->timer) != 0)
            goto count;
        goto idle;
    case 3:
    {
        u16 flags = FLAGS(owner) & 8;
        if (flags)
        {
            s32 speed, vx, ax;
            { u8 *p_ = &entity->attack; u8 v_; flags = 0; v_ = 0x41; *p_ = v_; }
            entity->hidden = 0;
            SPEED(entity) = speed = 0x7A0;
            entity->velX.whole = vx = 0x48000;
            entity->accelX.whole = ax = -0x1000;
            entity->velY.whole = flags;
            entity->accelY.whole = flags;
            entity->timer = 0x10;
            if (!owner->flipX)
            {
                SPEED(entity) = -speed;
                entity->velX.whole = -vx;
                entity->accelX.whole = -ax;
            }
            entity->state = 5;
            goto tail;
        }
        if (FLAGS(owner) & 0x10)
        {
            entity->attack = 0x46;
            entity->hidden = 0;
            entity->state = 4;
            entity->stage = flags;
            goto tail;
        }
        entity->attack = 0x3C;
        entity->hidden = pose->flag;
        if (!owner->flipX)
        {
            SPIN(entity) = pose->angle;
            entity->posX.part16.integer = owner->posX.part16.integer + pose->dx;
        }
        else
        {
            s32 an = pose->angle;
            SPIN(entity) = -0x8000 - an;
            entity->posX.part16.integer = owner->posX.part16.integer - pose->dx;
        }
        entity->posY.part16.integer = owner->posY.part16.integer + pose->dy;
        entity->spriteLayer = owner->spriteLayer + pose->layer;
        entity->flipX = owner->flipX;
        goto tail;
    }
    case 5:
        SPIN(entity) += SPEED(entity);
        if (!(entity->subState & 0x1F))
            PlaySong(0x135);
        if (FLAGS(owner) & 4)
            entity->state = 6;
        goto tail;
    case 6:
    {
        if (!(entity->subState & 0x1F))
            PlaySong(0x135);
        SPIN(entity) += SPEED(entity);
        EntityAccelerateAndMove(entity, 0x20000);
        if ((t = entity->timer) != 0)
            goto count;
        zero = owner->posX.whole - entity->posX.whole;
        if (zero < 0)
            zero = entity->posX.whole - owner->posX.whole;
        if (zero > 0x1FFFF)
            goto tail;
        FLAGS(owner) &= ~0xC;
        entity->timer = 0x34;
        entity->state = 7;
        goto tail;
    }
    case 7:
        SPIN(entity) += SPEED(entity);
        if (SPEED(entity) >= 0)
            SPEED(entity) = SPEED(entity) * 11 / 12;
        else
            SPEED(entity) = SPEED(entity) * 15 / 16;
        entity->posX.part16.integer = owner->posX.part16.integer;
        { u16 v_ = owner->posY.part16.integer - 0x26; register u16 *p_ asm("r3") = &entity->posY.part16.integer; *p_ = v_; } /* FAKEMATCH: pinned register */
        if ((t = entity->timer) == 0)
            goto idle;
    count:
        entity->timer = t - 1;
        goto tail;
    case 4:
        if (DeathWeapon_UpdateWallTraversal(entity, owner))
        {
            { u16 f_ = FLAGS(owner); register u32 m_ asm("r1") = 0xFFEF; m_ &= f_; FLAGS(owner) = m_; } /* FAKEMATCH: pinned register */
            goto idle;
        }
        goto tail;
    idle:
        entity->state = 3;
        goto tail;
    case 9:
        SPIN(entity) += 0x790;
        EntityAccelerateAndMove(entity, 0x80000);
        if (EntityIsOutsideScreen(entity, 3))
        {
            entity->state = 10;
            Player_SetSelectedTarget(NULL);
        }
        goto tail;
    case 8:
    default:
    tail:
    if ((zero = entity->ext.deathWeapon.mirror) == 0)
    {
        ObjAffine_Set(SPIN(entity), 0x10000, 0x10000, entity->flipX, zero, entity->affineIndex);
    }
    else
    {
        owner = &gEwramData->entityData[entity->ext.deathWeapon.primaryIndex];
        if (owner->state == 10)
        {
    case 10:
            ObjAffine_Free(entity->affineIndex);
        remove:
            EntityDelete(entity);
            return;
        }
        entity->hidden = owner->hidden;
        entity->palette = owner->palette;
        { u8 v_ = owner->spriteLayer; u8 *p_ = &entity->spriteLayer; zero = 0; *p_ = v_; }
        entity->flipX = owner->flipX;
        entity->posX.whole = owner->posX.whole;
        entity->posY.whole = owner->posY.whole;
        SPIN(entity) = -0x8000 + SPIN(owner);
        ObjAffine_Set(SPIN(entity), 0x10000, 0x10000, entity->flipX, zero, entity->affineIndex);
    }
    }
    if (entity->hidden)
        return;
    entity->subState++;
    if (entity->subState & 1)
    {
        struct EwramData_EntityData *fx;
        s32 x, y;
        s32 a;
        a = (s16)(SPIN(entity) - 0x4000);
        x = Sin(a + 0x4000);
        y = 0x80000;
        do { do { x = x * 48 + RandomSigned(y); } while (0); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        do { do { y = Sin(a) * 48 + RandomSigned(y); } while (0); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        fx = Effect_SpawnShared(entity->posX.whole + x, entity->posY.whole + y, 0, 6);
        if (fx != NULL)
            fx->accelY.whole = -0x800;
    }
    {
        s32 ang = (s16)(-0x4000 + SPIN(entity));
        s32 q = 0x4000;
        dx = Sin(ang + q) * 24;
        dy = Sin(ang) * 24;
        {
            u16 a = SPIN(entity);
            if (entity->flipX)
                a = (a << 16) + 0x80000000 >> 16;
            angle = (s16)a;
        }
        dx += Sin(angle + q) << 4;
        dy += Sin(angle) << 4;
    }
    HitBox_SetAttackRaw(entity, ((TO_PIXELS(dx) - 16) & 0xFF) + (((TO_PIXELS(dy) - 16) & 0xFF) << 8) + 0x20200000);
    Hit_CheckWithPlayer(box, entity);
}
