/* Objects 0x2D..0x31 0x0805872C..0x08059493. */
#include "code_08009A0.h"
#include "room_object_entity.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
#include <string.h>

#define PLAYER_ENTITY (gEwramData->playerState.player)

extern s32 EntityIsOutsideScreen(struct EwramData_EntityData *entity, s32 margin);
extern void Entity_HideAboveY(struct EwramData_EntityData *entity, s32 minY);
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
/* Target_IsInRect (code_0806CF2C.c) takes u16 there; the callers here pass
 * sign-extended values. */
extern s32 Target_IsInRect(s16 left, s16 top, s16 width, s16 height);
extern void Effect_UpdateParticle(struct EwramData_EntityData *entity);

/* Graphics and animations of objects 0x2D / 0x2E. */
extern u8 sObject2DGfx[];
extern u8 sObject2DObject2EPalette[];
extern u8 sObject2DSpriteSheet[];
extern u8 sObject2EGfx[];
extern u8 sObject2ESpriteSheet[];
extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];
extern u8 sObject2EAnimScript[];

u8 Object2D_LoadGraphics(struct EwramData_EntityData *entity);
u8 Object2E_LoadGraphics(struct EwramData_EntityData *entity);
void Object2E_UpdateState(struct EwramData_EntityData *entity);
void Object2EPiece_Update(struct EwramData_EntityData *entity);
void Object2E_SpawnRisingParticle(struct EwramData_EntityData *entity, s32 x, s32 y);
u8 Object2F_LoadGraphics(struct EwramData_EntityData *entity);
void Object2F_UpdateState(struct EwramData_EntityData *entity);
void Object30Leaf_Update(struct EwramData_EntityData *entity);
u8 Object30_LoadGraphics(struct EwramData_EntityData *entity);
void Object30_UpdateState(struct EwramData_EntityData *entity);
u8 Object31_LoadGraphics(struct EwramData_EntityData *entity);
void Object31_UpdateState(struct EwramData_EntityData *entity);
void Object31_SpawnDust(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY);
extern void EntityAccelerateAndMoveX(struct EwramData_EntityData *entity, s32 maxSpeed);
extern void EntityAccelerateAndMoveY(struct EwramData_EntityData *entity, s32 maxSpeed);

/* Object 0x31 per variant (spawnParam0): start x, start y, animation, unk_4F8. */
struct Object31Words {
    s32 v[4];
};
extern const u16 sObject31Xs[4];
extern const u16 sObject31Ys[4];
extern const u16 sObject31Anims[4];
extern const struct Object31Words sUnk_08118674;
extern void EntityMoveY(struct EwramData_EntityData *entity);
extern void EntityMoveX(struct EwramData_EntityData *entity);

/* Start x / y of object 0x2F per variant (spawnParam0). */
extern const u16 sObject2FXs[3];
extern const u16 sObject2FYs[3];
extern u8 sObject2FFamilyGfx[];
extern u8 sUnk_0820A1C4[];
extern u8 sObject2FFamilySpriteSheet[];

/**
 * @brief 5872C | Object 0x2D, state 0: substate 10 = idle animations 0 -> 1
 * -> 2 / 3, 4 -> 0 (each on animation end, 0x53D bit 0); go to substate 12
 * when the player is within 0x50 x 0x60 px, where animations 3, 4 -> 0,
 * 2 -> 4, 1 -> 3, and back to 10 when the player leaves. Hidden while
 * outside the screen.
 */
void Object2D_UpdateState(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player = PLAYER_ENTITY;
    s32 dx = player->posX.whole - entity->posX.whole;
    s32 dy = player->posY.whole - entity->posY.whole;
    s32 dist;

    switch (entity->subState)
    {
        case 10:
            if (entity->animScriptIndex == 0 && entity->animDone)
                Anim_StartIndexed(entity, 1, 0, 0);
            if (entity->animScriptIndex == 1 && entity->animDone)
                Anim_StartIndexed(entity, 2, 0, 0);
            if (entity->animScriptIndex == 3 && entity->animDone)
                Anim_StartIndexed(entity, 0, 0, 0);
            if (entity->animScriptIndex == 4 && entity->animDone)
                Anim_StartIndexed(entity, 0, 0, 0);
            if ((dx < 0 ? -dx : dx) > 0x4FFFFF)
                break;
            if ((dy < 0 ? -dy : dy) > 0x5FFFFF)
                break;
            entity->subState = 12;
            break;
        case 12:
            if (entity->animScriptIndex == 3 && entity->animDone)
                Anim_StartIndexed(entity, 0, 0, 0);
            if (entity->animScriptIndex == 4 && entity->animDone)
                Anim_StartIndexed(entity, 0, 0, 0);
            if (entity->animScriptIndex == 2 && entity->animDone)
                Anim_StartIndexed(entity, 4, 0, 0);
            if (entity->animScriptIndex == 1 && entity->animDone)
                Anim_StartIndexed(entity, 3, 0, 0);
            dist = dx;
            if (dist < 0)
                dist = -dist;
            if (dist > 0x4FFFFF)
                goto out_of_range;
            dist = dy;
            if (dist < 0)
                dist = -dist;
            if (dist > 0x5FFFFF)
            {
            out_of_range:
                entity->subState = 10;
                return;
            }
            break;
    }
    if (EntityIsOutsideScreen(entity, 3))
    {
        entity->hidden = 1;
    }
    else
    {
        entity->hidden = 0;
        Entity_HideAboveY(entity, -0x18);
    }
}

/**
 * @brief 588EC | Object 0x2D create: load the graphics (delete the object
 * when that fails), animation 2, start in substate 10.
 */
void Object2DCreate(struct EwramData_EntityData *entity)
{
    if (!Object2D_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 2, 0, 0);
    entity->animTimer = 0;
    entity->animStep = 6;
    Anim_Update(entity);
    entity->spriteLayer = 0x12;
    entity->state = 0;
    entity->subState = 10;
}

/**
 * @brief 58938 | Object 0x2D update.
 */
void Object2DUpdate(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (entity->state == 0)
        Object2D_UpdateState(entity);
    Anim_Update(entity);
}

/**
 * @brief 58958 | Load the graphics of object 0x2D; 0 when no slot is free.
 */
u8 Object2D_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject2DGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sObject2DObject2EPalette, 2);
    Sprite_Attach(entity, sObject2DSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 589A4 | Spawn a Object2EPiece_Update piece of object 0x2E at offset
 * (offsetX, offsetY) with random frame and facing, plus a Object2E_SpawnRisingParticle
 * effect 8 px below it.
 */
void Object2E_SpawnPiece(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Object2EPiece_Update);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = entity->posX.whole + offsetX;
    child->posY.whole = entity->posY.whole + offsetY;
    child->spriteLayer = entity->spriteLayer;
    gfx = ObjGfx_Find(sObject2EGfx);
    child->basePalette = entity->basePalette;
    Sprite_Attach(child, sObject2ESpriteSheet, gfx, child->basePalette);
    Anim_StartIndexed(child, 0, 0, 1);
    Anim_Update(child);
    child->animTimer = RandomNumberGenerator() % 8;
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    child->flipX = RandomNumberGenerator() % 2;
    child->scrollWithBg = 1;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
    Object2E_SpawnRisingParticle(child, child->posX.whole, child->posY.whole + 0x80000);
}

/**
 * @brief 58A70 | Object 0x2E create: load the graphics (delete the object
 * when that fails), hidden (0x53C bit 5) until triggered.
 */
void Object2ECreate(struct EwramData_EntityData *entity)
{
    if (!Object2E_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    entity->spriteLayer = 0x12;
    entity->state = 0;
    entity->hidden = 1;
}

/**
 * @brief 58AB8 | Object 0x2E update.
 */
void Object2EUpdate(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (entity->state == 0)
        Object2E_UpdateState(entity);
}

/**
 * @brief 58AD0 | Load the graphics of object 0x2E; 0 when no slot is free.
 */
u8 Object2E_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject2EGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sObject2DObject2EPalette, 5);
    Sprite_Attach(entity, sObject2ESpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 58B1C | Object 0x2E, state 0: when the area (x-0x20, y-0x20,
 * 0x40 x 0x70) is entered, spawn three pieces (left, above, right) and
 * play sound 0x78.
 */
void Object2E_UpdateState(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0
        && Target_IsInRect(entity->posX.part16.integer - 0x20, entity->posY.part16.integer - 0x20, 0x40, 0x70))
    {
        entity->subState++;
        Object2E_SpawnPiece(entity, -0x50000, 0);
        Object2E_SpawnPiece(entity, 0, -0x60000);
        Object2E_SpawnPiece(entity, 0x50000, 0);
        PlaySong(0x78);
    }
}

/**
 * @brief 58B84 | Object 0x2E piece update: animate.
 */
void Object2EPiece_Update(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    Anim_Update(entity);
}

/**
 * @brief 58B94 | Spawn a rising Effect_UpdateParticle effect at (x, y)
 * one layer below the entity.
 */
void Object2E_SpawnRisingParticle(struct EwramData_EntityData *entity, s32 x, s32 y)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateParticle);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = x;
    child->posY.whole = y;
    child->velY.whole = -0x10000;
    child->spriteLayer = entity->spriteLayer - 1;
    child->timer = 0;
    child->ext.particle.flags = 2;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(child, sCommonObjectSpriteSheet, gfx, 7);
    Anim_Start(child, (struct AnimScript *)sObject2EAnimScript, 0, 0);
    Anim_Update(child);
    child->scrollWithBg = 1;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
}

/**
 * @brief 58C1C | Object 0x2F create: deleted when bossEventFlags bit 2 is set or
 * the graphics fail; random frame, start position by variant (spawnParam0).
 */
void Object2FCreate(struct EwramData_EntityData *entity)
{
    struct EwramData **ewramRef = &gEwramData;
    u16 xs[3];
    u16 ys[3];

    memcpy(xs, sObject2FXs, sizeof(xs));
    memcpy(ys, sObject2FYs, sizeof(ys));
    if (((*ewramRef)->gameState.bossEventFlags & 4) || !Object2F_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 6, 0, 1);
    Anim_Update(entity);
    entity->animTimer = RandomNumberGenerator() % 6;
    entity->spriteLayer = 0x1A;
    entity->scrollWithBg = 0;
    entity->hidden = 1;
    entity->posX.part16.integer = xs[entity->spawnParam0];
    entity->posY.part16.integer = ys[entity->spawnParam0];
}

/**
 * @brief 58CD8 | Object 0x2F update.
 */
void Object2FUpdate(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (entity->state == 0)
        Object2F_UpdateState(entity);
    Anim_Update(entity);
}

/**
 * @brief 58CF8 | Load the graphics of object 0x2F; 0 when no slot is free.
 */
u8 Object2F_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject2FFamilyGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sUnk_0820A1C4, 9);
    Sprite_Attach(entity, sObject2FFamilySpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 58D44 | Object 0x2F, state 0: appear once stateFlags bit 12 is set.
 */
void Object2F_UpdateState(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram = gEwramData;

    if (entity->subState == 0 && (ewram->gameState.stateFlags & 0x1000))
    {
        entity->subState = 1;
        entity->hidden = 0;
    }
}

/**
 * @brief 58D7C | Spawn a falling leaf (Object30Leaf_Update) at a random x at the
 * top of the screen: kind 0..2 (unk_4F8) picks animation, fall speed and
 * parallax; random spin acceleration (accelY) and facing.
 */
void Object30_SpawnFallingLeaf(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Object30Leaf_Update);
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 gfx;
    u16 affineIndex;

    if (child == NULL)
        return;
    child->posX.part16.integer = RandomNumberGenerator() % 0xF0;
    child->posY.whole = 0;
    child->ext.object30Leaf.kind = RandomNumberGenerator() % 3;
    child->spriteLayer = entity->spriteLayer;
    gfx = ObjGfx_Find(sObject2FFamilyGfx);
    child->basePalette = entity->basePalette;
    Sprite_Attach(child, sObject2FFamilySpriteSheet, gfx, child->basePalette);
    Anim_StartIndexed(child, child->ext.object30Leaf.kind + 2, 0, 1);
    Anim_Update(child);
    affineIndex = ObjAffine_Alloc(1);
    if ((s16)affineIndex < 0)
    {
        EntityDelete(child);
        return;
    }
    child->affineIndex = affineIndex;
    child->affine = 1;
    ObjAffine_Set(0, 0x10000, 0x10000, child->flipX, child->flipY, child->affineIndex);
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    child->flipX = RandomNumberGenerator() % 2;
    if (child->ext.object30Leaf.kind == 0)
        child->velY.whole = 0xA000;
    else if (child->ext.object30Leaf.kind == 1)
        child->velY.whole = 0x8000;
    else
        child->velY.whole = 0xC000;
    child->accelY.whole = (RandomNumberGenerator() & 0x3FF) - 0x200;
    child->ext.object30Leaf.scrollX = bg->xPos.whole;
    child->scrollWithBg = 0;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
}

/**
 * @brief 58EDC | Falling leaf: fall, spin (affine angle in accelX), follow
 * the BG 1 scroll with parallax 1/4, 1/8 or 3/8 by kind; deleted (with its
 * affine slot) when outside the screen.
 */
void Object30Leaf_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 parallax;

    EntityMoveY(entity);
    entity->accelX.whole += entity->accelY.whole;
    parallax = bg->xPos.whole - (s32)entity->ext.object30Leaf.scrollX;
    if (entity->ext.object30Leaf.kind == 0)
        parallax /= 4;
    else if (entity->ext.object30Leaf.kind == 1)
        parallax /= 8;
    else
        parallax = parallax * 3 / 8;
    entity->posX.whole -= parallax;
    entity->ext.object30Leaf.scrollX = bg->xPos.whole;
    ObjAffine_Set(entity->accelX.whole, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    if (EntityIsOutsideScreen(entity, 3))
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
    else
    {
        Anim_Update(entity);
    }
}

/**
 * @brief 58F88 | Object 0x30 create (leaf spawner): load the graphics
 * (delete the object when that fails), hidden, layer 0x19.
 */
void Object30Create(struct EwramData_EntityData *entity)
{
    if (!Object30_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    entity->spriteLayer = 0x19;
    entity->state = 0;
    entity->hidden = 1;
}

/**
 * @brief 58FD0 | Object 0x30 update.
 */
void Object30Update(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (entity->state == 0)
        Object30_UpdateState(entity);
}

/**
 * @brief 58FE8 | Same as Object2F_LoadGraphics.
 */
u8 Object30_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject2FFamilyGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sUnk_0820A1C4, 9);
    Sprite_Attach(entity, sObject2FFamilySpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 59034 | Object 0x30, state 0: once stateFlags bit 12 is set, spawn a
 * leaf every 0x20..0x9F frames.
 */
void Object30_UpdateState(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram = gEwramData;

    switch (entity->subState)
    {
        case 0:
            if (ewram->gameState.stateFlags & 0x1000)
                entity->subState = 1;
            break;
        case 1:
            if (entity->timer-- > 1)
                break;
            entity->timer = (RandomNumberGenerator() % 128) + 0x20;
            Object30_SpawnFallingLeaf(entity);
            break;
    }
}

/**
 * @brief 59090 | Object 0x31 create: deleted when bossEventFlags bit 2 is set or
 * the graphics fail; position, animation and unk_4F8 by variant (spawnParam0);
 * variants 0 and 2 get an affine slot.
 */
void Object31Create(struct EwramData_EntityData *entity)
{
    struct EwramData **ewramRef = &gEwramData;
    u16 xs[4];
    u16 ys[4];
    u16 anims[4];
    struct Object31Words values;
    u16 affineIndex;

    memcpy(xs, sObject31Xs, sizeof(xs));
    memcpy(ys, sObject31Ys, sizeof(ys));
    memcpy(anims, sObject31Anims, sizeof(anims));
    values = sUnk_08118674;
    if (((*ewramRef)->gameState.bossEventFlags & 4) || !Object31_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, anims[entity->spawnParam0], 0, 1);
    Anim_Update(entity);
    entity->animTimer = RandomNumberGenerator() % 6;
    entity->spriteLayer = 0x1A;
    entity->scrollWithBg = 0;
    entity->flipX = 0;
    if (entity->spawnParam0 == 0 || entity->spawnParam0 == 2)
    {
        affineIndex = ObjAffine_Alloc(1);
        if ((s16)affineIndex < 0)
        {
            EntityDelete(entity);
            return;
        }
        entity->affineIndex = affineIndex;
        entity->affine = 1;
        ObjAffine_Set(0, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    }
    entity->posX.part16.integer = xs[entity->spawnParam0];
    entity->posY.part16.integer = ys[entity->spawnParam0];
    entity->ext.object31.delay = values.v[entity->spawnParam0];
}

/**
 * @brief 591EC | Object 0x31, state 0: once stateFlags bit 12 is set, wait
 * unk_4F8 frames, then fall (variants 1, 3) or - variants 0, 2 - jump up
 * sideways with a dust effect and fall spinning (affine angle in unk_4FC,
 * spin speed unk_500 up to 0x60); deleted after leaving the screen.
 */
void Object31_UpdateState(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            if (gEwramData->gameState.stateFlags & 0x1000)
                entity->subState++;
            break;
        case 1:
            if (--entity->ext.object31.delay > 0)
                break;
            if (entity->spawnParam0 == 0 || entity->spawnParam0 == 2)
            {
                entity->subState = 10;
                entity->ext.object31.angle = 0;
                entity->ext.object31.speed = 0;
                entity->ext.object31.step = 4;
                entity->accelX.whole = 0x400;
                entity->accelY.whole = 0x200;
                entity->timer = 0x3C;
                Object31_SpawnDust(entity, 0, 0xA0000);
            }
            else
            {
                entity->subState++;
                entity->accelY.whole = 0x400;
            }
            break;
        case 2:
            EntityAccelerateAndMoveY(entity, 0x10000);
            if (EntityIsOutsideScreen(entity, 3))
                entity->subState++;
            break;
        case 3:
            if (entity->spawnParam0 == 0 || entity->spawnParam0 == 2)
                ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
        case 10:
            EntityAccelerateAndMoveX(entity, 0x4000);
            EntityAccelerateAndMoveY(entity, 0x2000);
            if ((s8)--entity->timer > 0)
                break;
            entity->subState++;
            entity->accelX.whole = 0;
            entity->accelY.whole = 0x400;
            break;
        case 11:
            entity->ext.object31.speed += entity->ext.object31.step;
            if (entity->ext.object31.speed > 0x5F)
                entity->ext.object31.speed = 0x60;
            entity->ext.object31.angle += entity->ext.object31.speed;
            ObjAffine_Set(entity->ext.object31.angle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
            EntityMoveX(entity);
            EntityAccelerateAndMoveY(entity, 0x10000);
            if (EntityIsOutsideScreen(entity, 3))
                entity->subState = 3;
            break;
    }
    if (entity->spawnParam0 == 0 || entity->spawnParam0 == 2)
        ObjAffine_Set(entity->ext.object31.angle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    Anim_Update(entity);
}

/**
 * @brief 593A8 | Object 0x31 update.
 */
void Object31Update(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (entity->state == 0)
        Object31_UpdateState(entity);
}

/**
 * @brief 593C0 | Same as Object2F_LoadGraphics.
 */
u8 Object31_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject2FFamilyGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sUnk_0820A1C4, 9);
    Sprite_Attach(entity, sObject2FFamilySpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 5940C | Spawn a Effect_UpdateParticle dust effect (animation 5) at offset
 * (offsetX, offsetY), one layer below the entity.
 */
void Object31_SpawnDust(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateParticle);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = entity->posX.whole + offsetX;
    child->posY.whole = entity->posY.whole + offsetY;
    child->spriteLayer = entity->spriteLayer - 1;
    child->timer = 0;
    child->ext.particle.flags = 0;
    gfx = ObjGfx_Find(sObject2FFamilyGfx);
    Sprite_Attach(child, sObject2FFamilySpriteSheet, gfx, entity->basePalette);
    Anim_StartIndexed(child, 5, 0, 0);
    Anim_Update(child);
    child->scrollWithBg = 0;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
}
