/*
 * Balore (boss): body, eyes, parts, hands and their hit boxes, overlays,
 * intro and effects. One translation unit for the consecutive ROM range of
 * its code (0x080B3F4C-0x080B6C3B); the functions keep their ROM order.
 */
#include "player_core.h"
#include "balore_entity.h"
#include "structs/ewram.h"
#include "graphics.h"
#include "core.h"
#include "sound.h"
#include "gameplay.h"
#include "map_flags.h"
#include "entity_util.h"
#include "music.h"
#include "structs/entity_render_flags.h"
#include "player_level.h"
#include "in_game.h"
#include "syscalls.h"
#include "effects.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "palette.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "entity_hit.h"
#include "boss_defeat.h"


/*
 * Balore_UpdateDeath calls ObjPalette_LoadRange with three arguments (the slot
 * is left in r3); the declaration in graphics.h has two. Same symbol under a
 * second name. (BaloreEye_Update uses ArcTan2Wide from syscalls.h.)
 */
extern void ObjPalette_LoadRange3(s32 palette, u16 first, u16 count) asm("ObjPalette_LoadRange");

void BaloreHandBHitBox_Update(struct EwramData_EntityData *entity);
void BaloreHandB_Update(struct EwramData_EntityData *entity);
void Balore_CreateIntro(struct EwramData_EntityData *entity);
void BaloreIntro_Update(struct EwramData_EntityData *entity);
void Balore_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
void BaloreSpark_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void BaloreHandHitBox_OnHit(struct EwramData_EntityData *entity, s32 attacker);
void BaloreEmitter_Draw(struct EwramData_EntityData *entity);

/**
 * @brief B3F4C | Damage request at the entity's position: strength 90 / type 3
 * with enemyStatus byte 1 bit 0, else 80 or 55 / type 1 (bit 1).  Type 3 also
 * pushes the player (Player_SetKnockbackVelocity) by +-0x60000 along the entity's velX
 * direction and -0x20000 vertically.
 */
void BaloreHandHitBox_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage hit;

    hit.attributes = 0;
    hit.unused = 0;
    if (BALORE_HAND_HIT_FLAGS(entity) & 1)
    {
        hit.attack = 90;
        hit.reaction = 3;
    }
    else if (BALORE_HAND_HIT_FLAGS(entity) & 2)
    {
        hit.attack = 80;
        hit.reaction = 1;
    }
    else
    {
        hit.attack = 55;
        hit.reaction = 1;
    }
    hit.sourceX = entity->posX.whole;
    hit.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&hit);
    if (hit.reaction == 3)
    {
        s32 vx = 0x60000;
        if (entity->velX.whole < 0)
            vx = -vx;
        Player_SetKnockbackVelocity(vx, -0x20000);
    }
}

/**
 * @brief B3FC4 | Sprite-data setup used by BaloreOverlay_Update: allocates three
 * blocks (unk_4FC 4 words, unk_500 4 words, unk_504 0x20 words), links them
 * (A -> B -> C) and registers them with Sprite_Attach using byte 0x17 and
 * 0x2C of the template unk_4F8; then animation 0, draw callback
 * Sprite_DrawEntity.  Returns 1 if an allocation failed, else 0.
 */
s32 BaloreOverlay_SetupSprite(struct EwramData_EntityData *entity)
{
    u8 *info = (u8 *)entity->ext.baloreOverlay.info;
    struct SpriteSheet *a;
    struct SpriteFrame *b;
    void *c;

    if ((entity->ext.baloreOverlay.sheet = (s32)Heap_Alloc(4)) == 0)
        return 1;
    if ((entity->ext.baloreOverlay.frame = (s32)Heap_Alloc(4)) == 0)
        return 1;
    c = Heap_Alloc(0x20);
    entity->ext.baloreOverlay.parts = (s32)c;
    if (c == NULL)
        return 1;
    a = (struct SpriteSheet *)entity->ext.baloreOverlay.sheet;
    b = (struct SpriteFrame *)entity->ext.baloreOverlay.frame;
    a->frameCount = 1;
    a->scriptCount = 0;
    a->frames = b;
    a->scripts = 0;
    a->scriptTable = 0;
    b->originX = 0;
    b->originY = 0;
    b->boxCount = 0;
    b->partCount = 0;
    b->boxes = 0;
    b->parts = c;
    Sprite_Attach(entity, a, (s8)info[0x17], info[0x2C]);
    Anim_StartIndexed(entity, 0, 2, 1);
    entity->drawFunc = Sprite_DrawEntity;
    Anim_Update(entity);
    return 0;
}

/* Copy the parent's current frame parts into this entity's part list (mirrored when ext.baloreOverlay.mirror is set) and pick layer 0x1D/0x1E. */
void BaloreOverlay_UpdateSprite(struct EwramData_EntityData *entity)
{
    s32 count;
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    u8 idx_ = parent->animFrame;
    struct SpriteFrame *frames = sBaloreSpriteSheet.frames;
    struct SpritePart *src = frames[idx_].parts;
    struct SpriteFrame *frame = (struct SpriteFrame *)entity->ext.baloreOverlay.frame;
    struct SpritePart *dst = (struct SpritePart *)entity->ext.baloreOverlay.parts;
    s32 i;
    
    if ((s8)parent->ext.balore.eyeAnim <= 3)
    {
        if (entity->ext.baloreOverlay.mirror != 0)
            entity->spriteLayer = 0x1E;
        else
            entity->spriteLayer = 0x1D;
    }
    else
    {
        if (entity->ext.baloreOverlay.mirror != 0)
            entity->spriteLayer = 0x1D;
        else
            entity->spriteLayer = 0x1E;
    }
    count = 0;
    if (entity->ext.baloreOverlay.mirror != 0)
    {
        for (i = frames->partCount - 1; i >= 0; src++, i--)
        {
            if (src->flip != 0)
            {
                *dst = *src;
                dst->x = -dst->x - dst->width;
                dst++;
                count++;
            }
        }
    }
    else
    {
        for (i = frames->partCount - 1; i >= 0; src++, i--)
        {
            if (src->flip == 0)
            {
                *dst = *src;
                dst++;
                count++;
            }
        }
    }
    frame->partCount = count;
}

/* Affine overlay following its parent. */
/**
 * @brief B4114 | Overlay that follows its parent unk_4F8.  State 0 sets up
 * the sprite data (BaloreOverlay_SetupSprite) and an affine matrix (affineIndex, flag
 * affine); on failure the entity is deleted.  State 1 runs BaloreOverlay_UpdateSprite,
 * copies the parent's position and palette bits palette.  Bit 2 of the
 * parent's unk_502 forces state 2, which frees the matrix and deletes the
 * entity.  Each living frame the matrix gets the parent's angle unk_500 and
 * scale unk_504 (both axes) with ext.baloreOverlay.mirror.
 */
void BaloreOverlay_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);

    if (parent->ext.balore.flags & 4)
        entity->state = 2;
    switch (entity->state)
    {
    case 0:
        if (BaloreOverlay_SetupSprite(entity) == 0)
        {
            u16 matrix = ObjAffine_Alloc(1);

            if ((s16)matrix >= 0)
            {
                entity->affineIndex = matrix;
                entity->affine = 1;
                entity->state = 1;
                goto active;
            }
        }
        EntityDelete(entity);
        return;
    case 1:
    active:
        BaloreOverlay_UpdateSprite(entity);
        entity->posX.whole = parent->posX.whole;
        entity->posY.whole = parent->posY.whole;
        entity->palette = parent->palette;
        break;
    case 2:
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
        return;
    }
    ObjAffine_Set(parent->ext.balore.angle, parent->ext.balore.scale, parent->ext.balore.scale,
                 entity->ext.baloreOverlay.mirror, 0, entity->affineIndex);
}

/* called with int arguments here (no re-extension of the s16 values) */

#define FIXED_TO_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)

/**
 * @brief B41E0 | Balore part that follows the player: graphics
 * sBaloreSpriteSheet with the owner's tiles and palette, palette slot 0x1F
 * (state 3 when time is frozen); in state 1 it points from the owner
 * towards the player (ArcTan2 of the pixel distance, 32 / 16 px offset)
 * and is placed 3 * cos / 2 * sin + 2 px times the owner's unk_504 scale
 * from it; deleted with the owner's unk_502 bit 2. Animation 0x24 +
 * sBaloreEyeAnimOffsets[owner unk_4F6].
 */
void BaloreEye_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.balorePart.body;
    s32 dy = 0;
    s32 dx = 0;
    s32 o;
    s16 angle;

    if (owner->ext.balore.flags & 4)
        entity->state = 5;
    switch (entity->state)
    {
    case 0:
        Sprite_Attach(entity, &sBaloreSpriteSheet, (s8)owner->ext.balore.gfx, owner->basePalette);
        entity->drawFunc = Sprite_DrawEntity;
        entity->spriteLayer = 0x1F;
        if (gEwramData->gameState.bestiaryPreview)
        {
            entity->state = 3;
            break;
        }
        entity->state = 1;
    case 1:
        if (owner->state == 3)
            entity->state = 2;
        if (owner->ext.balore.flags & 2)
            entity->state = 4;
        o = (s16)owner->posX.part16.integer;
        dx = (s16)(FIXED_TO_INT(Target_GetX(owner)) - o - 0x20);
        o = owner->posY.part16.integer;
        angle = ArcTan2Wide(dx, (s16)(FIXED_TO_INT(Target_GetY(owner)) - o - 0x10));
        dx = Sin(angle + 0x4000) * 3;
        dy = Sin(angle) * 2 + 0x20000;
        dx = FIXED_TO_INT(dx) * owner->ext.balore.scale;
        dy = FIXED_TO_INT(dy) * owner->ext.balore.scale;
        break;
    case 2:
    case 3:
    case 4:
        break;
    case 5:
        EntityDelete(entity);
        return;
    }
    entity->posX.whole = owner->posX.whole + dx;
    entity->posY.whole = owner->posY.whole + dy;
    Anim_StartIndexed(entity, sBaloreEyeAnimOffsets[(s8)owner->ext.balore.eyeAnim] + 0x24, 2, 0);
    Anim_Update(entity);
}

void BalorePart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.balorePart.body;

    if (owner->ext.balore.flags & 2)
        entity->state = 6;
    switch (entity->state)
    {
    case 0:
        entity->flipX = 1;
        Sprite_Attach(entity, &sBaloreSpriteSheet, (s8)owner->ext.balore.gfx, owner->basePalette);
        entity->drawFunc = Sprite_DrawEntity;
        entity->spriteLayer = 0x1C;
        entity->ext.balorePart.anim = 0x1B;
        entity->state = 1;
    case 1:
        if (owner->state == 3)
            entity->state = 2;
        break;
    case 2:
        entity->timer = 0x20;
        entity->state = 3;
    case 3:
        if (entity->timer & 2)
            entity->ext.balorePart.anim = 0x1B;
        else
            entity->ext.balorePart.anim = 0x12;
        if (entity->timer != 0)
            entity->timer--;
        else
        {
            entity->ext.balorePart.anim = 0x12;
            entity->timer = 8;
            entity->state = 4;
        }
        break;
    case 4:
        if (entity->timer & 1)
            entity->hidden = 1;
        else
            entity->hidden = 0;
        if (entity->timer != 0)
            entity->timer--;
        else
        {
            entity->hidden = 1;
            entity->state = 5;
        }
        break;
    case 5:
        break;
    case 6:
        EntityDelete(entity);
        return;
    }
    entity->posX.whole = owner->posX.whole;
    entity->posY.whole = owner->posY.whole;
    entity->palette = owner->palette;
    Anim_StartIndexed(entity, entity->ext.balorePart.anim + sBalorePartAnimOffsets[(s8)owner->ext.balore.eyeAnim], 2, 0);
    Anim_Update(entity);
}

/**
 * @brief B44C4 | Balore part (created by EnemyBaloreCreate, owner in
 * unk_4FC): graphics sBaloreSpriteSheet with the owner's tiles and palette,
 * palette slot 0x1C; shown while the owner is not in state 3, hidden for
 * 16 frames when the owner's unk_502 bit 0 is set, deleted with bit 1;
 * follows the owner's position and blending, animation 9 +
 * sBaloreEyeAnimOffsets[owner unk_4F6].
 */
void BaloreBodyPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.balorePart.body;

    if (owner->ext.balore.flags & 2)
        entity->state = 4;
    switch (entity->state)
    {
    case 0:
        Sprite_Attach(entity, &sBaloreSpriteSheet, (s8)owner->ext.balore.gfx, owner->basePalette);
        entity->drawFunc = Sprite_DrawEntity;
        entity->spriteLayer = 0x1C;
        entity->state = 1;
    case 1:
        if (owner->state == 3)
        {
            entity->hidden = 0;
            entity->state = 3;
            break;
        }
        entity->hidden = 1;
        if (owner->ext.balore.flags & 1)
        {
            entity->hidden = 0;
            entity->timer = 0x10;
            entity->state = 2;
        }
        break;
    case 2:
        if (entity->timer != 0)
            entity->timer--;
        else
            entity->state = 1;
        break;
    case 4:
        EntityDelete(entity);
        return;
    }
    entity->posX.whole = owner->posX.whole;
    entity->posY.whole = owner->posY.whole;
    entity->palette = owner->palette;
    Anim_StartIndexed(entity, sBaloreEyeAnimOffsets[(s8)owner->ext.balore.eyeAnim] + 9, 2, 0);
    Anim_Update(entity);
}

/**
 * @brief B45C0 | Graphics setup: palette slot from ObjGfx_LoadResource (0 on
 * failure), sprite set sBalorePalette variant 7 (0 with the bestiaryPreview mode
 * flag) into unk_4F8 byte 0 and basePalette, slot into unk_4FB, registers
 * sBaloreSpriteSheet with Sprite_Attach, draw callback Sprite_DrawEntity, scrollWithBg.
 * Same shape as Object21_LoadGraphics.
 */
u8 Balore_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 slot = ObjGfx_LoadResource(sBaloreGfx, 0);

    if (slot < 0)
        return 0;
    if (!gEwramData->gameState.bestiaryPreview)
        entity->ext.balore.palette = ObjPalette_FindOrLoadVariant(sBalorePalette, 7);
    else
        entity->ext.balore.palette = ObjPalette_FindOrLoadVariant(sBalorePalette, 0);
    entity->ext.balore.gfx = slot;
    entity->basePalette = entity->ext.balore.palette;
    Sprite_Attach(entity, &sBaloreSpriteSheet, slot, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    entity->scrollWithBg = 1;
    return 1;
}

/* the field is typed as a pointer in the shared union */
#define SPEED(e) (*(s32 *)&(e)->spawnParam0)

/**
 * @brief B4640 | Step the speed spawnParam0 and the value unk_4F4 towards the
 * targets of @p p (clamped, never overshooting), turn the angle unk_500 by
 * turnStep towards targetAngle and aim the velocity at the point
 * (unk_508 + offsetX, unk_50A + offsetY); offsetX and targetAngle are
 * mirrored when the entity is not flipX.  Moves the entity (EntityMove) and
 * returns 0 while on the way; returns 1 without moving once the point is
 * within reach.
 * Generic steering code, but it lies inside Balore's ROM range and only
 * Balore's hands call it, so it stays in this file.
 */
s32 BaloreHand_SteerToPoint(struct EwramData_EntityData *entity, const struct BaloreFlightSteeringParameters *p)
{
    s32 target, rate;
    s16 dx, dy, angle;
    u16 x, y;

    target = p->targetSpeed;
    rate = p->stepSpeed;
    if (SPEED(entity) != target)
    {
        if (SPEED(entity) > target)
        {
            if ((SPEED(entity) -= rate) < target)
                SPEED(entity) = target;
        }
        else if (SPEED(entity) < target)
        {
            if ((SPEED(entity) += rate) > target)
                SPEED(entity) = target;
        }
    }
    target = p->targetRadius;
    rate = p->radiusStep;
    if (entity->ext.baloreHand.armLength != target)
    {
        if (entity->ext.baloreHand.armLength > target)
        {
            if ((entity->ext.baloreHand.armLength -= rate) < target)
                entity->ext.baloreHand.armLength = target;
        }
        else if (entity->ext.baloreHand.armLength < target)
        {
            if ((entity->ext.baloreHand.armLength += rate) > target)
                entity->ext.baloreHand.armLength = target;
        }
    }
    dx = p->offsetX;
    dy = p->offsetY;
    angle = p->targetAngle;
    if (!entity->flipX)
    {
        dx = -dx;
        angle = -angle;
    }
    /* separate s16 offsets and u16 results decide the register allocation */
    x = dx + entity->ext.baloreHand.homeX;
    y = dy + entity->ext.baloreHand.homeY;
    Angle_TurnTowards((u16 *)&entity->ext.baloreHand.armAngle, p->turnStep, angle);
    if (Entity_AimVelocityAtPoint(entity, x << 16, y << 16, SPEED(entity)) != 0)
        return 1;
    EntityMove(entity);
    return 0;
}

/* Steering speed passed on to BaloreHand_SteerToPoint. */
#define STEER_SPEED(e) ENTITY_PTR_514(e)

/**
 * @brief B46FC | Palette fade-in step: state 0 sets spawnParam0 = 0x40000 and a
 * 0x40-frame counter in timer.  While the counter runs, palette basePalette
 * (slot basePalette - 10 in sharedObjPaletteVariants/sharedObjPaletteSources) gets brightness level
 * counter / 8 + 8 via ObjPalette_LoadRange whenever the level changes.  Every frame
 * then continues with BaloreHand_SteerToPoint(entity, sBaloreHandSteeringParams1).
 */
s32 BaloreHand_FadeIn(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        STEER_SPEED(entity) = (struct EwramData_EntityData *)0x40000;
        entity->timer = 0x40;
        entity->subState = 1;
    case 1:
        break;
    default:
        return 0;
    }
    if (entity->timer != 0)
    {
        u8 pal, idx;
        u16 level;

        entity->timer--;
        pal = entity->basePalette;
        /* the original reloads the decremented counter */
        level = (*(volatile u8 *)&entity->timer >> 3) + 8; /* FAKEMATCH: volatile read forces the reload of the original */
        idx = pal - 10;
        if (gEwramData->gameState.sharedObjPaletteVariants[idx] != level)
        {
            gEwramData->gameState.sharedObjPaletteVariants[idx] = level;
            ObjPalette_LoadRange((u8 *)gEwramData->gameState.sharedObjPaletteSources[idx], level, 1, pal);
        }
    }
    return BaloreHand_SteerToPoint(entity, sBaloreHandSteeringParams1);
}

/* Flight state machine on BaloreHand_SteerToPoint. */
/* enemyStatus used as bytes by this entity */
struct BaloreHandAttackBytes {
    u8 animationIndex;
    u8 flags;
};
#define UNK_51C(e) ((struct BaloreHandAttackBytes *)&(e)->enemyStatus)

/**
 * @brief B4780 | Flight phase built on BaloreHand_SteerToPoint.  State 0 sets speed
 * 0x30000 and a 0x20-frame counter (timer), whose value / 16 + 0x2E goes
 * to byte 0 of enemyStatus while it runs.  State 1 flies with sBaloreHandSteeringParams2;
 * on arrival speed 0x54000, sound 0x86, bit 0 of byte 1 of enemyStatus, and
 * randomly (1/2) state 3 with a 0x2F-frame counter or state 2.  States 2
 * and 3 fly with sBaloreHandSwipeParams / sBaloreHandSteeringParams3; on arrival the bit is
 * cleared and 1 is returned, otherwise 0.
 */
s32 BaloreHand_UpdateSwipe(struct EwramData_EntityData *entity)
{
    const struct BaloreFlightSteeringParameters *params;

    switch (entity->subState)
    {
    case 0:
        SPEED(entity) = 0x30000;
        entity->subState = 1;
        entity->timer = 0x20;
    case 1:
        if (entity->timer != 0)
        {
            entity->timer--;
            UNK_51C(entity)->animationIndex = (*(volatile u8 *)&entity->timer >> 4) + 0x2E; /* FAKEMATCH: volatile read forces the reload of the original */
        }
        if (BaloreHand_SteerToPoint(entity, sBaloreHandSteeringParams2))
        {
            SPEED(entity) = 0x54000;
            PlaySong(0x86);
            if ((RandomNumberGenerator2() & 0x3F) < 0x20)
            {
                entity->subState = 3;
                entity->timer = 0x2F;
            }
            else
                entity->subState = 2;
            UNK_51C(entity)->flags |= 1;
        }
        break;
    case 2:
        if (BaloreHand_SteerToPoint(entity, sBaloreHandSwipeParams))
        {
            UNK_51C(entity)->flags &= 0xFE;
            return 1;
        }
        break;
    case 3:
        if (entity->timer != 0)
        {
            entity->timer--;
            UNK_51C(entity)->animationIndex = (*(volatile u8 *)&entity->timer >> 4) + 0x2E; /* FAKEMATCH: volatile read forces the reload of the original */
        }
        params = sBaloreHandSteeringParams3;
        if (BaloreHand_SteerToPoint(entity, params))
        {
            UNK_51C(entity)->flags &= 0xFE;
            return 1;
        }
        break;
    }
    return 0;
}

#define FLIGHT(e) ((e)->ext.baloreHand.steer)
#define UNK_51C_BYTE(e, i) (((u8 *)&(e)->enemyStatus)[i])

/* Four-stage flight attack: approach, turn, dive, then emit 16 sparks.
 * Return one when the attack completes. */
s32 BaloreHand_UpdateDive(struct EwramData_EntityData *entity)
{
    struct BaloreFlightSteeringParameters *p = FLIGHT(entity);
    s32 dx;
    s32 i;
    struct EwramData_EntityData *spark;

    switch (entity->subState)
    {
    case 0:
        SPEED(entity) = 0x30000;
        UNK_51C_BYTE(entity, 0) = 0x2E;
        dx = Target_GetX(entity) - entity->posX.whole;
        if (!entity->flipX)
            dx = -dx;
        p->offsetX = (dx < 0 ? -(-dx >> 16) : dx >> 16) - 0x20;
        if ((s16)p->offsetX < 0)
            p->offsetX = 0;
        else if ((s16)p->offsetX > 0x60)
            p->offsetX = 0x60;
        p->offsetY = -0x10;
        p->targetAngle = 0xF000;
        p->turnStep = 0x180;
        p->targetRadius = 0x4C0000;
        p->radiusStep = 0x20000;
        p->targetSpeed = 0x8000;
        p->stepSpeed = 0x1000;
        entity->subState = 1;
    case 1:
        if (BaloreHand_SteerToPoint(entity, p) == 0)
            break;
        SPEED(entity) = 0x8000;
        p->offsetY = -0x20;
        p->targetAngle = 0xE000;
        p->turnStep = 0x200;
        p->targetSpeed = 0x8000;
        entity->subState = 2;
        break;
    case 2:
        if (BaloreHand_SteerToPoint(entity, p) == 0)
            break;
        SPEED(entity) = 0x18000;
        p->offsetY = 0x28;
        p->targetAngle = 0x800;
        p->turnStep = 0x200;
        p->targetSpeed = 0x50000;
        p->stepSpeed = 0x4000;
        /* Set bit 1 while preserving the remaining flight flags. */
        UNK_51C_BYTE(entity, 1) = (UNK_51C_BYTE(entity, 1) & ~2) + 2;
        PlaySong(0x86);
        entity->subState = 3;
        break;
    case 3:
        if (BaloreHand_SteerToPoint(entity, p) == 0)
            break;
        dx = 0x460000;
        if (!entity->flipX)
            dx = -dx;
        dx += entity->posX.whole;
        for (i = 16; i != 0; i--)
        {
            spark = Effect_CreateSharedFrame(dx + (RandomSigned(0x10) << 16), entity->posY.whole + 0x1E0000, (RandomNumberGenerator() & 1) + 0x40, 6);
            if (spark != NULL)
            {
                spark->velY.whole = -0x30000 - ((RandomNumberGenerator() & 0x3F) << 12);
                spark->accelY.whole = 0x3000;
                spark->velX.whole = RandomSigned(0x40) << 11;
            }
        }
        PlaySong(0x80);
        UNK_51C_BYTE(entity, 1) &= ~2;
        ScreenShake_Request(0, 8, 3, 7);
        return 1;
    }
    return 0;
}

#define HIT_FLAGS(e) (BALORE_HAND_HIT_FLAGS((e)))
#define TO_PIXELS(x) ((x) < 0 ? -(-(x) >> 16) : (x) >> 16)

/* Balore hand's attached collision part: follows its angle and extension. */
void BaloreHandHitBox_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    struct EwramData_EntityData *body = entity->ext.balorePart.body;
    struct EwramData_EntityData *hand = entity->ext.baloreHitBox.hand;
    u16 matrix;
    s32 angle, radius, x, y;
    u8 flags;
    u8 state;
    if (entity->state != 0 && hand->state == 10)
        entity->state = 3;
    entity->accelX.whole = entity->posX.whole;
    entity->accelY.whole = entity->posY.whole;
    switch (entity->state)
    {
    case 0:
        matrix = ObjAffine_Alloc(1);
        if ((s16)matrix < 0)
        {
            EntityDelete(entity);
            return;
        }
        entity->affineIndex = matrix;
        entity->affine = 1;
        entity->spriteLayer = 0x19;
        Sprite_Attach(entity, &sBaloreSpriteSheet, (s8)body->ext.balore.gfx, hand->basePalette);
        Anim_StartIndexed(entity, 0x30, 2, 0);
        Anim_Update(entity);
        entity->drawFunc = Sprite_DrawEntity;
        Hit_Enable(entity, 8, BaloreHandHitBox_DamagePlayerOnContact, BaloreHandHitBox_OnHit);
        HitBox_SetAttackRaw(entity, 0x2828ECEC);
        HitBox_SetBodyRaw(entity, 0x2828ECEC);
        entity->state = 1;
        break;
    case 1:
        if (hand->state == 7)
        {
            entity->hit.flags |= 2;
            entity->state = 2;
        }
        break;
    case 2:
        state = hand->state;
        if (state == 1)
        {
            entity->hit.flags &= 0xFD;
            entity->state = state;
        }
        break;
    case 3:
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
        return;
    }
    entity->palette = hand->palette;
    entity->hidden = hand->hidden;
    flags = HIT_FLAGS(hand);
    HIT_FLAGS(entity) = flags;
    if (entity->hidden) return;
    if (hand->flipX)
        angle = hand->ext.baloreHand.armAngle;
    else
        angle = hand->ext.baloreHand.armAngle + 0x8000;
    radius = TO_PIXELS(hand->ext.baloreHand.armLength);
    x = radius * Sin(angle + 0x4000);
    entity->posX.whole = hand->posX.whole + x;
    y = Sin(angle) * radius;
    entity->posY.whole = hand->posY.whole + y;
    entity->velX.whole = entity->posX.whole - entity->accelX.whole;
    entity->velY.whole = entity->posY.whole - entity->accelY.whole;
    if (HIT_FLAGS(entity) & 4) Player_QueueEntity(entity, 3);
    Hit_CheckWithPlayer(&result, entity);
    ObjAffine_Set(hand->ext.baloreHand.armAngle, 0x10000, 0x10000, hand->flipX, 0, entity->affineIndex);
    if (entity->animScriptIndex != BALORE_HAND_ANIM(hand))
        Anim_StartIndexed(entity, BALORE_HAND_ANIM(hand), 2, 0);
    Anim_Update(entity);
}

/* Balore's hand: extension/rotation phases and coordinated retreat (B4C7C). */
/* The two low bytes of enemyStatus: animation byte and flag byte. */
struct HandBytes
{
    u8 anim;
    u8 flags;
};
#define FLAGS(e) (((struct HandBytes *)&(e)->enemyStatus)->flags)
#define ANIM(e) (BALORE_HAND_ANIM((e)))

/* State byte stores through a plain u8 pointer: a struct-member store is
 * expanded as (x & 0) | v, whose zero would be reused for subState. */
#define SET_STATE_BYTE(e, s)                \
    {                                       \
        *(u8 *)&(e)->state = (s);         \
        *(u8 *)&(e)->subState = 0;           \
    }

static inline s32 ToPixels(s32 x)
{
    return x < 0 ? -(-x >> 16) : x >> 16;
}

/**
 * @brief Hand update (first family): fade in, alternate the swipe and dive
 * attacks, then retract; deleted with the owner.
 */
void BaloreHand_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *body = entity->ext.balorePart.body;
    u16 matrix;
    u8 state = entity->state;
    u8 pal, level, index;

    if (state != 0 && (body->ext.balore.flags & 2) && state <= 6)
        entity->state = 7;
    switch (entity->state)
    {
    case 0:
        matrix = ObjAffine_Alloc(1);
        if ((s16)matrix < 0)
        {
            EntityDelete(entity);
            return;
        }
        entity->affineIndex = matrix;
        entity->affine = 1;
        entity->spriteLayer = 0x1A;
        entity->ext.baloreHand.steer = Heap_Alloc(7);
        entity->basePalette = ObjPalette_FindOrLoadVariant(sBalorePalette, 15);
        Sprite_Attach(entity, &sBaloreSpriteSheet, (s8)body->ext.balore.gfx, entity->basePalette);
        Anim_StartIndexed(entity, 0x2D, 2, 0);
        Anim_Update(entity);
        entity->drawFunc = Sprite_DrawEntity;
        ANIM(entity) = 0x30;
        entity->ext.baloreHand.armLength = 0x260000;
        entity->state = 1;
        break;
    case 1:
        entity->hidden = 0;
        FLAGS(entity) &= 0xFB;
        if (BaloreHand_FadeIn(entity))
            SET_STATE_BYTE(entity, 2);
        break;
    case 2:
    {
        /* follow the body's requested state */
        u8 next = body->ext.balore.handCommand;
        u8 test = next;

        asm("" : "+r"(test)); /* FAKEMATCH */
        if (test != 0 && test != entity->state)
        {
            entity->state = next;
            entity->subState = 0;
        }
        break;
    }
    case 3:
        FLAGS(entity) |= 4;
        *(s32 *)&entity->spawnParam0 = 0x40000;
        entity->state = 4;
    case 4:
        if (BaloreHand_SteerToPoint(entity, sBaloreHandSteeringParams1))
        {
            entity->state = entity->stage;
            entity->subState = 0;
        }
        break;
    case 5:
        FLAGS(entity) |= 4;
        if (BaloreHand_UpdateSwipe(entity))
        {
            SET_STATE_BYTE(entity, 3);
            entity->stage = 6;
        }
        break;
    case 6:
        FLAGS(entity) &= 0xFB;
        if (BaloreHand_UpdateDive(entity))
        {
            SET_STATE_BYTE(entity, 3);
            entity->stage = 2;
        }
        break;
    case 7:
        FLAGS(entity) &= 0xFB;
        if (entity->flipX)
            body->ext.balore.flags |= 8;
        else
            body->ext.balore.flags |= 16;
        if ((body->ext.balore.flags & 24) != 24)
            break;
        entity->timer = 16;
        entity->state = 8;
        entity->subState = 0;
    case 8:
        *(s32 *)&entity->spawnParam0 = 0x40000;
        if (entity->timer != 0)
        {
            entity->timer--;
            pal = entity->basePalette;
            level = 15 - (*(volatile u8 *)&entity->timer >> 1); /* FAKEMATCH: volatile read forces the reload of the original */
            index = pal - 10;
            if (gEwramData->gameState.sharedObjPaletteVariants[index] != level)
            {
                gEwramData->gameState.sharedObjPaletteVariants[index] = level;
                ObjPalette_LoadRange((void *)gEwramData->gameState.sharedObjPaletteSources[index], level, 1, pal);
            }
        }
        if (BaloreHand_SteerToPoint(entity, sBaloreHandSteeringParams0))
        {
            u16 old = body->ext.balore.flags;
            register u32 newFlags asm("r1"); /* FAKEMATCH: result in the constant register */
            newFlags = old | 32;
            newFlags &= 0xFFE7;
            body->ext.balore.flags = newFlags;
            if (newFlags & 2)
                entity->state = 10;
            else
            {
                entity->state = 9;
                entity->hidden = 1;
            }
        }
        break;
    case 9:
        if ((u8)body->ext.balore.handCommand == 1)
        {
            entity->hidden = 0;
            {
                /* the zero for subState is set up right after the address */
                u8 *anim = (u8 *)&entity->enemyStatus;
                u8 zero = 0;

                *anim = 0x30;
                entity->ext.baloreHand.armLength = 0x260000;
                entity->state = (u8)body->ext.balore.handCommand;
                entity->subState = zero;
            }
        }
        break;
    case 10:
        Heap_Free(entity->ext.baloreHand.steer);
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
        return;
    }
    if (entity->hidden)
        return;
    entity->ext.baloreHand.armScale = (ToPixels(entity->ext.baloreHand.armLength) << 16) / 76;
    ObjAffine_Set(entity->ext.baloreHand.armAngle, entity->ext.baloreHand.armScale, 0x10000, entity->flipX, 0, entity->affineIndex);
    Anim_Update(entity);
}

static inline s32 Sub16(s16 a, u16 b) { return (s16)a - b; }

/* Create a hand and its associated part at a room position. */
void Balore_CreateHand(struct EwramData_EntityData *owner, u16 x, u16 y, u8 flip, s32 kind)
{
    struct EwramData_EntityData *hand, *part;
    struct BgLayerState *bg;
    if (kind == 0) part = Entity_CreateObject(BaloreHand_Update);
    else part = Entity_CreateObject(BaloreHandB_Update);
    if (part != NULL)
    {
        part->ext.balorePart.body = owner;
        part->ext.baloreHand.armLength = 0;
        part->ext.baloreHand.homeX = x;
        part->ext.baloreHand.homeY = y;
        bg = &gEwramData->bgInfo[1];
        part->posX.part16.integer = Sub16(x, bg->xPos.part16.integer);
        part->posY.part16.integer = Sub16(y, bg->yPos.part16.integer);
        part->flipX = flip;
        part->scrollWithBg = 1;
        hand = part;
        if (kind == 0)
        {
            part->hidden = 1;
            part = Entity_CreateObject(BaloreHandHitBox_Update);
        }
        else part = Entity_CreateObject(BaloreHandBHitBox_Update);
        if (part != NULL)
        {
            part->ext.balorePart.body = owner;
            part->ext.baloreHitBox.hand = hand;
            part->scrollWithBg = 1;
            if (kind == 0) part->hidden = 1;
        }
    }
}

/* Affine effect following the owner; grows and rotates, then shrinks. */
void BaloreAffineEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.balorePart.body;
    u16 matrix;
    s32 state = entity->state;
    if (state != 0 && entity->state <= 2 && (owner->ext.balore.flags & 2))
        entity->state = 3;
    switch (entity->state)
    {
    case 0:
        matrix = ObjAffine_Alloc(1);
        if ((s16)matrix < 0)
        {
            EntityDelete(entity);
            return;
        }
        entity->affineIndex = matrix;
        entity->affine = 1;
        entity->spriteLayer = 0x13;
        entity->basePalette = ObjPalette_FindOrLoadVariant(sBalorePalette, 0x10);
        Sprite_Attach(entity, &sBaloreSpriteSheet, (s8)owner->ext.balore.gfx, entity->basePalette);
        entity->drawFunc = Sprite_DrawEntity;
        Anim_StartIndexed(entity, 0x31, 2, 0);
        Anim_Update(entity);
        entity->drawSkip = 1;
        if (entity->subState != 0)
        {
            entity->drawSkipCounter = 1;
            entity->flipX = 1;
            entity->ext.baloreEffect.angle = 0x5000;
            entity->ext.baloreEffect.spin = 0xE0;
            entity->state = 2;
        }
        else
        {
            entity->ext.baloreEffect.angle = 0x3000;
            entity->ext.baloreEffect.spin = 0x80;
            entity->state = 1;
        }
        break;
    case 1:
        if (entity->ext.baloreEffect.scale < 0x10000)
            entity->ext.baloreEffect.scale += 0x400;
        entity->ext.baloreEffect.angle += entity->ext.baloreEffect.spin;
        break;
    case 2:
        if (entity->ext.baloreEffect.scale < 0xC000)
            entity->ext.baloreEffect.scale += 0x200;
        entity->ext.baloreEffect.angle += entity->ext.baloreEffect.spin;
        break;
    case 3:
        if (entity->ext.baloreEffect.scale > 0)
        {
            entity->ext.baloreEffect.scale += -0x400;
            break;
        }
    case 4:
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
        return;
    }
    entity->posX.part16.integer = owner->posX.part16.integer - 21;
    entity->posY.part16.integer = owner->posY.part16.integer - 9;
    ObjAffine_Set(entity->ext.baloreEffect.angle, entity->ext.baloreEffect.scale, entity->ext.baloreEffect.scale, entity->flipX, 0, entity->affineIndex);
}

/**
 * @brief B5224 | Boss part with a hit box (BaloreSpark_DamagePlayerOnContact): load its own
 * palette, wait timer frames, then move sideways; when its animation ends
 * spawn falling sparks (Effect_SpawnShared) every fourth frame and delete itself.
 */
void BaloreSpark_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    struct EwramData_EntityData *parent = entity->ext.balorePart.body;
    struct EwramData_EntityData *spark;
    u8 timer;

    switch (entity->state)
    {
    case 0:
        entity->basePalette = ObjPalette_FindOrLoadVariant((void *)sBalorePalette, 0x10);
        Sprite_Attach(entity, (void *)&sBaloreSpriteSheet, BALORE_GFX_BYTE(parent), entity->basePalette);
        entity->drawFunc = Sprite_DrawEntity;
        Anim_StartIndexed(entity, 0x33, 2, 0);
        entity->spriteLayer = 0x13;
        Hit_Enable(entity, 8, BaloreSpark_DamagePlayerOnContact, 0);
        HitBox_SetAttackRaw(entity, 0x2810D8F8);
        entity->hit.flags |= 2;
        entity->hit.flags |= 4;
        entity->hidden = 1;
        entity->state = 1;
    case 1:
        timer = entity->timer;
        if (timer != 0)
        {
            entity->timer = timer - 1;
            break;
        }
        PlaySong(0x6E);
        entity->hidden = 0;
        entity->state = 2;
    case 2:
        EntityAccelerateAndMoveX(entity, 0x80000);
        Hit_CheckWithPlayer(&result, entity);
        if (entity->animStep == 3)
            entity->hit.flags &= ~2;
        else
            entity->hit.flags |= 2;
        if (Anim_Update(entity) != 3)
            break;
        spark = Effect_SpawnShared(entity->posX.whole + RandomSigned(0x100000), entity->posY.whole + RandomSigned(0x40000) - 0x40000, 1, 7);
        if (spark != NULL)
        {
            spark->velY.whole = RandomSigned(0x2000) - 0x8000;
            spark->accelY.whole = -spark->velY.whole / 64;
        }
        entity->state = 3;
        entity->timer = 8;
        break;
    case 3:
        if ((entity->timer & 3) == 0)
        {
            spark = Effect_SpawnShared(entity->posX.whole + RandomSigned(0x200000), entity->posY.whole + RandomSigned(0x40000) - 0x40000, 5, 6);
            if (spark != NULL)
            {
                spark->drawSkip = 1;
                spark->velY.whole = RandomSigned(0x2000) - 0x8000;
                spark->accelY.whole = -spark->velY.whole / 64;
            }
        }
        timer = entity->timer;
        if (timer != 0)
        {
            entity->timer = timer - 1;
            break;
        }
        entity->deleteRequested = 1;
        break;
    }
}

/* Spinning breath/beam emitter attached to its parent: spawns pieces along
 * the sweep and draws itself through an affine slot. */

/* Angle step of the sweep per frame (16-bit angle, sign by facing). */
#define SWEEP_SPEED(e) ENTITY_WORD_514(e)

static inline struct EwramData_EntityData *SpawnAtEntity(void *update, struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *spawned = Entity_CreateObject(update);

    if (spawned == NULL)
        return NULL;
    spawned->posX.whole = entity->posX.whole;
    spawned->posY.whole = entity->posY.whole;
    spawned->scrollWithBg = 1;
    return spawned;
}

/**
 * @brief B53F8 | Emitter update (state: 0 init, 1 sweep and spawn, 2 fade, 3 delete).
 *
 * Notes for matching: SpawnAtEntity() returns NULL early, so the caller's
 * NULL test is threaded away.  `v` is reused after the scale division; being
 * set again later keeps it the canonical CSE register, which gives
 * `cmp r0, #0; adds r6, r0, #7` for `v / 8`.
 */
void BaloreEmitter_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.balorePart.body;
    u16 slot;
    struct EwramData_EntityData *piece;
    s32 scale;
    s32 v;

    if (entity->state != 0)
    {
        if (entity->state <= 1 && (parent->ext.balore.flags & 2))
            entity->state = 2;
    }
    switch (entity->state)
    {
    case 0:
        slot = ObjAffine_Alloc(1);
        if ((s16)slot < 0)
        {
            EntityDelete(entity);
            return;
        }
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->spriteLayer = 0x14;
        entity->basePalette = ObjPalette_FindOrLoadVariant((void *)sBalorePalette, 0x10);
        Sprite_Attach(entity, (void *)&sBaloreSpriteSheet, BALORE_GFX_BYTE(parent), entity->basePalette);
        entity->drawFunc = BaloreEmitter_Draw;
        Anim_StartIndexed(entity, 0x32, 2, 0);
        Anim_Update(entity);
        entity->objMode = 1;
        entity->ext.baloreEmitter.size = 0x1000000;
        entity->timer = 0x40;
        entity->flipX = RandomNumberGenerator2() & 1;
        if (!entity->flipX)
        {
            entity->ext.baloreEmitter.angle = 0x7800;
            SWEEP_SPEED(entity) = -0x1C0;
        }
        else
        {
            entity->ext.baloreEmitter.angle = 0x800;
            SWEEP_SPEED(entity) = 0x1C0;
        }
        Sound_PlayTracked(0x66);
        entity->state = 1;
    case 1:
        if ((entity->timer & 3) == 0)
        {
            piece = SpawnAtEntity(BaloreSpark_Update, entity);
            if (piece != NULL)
            {
                piece->posX.whole = entity->posX.whole + Sin(entity->ext.baloreEmitter.angle + 0x4000) * 0xB0;
                piece->posY.part16.integer = 0x90;
                piece->velX.whole = ((0x40 - entity->timer) << 9) + 0x8000;
                if (entity->flipX)
                    piece->velX.whole = -piece->velX.whole;
                piece->accelX.whole = -piece->velX.whole / 64;
                piece->ext.balorePart.body = parent;
                piece->timer = ((0x40 - entity->timer) >> 2) + 8;
            }
        }
        entity->ext.baloreEmitter.scaleY = Sin(entity->timer << 9);
        entity->ext.baloreEmitter.angle += SWEEP_SPEED(entity);
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->state = 3;
        break;
    case 2:
        if (entity->ext.baloreEmitter.scaleY > 0)
        {
            entity->ext.baloreEmitter.scaleY += -0x400;
            break;
        }
    case 3:
        Sound_StopTracked(0x66);
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
        return;
    }
    entity->posX.part16.integer = parent->posX.part16.integer - 0x15;
    entity->posY.part16.integer = parent->posY.part16.integer - 9;
    v = entity->ext.baloreEmitter.size;
    if (v > 0x1000000)
        v = 0x1000000;
    {
        s32 eighth = v / 8;
        scale = eighth / 32;
    }
    v = Sin(entity->ext.baloreEmitter.angle + 0x4000) << 5;
    entity->ext.baloreEmitter.stepX = v;
    BALORE_EMITTER_STEP_Y(entity) = Sin(entity->ext.baloreEmitter.angle) << 5;
    entity->ext.baloreEmitter.stepX = entity->ext.baloreEmitter.stepX / 0x10800 * scale;
    BALORE_EMITTER_STEP_Y(entity) = BALORE_EMITTER_STEP_Y(entity) / 0x10800 * scale;
    ObjAffine_Set(entity->ext.baloreEmitter.angle, scale, entity->ext.baloreEmitter.scaleY, 0, 0, entity->affineIndex);
}

/* Two-hand spawn and fade-out phase (Balore area). */
/**
 * @brief B569C | Phase of the entity whose hands Balore_CreateHand creates
 * (code next to EnemyBaloreCreate).  State 0/1 counts 0x40 frames down in
 * timer and spawns a hand at (0x40, 0x80) flipped at 0x38 and one at
 * (0x1C0, 0x80) at 0x18; then sound 0x1D4 and state 2.  State 2 sinks by
 * 0x4000 per frame while the palette basePalette fades (level counter / 8 via
 * ObjPalette_LoadRange, cached in sharedObjPaletteVariants); at the end it clears bit 2 of flags
 * and switches to state = 2.
 */
void Balore_UpdateEnter(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        entity->subState = 1;
        entity->timer = 0x40;
    case 1:
        if (entity->timer != 0)
        {
            if (entity->timer == 0x38)
                Balore_CreateHand(entity, 0x40, 0x80, 1, 0);
            if (entity->timer == 0x18)
                Balore_CreateHand(entity, 0x1C0, 0x80, 0, 0);
            entity->timer--;
        }
        else
        {
            entity->subState = 2;
            entity->timer = 0x40;
            PlaySong(0x1D4);
        }
        break;
    case 2:
        entity->posY.whole += 0x4000;
        if (entity->timer != 0)
        {
            u8 pal, idx;
            u8 level;

            entity->timer--;
            pal = entity->basePalette;
            level = *(volatile u8 *)&entity->timer >> 3; /* FAKEMATCH: volatile read forces the reload of the original */
            idx = pal - 10;
            if (gEwramData->gameState.sharedObjPaletteVariants[idx] != level)
            {
                gEwramData->gameState.sharedObjPaletteVariants[idx] = level;
                ObjPalette_LoadRange((u8 *)gEwramData->gameState.sharedObjPaletteSources[idx], level, 1, pal);
            }
        }
        else
        {
            entity->hit.flags &= ~4;
            entity->state = 2;
            entity->subState = 0;
        }
        break;
    }
}

/* Hovering phase of the Balore-area body. */
/* lean towards the target, 0..8 with 4 = centred */
#define TILT(e) ((e)->ext.balore.eyeAnim)

/**
 * @brief B577C | Hover phase.  State 1 bobs vertically (sine of
 * timer << 8, / 8 per frame), leans 0..8 (unk_4F6) after the horizontal
 * distance to the target (Target_GetX) in 16-pixel steps and sets
 * unk_4FC = 5; at HP (hp) <= 1999 it moves on to state 2 (unk_4FC = 7),
 * which waits for bit 5 of unk_502, clears it, recentres the lean and
 * switches to state = 3.  State 3 counts timer down and restarts
 * state 1 with unk_4FC = 1 and timer = 0xF0.
 */
void Balore_UpdateHover(struct EwramData_EntityData *entity)
{
    s32 dx;

    switch (entity->subState)
    {
    case 0:
        entity->subState = 1;
    case 1:
        entity->posY.whole -= Sin(entity->timer << 8) / 8;
        entity->timer++;
        if (Target_GetX(entity) - entity->posX.whole < 0)
            dx = -((entity->posX.whole - Target_GetX(entity)) >> 16);
        else
            dx = (Target_GetX(entity) - entity->posX.whole) >> 16;
        TILT(entity) = dx / 16 + 4;
        if ((s8)TILT(entity) < 0)
            TILT(entity) = 0;
        else if ((s8)TILT(entity) > 8)
            TILT(entity) = 8;
        entity->ext.balore.handCommand = 5;
        if ((s16)entity->hp <= 1999)
            entity->subState = 2;
        break;
    case 2:
        entity->ext.balore.handCommand = 7;
        if (entity->ext.balore.flags & 0x20)
        {
            /* u16 view of the flags keeps the 0xFFDF mask */
            entity->ext.balore.flags = (u16)entity->ext.balore.flags & 0xFFDF;
            TILT(entity) = 4;
            entity->state = 3;
            entity->subState = 0;
        }
        break;
    case 3:
        if (entity->timer != 0)
            entity->timer--;
        else
        {
            entity->ext.balore.handCommand = 1;
            entity->subState = 1;
            entity->timer = 0xF0;
        }
        break;
    }
}

/* Balore-area effect spawning and timer phases.
 * The local register bindings preserve agbcc's callback reloads and child
 * stores. Matching does not establish the original source spelling. */

/**
 * @brief Attack state (state 3): spawn the affine effect and the spark emitter
 * (BaloreAffineEffect_Update / BaloreEmitter_Update), then return to the
 * hover state.
 */
void Balore_UpdateAttack(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *effect;

    switch (entity->subState)
    {
    case 0:
        {
            register EntityFunc callback asm("r0") = BaloreAffineEffect_Update; /* FAKEMATCH: pinned register */
            effect = Entity_CreateObject(callback);
        }
        if (effect != NULL)
        {
            effect->posX.whole = entity->posX.whole;
            effect->posY.whole = entity->posY.whole;
            effect->scrollWithBg = 1;
            effect->ext.balorePart.body = entity;
        }
        {
            EntityFunc callback = BaloreAffineEffect_Update;
            effect = Entity_CreateObject(callback);
        }
        if (effect != NULL)
        {
            effect->posX.whole = entity->posX.whole;
            effect->posY.whole = entity->posY.whole;
            effect->scrollWithBg = 1;
            {
                register struct EwramData_EntityData *child asm("r1") = effect; /* FAKEMATCH: pinned register */
                child->subState = 1;
                child->ext.balorePart.body = entity;
            }
        }
        PlaySong(0x1D5);
        entity->defense = 10;
        HitBox_SetBodyRaw(entity, 0x1010F0F0);
        entity->subState = 1;
        entity->timer = 0x60;
    case 1:
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        effect = Entity_CreateObject(BaloreEmitter_Update);
        if (effect != NULL)
        {
            effect->posX.whole = entity->posX.whole;
            effect->posY.whole = entity->posY.whole;
            effect->scrollWithBg = 1;
            effect->ext.balorePart.body = entity;
        }
        entity->subState = 2;
        break;
    case 2:
        if ((s16)entity->hp <= 999)
            entity->timer = 0x50;
        else
            entity->timer = 0x80;
        entity->subState = 1;
        break;
    }
}

/* Boss part effect: sparks, debris bursts and a fading palette countdown. */
static inline u8 *Slot(u8 *base, u32 i) { return base + i; }

/**
 * @brief Death state (state 4): defeat sequence; returns nonzero when the
 * entity can be removed.
 */
s32 Balore_UpdateDeath(struct EwramData_EntityData *entity)
{
    s32 one;
    struct EwramData_EntityData *fx;
    s32 a;
    u8 b;
    u8 idx;
    u8 *p;
    s32 x, y;

    switch (entity->subState)
    {
    case 0:
        if (!(gEwramData->gameState.stateFlags & 0x10))
        {
            BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x10000, 0, 4);
            ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x10000, 0, 4);
        }
        entity->subState = 1;
        entity->timer = 4;
    case 1:
        if (entity->timer)
        {
            entity->timer--;
            break;
        }
        if (!(gEwramData->gameState.stateFlags & 0x10))
        {
            BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x20);
            ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x20);
        }
        entity->timer = 0x20;
        entity->subState = 3;
        PlaySong(0x1D6);
        break;
    case 2:
        if (entity->timer)
        {
            entity->timer--;
            break;
        }
        entity->timer = 0x80;
        entity->subState = 3;
        
        break;
    case 3:
        one = 0;
        if (entity->timer & 1)
        {
             x = ((s16)entity->posX.part16.integer - 0x10) << 16; 
            y = ((s16)entity->posY.part16.integer - 0x10) << 16;
             fx = Effect_SpawnShared(x, y, 0xD, 8);
            if (fx != NULL)
            {
                s32 r = RandomNumberGenerator();
                one = 1; fx->flipX = r & one;
                fx->flipY = RandomNumberGenerator() & one; asm("" :: "r"(one)); /* FAKEMATCH: one stays live, so ands binds to the RNG result */
            }
        }
        if (entity->timer)
        {
            entity->timer--;
            break;
        }
        entity->timer = 0x80;
        entity->subState = 4;
        break;
    case 4:
        if (entity->timer > 0x20)
        {
             x = ((s16)entity->posX.part16.integer - 0x15) << 16; 
            y = ((s16)entity->posY.part16.integer - 9) << 16;
            fx = Effect_SpawnShared(x, y, 0xF, 7);
            if (fx != NULL)
            {
                a = (RandomNumberGenerator2() & 0xFF) << 8;
                fx->velX.whole = Sin(a + 0x4000) * entity->timer / 32;
                fx->velY.whole = entity->timer * Sin(a) / 32;
                fx->accelX.whole = fx->velX.whole / 32;
                fx->accelY.whole = fx->velY.whole / 32;
                {
                s32 r = RandomNumberGenerator();
                one = 1; fx->flipX = r & one;
                fx->flipY = RandomNumberGenerator() & one; asm("" :: "r"(one)); /* FAKEMATCH: one stays live, so ands binds to the RNG result */
                }
                if (!(entity->timer & 7))
                    PlaySong(0x77);
            }
        }
        if (entity->ext.balore.scale > 0x8000)
            entity->ext.balore.scale -= 0x100;
        if (entity->timer)
        {
            entity->timer--;
            {
                register u8 s asm("r3") = entity->basePalette; /* FAKEMATCH: pinned register */
                b = 7 - (*(vu8 *)&entity->timer >> 4);
                idx = s - 10;
            p = Slot(gEwramData->gameState.sharedObjPaletteVariants, idx); asm("" :: "r"(s)); /* FAKEMATCH: s keeps r3 busy, so reloads use r4 */
            if (*p != b)
            {
                *p = b;
                { s32 a_ = gEwramData->gameState.sharedObjPaletteSources[idx]; asm("" :: "r"(s)); ObjPalette_LoadRange3(a_, b, 1); } /* FAKEMATCH: same */
            }
            break;
            }
        }
        entity->ext.balore.flags |= 4;
        entity->subState = 5;
        entity->timer = 0x20;
        break;
    case 5:
        if (entity->timer)
        {
            entity->timer--;
            break;
        }
        PlaySong(0x7C);
        ScreenShake_Request(0, 0x20, 0, 7);
        entity->subState = 6;
        entity->timer = 0x20;
        break;
    case 6:
        if (entity->timer)
        {
            entity->timer--;
            break;
        }
        SoulPickup_SpawnFromEnemyDefinition((s16)entity->posX.part16.integer - 0x10, (s16)entity->posY.part16.integer, entity->enemyId, 6);
        BossRecoveryPickup_SpawnWide((s16)entity->posX.part16.integer, 0x20, 6, 0x60, 1);
        return 1;
        entity->timer--;
        break;
    }
    return 0;
}

/*
 * Balore initialization, including persistent-state and frozen-state paths.
 * The two pinned registers keep cse from carrying the gEwramData address and
 * the constant 4 across the skipped part blocks. Without them the function
 * matches only with -fno-cse-skip-blocks, which changes other functions of
 * this file.
 */
void EnemyBaloreCreate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part;

    if (entity->spawnParam0 != 0 && !gEwramData->gameState.bossRushMode)
    {
        if (gEwramData->gameState.bossEventFlags & 0x40)
        {
            EntityDelete(entity);
            return;
        }
        gEwramData->gameState.stateFlags |= 2;
        Balore_CreateIntro(entity);
        return;
    }
    if ((u8)Balore_LoadGraphics(entity) == 0)
    {
        EntityDelete(entity);
        return;
    }
    {
        register struct EwramData **ew asm("r0") = &gEwramData; /* FAKEMATCH: pinned register */
        u8 flags = *((u8 *)*ew + 0x8E);
        u32 mask = 0x40;
        if (!(mask &= flags))
            entity->posY.part16.integer -= 0x10;
    }
    Anim_StartIndexed(entity, 0, 2, 1);
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x1E;
    entity->ext.balore.eyeAnim = 4;
    entity->ext.balore.scale = 0x10000;
    part = Entity_CreateObject(BaloreEye_Update);
    if (part != NULL)
    {
        part->posX.whole = entity->posX.whole;
        part->posY.whole = entity->posY.whole;
        part->scrollWithBg = 1;
        part->ext.balorePart.body = entity;
    }
    part = Entity_CreateObject(BalorePart_Update);
    if (part != NULL)
    {
        part->posX.whole = entity->posX.whole;
        part->posY.whole = entity->posY.whole;
        part->scrollWithBg = 1;
        part->ext.balorePart.body = entity;
    }
    part = Entity_CreateObject(BaloreBodyPart_Update);
    if (part != NULL)
    {
        part->posX.whole = entity->posX.whole;
        part->posY.whole = entity->posY.whole;
        part->scrollWithBg = 1;
        part->ext.balorePart.body = entity;
    }
    entity->flipX = 0;
    {
        u8 flags = *((u8 *)gEwramData + 0x8E);
        u32 mask = 0x40;
        if (mask &= flags)
        {
            entity->state = 5;
            return;
        }
    }
    Hit_Enable(entity, 8, NULL, Balore_OnHit);
    HitBox_SetBodyRaw(entity, 0x1010F010);
    {
        u32 flags = entity->hit.flags;
        u32 mask = 2;
        mask |= flags;
        {
            register u32 next asm("r1") = 4; /* FAKEMATCH: pinned register */
            mask |= next;
        }
        entity->hit.flags = mask;
    }
    Player_SetSelectedTarget(entity);
    gEwramData->gameState.stateFlags |= 2;
}

void EnemyBaloreUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    if (entity->state != 4) Enemy_UpdateHitFlash(entity);
    switch (entity->state)
    {
    case 0:
        Music_SetCurrent(0x11);
        entity->state = 1;
        entity->subState = 0;
        break;
    case 1: Balore_UpdateEnter(entity); break;
    case 2: Balore_UpdateHover(entity); break;
    case 3: Balore_UpdateAttack(entity); break;
    case 5: Anim_Update(entity); return;
    case 4:
        if (Balore_UpdateDeath(entity))
        {
            EntityDelete(entity);
            return;
        }
        break;
    }
    *(u16 *)&entity->ext.balore.flags &= 0xFFFE;
    if (entity->state != 4 && Entity_IsInRect(entity, -8, 0, 0x100, 0xB0))
        Hit_CheckWithPlayer(&result, entity);
    Anim_StartIndexed(entity, sBaloreEyeAnimOffsets[(s8)entity->ext.balore.eyeAnim], 2, 0);
    Anim_Update(entity);
}

void BaloreDebris_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.balorePart.body;
    if (parent->state == 10) entity->state = 2;
    switch (entity->state)
    {
    case 0:
        Sprite_Attach(entity, sBaloreSpriteSheet2, (s8)parent->ext.balore.gfx,
                     parent->basePalette);
        Anim_StartIndexed(entity, entity->subState, 2, 0);
        entity->drawFunc = Sprite_DrawEntity;
        entity->state = 1;
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        if (entity->animFlags & 4)
        {
            EntityDelete(entity);
            return;
        }
        break;
    case 2:
        EntityDelete(entity);
        return;
    }
    Anim_Update(entity);
}

/* The original reads the y speeds through a pointer two bytes into the table. */
#define ORBIT_SPEED_Y ((const s16 *)((const u8 *)sBaloreIntroPieceOrbitSpeeds + 2))

/**
 * @brief B5F1C | Orbiting part of a boss (parent unk_4FC): spawn with the
 * parent graphics, fly out along angle unk_504 when the parent reaches state
 * 3, then circle it with a shrinking radius from state 4 and delete itself;
 * deleted at once when the parent enters state 10.
 */
void BaloreIntroPiece_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.balorePiece.owner;
    s32 i;
    s32 angle;
    u8 timer;

    if (parent->state == 10)
        entity->state = 6;
    switch (entity->state)
    {
    case 0:
        Sprite_Attach(entity, (void *)sBaloreSpriteSheet2, BALORE_GFX_BYTE(parent), parent->basePalette);
        Anim_StartIndexed(entity, 1, 2, 1);
        entity->animTimer = RandomNumberGenerator() & 7;
        entity->drawFunc = Sprite_DrawEntity;
        entity->spriteLayer = 9;
        entity->state = 1;
    case 1:
        if (parent->state != 3)
            break;
        i = entity->ext.balorePiece.index;
        if (i > 15)
            i = 31 - i;
        angle = i << 11;
        entity->velX.whole = Sin(angle + 0x4000);
        entity->accelX.whole = entity->velX.whole / 4;
        entity->velY.whole = Sin(angle) / 2;
        entity->accelY.whole = entity->velY.whole / 4;
        entity->state = 2;
        entity->timer = entity->ext.balorePiece.index;
        break;
    case 2:
        timer = entity->timer;
        if (timer != 0)
        {
            entity->timer = timer - 1;
            break;
        }
        Anim_StartIndexed(entity, 2, 2, 1);
        entity->timer = 0x50;
        entity->state = 3;
    case 3:
        EntityAccelerateAndMove(entity, 0x80000);
        timer = entity->timer;
        if (timer != 0)
        {
            entity->timer = timer - 1;
            break;
        }
        entity->state = 4;
        break;
    case 4:
        if (parent->state != 4)
            break;
        entity->ext.balorePiece.angle = RandomNumberGenerator();
        entity->ext.balorePiece.radius = entity->ext.balorePiece.index + 0x80;
        entity->velX.part16.integer = sBaloreIntroPieceOrbitSpeeds[entity->ext.balorePiece.index & 0xF].x;
        entity->velY.part16.integer = ORBIT_SPEED_Y[(entity->ext.balorePiece.index & 0xF) * 2];
        entity->state = 5;
        break;
    case 5:
        entity->ext.balorePiece.angle += 0x300;
        entity->posX.whole = parent->posX.whole + entity->velX.whole
            + Sin(entity->ext.balorePiece.angle + 0x4000) * entity->ext.balorePiece.radius;
        entity->posY.whole = parent->posY.whole + entity->velY.whole
            + Sin(entity->ext.balorePiece.angle) * entity->ext.balorePiece.radius;
        if (entity->ext.balorePiece.radius > 0x20)
            entity->ext.balorePiece.radius -= entity->ext.balorePiece.radius >> 5;
        if (--entity->ext.balorePiece.radius <= 0)
            entity->state = 6;
        break;
    case 6:
        EntityDelete(entity);
        return;
    }
    Anim_Update(entity);
}

/**
 * @brief B60EC | Boss part next to BaloreIntroPiece_Update: appear with the parent
 * graphics, flash its palette slot (sUnk_085287D6) while counting down, then
 * jump up, fly sideways and delete itself after the animation ends; deleted
 * at once when the parent enters state 10.
 */
void BaloreIntroPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.balorePiece.owner;

    if (parent->state == 10)
        entity->state = 5;
    switch (entity->state)
    {
    case 0:
        Sprite_Attach(entity, (void *)sBaloreSpriteSheet2, BALORE_GFX_BYTE(parent), parent->basePalette);
        Anim_StartIndexed(entity, 3, 2, 0);
        entity->drawFunc = Sprite_DrawEntity;
        entity->spriteLayer = 0x1A;
        entity->state = 1;
        entity->timer = 0x10;
    case 1:
        if (entity->timer != 0)
        {
            entity->timer--;
            goto cycle;
        }
        Anim_StartIndexed(entity, 4, 2, 0);
        entity->state = 2;
        entity->timer = 0x30;
        PlaySong(0x102);
        break;
    case 2:
        if (entity->timer == 0)
            goto start_fall;
        if (--entity->timer <= 0x20)
            break;
    cycle:
        {
            u8 palette = parent->basePalette;
            u8 value = sUnk_085287D6[(entity->timer >> 1) & 3];
            u8 slot = palette - 10;
            u8 *slots = gEwramData->gameState.sharedObjPaletteVariants;

            if (slots[slot] != value)
            {
                slots[slot] = value;
                ObjPalette_LoadRange((u8 *)gEwramData->gameState.sharedObjPaletteSources[slot], value, 1, palette);
            }
        }
        break;
    start_fall:
        Anim_StartIndexed(entity, 0xE, 2, 1);
        entity->state = 3;
        entity->timer = 0;
        entity->velY.whole = -0x1F000;
        entity->accelY.whole = 0xC00;
        PlaySong(0x101);
        break;
    case 3:
        EntityAccelerateAndMoveY(entity, 0x80000);
        if (entity->timer <= 0x4F)
        {
            entity->timer++;
            break;
        }
        entity->velY.whole = 0;
        entity->accelY.whole = 0;
        Anim_StartIndexed(entity, 6, 2, 0);
        entity->state = 4;
        entity->velX.whole = 0x60000;
        entity->accelX.whole = -0x8000;
        break;
    case 4:
        if (entity->velX.whole > 0x20000)
            EntityAccelerateAndMoveX(entity, 0x80000);
        if (!(entity->animFlags & 4))
            break;
    case 5:
        EntityDelete(entity);
        return;
    }
    Anim_Update(entity);
}

/* Second Balore hand family's attached part, spawning effects at animation 8. */
void BaloreHandBHitBox_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *body = entity->ext.balorePart.body;
    struct EwramData_EntityData *hand = entity->ext.baloreHitBox.hand;
    struct EwramData_EntityData *child;
    u16 matrix;
    s32 x, y;
    if (hand->hidden || body->state == 10)
        entity->state = 4;
    switch (entity->state)
    {
    case 0:
        matrix = ObjAffine_Alloc(1);
        if ((s16)matrix < 0)
        {
            EntityDelete(entity);
            return;
        }
        entity->affineIndex = matrix;
        entity->affine = 1;
        entity->spriteLayer = hand->spriteLayer;
        Sprite_Attach(entity, sBaloreSpriteSheet2, (s8)body->ext.balore.gfx, hand->basePalette);
        Anim_StartIndexed(entity, 10, 2, 0);
        entity->drawFunc = Sprite_DrawEntity;
        entity->state = 1;
        break;
    case 1:
        if (entity->animScriptIndex == 8)
        {
            entity->state = 2;
            entity->timer = 16;
        }
        break;
    case 2:
        if (entity->timer != 0)
        {
            if ((entity->timer & 3) == 0)
            {
                struct EwramData_EntityData *spark;
                x = (s16)(0xFFF8 + entity->posX.part16.integer + (RandomNumberGenerator() & 7));
                y = (s16)(entity->posY.part16.integer - 22);
                spark = EntityCreateInRange(0x49, 0xCF, BaloreDebris_Update);
                if (spark != NULL)
                {
                    spark->ext.balorePart.body = body;
                    spark->subState = 12;
                    spark->posX.part16.integer = x;
                    spark->posY.part16.integer = y;
                    spark->spriteLayer = 0x1C;
                    spark->scrollWithBg = 1;
                    spark->spriteLayer = 0x14;
                }
            }
            entity->timer--;
        }
        else
        {
            x = (s16)entity->posX.part16.integer;
            y = (s16)(entity->posY.part16.integer + 24);
            child = EntityCreateInRange(0x49, 0xCF, BaloreDebris_Update);
            if (child != NULL)
            {
                child->ext.balorePart.body = body;
                child->subState = 11;
                child->posX.part16.integer = x;
                child->posY.part16.integer = y;
                child->spriteLayer = 0x1C;
                child->scrollWithBg = 1;
            }
            entity->state = 3;
        }
        break;
    case 3:
        break;
    case 4:
        if (entity->affine) ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
        return;
    }
    entity->palette = hand->palette;
    if (hand->flipX)
        x = hand->ext.baloreHand.armAngle;
    else
        x = hand->ext.baloreHand.armAngle + 0x8000;
    y = TO_PIXELS(hand->ext.baloreHand.armLength);
    entity->posX.whole = hand->posX.whole + y * Sin(x + 0x4000);
    entity->posY.whole = hand->posY.whole + y * Sin(x);
    ObjAffine_Set(hand->ext.baloreHand.armAngle, 0x10000, 0x10000, hand->flipX, 0, entity->affineIndex);
    if (entity->animScriptIndex != BALORE_HAND_ANIM(hand))
        Anim_StartIndexed(entity, BALORE_HAND_ANIM(hand), 2, 0);
    Anim_Update(entity);
}

#define STEP(e) (*(s32 *)&(e)->spawnParam0)

static inline void FadeIn(struct EwramData_EntityData *entity)
{
    u8 pal = entity->basePalette;
    /* Reload the byte counter after its decrement, as in the original. */
    s32 level = (*(volatile u8 *)&entity->timer >> 1) + 8; /* FAKEMATCH: volatile read forces the reload of the original */
    u8 index = pal - 10;
    if (gEwramData->gameState.sharedObjPaletteVariants[index] != level)
    {
        gEwramData->gameState.sharedObjPaletteVariants[index] = level;
        ObjPalette_LoadRange((void *)gEwramData->gameState.sharedObjPaletteSources[index], level, 1, pal);
    }
}

static inline void FadeOut(struct EwramData_EntityData *entity)
{
    u8 pal = entity->basePalette;
    u8 level = 15 - (*(volatile u8 *)&entity->timer >> 3); /* FAKEMATCH: volatile read forces the reload of the original */
    u8 index = pal - 10;
    if (gEwramData->gameState.sharedObjPaletteVariants[index] != level)
    {
        gEwramData->gameState.sharedObjPaletteVariants[index] = level;
        ObjPalette_LoadRange((void *)gEwramData->gameState.sharedObjPaletteSources[index], level, 1, pal);
    }
}

/* Second family of Balore hand: fade, extend, attack and retract. */
void BaloreHandB_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *body = entity->ext.balorePart.body;
    u16 matrix;
    s32 scale;
    if (body->state == 10) entity->state = 5;
    switch (entity->state)
    {
    case 0:
        matrix = ObjAffine_Alloc(1);
        if ((s16)matrix < 0)
        {
            EntityDelete(entity);
            return;
        }
        entity->affineIndex = matrix;
        entity->affine = 1;
        entity->spriteLayer = 0x1A;
        entity->basePalette = ObjPalette_FindOrLoadVariant(sBalorePalette, 15);
        Sprite_Attach(entity, sBaloreSpriteSheet2, (s8)body->ext.balore.gfx, entity->basePalette);
        Anim_StartIndexed(entity, 7, 2, 0);
        entity->drawFunc = Sprite_DrawEntity;
        ANIM(entity) = 10;
        entity->ext.baloreHand.armLength = 0x260000;
        STEP(entity) = 0x80000;
        entity->timer = 16;
        entity->state = 1;
    case 1:
        if (entity->timer != 0)
        {
            entity->timer--;
            FadeIn(entity);
        }
        if (BaloreHand_SteerToPoint(entity, sBaloreHandBSteeringParams0))
        {
            Music_SetCurrent(17);
            ANIM(entity) = 9;
            entity->timer = 8;
            entity->state = 2;
        }
        break;
    case 2:
        if (--entity->timer == 0)
        {
            PlaySong(0x9F);
            ANIM(entity) = 8;
        }
        if (BaloreHand_SteerToPoint(entity, sBaloreHandBSteeringParams1))
        {
            PlaySong(0x9F);
            entity->state = 3;
        }
        break;
    case 3:
        if (body->state == 8)
        {
            STEP(entity) = 0x8000;
            entity->timer = 64;
            entity->state = 4;
        }
        break;
    case 4:
        if (entity->timer != 0)
        {
            entity->timer--;
            FadeOut(entity);
        }
        if (BaloreHand_SteerToPoint(entity, sBaloreHandBSteeringParams2))
        {
            entity->hidden = 1;
            entity->state = 5;
        }
        break;
    case 5:
        if (entity->affine) ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
        return;
    }
    if (entity->hidden) return;
    scale = (s32)((u32)ToPixels(entity->ext.baloreHand.armLength) << 16) / 76;
    entity->ext.baloreHand.armScale = scale;
    ObjAffine_Set(entity->ext.baloreHand.armAngle, scale, 0x10000, entity->flipX, 0, entity->affineIndex);
    Anim_Update(entity);
}

/* Row of 32 child entities. */
/* palette slot + sprite set variant 0x11; same shape as Balore_LoadGraphics */
static inline u8 LoadGfx(struct EwramData_EntityData *entity)
{
    s32 slot = ObjGfx_LoadResource(sUnk_0811940C, 0);

    if (slot < 0)
        return 0;
    entity->ext.balore.palette = ObjPalette_FindOrLoadVariant(sBalorePalette, 0x11);
    entity->ext.balore.gfx = slot;
    entity->basePalette = entity->ext.balore.palette;
    Sprite_Attach(entity, sBaloreSpriteSheet2, slot, entity->basePalette);
    entity->scrollWithBg = 1;
    return 1;
}

/**
 * @brief B67A8 | Create: update becomes BaloreIntro_Update, graphics
 * (sUnk_0811940C / sBalorePalette variant 0x11 / sBaloreSpriteSheet2) or delete,
 * animation 4, priority 0x14, hidden.  Then up to 32 children
 * BaloreIntroPiece_Update (slots 0x49..0xCF) with owner unk_4FC, index i in unk_504,
 * X = own X - 0x80 + 8 * i and Y = 0x20; stops at the first failure.
 */
void Balore_CreateIntro(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *e;
    s32 i, offset;
    s16 *x;

    entity->updateFunc = BaloreIntro_Update;
    if (!LoadGfx(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 4, 2, 1);
    Anim_Update(entity);
    entity->spriteLayer = 0x14;
    entity->hidden = 1;
    /* the index starts before the X pointer is taken (register order) */
    i = 0;
    x = &entity->posX.part16.integer;
    for (offset = -0x80; i < 32; offset += 8, i++)
    {
        e = EntityCreateInRange(0x49, 0xCF, BaloreIntroPiece_Update);
        if (e == NULL)
            return;
        e->ext.balorePiece.owner = entity;
        e->ext.balorePiece.index = i;
        e->posX.part16.integer = *x + offset;
        e->posY.part16.integer = 0x20;
        e->scrollWithBg = 1;
    }
}

/**
 * @brief B6880 | Balore entrance cutscene: waits until the player is near
 * (Entity_DistanceToPoint distance <= 0xBFFFFF: takes the input, stateFlags bit 0, song
 * 0x1010), then closer (<= 0x3FFFFF, Music_SetCurrent(0x10)); after a 0xA0-frame
 * rumble (song 0xFF on frame 0x50) it spawns BaloreIntroPart_Update, waits, runs
 * Balore_CreateHand and a few timers, then releases the input, creates the real
 * Balore (EnemyBaloreUpdate / EnemyBaloreCreate) at its position and
 * deletes itself. While the input is taken the player walks towards it in
 * state 2 and is frozen (0x80) after state 5.
 */
void BaloreIntro_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *e;
    s32 input;

    switch (entity->state)
    {
    case 0:
        entity->timer = 0x80;
        entity->state = 1;
        break;
    case 1:
    {
        struct EwramData_EntityData *player = gEwramData->playerState.player;
        if (Entity_DistanceToPoint(entity, player->posX.whole, player->posY.whole) > 0xBFFFFF)
            break;
        entity->state = 2;
        PlaySong(0x1010);
        gEwramData->gameState.stateFlags |= 1;
        Game_SetMenusEnabled(0);
        break;
    }
    case 2:
    {
        struct EwramData_EntityData *player = gEwramData->playerState.player;
        if (Entity_DistanceToPoint(entity, player->posX.whole, player->posY.whole) > 0x3FFFFF)
            break;
        entity->state = 3;
        Music_SetCurrent(0x10);
        break;
    }
    case 3:
        entity->state = 4;
        entity->timer = 0xA0;
        PlaySong(0x100);
        break;
    case 4:
        if (entity->timer)
        {
            if (entity->timer == 0x50)
                PlaySong(0xFF);
            entity->timer--;
            break;
        }
        e = EntityCreateInRange(0x49, 0xCF, BaloreIntroPart_Update);
        if (e)
        {
            e->ext.balorePiece.owner = entity;
            e->posX.whole = entity->posX.whole;
            e->posY.whole = entity->posY.whole;
            e->scrollWithBg = 1;
        }
        entity->timer = 0x80;
        entity->state = 5;
    case 5:
        if (entity->timer)
        {
            entity->timer--;
            break;
        }
        entity->state = 6;
        break;
    case 6:
        Balore_CreateHand(entity, 0x40, 0x90, 1, 1);
        entity->state = 7;
        entity->timer = 0x70;
        break;
    case 7:
        if (entity->timer)
        {
            entity->timer--;
            break;
        }
        entity->state = 8;
        entity->timer = 0x30;
        break;
    case 8:
        if (entity->timer)
        {
            entity->timer--;
            break;
        }
        entity->state = 9;
        entity->timer = 0;
        break;
    case 9:
        if (entity->timer)
        {
            entity->timer--;
            break;
        }
        entity->state = 10;
        break;
    case 10:
        entity->hidden = 1;
        entity->state = 11;
        break;
    case 11:
        gEwramData->gameState.stateFlags &= ~1;
        Game_SetMenusEnabled(1);
        SharedObjPalette_ResetSlots();
        ObjGfx_Unload((s8)entity->ext.balore.gfx);
        e = Entity_CreateObject(EnemyBaloreUpdate);
        if (e)
        {
            e->enemyId = entity->enemyId;
            e->spawnIndex = entity->spawnIndex;
            e->scrollWithBg = 1;
            e->posX.whole = entity->posX.whole;
            e->posY.whole = entity->posY.whole;
            EnemyBaloreCreate(e);
        }
        EntityDelete(entity);
        return;
    }
    Anim_Update(entity);
    if (gEwramData->gameState.stateFlags & 1)
    {
        input = 0;
        if (entity->state == 2)
        {
            s32 px = gEwramData->playerState.player->posX.whole;
            input = entity->posX.whole > px ? 0x10 : 0x20;
        }
        else if (entity->state > 5)
            input = 0x80;
        SetPlayerInput(2, input);
    }
}

/**
 * @brief B6AE8 | Hit: set bit 0 of unk_502; when killed (Enemy_TakeDamage)
 * also bit 1, Enemy_DropRewards, palette into 0x53E bits 3-6, state 4 and
 * Player_SetSelectedTarget(NULL).
 */
void Balore_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    entity->ext.balore.flags |= 1;
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        entity->ext.balore.flags |= 2;
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->state = 4;
        entity->subState = 0;
        Player_SetSelectedTarget(NULL);
    }
}

/**
 * @brief B6B3C | Player_ApplyContactDamage with (0x4B, 2, 0, 1) at the entity position.
 */
void BaloreSpark_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 2;
    contactDamage.unused = 0;
    contactDamage.attack = 0x4B;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief B6B68 | HitEffect_SpawnAtContact(attacker, entity, 0) and play sound 0x6A.
 */
void BaloreHandHitBox_OnHit(struct EwramData_EntityData *entity, s32 attacker)
{
    HitEffect_SpawnAtContact(attacker, entity, 0);
    PlaySong(0x6A);
}

/* Integer part of a 16.16 value, rounded towards zero. */
#define FIXED_TO_INT_TRUNC(a) (((a) < 0) ? -(-(a) >> 16) : ((a) >> 16))

/**
 * @brief B6B80 | Angle from the entity (offset by 0x20, 0x10) to the
 * target position given by Target_GetX / Target_GetY.
 */
u16 Balore_GetAngleToTarget(struct EwramData_EntityData *entity)
{
    s16 pos = entity->posX.part16.integer;
    s32 dx = (s16)(FIXED_TO_INT_TRUNC(Target_GetX(entity)) - pos - 0x20);
    s32 dy;

    pos = entity->posY.part16.integer;
    dy = (s16)(FIXED_TO_INT_TRUNC(Target_GetY(entity)) - pos - 0x10);
    return ArcTan2(dx, dy);
}

/* BaloreEmitter_Draw: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */

/**
 * @brief B6BF4 | Like Entity_DrawWithVelocityCopies, but 8 copies with the step ext.baloreEmitter.stepX / _Y.
 */
void BaloreEmitter_Draw(struct EwramData_EntityData *entity)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;
    s32 i;

    entity->posX.whole = oldX + entity->ext.baloreEmitter.stepX / 2;
    entity->posY.whole = oldY + BALORE_EMITTER_STEP_Y(entity) / 2;
    for (i = 8; i != 0; i--)
    {
        Sprite_Draw(entity);
        entity->posX.whole += entity->ext.baloreEmitter.stepX;
        entity->posY.whole += BALORE_EMITTER_STEP_Y(entity);
    }
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}
