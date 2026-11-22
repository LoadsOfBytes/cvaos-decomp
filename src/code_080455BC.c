/* Effect_CreatePlayerFlash. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];
extern u8 *const sPlayerFlashInfo[];
void Effect_DrawPlayerFlash(struct EwramData_EntityData *entity);

/**
 * @brief Create the table-selected player flash; kind zero allocates an affine slot and plays sound 0xBC; retain the allocation assumption.
 */
struct EwramData_EntityData *Effect_CreatePlayerFlash(s32 kind)
{
    u8 *info = sPlayerFlashInfo[kind];
    struct EwramData_EntityData *entity = EntityCreateInRange(0x16, 0x19, (u32 *)EntityUpdateNothing);
    s32 gfx;

    entity->spriteLayer = 1;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, 6);
    entity->drawFunc = (u32 *)Effect_DrawPlayerFlash;
    entity->animFrame = info[0];
    entity->unk_4FC.part8.unk_4FD = info[1];
    if (kind == 0)
    {
        {
            u16 matrix = ObjAffine_Alloc(1);

            entity->affineIndex = matrix;
        }
        entity->affine = 1;
        PlaySong(0xBC);
    }
    entity->unk_4F8.whole = (s32)info;
    entity->unk_4FC.part8.unk_4FC = kind;
    return entity;
}
