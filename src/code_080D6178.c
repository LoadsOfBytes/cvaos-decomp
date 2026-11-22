/* BossDefeatParticle_Update (effect). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern u8 sCommonObjectGfx[];
extern u8 sDefeatParticleSpriteSheet[];

/**
 * @brief Attach shared effect graphics with palette 8 and layer 0x10,
 * then accelerate and move until animation completion. Used by
 * multiple defeat controllers; preserve initialization fallthrough.
 */
void BossDefeatParticle_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
        {
            s32 gfx = ObjGfx_Find(sCommonObjectGfx);

            Sprite_Attach(entity, sDefeatParticleSpriteSheet, gfx, 8);
            Anim_StartIndexed(entity, 0, 0, 0);
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
            entity->spriteLayer = 0x10;
            entity->objMode = 1;
            entity->state++;
        }
        case 1:
            EntityAccelerateAndMove(entity, 0x80000);
            if (Anim_Update(entity) == 3)
                EntityDelete(entity);
            break;
    }
}
