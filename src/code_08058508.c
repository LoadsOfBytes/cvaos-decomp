/* Objects 0x2B and 0x2C 0x08058508..0x0805872B. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern void EntityMoveX(struct EwramData_EntityData *entity);
extern s32 EntityIsOutsideScreen(struct EwramData_EntityData *entity, s32 margin);

/* Graphics of objects 0x2B / 0x2C. */
extern u8 sObject2BGfx[];
extern u8 sUnk_0820A1C4[];
extern u8 sObject2BPaletteAnim[];
extern u8 sObject2BSpriteSheet[];
extern u8 sObject2CGfx[];
extern u8 sObject2CSpriteSheet[];

u8 Object2B_LoadGraphics(struct EwramData_EntityData *entity);
void Object2CLayer_Update(struct EwramData_EntityData *entity);
void Object2C_SetupLayer(struct EwramData_EntityData *entity);
u8 Object2C_LoadGraphics(struct EwramData_EntityData *entity);

/**
 * @brief 58508 | Object 0x2B create: load the graphics (delete the object
 * when that fails), animation 1, layer 0x12.
 */
void Object2BCreate(struct EwramData_EntityData *entity)
{
    if (!Object2B_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 1, 0, 1);
    Anim_Update(entity);
    entity->spriteLayer = 0x12;
    entity->state = 0;
}

/**
 * @brief 58544 | Object 0x2B update: animate.
 */
void Object2BUpdate(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    Anim_Update(entity);
}

/**
 * @brief 58554 | Load the graphics of object 0x2B (with palette data
 * sObject2BPaletteAnim); 0 when no slot is free.
 */
u8 Object2B_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject2BGfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sUnk_0820A1C4, 1);
    PaletteAnim_Start(sUnk_0820A1C4, sObject2BPaletteAnim, entity->unk_510.part8.basePalette);
    Sprite_Attach(entity, sObject2BSpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 585B0 | Object 0x2C create: spawn four Object2CLayer_Update layers at a
 * random x, y 0x1E/0x3E/0x56/0x66, moving right with speeds 0x4000..0x1000.
 */
void Object2CCreate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;
    s32 i;
    s32 zero;

    for (i = 0, zero = 0; i <= 3; i++)
    {
        child = EntityCreateInRange(0x49, 0xCF, (u32 *)Object2CLayer_Update);
        if (child == NULL)
            continue;
        child->unk_518.part8.spawnIndex = zero;
        child->unk_518.part8.enemyId = entity->unk_518.part8.enemyId;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
        Object2C_SetupLayer(child);
        child->posX.whole = RandomNumberGenerator() & 0xFFFFFF;
        switch (i)
        {
            case 0:
                child->posY.part16.integer = 0x1E;
                child->velX.whole = 0x4000;
                child->animFrame = zero;
                break;
            case 1:
                child->posY.part16.integer = 0x3E;
                child->velX.whole = 0x3000;
                child->animFrame = i;
                break;
            case 2:
                child->posY.part16.integer = 0x56;
                child->velX.whole = 0x2000;
                child->animFrame = i;
                break;
            case 3:
                child->posY.part16.integer = 0x66;
                child->velX.whole = 0x1000;
                child->animFrame = i;
                break;
        }
    }
}

/**
 * @brief 58664 | Object 0x2C update: nothing.
 */
void Object2CUpdate(struct EwramData_EntityData *entity)
{
    u32 collision[4];
}

/**
 * @brief 5866C | Object 0x2C layer: move right and wrap to x -0x3C after
 * leaving the screen.
 */
void Object2CLayer_Update(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        EntityMoveX(entity);
        if (EntityIsOutsideScreen(entity, 6))
            entity->posX.part16.integer = -0x3C;
    }
}

/**
 * @brief 5869C | Set up an object 0x2C layer: load the graphics (delete it
 * when that fails), layer 0x1D.
 */
void Object2C_SetupLayer(struct EwramData_EntityData *entity)
{
    if (!Object2C_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    entity->spriteLayer = 0x1D;
    entity->state = 0;
    entity->scrollWithBg = 0;
}

/**
 * @brief 586E0 | Load the graphics of object 0x2C; 0 when no slot is free.
 */
u8 Object2C_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject2CGfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sUnk_0820A1C4, 0);
    Sprite_Attach(entity, sObject2CSpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}
