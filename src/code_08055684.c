/* Unreferenced draw routine next to Object1DUpdate (warp room): draw the
 * object, then on alternating frame pairs a pulsing affine copy (frame 6) 28 px
 * above it. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
void Object1D_DrawPulsingOverlay(struct EwramData_EntityData *entity)
{
    u8 frame = entity->animFrame;
    entity->affine = 0;
    Sprite_Draw(entity);
    entity->affine = 1;
    if (gEwramData->frameCounter & 2)
    {
        s32 scale = (Sin(gEwramData->frameCounter << 11) >> 3) + 0xE000;
        ObjAffine_Set(0, scale, scale, 0, 0, (u8)entity->unk_4FC.part8.unk_4FC);
        entity->objMode = 1;
        entity->posY.part16.integer -= 28;
        entity->animFrame = 6;
        Sprite_Draw(entity);
        entity->objMode = 0;
        entity->posY.part16.integer += 28;
    }
    entity->animFrame = frame;
}
