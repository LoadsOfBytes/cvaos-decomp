/* Disc Armor thrown disc. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code_08001194.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "entity_hit.h"
#include "syscalls.h"
#include "structs/ewram.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern void Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);
extern s32 __divsi3(s32, s32);
extern s32 DiscArmorRing_Aim(struct EwramData_EntityData *disc, struct EwramData_EntityData *entity,
                        struct EwramData_EntityData *owner);
extern void DiscArmorRing_Update(struct EwramData_EntityData *entity);
extern void DiscArmorDisc_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void DiscArmorSpark_Update(struct EwramData_EntityData *entity);
extern u8 sDiscArmorGfx[], sDiscArmorPalette[], sDiscArmorSpriteSheet[];
extern u8 sDiscArmorWalkProfile[];

#define INT_TOWARD_ZERO(v) ((v) < 0 ? -((-(v)) >> 16) : (v) >> 16)
#define ABS_DIFF(a, b) ((a) - (b) < 0 ? (b) - (a) : (a) - (b))

static inline s32 PixelDelta(s16 pos, s32 centre)
{
    if (centre < 0)
        return pos + ((-centre) >> 16);
    return pos - (centre >> 16);
}

static inline s16 PushOutOfFloor(struct EwramData_EntityData *entity, s16 x, s16 y)
{
    s16 push = Collision_GetPushUp(x, y);

    if (push != 0)
        entity->posX.whole += push << 16;
    return push;
}

static inline void SpawnSpark(struct EwramData_EntityData *entity, u8 type)
{
    s32 x = (s16)entity->posX.part16.integer;
    s32 y = (s16)(entity->posY.part16.integer + 16);
    s32 flip = entity->flipX;
    struct EwramData_EntityData *spark = EntityCreateInRange(0x49, 0xCF, (u32 *)DiscArmorSpark_Update);

    if (spark != NULL)
    {
        spark->unk_4F8.part8.unk_4F8 = type;
        spark->posX.part16.integer = x;
        spark->posY.part16.integer = y;
        spark->flipX = flip ^ 1;
    }
}

/**
 * @brief Thrown disc update: fly out from the owner and back along its path with
 * the ring effect (DiscArmorRing_Update) and sparks, signal the owner
 * (unk_4F8 bit 1) when it returns.
 */
void DiscArmorDisc_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->unk_4F4.unk_4F4_entity;
    struct EwramData_EntityData *ring = entity->unk_4FC.unk_4FC_entity;
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;
    struct HitResult hit;
    s32 amp;
    u16 xu;

    if (entity->state <= 5)
    {
        amp = DiscArmorRing_Aim(ring, entity, owner);
        if (amp <= 0x600000)
        {
            if ((s16)owner->unk_518.part16.hp > 0 && !(*(u16 *)&owner->unk_51C & 0x100))
                goto check;
        }
        else
            owner->unk_4F8.whole |= 2;
        if (entity->flipX)
            entity->unk_510.whole += -0x10000;
        else
            entity->unk_510.whole += 0x10000;
        if (entity->state == 5)
            entity->unk_510.whole = 0;
        entity->state = 6;
        entity->unk_50C.whole = entity->unk_510.whole;
        entity->velY.whole = (s32)entity->unk_514.unk_514_32;
        entity->accelY.whole = 0x4000;
        entity->accelX.whole = 0;
        goto run;
    check:
        if (!(owner->unk_4F8.whole & 1))
        {
            if (entity->affine)
                ObjAffine_Free(entity->affineIndex);
            if (entity->unk_4FC.unk_4FC_entity != NULL)
                entity->unk_4FC.unk_4FC_entity->state = 2;
            goto delete;
        }
    }
run:
    Hit_CheckWithPlayer(&hit, (u8 *)entity);
    if (gEwramData->gameState.timeStopTimer != 0)
        return;

    switch (entity->state)
    {
    case 0:
    {
        u16 matrix;

        if (!Entity_LoadGraphics(entity, sDiscArmorGfx, sDiscArmorPalette, 0, sDiscArmorSpriteSheet))
            goto delete;
        matrix = ObjAffine_Alloc(1);
        if ((s16)matrix < 0)
            goto delete;
        Anim_StartIndexed(entity, 6, 0, 0);
        Anim_Update(entity);
        entity->affineIndex = matrix;
        entity->affine = 1;
        entity->spriteLayer = 0x13;
        ring = Entity_CreateObject((u32 *)DiscArmorRing_Update);
        entity->unk_4FC.unk_4FC_entity = ring;
        entity->unk_500.whole = 0x8000;
        {
            s32 turn = 0x80;

            entity->unk_504.whole = turn;
            if (entity->flipX)
            {
                entity->unk_500.whole = 0;
                entity->unk_504.whole = -turn;
            }
        }
        entity->posX.whole = owner->unk_4FC.whole;
        entity->posY.whole = owner->unk_500.whole;
        Hit_Enable(entity, 8, DiscArmorDisc_DamagePlayerOnContact, 0);
        HitBox_SetAttackRaw(entity, 0x1818F4F4);
        HitBox_SetBodyRaw(entity, 0x1818F4F4);
        entity->scrollWithBg = 1;
        Afterimage_Create(entity, 0x60, 0xC, 2);
        Afterimage_Create(entity, 0x60, 6, 2);
        PlaySong(0x74);
        entity->state = 1;
    }
    case 1:
    {
        u16 yu;
        s16 x, y;
        s16 push, push2, push3;

        amp = INT_TOWARD_ZERO(Sin(entity->unk_50C.whole) * 0x60);
        entity->unk_50C.whole += 0x200;
        entity->unk_500.whole += entity->unk_504.whole;
        if (entity->unk_50C.whole > 0x4000)
        {
            entity->unk_50C.whole = 0x4000;
            entity->accelY.whole += 0x4000;
            if (entity->accelY.whole > 0x80000)
                entity->accelY.whole = 0x80000;
            entity->velY.whole += entity->accelY.whole;
        }
        else
            entity->velY.whole = Sin(entity->unk_500.whole) * amp;
        entity->velX.whole = amp * Sin(entity->unk_500.whole + 0x4000);
        entity->posX.whole = owner->unk_4FC.whole + entity->velX.whole;
        entity->posY.whole = owner->unk_500.whole + entity->velY.whole;
        xu = GetEntityRoomXPositionInteger(entity);
        y = GetEntityRoomYPositionInteger(entity);
        x = xu;
        push = Collision_GetPushRight(x - 16, y);
        if (push != 0)
        {
            entity->posX.whole += push << 16;
            entity->unk_50C.whole = 0;
            entity->state = 2;
            break;
        }
        push3 = Collision_GetPushLeft(x + 16, y);
        if (push3 != 0)
        {
            entity->posX.whole += push3 << 16;
            entity->unk_50C.whole = push;
            entity->state = 2;
        }
        else
        {
            push2 = PushOutOfFloor(entity, x, y + 16);
            if (push2 == 0)
                break;
            entity->velY.whole += push2 << 16;
            if (entity->unk_50C.whole < 0x4000)
            {
                entity->accelY.whole = -0x30000;
                entity->unk_50C.whole = push3;
            }
            entity->unk_50C.whole = push3;
            entity->state = 3;
        }
        break;
    }
    case 2:
    {
        u16 yu;
        s16 push;

        entity->accelY.whole += 0x4000;
        if (entity->accelY.whole > 0x80000)
            entity->accelY.whole = 0x80000;
        entity->velY.whole += entity->accelY.whole;
        entity->posX.whole = owner->unk_4FC.whole + entity->velX.whole;
        entity->posY.whole = owner->unk_500.whole + entity->velY.whole;
        xu = GetEntityRoomXPositionInteger(entity);
        yu = GetEntityRoomYPositionInteger(entity);
        if (PushOutOfFloor(entity, xu, yu + 16) == 0)
            break;
        entity->unk_50C.whole = 0;
        entity->state = 3;
        break;
    }
    case 3:
    {
        u16 yu;
        s16 push;

        entity->accelY.whole += 0x4000;
        if (entity->accelY.whole > 0x80000)
            entity->accelY.whole = 0x80000;
        entity->velY.whole += entity->accelY.whole;
        entity->posX.whole = owner->unk_4FC.whole + entity->velX.whole;
        entity->posY.whole = owner->unk_500.whole + entity->velY.whole;
        xu = GetEntityRoomXPositionInteger(entity);
        yu = GetEntityRoomYPositionInteger(entity);
        push = PushOutOfFloor(entity, xu, yu + 16);
        if (push == 0)
            break;
        SpawnSpark(entity, 5);
        PlaySong(0x104);
        if (entity->accelY.whole < 0x8000)
        {
            entity->state = 4;
            entity->accelY.whole = 0x4000;
            entity->velY.whole = 0;
        }
        else
        {
            entity->accelY.whole = -entity->accelY.whole / 2;
            entity->velY.whole += push << 16;
        }
        break;
    }
    case 4:
    {
        s32 res;
        s32 dx, dy;

        if (entity->timer <= 3)
            SpawnSpark(entity, 5);
        if (!(entity->timer & 0xF))
            PlaySong(0x73);
        entity->timer++;
        if (entity->unk_50C.whole < 0x40000)
            entity->unk_50C.whole += 0x1000;
        res = Entity_Walk(entity, -entity->unk_50C.whole, sDiscArmorWalkProfile + 12, 8);
        {
            s8 three = 3; /* the mask is a variable: and in QImode puts it first */
            if (res & three)
            {
                owner->unk_4F8.whole |= 2;
                entity->state = 5;
            }
            asm("" :: "r"(three)); /* FAKEMATCH: extra ref so the mask outranks res (r2) */
            if ((res & 8) && !(u8)(entity->timer & three))
                SpawnSpark(entity, 4);
        }
        if (ABS_DIFF((s16)owner->posX.part16.integer, (s16)entity->posX.part16.integer) <= 0x2F)
        {
            owner->unk_4F8.whole |= 2;
            entity->state = 5;
            entity->unk_500.whole = ArcTan2(
                PixelDelta(entity->posX.part16.integer, owner->unk_4FC.whole),
                ({
                    s32 d_ = PixelDelta(entity->posY.part16.integer, owner->unk_500.whole);
                    register s32 s_ asm("r0") = d_ << 16; /* FAKEMATCH */
                    asm("" : "+r"(s_)); /* FAKEMATCH: shift temp of the s16 argument in r0 */
                    s_ >> 16;
                }));
            dx = (entity->posX.whole - (s32)owner->unk_4FC.whole) >> 16;
            dx *= dx;
            dy = (entity->posY.whole - owner->unk_500.whole) >> 16;
            dy *= dy;
            entity->unk_508.whole = Sqrt(dx + dy) << 16;
            entity->unk_508.whole = entity->unk_508.whole * 2 / 3;
            entity->unk_504.whole = 0;
            entity->timer = 0;
            PlaySong(0x103);
        }
        break;
    }
    case 5:
    {
        s32 turn;

        entity->unk_508.whole += -0x10000;
        if (++entity->timer > 4)
        {
            amp = entity->timer - 4;

            if (amp <= 3)
                entity->unk_508.whole += Sin(amp << 12) << 3;
        }
        if (entity->unk_508.whole < 0x20000)
            entity->unk_508.whole = 0x20000;
        amp = INT_TOWARD_ZERO(entity->unk_508.whole);
        entity->velY.whole = Sin(entity->unk_500.whole) * amp;
        entity->velX.whole = amp * Sin(entity->unk_500.whole + 0x4000);
        entity->posX.whole = owner->unk_4FC.whole + entity->velX.whole;
        entity->posY.whole = owner->unk_500.whole + entity->velY.whole;
        amp = entity->unk_504.whole += -0x140;
        if (entity->flipX)
            amp = -amp;
        entity->unk_500.whole += amp;
        break;
    }
    case 6:
    {
        s32 res;
        s32 speed;

        if (entity->unk_4FC.unk_4FC_entity != NULL)
        {
            ring = entity->unk_4FC.unk_4FC_entity;
            ring->state = 2;
            entity->unk_4FC.unk_4FC_entity = NULL;
        }
        if (EntityIsOutsideScreen(entity, 4))
            goto free_delete;
        res = Entity_Walk(entity, entity->unk_50C.whole, sDiscArmorWalkProfile + 12, 2);
        if (res & 0x40)
            entity->unk_50C.whole = entity->unk_50C.whole > 0 ? entity->unk_50C.whole - 0x2000 : entity->unk_50C.whole + 0x2000;
        else if (res & 0x20)
            entity->unk_50C.whole = entity->unk_50C.whole > 0 ? entity->unk_50C.whole + 0x2000 : entity->unk_50C.whole - 0x2000;
        else if (entity->unk_50C.whole >= 0)
        {
            if ((entity->unk_50C.whole += -0x100) <= 0)
            {
                entity->unk_50C.whole = 0;
                entity->timer = 0x20;
                entity->state = 7;
            }
        }
        else
        {
            if ((entity->unk_50C.whole += 0x100) > 0)
            {
                entity->unk_50C.whole = 0;
                entity->timer = 0x20;
                entity->state = 7;
            }
        }
        speed = entity->unk_50C.whole;
        entity->unk_4F8.whole += speed / 64;
        if (res & 3)
            entity->unk_50C.whole = -speed / 2;
        break;
    }
    case 7:
        entity->hit.flags |= 2;
        entity->drawSkip = 1;
        if (entity->timer == 0)
        {
        free_delete:
            ObjAffine_Free(entity->affineIndex);
        delete:
            EntityDelete(entity);
            return;
        }
        entity->timer--;
        break;
    }

    if (entity->state <= 5)
    {
        DiscArmorRing_Aim(ring, entity, owner);
        if (entity->flipX)
            entity->unk_4F8.whole += -0x2F00;
        else
            entity->unk_4F8.whole += 0x2F00;
    }
    entity->unk_510.whole = entity->posX.whole - oldX;
    entity->unk_514.unk_514_32 = (struct EwramData_EntityData *)(entity->posY.whole - oldY);
    ObjAffine_Set(entity->unk_4F8.whole, 0x10000, 0x10000, 0, 0, entity->affineIndex);
}
