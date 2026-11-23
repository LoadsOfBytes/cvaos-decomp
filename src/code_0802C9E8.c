/* Red soul trail entity. The s16 layer
 * temporary was found with a permuter. */
#include "code_08009A0.h"
#include "red_soul_entity.h"
#include "code_08001194.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

struct Unk_0802A330 {
    s16 x;
    s16 y;
};

/**
 * @brief Create a trail entity with the given update and draw callbacks that
 * copies the owner sprite, palette, facing and red soul slot, filling
 * the 10-entry room position history with the owner position. Return
 * NULL when no entity or sprite is available.
 */
struct EwramData_EntityData *RedSoul_CreateHistoryTrail(struct EwramData_EntityData *owner, u32 *update, u32 *draw)
{
    s32 i;
    s32 graphics = Sprite_GetTileSlot(owner);
    u8 *index = &owner->spriteSlot;
    struct SpriteSheet *tmp = gEwramData->spriteSlots[*index].sheet;
    struct SpriteSheet *sprite = tmp;
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, update);
    struct Unk_0802A330 *trail;

    if (entity == NULL)
        return NULL;
    if (sprite == NULL)
    {
        EntityDelete(entity);
        return NULL;
    }
    Sprite_Attach(entity, sprite, graphics, owner->palette);
    {
        s16 layer = owner->spriteLayer + 1;
        entity->spriteLayer = layer;
    }
    entity->flipX = owner->flipX;
    entity->stage = RED_SOUL_SLOT(owner);
    trail = (struct Unk_0802A330 *)&entity->unk_4F4;
    for (i = 9; i != 0; i--)
    {
        trail[i].x = GetEntityRoomXPositionInteger(owner);
        trail[i].y = GetEntityRoomYPositionInteger(owner);
    }
    entity->drawFunc = draw;
    entity->scrollWithBg = 1;
    return entity;
}
