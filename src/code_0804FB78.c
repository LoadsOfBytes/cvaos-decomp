/* Spawn falling pieces around the entity, then finish. (flipX written through an s8 byte pointer so the
 * mask stays before the address; drawSkip = 1 shares the flipY constant). */
#include "code_08009A0.h"
#include "room_object_entity.h"
#include "code/code_08039340.h"
#include "code/code_080211F0.h"
#include "structs/ewram.h"

extern void BreakableBlockDust_Update(struct EwramData_EntityData *entity);

/**
 * @brief Child of object 0x08 (breakable block) when hit: for 32 frames spawn
 * dust pieces around the block (spread by its orientation), then
 * delete itself.
 */
void BreakableBlock_SpawnDust(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *piece;

    switch (entity->state)
    {
    case 0:
        entity->ext.object08Dust.gfx = ObjGfx_Find((u8 *)0x081C15F4);
        entity->timer = 0x20;
        entity->state++;
    case 1:
        if ((entity->timer & 3) == 0)
        {
            piece = EntityCreateInRange(0x49, 0xCF, (u32 *)BreakableBlockDust_Update);
            if (piece != NULL)
            {
                Sprite_Attach(piece, (void *)0x0820ED60, entity->ext.object08Dust.gfx, 6);
                Anim_StartIndexed(piece, 5, 0, 0);
                Anim_Update(piece);
                piece->spriteLayer = entity->spriteLayer;
                switch ((u8)entity->ext.object08.kind)
                {
                case 1:
                case 2:
                    piece->posX.part16.integer = entity->posX.part16.integer + RandomSigned(0x20);
                    piece->posY.part16.integer = entity->posY.part16.integer + RandomSigned(0x40);
                    break;
                case 3:
                case 4:
                    piece->posX.part16.integer = entity->posX.part16.integer + RandomSigned(0x40);
                    piece->posY.part16.integer = entity->posY.part16.integer + RandomSigned(0x20);
                    break;
                }
                { s32 f = (RandomNumberGenerator() >> 7) & 1; s8 *p = (s8 *)piece + 0x58; s32 t = f << 6; *p = (*p & ~0x40) | t; }
                { s32 f = (RandomNumberGenerator() >> 7) & 1; piece->flipY = f; }
                piece->scrollWithBg = 1;
                piece->drawFunc = (u32 *)Sprite_DrawEntity;
                piece->drawSkip = 1;
            }
        }
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->deleteRequested = 1;
        break;
    }
}
