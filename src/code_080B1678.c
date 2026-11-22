/* Disc Armor thrown disc. */
#include "code_08009A0.h"
#include "enemy_status.h"
#include "disc_armor_entity.h"
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
    struct EwramData_EntityData *owner = DISC_ARMOR_DISC_OWNER(entity);
    struct EwramData_EntityData *ring = DISC_ARMOR_DISC_RING(entity);
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
            if ((s16)owner->unk_518.part16.hp > 0 && !(ENEMY_STATUS(owner) & ENEMY_STATUS_PETRIFIED))
                goto check;
        }
        else
            DISC_ARMOR_FLAGS(owner) |= 2;
        if (entity->flipX)
            DISC_ARMOR_DISC_LAST_DX(entity) += -0x10000;
        else
            DISC_ARMOR_DISC_LAST_DX(entity) += 0x10000;
        if (entity->state == 5)
            DISC_ARMOR_DISC_LAST_DX(entity) = 0;
        entity->state = 6;
        DISC_ARMOR_DISC_PHASE(entity) = DISC_ARMOR_DISC_LAST_DX(entity);
        entity->velY.whole = (s32)DISC_ARMOR_DISC_LAST_DY(entity);
        entity->accelY.whole = 0x4000;
        entity->accelX.whole = 0;
        goto run;
    check:
        if (!(DISC_ARMOR_FLAGS(owner) & 1))
        {
            if (entity->affine)
                ObjAffine_Free(entity->affineIndex);
            if (DISC_ARMOR_DISC_RING(entity) != NULL)
                DISC_ARMOR_DISC_RING(entity)->state = 2;
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
        DISC_ARMOR_DISC_RING(entity) = ring;
        DISC_ARMOR_DISC_ORBIT_ANGLE(entity) = 0x8000;
        {
            s32 turn = 0x80;

            DISC_ARMOR_DISC_TURN(entity) = turn;
            if (entity->flipX)
            {
                DISC_ARMOR_DISC_ORBIT_ANGLE(entity) = 0;
                DISC_ARMOR_DISC_TURN(entity) = -turn;
            }
        }
        entity->posX.whole = DISC_ARMOR_HAND_X(owner);
        entity->posY.whole = DISC_ARMOR_HAND_Y(owner);
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

        amp = INT_TOWARD_ZERO(Sin(DISC_ARMOR_DISC_PHASE(entity)) * 0x60);
        DISC_ARMOR_DISC_PHASE(entity) += 0x200;
        DISC_ARMOR_DISC_ORBIT_ANGLE(entity) += DISC_ARMOR_DISC_TURN(entity);
        if (DISC_ARMOR_DISC_PHASE(entity) > 0x4000)
        {
            DISC_ARMOR_DISC_PHASE(entity) = 0x4000;
            entity->accelY.whole += 0x4000;
            if (entity->accelY.whole > 0x80000)
                entity->accelY.whole = 0x80000;
            entity->velY.whole += entity->accelY.whole;
        }
        else
            entity->velY.whole = Sin(DISC_ARMOR_DISC_ORBIT_ANGLE(entity)) * amp;
        entity->velX.whole = amp * Sin(DISC_ARMOR_DISC_ORBIT_ANGLE(entity) + 0x4000);
        entity->posX.whole = DISC_ARMOR_HAND_X(owner) + entity->velX.whole;
        entity->posY.whole = DISC_ARMOR_HAND_Y(owner) + entity->velY.whole;
        xu = GetEntityRoomXPositionInteger(entity);
        y = GetEntityRoomYPositionInteger(entity);
        x = xu;
        push = Collision_GetPushRight(x - 16, y);
        if (push != 0)
        {
            entity->posX.whole += push << 16;
            DISC_ARMOR_DISC_PHASE(entity) = 0;
            entity->state = 2;
            break;
        }
        push3 = Collision_GetPushLeft(x + 16, y);
        if (push3 != 0)
        {
            entity->posX.whole += push3 << 16;
            DISC_ARMOR_DISC_PHASE(entity) = push;
            entity->state = 2;
        }
        else
        {
            push2 = PushOutOfFloor(entity, x, y + 16);
            if (push2 == 0)
                break;
            entity->velY.whole += push2 << 16;
            if (DISC_ARMOR_DISC_PHASE(entity) < 0x4000)
            {
                entity->accelY.whole = -0x30000;
                DISC_ARMOR_DISC_PHASE(entity) = push3;
            }
            DISC_ARMOR_DISC_PHASE(entity) = push3;
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
        entity->posX.whole = DISC_ARMOR_HAND_X(owner) + entity->velX.whole;
        entity->posY.whole = DISC_ARMOR_HAND_Y(owner) + entity->velY.whole;
        xu = GetEntityRoomXPositionInteger(entity);
        yu = GetEntityRoomYPositionInteger(entity);
        if (PushOutOfFloor(entity, xu, yu + 16) == 0)
            break;
        DISC_ARMOR_DISC_PHASE(entity) = 0;
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
        entity->posX.whole = DISC_ARMOR_HAND_X(owner) + entity->velX.whole;
        entity->posY.whole = DISC_ARMOR_HAND_Y(owner) + entity->velY.whole;
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
        if (DISC_ARMOR_DISC_PHASE(entity) < 0x40000)
            DISC_ARMOR_DISC_PHASE(entity) += 0x1000;
        res = Entity_Walk(entity, -DISC_ARMOR_DISC_PHASE(entity), sDiscArmorWalkProfile + 12, 8);
        {
            s8 three = 3; /* the mask is a variable: and in QImode puts it first */
            if (res & three)
            {
                DISC_ARMOR_FLAGS(owner) |= 2;
                entity->state = 5;
            }
            asm("" :: "r"(three)); /* FAKEMATCH: extra ref so the mask outranks res (r2) */
            if ((res & 8) && !(u8)(entity->timer & three))
                SpawnSpark(entity, 4);
        }
        if (ABS_DIFF((s16)owner->posX.part16.integer, (s16)entity->posX.part16.integer) <= 0x2F)
        {
            DISC_ARMOR_FLAGS(owner) |= 2;
            entity->state = 5;
            DISC_ARMOR_DISC_ORBIT_ANGLE(entity) = ArcTan2(
                PixelDelta(entity->posX.part16.integer, DISC_ARMOR_HAND_X(owner)),
                ({
                    s32 d_ = PixelDelta(entity->posY.part16.integer, DISC_ARMOR_HAND_Y(owner));
                    register s32 s_ asm("r0") = d_ << 16; /* FAKEMATCH */
                    asm("" : "+r"(s_)); /* FAKEMATCH: shift temp of the s16 argument in r0 */
                    s_ >> 16;
                }));
            dx = (entity->posX.whole - (s32)DISC_ARMOR_HAND_X(owner)) >> 16;
            dx *= dx;
            dy = (entity->posY.whole - DISC_ARMOR_HAND_Y(owner)) >> 16;
            dy *= dy;
            DISC_ARMOR_DISC_RADIUS(entity) = Sqrt(dx + dy) << 16;
            DISC_ARMOR_DISC_RADIUS(entity) = DISC_ARMOR_DISC_RADIUS(entity) * 2 / 3;
            DISC_ARMOR_DISC_TURN(entity) = 0;
            entity->timer = 0;
            PlaySong(0x103);
        }
        break;
    }
    case 5:
    {
        s32 turn;

        DISC_ARMOR_DISC_RADIUS(entity) += -0x10000;
        if (++entity->timer > 4)
        {
            amp = entity->timer - 4;

            if (amp <= 3)
                DISC_ARMOR_DISC_RADIUS(entity) += Sin(amp << 12) << 3;
        }
        if (DISC_ARMOR_DISC_RADIUS(entity) < 0x20000)
            DISC_ARMOR_DISC_RADIUS(entity) = 0x20000;
        amp = INT_TOWARD_ZERO(DISC_ARMOR_DISC_RADIUS(entity));
        entity->velY.whole = Sin(DISC_ARMOR_DISC_ORBIT_ANGLE(entity)) * amp;
        entity->velX.whole = amp * Sin(DISC_ARMOR_DISC_ORBIT_ANGLE(entity) + 0x4000);
        entity->posX.whole = DISC_ARMOR_HAND_X(owner) + entity->velX.whole;
        entity->posY.whole = DISC_ARMOR_HAND_Y(owner) + entity->velY.whole;
        amp = DISC_ARMOR_DISC_TURN(entity) += -0x140;
        if (entity->flipX)
            amp = -amp;
        DISC_ARMOR_DISC_ORBIT_ANGLE(entity) += amp;
        break;
    }
    case 6:
    {
        s32 res;
        s32 speed;

        if (DISC_ARMOR_DISC_RING(entity) != NULL)
        {
            ring = DISC_ARMOR_DISC_RING(entity);
            ring->state = 2;
            DISC_ARMOR_DISC_RING(entity) = NULL;
        }
        if (EntityIsOutsideScreen(entity, 4))
            goto free_delete;
        res = Entity_Walk(entity, DISC_ARMOR_DISC_PHASE(entity), sDiscArmorWalkProfile + 12, 2);
        if (res & 0x40)
            DISC_ARMOR_DISC_PHASE(entity) = DISC_ARMOR_DISC_PHASE(entity) > 0 ? DISC_ARMOR_DISC_PHASE(entity) - 0x2000 : DISC_ARMOR_DISC_PHASE(entity) + 0x2000;
        else if (res & 0x20)
            DISC_ARMOR_DISC_PHASE(entity) = DISC_ARMOR_DISC_PHASE(entity) > 0 ? DISC_ARMOR_DISC_PHASE(entity) + 0x2000 : DISC_ARMOR_DISC_PHASE(entity) - 0x2000;
        else if (DISC_ARMOR_DISC_PHASE(entity) >= 0)
        {
            if ((DISC_ARMOR_DISC_PHASE(entity) += -0x100) <= 0)
            {
                DISC_ARMOR_DISC_PHASE(entity) = 0;
                entity->timer = 0x20;
                entity->state = 7;
            }
        }
        else
        {
            if ((DISC_ARMOR_DISC_PHASE(entity) += 0x100) > 0)
            {
                DISC_ARMOR_DISC_PHASE(entity) = 0;
                entity->timer = 0x20;
                entity->state = 7;
            }
        }
        speed = DISC_ARMOR_DISC_PHASE(entity);
        DISC_ARMOR_DISC_ANGLE(entity) += speed / 64;
        if (res & 3)
            DISC_ARMOR_DISC_PHASE(entity) = -speed / 2;
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
            DISC_ARMOR_DISC_ANGLE(entity) += -0x2F00;
        else
            DISC_ARMOR_DISC_ANGLE(entity) += 0x2F00;
    }
    DISC_ARMOR_DISC_LAST_DX(entity) = entity->posX.whole - oldX;
    DISC_ARMOR_DISC_LAST_DY(entity) = (struct EwramData_EntityData *)(entity->posY.whole - oldY);
    ObjAffine_Set(DISC_ARMOR_DISC_ANGLE(entity), 0x10000, 0x10000, 0, 0, entity->affineIndex);
}
