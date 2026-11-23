#include "code_08009A0.h"
#include "save_point_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void SavePoint_UpdateAnimPart(struct EwramData_EntityData *);
extern u8 sSaveWarpRoomSpriteSheet[];

struct PartInit
{
    s32 anim;
    s32 palette;
    void *update;
    s32 layer;
};

/**
 * @brief Spawn the save point part sprites 0 and 3 of the four-entry part
 * table (update SavePoint_UpdateAnimPart) at the parent position.
 * Return 1 when at least one part was created.
 */
s32 SavePoint_SpawnAnimParts(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *entity;
    s32 created = 0;
    s32 i;
    struct PartInit parts[4] = {
        { 0, parent->ext.savePoint.frameBase + 2, SavePoint_UpdateAnimPart, 0x13 },
        { 2, parent->ext.savePoint.frameBase + 3, SavePoint_UpdateAnimPart, 0x14 },
        { 7, parent->ext.savePoint.frameBase + 3, SavePoint_UpdateAnimPart, 0x14 },
        { 4, parent->ext.savePoint.frameBase + 1, SavePoint_UpdateAnimPart, 0x14 },
    };

    for (i = 0; i <= 3; i++)
    {
        if ((u32)(i - 1) <= 1)
            continue;
        entity = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
        if (entity == NULL)
            continue;
        entity->posX.whole = parent->posX.whole;
        entity->posY.whole = parent->posY.whole;
        if (parts[i].anim == 2 || parts[i].anim == 7)
        {
            entity->posY.part16.integer -= 0x9F;
            if (parts[i].anim == 2)
                entity->posX.part16.integer -= 0x30;
            else
                entity->posX.part16.integer += 0x30;
        }
        entity->scrollWithBg = 1;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Sprite_Attach(entity, sSaveWarpRoomSpriteSheet, parent->ext.savePoint.graphics, parts[i].palette);
        Anim_StartIndexed(entity, parts[i].anim, 0, 1);
        Anim_Update(entity);
        entity->spriteLayer = parts[i].layer;
        entity->updateFunc = (u32 *)parts[i].update;
        entity->ext.savePointPart.owner = parent;
        entity->timer = 4;
        created = 1;
    }
    return created;
}
