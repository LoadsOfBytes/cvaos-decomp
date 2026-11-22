/* BuerChargeEffect_Update. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern u8 sBaelBuerGfx[];
extern u8 sBaelBuerSpriteSheet[];

/**
 * @brief Use cached Buer graphics and palette 6, run the selected animation with acceleration, then request deletion when animation finishes.
 */
void BuerChargeEffect_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
        {
            s32 gfx = ObjGfx_Find(sBaelBuerGfx);

            if (gfx < 0)
            {
                EntityDelete(entity);
                return;
            }
            entity->unk_510.part8.basePalette = 6;
            Sprite_Attach(entity, sBaelBuerSpriteSheet, gfx, 6);
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
            Anim_StartIndexed(entity, entity->unk_4F4.part8.unk_4F6, 0, 0);
            entity->spriteLayer = 0x11;
            entity->scrollWithBg = 1;
            entity->state++;
        }
        case 1:
            EntityAccelerateAndMove(entity, 0x80000);
            break;
    }
    if (Anim_Update(entity) == 3)
        entity->deleteRequested = 1;
}
