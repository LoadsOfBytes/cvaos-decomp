/* ChaosDefeatParticle_Update: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern u8 sCommonObjectGfx[];
extern u8 sDefeatParticleSpriteSheet[];

/**
 * @brief Attach shared effect graphics with palette 8 and layer 0x19,
 * then accelerate and move until animation completion. Created by
 * the Chaos defeat sweep.
 */
void ChaosDefeatParticle_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
        {
            s32 gfx = ObjGfx_Find(sCommonObjectGfx);

            Sprite_Attach(entity, sDefeatParticleSpriteSheet, gfx, 8);
            Anim_StartIndexed(entity, 0, 0, 0);
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
            entity->spriteLayer = 0x19;
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
