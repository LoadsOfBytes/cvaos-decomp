/* Page-title entity (slot 20) state machine (state at unk_508): state 1
 * places the entity at (0x35, 9) and moves to state 2; state 2 enables affine
 * slot unk_510 on the first frame and grows the scale by 0x1000 per frame for
 * 16 frames, then disables the affine flag; both draw via Sprite_Draw. */
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"

/* The canonical u16 affine-slot field preserves the original value/address
 * ordering without a separate local bitfield view. */

void MenuPageTitle_Update(struct EwramData_EntityData *entity)
{
    s32 t;

    switch (entity->unk_508.part8.unk_508)
    {
        case 0:
            break;
        case 1:
            entity->posX.part16.integer = 0x35;
            entity->posY.part16.integer = 9;
            entity->timer = 0;
            entity->unk_508.part8.unk_508 = 2;
        case 2:
            if (entity->timer == 0)
            {
                entity->palette = 14;
                {
                    entity->affineIndex = entity->unk_510.part16.unk_510;
                }
                entity->affine = 1;
            }
            if (entity->timer < 16)
            {
                t = entity->timer++;
                ObjAffine_Set(0, t << 12, t << 12, 0, 0, entity->unk_510.part16.unk_510);
            }
            else
            {
                entity->affine = 0;
            }
            Sprite_Draw(entity);
            break;
    }
}
