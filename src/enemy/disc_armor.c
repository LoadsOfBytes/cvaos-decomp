/*
 * Disc Armor: sparks, ring and disc, walk, throw, death, create, update, hits, ring drawing and disc contact damage.
 * Built without -f2003-patch (DiscArmor_UpdateWalk needs it off; the rest compiles the same either way). DiscArmor_OnHit passes two unused arguments to Entity_FaceTarget (cast marked at the call).
 */
#include "core.h"
#include "disc_armor_entity.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "gba.h"
#include "syscalls.h"
#include "enemy_status.h"
#include "bg_and_collision.h"
#include "gameplay.h"
#include "entity_hit.h"
#include "player_core.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "effects.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "sound.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, u8, void *);
extern void Entity_FaceTarget(struct EwramData_EntityData *entity);

void DiscArmorRing_Draw(struct EwramData_EntityData *);
void DiscArmorDisc_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void DiscArmor_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

/* from sound.h (not included: see the module notes) */

/* Disc Armor disc projectile (uses the Disc Armor graphics of EnemyDiscArmorCreate). */

/* State 0 loads graphics, starts the animation selected by unk_4F8 and sets
 * spriteLayer = 0x12 and scrollWithBg.  Every frame (including the first) it then
 * accelerates with 0x80000 and sets deleteRequested when the animation returns 3. */
void DiscArmorSpark_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sDiscArmorGfx, sDiscArmorPalette, 0, sDiscArmorSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        Anim_StartIndexed(entity, (u8)entity->ext.discArmorSpark.anim, 0, 0);
        entity->spriteLayer = 0x12;
        entity->scrollWithBg = 1;
        entity->state++;
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
    }
    if (Anim_Update(entity) == 3)
        entity->deleteRequested = 1;
}

/* Disc Armor disc ring. */

/**
 * @brief B14B4 | Disc Armor disc ring effect.  State 0 loads the disc graphics,
 * allocates an affine matrix (affineIndex, flag affine), sets the draw
 * callback DiscArmorRing_Draw, animation 7, spriteLayer = 0x14 and scrollWithBg; failure
 * deletes the entity.  State 2 frees the matrix and deletes the entity.
 * Otherwise the ring scale follows the distance unk_504 (capped at 0x600000,
 * divided by 3 and 32): unk_4FC/unk_500 become cos/sin of the angle unk_4F8
 * times the scale and the matrix is rotated/scaled accordingly.
 */
void DiscArmorRing_Update(struct EwramData_EntityData *entity)
{
    s32 dist, scale, y;

    switch (entity->state)
    {
    case 0:
        if (Entity_LoadGraphics(entity, sDiscArmorGfx, sDiscArmorPalette, 0, sDiscArmorSpriteSheet))
        {
            u16 matrix = ObjAffine_Alloc(1);

            if ((s16)matrix >= 0)
            {
                entity->drawFunc = DiscArmorRing_Draw;
                entity->affineIndex = matrix;
                entity->affine = 1;
                entity->spriteLayer = 0x14;
                Anim_StartIndexed(entity, 7, 0, 0);
                Anim_Update(entity);
                entity->scrollWithBg = 1;
                entity->state++;
                break;
            }
        }
        goto delete;
    case 2:
        if (entity->affine)
            ObjAffine_Free(entity->affineIndex);
    delete:
        EntityDelete(entity);
        return;
    }
    dist = entity->ext.discArmorRing.length;
    if (dist > 0x600000)
        dist = 0x600000;
    dist /= 3;
    scale = dist / 32;
    entity->ext.discArmorRing.stepX = Sin(entity->ext.discArmorRing.angle + 0x4000) << 5;
    y = Sin(entity->ext.discArmorRing.angle) << 5;
    entity->ext.discArmorRing.stepY = y;
    entity->ext.discArmorRing.stepX = (s32)entity->ext.discArmorRing.stepX / 0x10000 * scale;
    entity->ext.discArmorRing.stepY = y / 0x10000 * scale;
    ObjAffine_Set(entity->ext.discArmorRing.angle, scale, 0x10000, 0, 0, entity->affineIndex);
}

/* Disc Armor disc aiming. */
/* Integer part of a 16.16 value, rounded towards zero. */
#define INT_TOWARD_ZERO(v) ((v) < 0 ? -((-(v)) >> 16) : (v) >> 16)

/* Pixel difference pos - centre, with the 16.16 centre rounded towards zero. */
static inline s32 PixelDelta(s16 pos, s32 centre)
{
    if (centre < 0)
        return pos + ((-centre) >> 16);
    return pos - (centre >> 16);
}

/**
 * @brief B15EC | Disc Armor disc aiming: point `disc` from `owner`'s stored
 * centre (unk_4FC/unk_500, 16.16) towards `entity`.  unk_4F8 gets the angle,
 * unk_504 the distance (16.16 pixels) and the disc is moved onto the centre.
 * Returns that distance, or 0 without a disc.
 */
s32 DiscArmorRing_Aim(struct EwramData_EntityData *disc, struct EwramData_EntityData *entity,
                 struct EwramData_EntityData *owner)
{
    s32 dx, dy, distance;

    if (disc != NULL)
    {
        /* X via the inline helper, Y via the macro: this mix gives the
         * original register allocation. */
        disc->ext.discArmorRing.angle = ArcTan2(PixelDelta(entity->posX.part16.integer, owner->ext.discArmorHand.x),
                                           entity->posY.part16.integer - INT_TOWARD_ZERO(owner->ext.discArmorHand.y));
        dx = (entity->posX.whole - (s32)owner->ext.discArmorHand.x) >> 16;
        dx *= dx;
        dy = (entity->posY.whole - owner->ext.discArmorHand.y) >> 16;
        dy *= dy;
        distance = Sqrt(dx + dy) << 16;
        disc->ext.discArmorRing.length = distance;
        disc->posX.whole = owner->ext.discArmorHand.x;
        disc->posY.whole = owner->ext.discArmorHand.y;
        return distance;
    }
    return 0;
}

/* Disc Armor thrown disc. */

#define ABS_DIFF(a, b) ((a) - (b) < 0 ? (b) - (a) : (a) - (b))

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
    struct EwramData_EntityData *spark = EntityCreateInRange(0x49, 0xCF, DiscArmorSpark_Update);

    if (spark != NULL)
    {
        spark->ext.discArmorSpark.anim = type;
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
    struct EwramData_EntityData *owner = entity->ext.discArmorDisc.owner;
    struct EwramData_EntityData *ring = entity->ext.discArmorDisc.ring;
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
            if ((s16)owner->hp > 0 && !(ENEMY_STATUS(owner) & ENEMY_STATUS_PETRIFIED))
                goto check;
        }
        else
            owner->ext.discArmor.flags |= 2;
        if (entity->flipX)
            DISC_ARMOR_DISC_LAST_DX(entity) += -0x10000;
        else
            DISC_ARMOR_DISC_LAST_DX(entity) += 0x10000;
        if (entity->state == 5)
            DISC_ARMOR_DISC_LAST_DX(entity) = 0;
        entity->state = 6;
        entity->ext.discArmorDisc.phase = DISC_ARMOR_DISC_LAST_DX(entity);
        entity->velY.whole = (s32)DISC_ARMOR_DISC_LAST_DY(entity);
        entity->accelY.whole = 0x4000;
        entity->accelX.whole = 0;
        goto run;
    check:
        if (!(owner->ext.discArmor.flags & 1))
        {
            if (entity->affine)
                ObjAffine_Free(entity->affineIndex);
            if (entity->ext.discArmorDisc.ring != NULL)
                entity->ext.discArmorDisc.ring->state = 2;
            goto delete;
        }
    }
run:
    Hit_CheckWithPlayer(&hit, entity);
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
        ring = Entity_CreateObject(DiscArmorRing_Update);
        entity->ext.discArmorDisc.ring = ring;
        entity->ext.discArmorDisc.orbitAngle = 0x8000;
        {
            s32 turn = 0x80;

            entity->ext.discArmorDisc.turn = turn;
            if (entity->flipX)
            {
                entity->ext.discArmorDisc.orbitAngle = 0;
                entity->ext.discArmorDisc.turn = -turn;
            }
        }
        entity->posX.whole = owner->ext.discArmorHand.x;
        entity->posY.whole = owner->ext.discArmorHand.y;
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

        amp = INT_TOWARD_ZERO(Sin(entity->ext.discArmorDisc.phase) * 0x60);
        entity->ext.discArmorDisc.phase += 0x200;
        entity->ext.discArmorDisc.orbitAngle += entity->ext.discArmorDisc.turn;
        if (entity->ext.discArmorDisc.phase > 0x4000)
        {
            entity->ext.discArmorDisc.phase = 0x4000;
            entity->accelY.whole += 0x4000;
            if (entity->accelY.whole > 0x80000)
                entity->accelY.whole = 0x80000;
            entity->velY.whole += entity->accelY.whole;
        }
        else
            entity->velY.whole = Sin(entity->ext.discArmorDisc.orbitAngle) * amp;
        entity->velX.whole = amp * Sin(entity->ext.discArmorDisc.orbitAngle + 0x4000);
        entity->posX.whole = owner->ext.discArmorHand.x + entity->velX.whole;
        entity->posY.whole = owner->ext.discArmorHand.y + entity->velY.whole;
        xu = GetEntityRoomXPositionInteger(entity);
        y = GetEntityRoomYPositionInteger(entity);
        x = xu;
        push = Collision_GetPushRight(x - 16, y);
        if (push != 0)
        {
            entity->posX.whole += push << 16;
            entity->ext.discArmorDisc.phase = 0;
            entity->state = 2;
            break;
        }
        push3 = Collision_GetPushLeft(x + 16, y);
        if (push3 != 0)
        {
            entity->posX.whole += push3 << 16;
            entity->ext.discArmorDisc.phase = push;
            entity->state = 2;
        }
        else
        {
            push2 = PushOutOfFloor(entity, x, y + 16);
            if (push2 == 0)
                break;
            entity->velY.whole += push2 << 16;
            if (entity->ext.discArmorDisc.phase < 0x4000)
            {
                entity->accelY.whole = -0x30000;
                entity->ext.discArmorDisc.phase = push3;
            }
            entity->ext.discArmorDisc.phase = push3;
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
        entity->posX.whole = owner->ext.discArmorHand.x + entity->velX.whole;
        entity->posY.whole = owner->ext.discArmorHand.y + entity->velY.whole;
        xu = GetEntityRoomXPositionInteger(entity);
        yu = GetEntityRoomYPositionInteger(entity);
        if (PushOutOfFloor(entity, xu, yu + 16) == 0)
            break;
        entity->ext.discArmorDisc.phase = 0;
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
        entity->posX.whole = owner->ext.discArmorHand.x + entity->velX.whole;
        entity->posY.whole = owner->ext.discArmorHand.y + entity->velY.whole;
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
        if (entity->ext.discArmorDisc.phase < 0x40000)
            entity->ext.discArmorDisc.phase += 0x1000;
        res = Entity_Walk(entity, -entity->ext.discArmorDisc.phase, sDiscArmorWalkProfile + 12, 8);
        {
            s8 three = 3; /* the mask is a variable: and in QImode puts it first */
            if (res & three)
            {
                owner->ext.discArmor.flags |= 2;
                entity->state = 5;
            }
            asm("" :: "r"(three)); /* FAKEMATCH: extra ref so the mask outranks res (r2) */
            if ((res & 8) && !(u8)(entity->timer & three))
                SpawnSpark(entity, 4);
        }
        if (ABS_DIFF((s16)owner->posX.part16.integer, (s16)entity->posX.part16.integer) <= 0x2F)
        {
            owner->ext.discArmor.flags |= 2;
            entity->state = 5;
            entity->ext.discArmorDisc.orbitAngle = ArcTan2(
                PixelDelta(entity->posX.part16.integer, owner->ext.discArmorHand.x),
                ({
                    s32 d_ = PixelDelta(entity->posY.part16.integer, owner->ext.discArmorHand.y);
                    register s32 s_ asm("r0") = d_ << 16; /* FAKEMATCH */
                    asm("" : "+r"(s_)); /* FAKEMATCH: shift temp of the s16 argument in r0 */
                    s_ >> 16;
                }));
            dx = (entity->posX.whole - (s32)owner->ext.discArmorHand.x) >> 16;
            dx *= dx;
            dy = (entity->posY.whole - owner->ext.discArmorHand.y) >> 16;
            dy *= dy;
            entity->ext.discArmorDisc.radius = Sqrt(dx + dy) << 16;
            entity->ext.discArmorDisc.radius = entity->ext.discArmorDisc.radius * 2 / 3;
            entity->ext.discArmorDisc.turn = 0;
            entity->timer = 0;
            PlaySong(0x103);
        }
        break;
    }
    case 5:
    {
        s32 turn;

        entity->ext.discArmorDisc.radius += -0x10000;
        if (++entity->timer > 4)
        {
            amp = entity->timer - 4;

            if (amp <= 3)
                entity->ext.discArmorDisc.radius += Sin(amp << 12) << 3;
        }
        if (entity->ext.discArmorDisc.radius < 0x20000)
            entity->ext.discArmorDisc.radius = 0x20000;
        amp = INT_TOWARD_ZERO(entity->ext.discArmorDisc.radius);
        entity->velY.whole = Sin(entity->ext.discArmorDisc.orbitAngle) * amp;
        entity->velX.whole = amp * Sin(entity->ext.discArmorDisc.orbitAngle + 0x4000);
        entity->posX.whole = owner->ext.discArmorHand.x + entity->velX.whole;
        entity->posY.whole = owner->ext.discArmorHand.y + entity->velY.whole;
        amp = entity->ext.discArmorDisc.turn += -0x140;
        if (entity->flipX)
            amp = -amp;
        entity->ext.discArmorDisc.orbitAngle += amp;
        break;
    }
    case 6:
    {
        s32 res;
        s32 speed;

        if (entity->ext.discArmorDisc.ring != NULL)
        {
            ring = entity->ext.discArmorDisc.ring;
            ring->state = 2;
            entity->ext.discArmorDisc.ring = NULL;
        }
        if (EntityIsOutsideScreen(entity, 4))
            goto free_delete;
        res = Entity_Walk(entity, entity->ext.discArmorDisc.phase, sDiscArmorWalkProfile + 12, 2);
        if (res & 0x40)
            entity->ext.discArmorDisc.phase = entity->ext.discArmorDisc.phase > 0 ? entity->ext.discArmorDisc.phase - 0x2000 : entity->ext.discArmorDisc.phase + 0x2000;
        else if (res & 0x20)
            entity->ext.discArmorDisc.phase = entity->ext.discArmorDisc.phase > 0 ? entity->ext.discArmorDisc.phase + 0x2000 : entity->ext.discArmorDisc.phase - 0x2000;
        else if (entity->ext.discArmorDisc.phase >= 0)
        {
            if ((entity->ext.discArmorDisc.phase += -0x100) <= 0)
            {
                entity->ext.discArmorDisc.phase = 0;
                entity->timer = 0x20;
                entity->state = 7;
            }
        }
        else
        {
            if ((entity->ext.discArmorDisc.phase += 0x100) > 0)
            {
                entity->ext.discArmorDisc.phase = 0;
                entity->timer = 0x20;
                entity->state = 7;
            }
        }
        speed = entity->ext.discArmorDisc.phase;
        entity->ext.discArmorDisc.angle += speed / 64;
        if (res & 3)
            entity->ext.discArmorDisc.phase = -speed / 2;
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
            entity->ext.discArmorDisc.angle += -0x2F00;
        else
            entity->ext.discArmorDisc.angle += 0x2F00;
    }
    DISC_ARMOR_DISC_LAST_DX(entity) = entity->posX.whole - oldX;
    DISC_ARMOR_DISC_LAST_DY(entity) = (struct EwramData_EntityData *)(entity->posY.whole - oldY);
    ObjAffine_Set(entity->ext.discArmorDisc.angle, 0x10000, 0x10000, 0, 0, entity->affineIndex);
}

static inline s32 WalkSpeed(s32 frame, s32 base) { return (frame << 13) + base; }
void DiscArmor_UpdateWalk(struct EwramData_EntityData *entity)
{
    u8 frame;
    s32 speed;
    s32 result;
    s32 attack;
    s32 distance;

    if (entity->subState == 0)
    {
        if (entity->spawnParam0 == 0)
        {
            entity->subState = 1;
            Anim_StartIndexed(entity, 0, 0, 1);
        }
        else
        {
            entity->subState = 2;
            Anim_StartIndexed(entity, 1, 0, 1);
        }
        entity->timer = 0;
    }
    switch (entity->subState)
    {
    case 2:
        frame = entity->animStep % 9;
        speed = WalkSpeed(frame, 0x1000);
        if (frame > 6)
            speed = 0;
        result = Entity_Walk(entity, speed, sDiscArmorWalkProfile, 20);
        if (*(u16 *)&entity->animStep == 0x11 && entity->timer++ > entity->spawnParam0)
        {
            entity->timer = 0;
            entity->flipX ^= 1;
        }
        if ((result & 0x1B) != 8)
            entity->flipX ^= 1;
        if (entity->animTimer != 0)
            return;
        if (entity->animStep != 0x11)
            return;
    case 1:
        if (Entity_GetTargetDistance(entity) > 0x4F)
        {
            if ((entity->flipX && (s8)Entity_GetTargetDirection(entity) == 1)
                || (!entity->flipX && (s8)Entity_GetTargetDirection(entity) == -1))
            {
                distance = Entity_GetTargetDistance(entity);
                attack = 1;
                if (distance <= 0x8B)
                    goto test;
            }
            attack = 0;
        test:
            if (!attack)
                return;
        }
        entity->state = 1;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/* Disc Armor throw. */

/**
 * @brief B205C | Disc Armor throw (attack state).  State 0 faces the player,
 * starts animation 2 and sets unk_4F8 = 1 and the hitbox-like pair
 * unk_504/unk_506 = -20/-36.  State 1 spawns the disc DiscArmorDisc_Update at
 * animation frame 0x10C (owner in unk_4F4, same facing, attack * 1.25) and
 * switches to animation 3 / state 2 once unk_4F8 bit 1 is set (by the disc).
 * State 2 eases the pair back to -10/-16 and, when the animation reports
 * flag 4, clears unk_4F8 and the states and faces the player again.
 */
void DiscArmor_UpdateThrow(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *disc;

    switch (entity->subState)
    {
    case 0:
        Entity_FaceTarget(entity);
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->subState = 1;
        entity->ext.discArmor.flags = 1;
        entity->ext.discArmorHand.offsetX = -20;
        entity->ext.discArmorHand.offsetY = -36;
        break;
    case 1:
        if (*(u16 *)&entity->animStep == 0x10C)
        {
            disc = Entity_CreateObject(DiscArmorDisc_Update);
            if (disc != NULL)
            {
                disc->ext.discArmorDisc.owner = entity;
                disc->flipX = entity->flipX;
                disc->attack = entity->attack + (entity->attack >> 2);
            }
        }
        if (entity->ext.discArmor.flags & 2)
        {
            Anim_StartIndexed(entity, 3, 0, 0);
            entity->subState = 2;
        }
        break;
    case 2:
        if (entity->ext.discArmorHand.offsetX < -10)
            entity->ext.discArmorHand.offsetX++;
        if (entity->ext.discArmorHand.offsetY < -16)
            entity->ext.discArmorHand.offsetY++;
        if (entity->animFlags & 4)
        {
            entity->ext.discArmor.flags = 0;
            entity->state = 0;
            entity->stage = 0;
            entity->subState = 0;
            Entity_FaceTarget(entity);
        }
        break;
    }
}

/* Disc Armor death. */

/**
 * @brief B2148 | Disc Armor death, a variant of the Axe Armor death
 * AxeArmor_UpdateDeath: state 0 starts animation 8 and an upward velocity; state 1
 * emits rising debris (type 1/7) on even frames and a sound plus effect 16/7
 * every eighth frame, without a frame limit.  Returns 1 once the animation
 * reports flag 4.
 */
s32 DiscArmor_UpdateDeath(struct EwramData_EntityData *entity)
{
    u8 state = entity->subState;
    switch (state)
    {
    case 0:
        Anim_StartIndexed(entity, 8, 0, 0);
        entity->subState = 1;
        entity->velY.whole = -0x280000;
        entity->velX.whole = state;
        PlaySong(0x6F);
        break;
    case 1:
    {
        u32 timer;
        s32 y, phase, x;
        timer = entity->timer + 1;
        entity->timer = timer;
        y = entity->posY.whole + entity->velY.whole;
        entity->velY.whole += 0x12000;
        phase = entity->velX.whole;
        entity->velX.whole = phase + 0x5000;
        if ((timer & state) == 0)
        {
            struct EwramData_EntityData *debris;
            s32 offset = Sin(phase + 0x9000);
            x = entity->posX.whole + (offset << 4) + RandomSigned(0x40000);
            debris = Effect_SpawnShared(x, y, 1, 7);
            if (debris != NULL)
            {
                debris->velY.whole = 0x8000;
                debris->accelY.whole = -0x1000;
            }
        }
        if ((entity->timer & 7) == 0)
        {
            PlaySong(0x6E);
            x = entity->posX.whole + (RandomSigned(16) << 16);
            y += RandomSigned(4) << 16;
            Effect_SpawnShared(x, y, 16, 7);
        }
        if (entity->animFlags & 4)
            return 1;
        break;
    }
    }
    return 0;
}

void EnemyDiscArmorCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sDiscArmorGfx, sDiscArmorPalette, 0, sDiscArmorSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x12;
    Anim_StartIndexed(entity, 1, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        return;
    }
    Entity_EnableBoxes(entity, Enemy_OnContact, DiscArmor_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->posY.part16.integer++;
    entity->scrollWithBg = 1;
    entity->state = 0;
}

void EnemyDiscArmorUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    s32 y = entity->ext.discArmorHand.offsetY;
    s32 x = (s16)entity->ext.discArmorHand.offsetX;
    if (entity->flipX) x = (u16)-x;
    entity->ext.discArmorHand.x = entity->posX.whole + (x << 16);
    entity->ext.discArmorHand.y = entity->posY.whole + (y << 16);
    if (Enemy_PreUpdate(entity)) return;
    switch (entity->state)
    {
    case 0: DiscArmor_UpdateWalk(entity); break;
    case 1: DiscArmor_UpdateThrow(entity); break;
    case 2:
        if (DiscArmor_UpdateDeath(entity))
        {
            EntityDelete(entity);
            return;
        }
        break;
    case 3: Anim_Update(entity); return;
    }
    if (entity->state != 2) Hit_CheckWithPlayer(&result, entity);
    Entity_AnimateWithBoxes(entity);
}

/* DiscArmor_OnHit. */

/**
 * @brief B2370 | Hit: Entity_FaceTarget in state 0; when killed (Enemy_TakeDamage),
 * Enemy_DropRewards, palette into 0x53E bits 3-6 and state 2.
 */
void DiscArmor_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    if (entity->state == 0)
        ((void (*)(struct EwramData_EntityData *, s32, u16 *))Entity_FaceTarget)(entity, attacker, hit); /* FAKEMATCH: called with two unused arguments */

    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/* DiscArmorRing_Draw: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */

/**
 * @brief B23C4 | Like Entity_DrawWithVelocityCopies, but 3 copies with the step unk_4FC / unk_500.
 */
void DiscArmorRing_Draw(struct EwramData_EntityData *entity)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;
    s32 i;

    entity->posX.whole = oldX + (s32)entity->ext.discArmorRing.stepX / 2;
    entity->posY.whole = oldY + entity->ext.discArmorRing.stepY / 2;
    for (i = 3; i != 0; i--)
    {
        Sprite_Draw(entity);
        entity->posX.whole += (s32)entity->ext.discArmorRing.stepX;
        entity->posY.whole += entity->ext.discArmorRing.stepY;
    }
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   DiscArmorDisc_DamagePlayerOnContact = MinotaurAxe_DamagePlayerOnContact */

/**
 * @brief B240C | Same as MinotaurAxe_DamagePlayerOnContact.
 */
void DiscArmorDisc_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 1;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}
