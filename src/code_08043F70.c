/* Spawn an item object at (x, y) from table 085250E4[index] (palette,
 * frame, animated flag): graphics 081C15F4 / 0820ED60, touch callback
 * Pickup_Collect, box 0x1010F8F8 by value (x -8, y -8, w 16, h 16);
 * updated by Pickup_Update. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
struct Unk_085250E4 { u8 pad_0[8]; u8 palette; u8 frame; u16 animated; };
extern struct Unk_085250E4 sGoldDrops[];
extern void Pickup_Collect(struct EwramData_EntityData *);
extern void Pickup_Update(struct EwramData_EntityData *);
extern u8 sCommonObjectGfx[], sCommonObjectSpriteSheet[];

/**
 * @brief Create a table-selected gold pickup with its frame or animation, palette and collection callback; return NULL on entity allocation failure.
 */
struct EwramData_EntityData *Pickup_CreateGold(s32 x, s32 y, s32 index)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Pickup_Update);
    if (entity != NULL)
    {
        struct Unk_085250E4 *info = &sGoldDrops[index];
        s32 graphics;
        entity->spriteLayer = 9;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, info->palette);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        if (info->animated)
        {
            entity->accelY.part16.integer = -8;
            entity->stage = 1;
            Anim_StartIndexed(entity, info->frame, 0, 1);
            Anim_Update(entity);
        }
        else
            entity->animFrame = info->frame;
        Hit_Enable(entity, 0xB, Pickup_Collect, 0);
        entity->hit.flags |= 4;
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->posX.part16.integer = x;
        entity->posY.part16.integer = y;
        entity->scrollWithBg = 1;
        entity->enemyId = 1;
        entity->spawnParam1 = index;
    }
    return entity;
}
