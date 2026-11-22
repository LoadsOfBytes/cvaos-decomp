/* Particles 0x080552C4..0x080554BB. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern void SaveSparkle_Draw(struct EwramData_EntityData *entity);
void SaveSparkle_Update(struct EwramData_EntityData *entity);

extern u8 sSaveWarpRoomGfx[];
extern u8 sSaveWarpRoomSpriteSheet[];
void SaveSparkle_Create(s32 x, s32 y, s32 accelX, s32 accelY, s32 phase, u8 palette, struct EwramData_EntityData *entity);

/**
 * @brief Set up the sparkle animation, fly 0xC frames with acceleration
 * accelX / accelY, then rise 8 frames and hide.
 */
void SaveSparkle_Update(struct EwramData_EntityData *entity)
{
    s32 gfx;

    switch (entity->state)
    {
        case 0:
            gfx = ObjGfx_Find(sSaveWarpRoomGfx);
            Sprite_Attach(entity, sSaveWarpRoomSpriteSheet, gfx, entity->palette);
            entity->drawFunc = (u32 *)SaveSparkle_Draw;
            Anim_StartIndexed(entity, 3, 0, 1);
            entity->timer = 0xC;
            entity->scrollWithBg = 1;
            entity->velY.whole = -0x10000;
            entity->state++;
            break;
        case 1:
            entity->posX.whole += entity->velX.whole;
            entity->posY.whole += entity->velY.whole;
            entity->velX.whole += entity->accelX.whole;
            entity->velY.whole += entity->accelY.whole;
            if (--entity->timer == 0)
            {
                entity->deleteRequested = 1;
                entity->state = 2;
                entity->timer = 0;
            }
            break;
        case 2:
            entity->posY.whole -= entity->velY.whole;
            if (++entity->timer > 8)
                entity->deleteRequested = 1;
            break;
    }
    Anim_Update(entity);
}

/**
 * @brief Spawn a sparkle at a random spot inside a width x height box around
 * the entity, with its velocity (accelX / accelY) and palette.
 */
void SaveSparkle_SpawnAround(struct EwramData_EntityData *entity, u16 width, u16 height)
{
    s32 x = entity->posX.whole + ((u16)((RandomNumberGenerator() & (width - 1)) - (width >> 1)) << 16);
    s32 y = entity->posY.whole + ((u16)((RandomNumberGenerator() & (height - 1)) - (height >> 1)) << 16);

    SaveSparkle_Create(x, y, entity->accelX.whole, entity->accelY.whole, 0, entity->palette, entity);
}

/**
 * @brief Create a sparkle (update SaveSparkle_Update) at (x, y) with velocity,
 * lifetime, a random drift, palette and owner, one layer below it.
 */
void SaveSparkle_Create(s32 x, s32 y, s32 accelX, s32 accelY, s32 phase, u8 palette, struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)SaveSparkle_Update);

    if (child != NULL)
    {
        child->posX.whole = x;
        child->posY.whole = y;
        child->accelX.whole = accelX;
        child->accelY.whole = accelY;
        child->unk_4FC.whole = phase;
        child->unk_500.whole = ((RandomNumberGenerator() & 0x1F) << 16) - 0x100000;
        child->unk_504.whole = ((RandomNumberGenerator() & 0x1F) << 16) - 0x100000;
        child->palette = palette;
        child->unk_518.whole = (s32)entity;
        child->spriteLayer = entity->spriteLayer + 1;
        {
            s32 affineSlot = entity->affineIndex;

            child->affineIndex = affineSlot;
        }
    }
}
