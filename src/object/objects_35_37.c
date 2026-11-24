/*
 * Room objects 0x35-0x37 (children, carry velocity, corrections) and the shop open helpers.
 */
#include "objects_35_37.h"
#include "bg_and_collision.h"
#include "room_object_entity.h"
#include "core.h"
#include "player_core.h"
#include "gameplay.h"
#include "sound.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "gba.h"
#include "syscalls.h"
#include "bg_effects.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "palette.h"
#include "palette_state.h"
#include "text12.h"
#include "data/data_0E5364.h"
#include "data/data_118B48.h"
#include "math_inline.h"
#include "obj_palette.h"

/* Object35Create (0x0805A18C). */
u8 Object35_LoadGraphics(struct EwramData_EntityData *entity);
void Object35_SpawnChild(struct EwramData_EntityData *parent);

/* Event flag param_1 of the bit array gameFlagsTail (gEwramData + 0x344). */
#define OBJECT_FLAG(ewram, flag) ((ewram)->gameState.gameFlagsTail[(flag) >> 5] & (1 << ((flag) & 0x1F)))


void Object35Child_Update(struct EwramData_EntityData *entity);
void Object35_Draw(struct EwramData_EntityData *entity);
u8 Object37_LoadGraphics(struct EwramData_EntityData *);
void Object37_SpawnChild(struct EwramData_EntityData *);
void Object37Child_Update(struct EwramData_EntityData *);

/**
 * @brief 5A18C | Object 0x35 create (door?): variant != 0 is deleted when its
 * flag spawnParam1 is set. Variant 0 (state 0) gets a hit box and is raised by
 * 6 px when its flag is set; variant 1 (state 10) sets up the window
 * (winOut 0x3F / 0x2F) and three Collision_SetBlock calls at its screen
 * position.
 */
void Object35Create(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram = gEwramData;
    u32 box = 0x0C10E8F8;
    s32 x;
    s32 y;

    if ((entity->spawnParam0 != 0
            && OBJECT_FLAG(ewram, entity->spawnParam1))
        || !Object35_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    if (entity->spawnParam0 == 0)
    {
        Hit_Enable(entity, 8, NULL, 0);
        HitBox_SetBody(entity, &box);
        HitBox_SetAttack(entity, &box);
    }
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    entity->spriteLayer = 0x12;
    if (entity->spawnParam0 == 0)
    {
        entity->animFrame = 2;
        Object35_SpawnChild(entity);
    }
    else
    {
        entity->animFrame = 1;
    }
    entity->state = 0;
    if (entity->spawnParam0 == 0)
    {
        entity->subState = 0;
        if (OBJECT_FLAG(gEwramData, entity->spawnParam1))
        {
            entity->posY.whole += 0x60000;
            entity->subState = 2;
        }
    }
    else
    {
        entity->subState = 10;
        gDisplayRegisters.winOut_L = 0x3F;
        gDisplayRegisters.winOut_H = 0x2F;
        x = (s16)entity->posX.part16.integer;
        {
            struct BgLayerState *bg = &gEwramData->bgInfo[1];

            x += (s16)bg->xPos.part16.integer;
            y = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer;
        }
        Collision_SetBlock(x, y - 8);
        Collision_SetBlock(x, y - 0x18);
        Collision_SetBlock(x, y - 0x28);
    }
    entity->scrollWithBg = 1;
    entity->flipX = 0;
    entity->ext.object35.homeX = entity->posX.whole;
    entity->ext.object35.homeY = entity->posY.whole;
}

/* Object35_UpdateState, Object35_SpawnChild. */

#define FLAG(e) ((e)->spawnParam1)

/**
 * @brief 5A30C | Update of a floor switch entity (presumably). State 0:
 * after the player (standingPlatform) has stood on it for 8 frames set event flag
 * spawnParam1 (gameState.gameFlagsTail bit set), song 0x133; 1: sink for 5 frames.
 * State 10: wait for the flag, song 0x113; 11: rise with OBJ window on and
 * dust particles every 8 frames, after 0x60 frames refresh the three map
 * tiles above; 12: OBJ window off.
 */
void Object35_UpdateState(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];

    switch (entity->subState)
    {
        case 0:
            if (gEwramData->playerState.standingPlatform == entity)
            {
                if ((s32)++entity->ext.object35.standFrames > 7)
                {
                    entity->subState++;
                    entity->velY.whole = 0;
                    entity->timer = 0;
                    gEwramData->gameState.gameFlagsTail[FLAG(entity) >> 5] |= 1 << (FLAG(entity) & 0x1F);
                    PlaySong(0x133);
                }
            }
            else
            {
                entity->ext.object35.standFrames = 0;
            }
            break;
        case 1:
            EntityMoveY(entity);
            entity->velY.whole = 0x10000;
            if (entity->timer++ > 4)
            {
                entity->subState++;
                entity->ext.object35.pressed = 0;
            }
            break;
        case 10:
            if (gEwramData->gameState.gameFlagsTail[FLAG(entity) >> 5] & (1 << (FLAG(entity) & 0x1F)))
            {
                entity->subState++;
                entity->velY.whole = 0x8000;
                entity->timer = 0;
                PlaySong(0x113);
            }
            break;
        case 11:
        {
            s32 phase;

            EntityMoveY(entity);
            gUnk_03002CB0.dispCnt |= 0x8000;
            entity->ext.object35.pressed = 1;
            phase = entity->timer & 7;
            if (phase == 0)
            {
                s32 vx = (RandomNumberGenerator() & 0x7FFF) - 0x4000;
                s32 vy = -0x8000 - (RandomNumberGenerator() & 0x7FFF);

                Effect_SpawnParticleWithDamping(entity->ext.object35.homeX, entity->ext.object35.homeY, vx, vy,
                             entity->spriteLayer - 1, phase, 0x20, 0xF, 6);
            }
            if (entity->timer++ > 0x5F)
            {
                s32 x;
                s32 y;

                entity->subState++;
                x = OBJECT35_HOME_PIXEL_X(entity) + (s16)bg->xPos.part16.integer;
                y = OBJECT35_HOME_PIXEL_Y(entity) + (s16)bg->yPos.part16.integer;
                Collision_ClearBlock(x, y - 8);
                Collision_ClearBlock(x, y - 0x18);
                Collision_ClearBlock(x, y - 0x28);
            }
            break;
        }
        case 12:
            gUnk_03002CB0.dispCnt &= 0x7FFF;
            break;
    }
}

/**
 * @brief 5A50C | Spawn the child part of parent (slots 0x49..0xCF, update
 * Object35Child_Update): same position, one layer below, graphics sObject35Gfx
 * with palette sObject35SpriteSheet and parent's palette slot, frame 3, linked
 * to parent through unk_504.
 */
void Object35_SpawnChild(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, Object35Child_Update);
    s32 gfx;

    if (child != NULL)
    {
        child->posX.whole = parent->posX.whole;
        child->posY.whole = parent->posY.whole;
        child->spriteLayer = parent->spriteLayer - 1;
        gfx = ObjGfx_Find(sObject35Gfx);
        child->basePalette = parent->basePalette;
        Sprite_Attach(child, sObject35SpriteSheet, gfx, child->basePalette);
        child->animFrame = 3;
        child->hit.flags |= 4;
        child->hit.flags |= 2;
        child->ext.object35Child.owner = parent;
        child->scrollWithBg = 1;
        child->drawFunc = Sprite_DrawEntity;
    }
}

/* Objects 0x35 / 0x36 helpers 0x0805A590..0x0805A747. */

/**
 * @brief 5A590 | Object 0x35 update: Player_QueueEntity(.., 1) while spawnParam0 is 0,
 * Object35_UpdateState in state 0.
 */
void Object35Update(struct EwramData_EntityData *entity)
{
    u8 hitInfo[0x10];

    if (entity->spawnParam0 == 0)
        Player_QueueEntity(entity, 1);
    if (entity->state == 0)
        Object35_UpdateState(entity);
}

/**
 * @brief 5A5BC | Nothing.
 */
void Object35Child_Update(struct EwramData_EntityData *entity)
{
}

/**
 * @brief 5A5C0 | Draw; when unk_500 is set first draw a copy as OBJ window
 * (0x53E mode 2) at y = unk_4F8 + 0x30.
 */
void Object35_Draw(struct EwramData_EntityData *entity)
{
    s32 oldY = entity->posY.whole;

    if (entity->ext.object35.pressed != 0)
    {
        entity->objMode = 2;
        gUnk_03002CB0.dispCnt |= DCNT_WINOBJ;
        entity->posY.whole = entity->ext.object35.homeY + 0x300000;
        Sprite_Draw(entity);
        entity->objMode = 0;
    }
    entity->posY.whole = oldY;
    Sprite_Draw(entity);
}

/**
 * @brief 5A618 | Load the graphics (sObject35Gfx), palette and animation
 * set sObject35SpriteSheet; drawFunc Object35_Draw when spawnParam0 is set.
 */
u8 Object35_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject35Gfx, 0);

    if (gfx < 0)
        return 0;

    entity->basePalette = ObjPalette_FindOrLoadVariant(sObject35Palette, 0);
    Sprite_Attach(entity, sObject35SpriteSheet, gfx, entity->basePalette);
    if (entity->spawnParam0 != 0)
        entity->drawFunc = Object35_Draw;
    else
        entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 5A678 | Object 0x36 update: rotate BG2 (angle unk_4F8, scale
 * unk_500) around the entity position, then advance the angle by unk_4FE.
 */
void Object36Update(struct EwramData_EntityData *entity)
{
    struct BgAffineSrcData affineSrc;

    affineSrc.texX = 0x8000;
    affineSrc.texY = 0x8000;
    affineSrc.scrX = entity->posX.part16.integer;
    affineSrc.scrY = entity->posY.part16.integer;
    affineSrc.sx = entity->ext.object36.scale;
    affineSrc.sy = entity->ext.object36.scale;
    affineSrc.alpha = entity->ext.object36.angle;
    BgAffineSet(&affineSrc, (struct BgAffineDstData *)&gDisplayRegisters.bg2PA, 1);
    entity->ext.object36.angle += entity->ext.object36.spin;
}

/**
 * @brief 5A6C8 | Object 0x36 create: rotation speed and scale from the
 * sObject36Shapes entry selected by spawnParam0, then Player_RegisterSolidObject.
 */
void Object36Create(struct EwramData_EntityData *entity)
{
    s16 shapes[9][2];

    memcpy(shapes, sObject36Shapes, sizeof(shapes));
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    entity->hidden = 1;
    entity->ext.object36.radius = shapes[entity->spawnParam0][0] / 2;
    entity->ext.object36.spin = shapes[entity->spawnParam0][1];
    entity->ext.object36.scale = 0x200 - shapes[entity->spawnParam0][0] * 2;
    Player_RegisterSolidObject(entity, sObject36SolidCallbacks);
}

/* Object36_FloorCorrection: landing depth of a point on top of a round object; matched. */
#define TO_PIXELS(d) ((d) < 0 ? -(-(d) >> 16) : (d) >> 16)
static inline s32 ToPixels(s32 x) { return TO_PIXELS(x); }

/**
 * @brief Floor correction slot of the object 0x36 solid table: return the
 * push-up depth when the point is on the rotating surface, else 0.
 */
s16 Object36_FloorCorrection(struct EwramData_EntityData *entity, u16 tx, u16 ty)
{
    struct BgLayerState *cam;
    s32 ex;
    s32 ey;
    s32 sx;
    s32 sy;
    s32 dx;
    s32 dy;
    s32 r;
    s32 d;
    register s32 dist asm("r2"); /* FAKEMATCH: pinned register */

    cam = &gEwramData->bgInfo[1];
    ex = ToPixels(entity->posX.whole + cam->xPos.whole);
    {
        s32 t = entity->posY.whole + cam->yPos.whole;
        if (t < 0)
            ey = -(-t >> 16);
        else
            ey = t >> 16;
    }
    sy = (s16)ty;
    if (sy > ey - 4)
        return 0;
    sx = (s16)tx;
    {
        s32 x = ex - sx;
        dy = ey - sy;
        x *= x;
        dy *= dy;
        {
            s32 s = x + dy;
            dist = Sqrt(s);
        }
    }
    dx = sx - ex;
    dy = ey - sy;
    if (dx < 0)
        dx = -dx;
    r = (s16)entity->ext.object36.radius;
    if (dx > r - 6)
        return 0;
    if (dist < r - 8)
        return 0;
    if (dist >= r + 8)
        return 0;
    if (gEwramData->playerState.player->velY.whole < 0)
        return 0;
    /* the loop doubles the reference count of dy so that it gets r4 */
    do /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
    {
        s32 t = ex - sx;
        register s32 rr asm("r2") = r * r; /* FAKEMATCH: pinned register */
        t *= t;
        d = dy - (u16)Sqrt(rr - t);
    }
    while (0);
    if (d >= 0)
        return 0;
    return d;
}

/**
 * @brief Wall-right correction slot of the object 0x36 solid table: none,
 * return 0.
 */
s32 Object36_WallRightCorrection(struct EwramData_EntityData *entity)
{
    return 0;
}

/**
 * @brief Wall-left correction slot of the object 0x36 solid table: none, return
 * 0.
 */
s32 Object36_WallLeftCorrection(struct EwramData_EntityData *entity)
{
    return 0;
}

/* Object36_GetCarryVelocity: bounce vector of a round object against a point; matched. */
static inline u16 Distance(s32 x, s32 y, s32 tx, s32 ty)
{
    x -= tx;
    y -= ty;
    x *= x;
    y *= y;
    {
        register s32 s asm("r0") = x + y; /* FAKEMATCH: pinned register */
        return Sqrt(s);
    }
}

/**
 * @brief Carry callback slot of the object 0x36 solid table: return the carry
 * value for a point on the surface, else 0.
 */
s32 Object36_GetCarryVelocity(struct EwramData_EntityData *entity, u16 tx, u16 ty)
{
    struct BgLayerState *cam;
    s32 ex;
    s32 ey;
    s32 sx;
    s32 sy;
    s32 a;
    s32 b;
    s32 r;
    s32 second;
    s32 dy;

    cam = &gEwramData->bgInfo[1];
    {
        s32 d = entity->posX.whole + cam->xPos.whole;
        if (d < 0)
            ex = -(-d >> 16);
        else
            ex = d >> 16;
    }
    ey = ToPixels(entity->posY.whole + cam->yPos.whole);
    sy = (s16)ty;
    if (sy > ey - 4)
        return 0;
    sx = (s16)tx;
    {
        u16 dist = Distance(ex, ey, sx, sy);
        s32 radius = (s16)entity->ext.object36.radius + 6;
        dy = ey - sy;
        if (dist >= radius)
            return 0;
    }
    if (gEwramData->playerState.player->ext.player.actionFlags & 2)
        return 0;
    {
        register s32 dx asm("r1") = ex - sx; /* FAKEMATCH: pinned register */
        a = ArcTan2(dx, dy);
    }
    b = a - entity->ext.object36.spin;
    r = Sin(a + 0x4000);
    r *= (s16)entity->ext.object36.radius;
    {
        s32 second2 = Sin(b + 0x4000);
        {
            register s32 rad asm("r0") = (s16)entity->ext.object36.radius; /* FAKEMATCH: pinned register */
            second2 *= rad;
        }
        return r - second2;
    }
}

/**
 * @brief Ceiling correction slot of the object 0x36 solid table sObject36SolidCallbacks:
 * none, return 0.
 */
s32 Object36_CeilingCorrection(struct EwramData_EntityData *entity)
{
    return 0;
}

/* Keep the entity and room coordinate reads in their original order. */
static inline s32 EntityRoomX(struct EwramData_EntityData *entity, struct EwramData **ram)
{
    s32 x = (s16)entity->posX.part16.integer;
    struct BgLayerState *bg = &(*ram)->bgInfo[1];
    return x + (s16)bg->xPos.part16.integer;
}

static inline s32 EntityRoomY(struct EwramData_EntityData *entity, struct EwramData **ram)
{
    s32 y = (s16)entity->posY.part16.integer;
    struct BgLayerState *bg = &(*ram)->bgInfo[1];
    return y + (s16)bg->yPos.part16.integer;
}

/* Initialize object 0x37, including its five collision probes. */
void Object37Create(struct EwramData_EntityData *entity)
{
    struct EwramData **ewram = &gEwramData;
    u16 graphics;
    s32 scale;
    s32 x, y;
    if (!Object37_LoadGraphics(entity))
        goto fail;
    entity->animFrame = 1;
    entity->flipX = 0;
    entity->spriteLayer = 9;
    graphics = ObjAffine_Alloc(1);
    if ((s16)graphics < 0)
    {
    fail:
        EntityDelete(entity);
        return;
    }
    entity->affineIndex = graphics;
    entity->affine = 1;
    scale = 0x10000;
    ObjAffine_Set(0, scale, scale, entity->flipX, entity->flipY, entity->affineIndex);
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    entity->state = 0;
    x = entity->posX.whole - (*ewram)->playerState.player->posX.whole;
    if (Abs(x) < 0x300000)
    {
        entity->ext.object37.angle = 0x3000;
        entity->subState = 1;
        ObjAffine_Set(0x3000, scale, scale, entity->flipX, entity->flipY, entity->affineIndex);
        entity->ext.object37.closed = 0;
    }
    else
    {
        entity->ext.object37.closed = 1;
        x = EntityRoomX(entity, ewram);
        y = EntityRoomY(entity, ewram);
        x -= 0x10;
        Collision_SetBlock(x, y + 8);
        Collision_SetBlock(x, y - 8);
        Collision_SetBlock(x, y - 0x18);
        Collision_SetBlock(x, y - 0x28);
        Collision_SetBlock(x, y - 0x38);
    }
    Object37_SpawnChild(entity);
}

/* Keep the entity and room coordinate reads in their original order. */

/* Keep the mask value (0 or 0x40), as used by the original condition. */
static inline u8 FlipMask(struct EwramData_EntityData *entity)
{
    return ((u8 *)entity)[0x58] & 0x40;
}

/**
 * @brief Main behaviour of object 0x37, called by Object37Update.
 */
void Object37_UpdateState(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player = gEwramData->playerState.player;
    s32 distance = entity->posX.whole - player->posX.whole;
    s32 x, y;
    switch (entity->subState)
    {
    case 0:
        if ((s32)entity->ext.object37.angle > 0)
        {
            entity->ext.object37.angle -= 0x100;
            if ((s32)entity->ext.object37.angle < 0)
                entity->ext.object37.angle = 0;
        }
        if (Abs(distance) >= 0x300000 && FlipMask(player))
        {
            entity->subState = 1;
            PlaySong(0x113);
        }
        break;
    case 1:
        if ((s32)entity->ext.object37.angle < 0x3000)
        {
            entity->ext.object37.angle += 0x100;
            if ((s32)entity->ext.object37.angle > 0x3000)
                entity->ext.object37.angle = 0x3000;
        }
        else if (entity->ext.object37.closed != 0)
        {
            entity->ext.object37.closed = 0;
            x = EntityRoomX(entity, &gEwramData);
            y = EntityRoomY(entity, &gEwramData);
            x -= 0x10;
            Collision_ClearBlock(x, y + 8);
            Collision_ClearBlock(x, y - 8);
            Collision_ClearBlock(x, y - 0x18);
            Collision_ClearBlock(x, y - 0x28);
            Collision_ClearBlock(x, y - 0x38);
        }
        if (Abs(distance) >= 0x300000 && !FlipMask(player))
        {
            entity->subState = 0;
            entity->ext.object37.closed = 1;
{
                s32 px = (s16)entity->posX.part16.integer;
                struct BgLayerState *bg = &gEwramData->bgInfo[1];
                x = px + (s16)bg->xPos.part16.integer;
                y = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer;
            }
            x -= 0x10;
            Collision_SetBlock(x, y + 8);
            Collision_SetBlock(x, y - 8);
            Collision_SetBlock(x, y - 0x18);
            Collision_SetBlock(x, y - 0x28);
            Collision_SetBlock(x, y - 0x38);
            PlaySong(0x113);
        }
        break;
    }
    ObjAffine_Set(entity->ext.object37.angle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
}

/**
 * @brief Create the child entity of object 0x37 (Object37Child_Update).
 */
void Object37_SpawnChild(struct EwramData_EntityData *owner)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, Object37Child_Update);
    struct EwramData **ewram = &gEwramData;
    s32 graphics;
    u16 transform;
    s32 two, scale;
    if (!entity)
        return;
    entity->posX.whole = owner->posX.whole;
    entity->posY.whole = owner->posY.whole;
    entity->spriteLayer = owner->spriteLayer + 1;
    graphics = ObjGfx_Find(sObject37Gfx);
    entity->basePalette = owner->basePalette;
    Sprite_Attach(entity, sObject37SpriteSheet, graphics, entity->basePalette);
    entity->animFrame = 2;
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    transform = ObjAffine_Alloc(1);
    if ((s16)transform < 0)
    {
        EntityDelete(entity);
        return;
    }
    entity->affineIndex = transform;
    entity->affine = 1;
    ObjAffine_Set(0, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    ENTITY_OWNER(entity) = owner;
    entity->scrollWithBg = 1;
    entity->drawFunc = Sprite_DrawEntity;
    if (Abs(entity->posX.whole - (*ewram)->playerState.player->posX.whole) < 0x300000)
    {
        entity->ext.object37Child.angle = -0x2000;
        entity->subState = 1;
        ObjAffine_Set(-0x2000, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    }
}

/* Helpers 0x0805AD9C..0x0805AFBF (object 0x37 and screen setup). */

/**
 * @brief 5AD9C | Copy subState from the parent (unk_4F8); move unk_4FC toward
 * 0 (subState 0) or -0x2000 (subState 1) by 0xAA per frame and use it as the
 * rotation of ObjAffine_Set.
 */
void Object37Child_Update(struct EwramData_EntityData *entity)
{
    entity->subState = ENTITY_OWNER(entity)->subState;
    switch (entity->subState)
    {
        case 0:
            if ((s32)entity->ext.object37Child.angle < 0)
            {
                entity->ext.object37Child.angle += 0xAA;
                if ((s32)entity->ext.object37Child.angle > 0)
                    entity->ext.object37Child.angle = 0;
            }
            break;
        case 1:
            if ((s32)entity->ext.object37Child.angle > -0x2000)
            {
                entity->ext.object37Child.angle -= 0xAA;
                if ((s32)entity->ext.object37Child.angle < -0x2000)
                    entity->ext.object37Child.angle = -0x2000;
            }
            break;
    }
    ObjAffine_Set(entity->ext.object37Child.angle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
}

/**
 * @brief 5AE00 | Object 0x37 update.
 */
void Object37Update(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (entity->state == 0)
        Object37_UpdateState(entity);
}

/**
 * @brief 5AE18 | Load the graphics of object 0x37; 0 when no slot is free.
 */
u8 Object37_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject37Gfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sObject37Palette, 0);
    Sprite_Attach(entity, sObject37SpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief Set up the shop BG layers, palette and language-specific graphics
 * and the text window.
 */
void Shop_SetupBackgrounds(void)
{
    u8 *localizedGfx;

    gUnk_03002CB0.dispCnt &= 0x13FF;
    gDisplayRegisters.bgCnt[0] = 0x1C04;
    gDisplayRegisters.bgCnt[1] = 0x1D01;
    gDisplayRegisters.bgOfs[0].hOfs = 0;
    gDisplayRegisters.bgOfs[0].vOfs = 0;
    BgPalette_Load(sShopBgPalette);
    switch (gEwramData->gameState.language)
    {
        case 2:
            Gfx_CopyResource(sShopBgGfxLanguage2, (void *)VRAM_BASE, 0);
            localizedGfx = sLocalizedWindowGfxLanguage2;
            break;
        case 3:
            Gfx_CopyResource(sShopBgGfxLanguage3, (void *)VRAM_BASE, 0);
            Gfx_CopyResource(sShopBgGfxLanguage3Extra, (void *)(VRAM_BASE + 0x2000), 0);
            localizedGfx = sLocalizedWindowGfxLanguage3;
            break;
        default:
            Gfx_CopyResource(sShopBgGfx, (void *)VRAM_BASE, 0);
            localizedGfx = sLocalizedWindowGfx;
            break;
    }
    Gfx_CopyResource(localizedGfx, (void *)(VRAM_BASE + 0xA000), 0);
    Text_SetTileBuffer((void *)(VRAM_BASE + 0x4000));
    BgPalette_LoadRange(sTextBgPalette, 0, 1, 0xF);
    Text_SetForegroundColor(8);
    Text_SetBackgroundColor(0);
    Text12_SetRegion(0, 0, 0x29, 4);
    Text12_ClearRegion();
    Text12_SetRegion(0, 1, 0x25, 3);
    Text12_SetCursor(0, 1);
}

/**
 * @brief Save the display, palette and BG1 map state, pause the game and
 * enter in-game sub-mode 9 (the shop) at step 0.
 */
void Shop_Open(void)
{
    struct GameState *gameState = &gEwramData->gameState;

    Display_SaveState();
    Palette_SaveState();
    BgMap_SaveSelection(1);
    gEwramData->bgControlMinWriteLevel = gEwramData->unusedWriteLevel = 3;
    gDisplayRegisters.bldCnt = 0xFF;
    gEwramData->paused = 1;
    gameState->subMode = 9;
    gameState->subModeStep = 0;
}
