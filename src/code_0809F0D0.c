#include "code_08009A0.h"
#include "legion_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern struct EwramData_EntityData *Entity_CreateObject(u32 *update);
extern void LegionCoreDetail_Update(struct EwramData_EntityData *entity);
extern u8 sLegionSpriteSheet[];

/**
 * @brief 9F0D0 | Create a Legion core part LegionCoreDetail_Update (file 0x0809E0C0)
 * at offset (x, y) from the owner, shifted per kind 0..4 (+1/-37, -22/+2,
 * -3/-21, +28/-1, -3/+20 px; kept in unk_4F8 / unk_4FC): kind in unk_504,
 * layer +4, owner's palette, graphics sLegionSpriteSheet with the given tiles,
 * frame kind + 4, animMode = 2, owner in unk_4F4, boxes off. Returns the
 * part or NULL.
 */
struct EwramData_EntityData *LegionCorePart_CreateDetail(struct EwramData_EntityData *entity, s32 x, s32 y, s32 kind, s32 tiles)
{
    struct EwramData_EntityData *part = Entity_CreateObject((u32 *)LegionCoreDetail_Update);

    if (part != NULL)
    {
    LEGION_PART_OFFSET_X(part) = x;
    LEGION_PART_OFFSET_Y(part) = y;
    switch (kind)
    {
    case 0:
        LEGION_PART_OFFSET_X(part) += 0x10000;
        LEGION_PART_OFFSET_Y(part) += -0x250000;
        break;
    case 1:
        LEGION_PART_OFFSET_X(part) += -0x160000;
        LEGION_PART_OFFSET_Y(part) += 0x20000;
        break;
    case 2:
        LEGION_PART_OFFSET_X(part) += -0x30000;
        LEGION_PART_OFFSET_Y(part) += -0x150000;
        break;
    case 3:
        LEGION_PART_OFFSET_X(part) += 0x1C0000;
        LEGION_PART_OFFSET_Y(part) += -0x10000;
        break;
    case 4:
        LEGION_PART_OFFSET_X(part) += -0x30000;
        LEGION_PART_OFFSET_Y(part) += 0x140000;
        break;
    }
    part->posX.whole = entity->posX.whole + LEGION_PART_OFFSET_X(part);
    part->posY.whole = entity->posY.whole + LEGION_PART_OFFSET_Y(part);
    LEGION_PART_KIND(part) = kind;
    part->spriteLayer = entity->spriteLayer + 4;
    part->unk_510.part8.basePalette = entity->unk_510.part8.basePalette;
    Sprite_Attach(part, sLegionSpriteSheet, tiles, part->unk_510.part8.basePalette);
    part->animFrame = kind + 4;
    part->animMode = 2;
    part->unk_518.part8.enemyId = entity->unk_518.part8.enemyId;
    LEGION_PART_OWNER(part) = entity;
    part->scrollWithBg = 0;
    part->flipX = 0;
    part->hit.flags |= 4;
    part->hit.flags |= 2;
    part->drawFunc = (u32 *)Sprite_DrawEntity;
    return part;
    }
    return NULL;
}
