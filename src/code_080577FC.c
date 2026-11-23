/* Objects 0x23 and 0x24 0x080577FC..0x080579A3. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

/* Graphics of objects 0x23 and 0x24 (ObjGfx_LoadResource / ObjPalette_FindOrLoadVariant /
 * Sprite_Attach arguments). */
extern u8 sObject21Object23Gfx[];
extern u8 sUnk_08209FD8[];
extern u8 sObject21Object23SpriteSheet[];
extern u8 sObject24Gfx[];
extern u8 sObject24SpriteSheet[];

u8 Object23_LoadGraphics(struct EwramData_EntityData *entity);
u8 Object24_LoadGraphics(struct EwramData_EntityData *entity);

/**
 * @brief 577FC | Object 0x23 update: animate.
 */
void Object23Update(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    Anim_Update(entity);
}

/**
 * @brief 5780C | Object 0x23 create: load the graphics (delete the object
 * when that fails), set animation 1 and layer 0x14.
 */
void Object23Create(struct EwramData_EntityData *entity)
{
    if (!Object23_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 1, 0, 1);
    Anim_Update(entity);
    entity->spriteLayer = 0x14;
    entity->objMode = 1;
    entity->state = 0;
}

/**
 * @brief 57858 | Load the graphics of object 0x23; 0 when no slot is free.
 */
u8 Object23_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject21Object23Gfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sUnk_08209FD8, 3);
    Sprite_Attach(entity, sObject21Object23SpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 578A4 | Object 0x24 create: load the graphics (delete the object
 * when that fails), set animation 2 with random animTimer (0..15) and
 * animStep (0..2), layer 0x13.
 */
void Object24Create(struct EwramData_EntityData *entity)
{
    if (!Object24_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 2, 0, 1);
    entity->animTimer = RandomNumberGenerator() % 16;
    entity->animStep = RandomNumberGenerator() % 3;
    Anim_Update(entity);
    entity->spriteLayer = 0x13;
    entity->objMode = 1;
    entity->state = 0;
}

/**
 * @brief 5790C | Object 0x24 update: animate on 7 of 8 frames at random and
 * toggle 0x53C bit 5 every frame (flicker).
 */
void Object24Update(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if ((RandomNumberGenerator() & 0xFF) < 0xE0)
        Anim_Update(entity);
    if (entity->timer++ & 1)
        entity->hidden = 1;
    else
        entity->hidden = 0;
}

/**
 * @brief 57958 | Load the graphics of object 0x24; 0 when no slot is free.
 */
u8 Object24_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject24Gfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sUnk_08209FD8, 7);
    Sprite_Attach(entity, sObject24SpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}
