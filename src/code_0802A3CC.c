/* The s16 layer and u8 facing temporaries were found with a permuter. */
#include "code_08009A0.h"
#include "red_soul_entity.h"
#include "code_08001194.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

struct Unk_0802A330 {
    s16 x;
    s16 y;
};

void RedSoulHistoryTrail_Update(struct EwramData_EntityData *entity);

/**
 * @brief Create a history trail entity (RedSoulHistoryTrail_Update) copying
 * the ghost sprite, palette and facing, with its position history
 * filled with the current position.
 */
struct EwramData_EntityData *SkillGhost_CreateHistoryTrail(struct EwramData_EntityData *owner)
{
    s32 graphics = Sprite_GetTileSlot(owner);
    u8 *index = &owner->spriteSlot;
    struct SpriteSheet *sprite = gEwramData->spriteSlots[*index].sheet;
    struct SpriteSheet *s = sprite;
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)RedSoulHistoryTrail_Update);
    struct Unk_0802A330 *trail;
    s32 i;

    if (entity == NULL)
        return NULL;
    if (sprite == NULL)
    {
        EntityDelete(entity);
        return NULL;
    }
    Sprite_Attach(entity, s, graphics, owner->palette);
    {
        s16 layer = owner->spriteLayer + 1;
        entity->spriteLayer = layer;
    }
    {
        u8 facing = owner->flipX;
        entity->flipX = facing;
    }
    entity->stage = RED_SOUL_SLOT(owner);
    trail = (struct Unk_0802A330 *)&entity->unk_4F4;
    for (i = 9; i != 0; i--)
    {
        trail[i].x = GetEntityRoomXPositionInteger(owner);
        trail[i].y = GetEntityRoomYPositionInteger(owner);
    }
    entity->scrollWithBg = 1;
    return entity;
}
