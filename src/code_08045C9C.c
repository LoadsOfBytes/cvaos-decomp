#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"

extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];

/**
 * @brief Install the sprite draw callback in state zero, move with acceleration and request deletion when the animation returns three.
 */
void Effect_UpdateSharedAnimation(struct EwramData_EntityData *entity)
{
    if (entity->state == 0)
    {
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->state = 1;
    }
    entity->posX.whole += entity->velX.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velY.whole += entity->accelY.whole;
    if (Anim_Update(entity) == 3) entity->deleteRequested = 1;
}

/**
 * @brief 45CEC | Spawn a Effect_UpdateSharedAnimation particle at (x, y) with the shared
 * graphics 081C15F4/0820ED60, animation script and palette palette.
 */
struct EwramData_EntityData *Effect_SpawnShared(s32 x, s32 y, s32 script, u32 palette)
{
    struct EwramData_EntityData *entity;
    s32 gfx;

    entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateSharedAnimation);
    if (entity != NULL)
    {
        gfx = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, palette);
        Anim_StartIndexed(entity, script, 0, 0);
        entity->posX.whole = x;
        entity->posY.whole = y;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
    }
    return entity;
}

/**
 * @brief Move with acceleration and request deletion outside the existing screen rectangle or when a nonzero countdown expires.
 */
void Effect_UpdateBoundedParticle(struct EwramData_EntityData *entity)
{
    entity->posX.whole += entity->velX.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velY.whole += entity->accelY.whole;
    if (Entity_IsInRect(entity, -0x10, 0, 0x110, 0xB0) == 0)
        entity->deleteRequested = 1;
    if (entity->timer != 0 && --entity->timer == 0)
        entity->deleteRequested = 1;
}

/**
 * @brief Create a shared-graphics frame particle at the supplied position and palette; return NULL on entity allocation failure.
 */
struct EwramData_EntityData *Effect_CreateSharedFrame(s32 x, s32 y, s32 frame, u32 palette)
{
    struct EwramData_EntityData *entity;
    s32 gfx;

    entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateBoundedParticle);
    if (entity != NULL)
    {
        gfx = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, palette);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->animFrame = frame;
        entity->posX.whole = x;
        entity->posY.whole = y;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
    }
    return entity;
}
