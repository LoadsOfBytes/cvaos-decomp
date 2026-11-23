/* Spawn a pickup-like object at (x, y): graphics 081C15F4 / 0820ED60
 * (palette 7), touch callback Pickup_Collect, box 0x0808FCFC passed by value
 * (x -4, y -4, w 8, h 8); large (frame 0x27, spawnParam1 15, update
 * Pickup_Update) or small (frame 0x28, spawnParam1 3); optionally thrown up. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern void Pickup_UpdateDriftingMana(struct EwramData_EntityData *);
extern void Pickup_Collect(struct EwramData_EntityData *);
extern void Pickup_Update(struct EwramData_EntityData *);
extern u8 sCommonObjectGfx[], sCommonObjectSpriteSheet[];

/**
 * @brief Create a small or large MP pickup with the existing hit callback, frame, percentage and optional upward velocity; return NULL on allocation failure.
 */
struct EwramData_EntityData *Pickup_CreateMana(s32 x, s32 y, s32 large, s32 jump)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Pickup_UpdateDriftingMana);
    s32 graphics;

    if (entity != NULL)
    {
        entity->spriteLayer = 0x13;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, 7);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Hit_Enable(entity, 0xB, Pickup_Collect, 0);
        entity->hit.flags |= 4;
        HitBox_SetAttackRaw(entity, 0x0808FCFC);
        entity->posX.part16.integer = x;
        entity->posY.part16.integer = y;
        entity->scrollWithBg = 1;
        entity->enemyId = 0;
        if (large)
        {
            entity->animFrame = 0x27;
            entity->spawnParam1 = 0xF;
            entity->updateFunc = (u32 *)Pickup_Update;
        }
        else
        {
            entity->animFrame = 0x28;
            entity->spawnParam1 = 3;
        }
        if (jump)
            entity->velY.whole = -0x20000;
    }
    return entity;
}
