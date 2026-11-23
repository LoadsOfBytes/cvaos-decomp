/* Object35Create (0x0805A18C). */
#include "code_08001194.h"
#include "room_object_entity.h"
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
#include "structs/main.h"


u8 Object35_LoadGraphics(struct EwramData_EntityData *entity);
void Object35_SpawnChild(struct EwramData_EntityData *parent);

/* Event flag param_1 of the bit array gameFlagsTail (gEwramData + 0x344). */
#define OBJECT_FLAG(ewram, flag) ((ewram)->gameState.gameFlagsTail[(flag) >> 5] & (1 << ((flag) & 0x1F)))

/**
 * @brief 5A18C | Object 0x35 create (door?): variant != 0 is deleted when its
 * flag spawnParam1 is set. Variant 0 (state 0) gets a hit box and is raised by
 * 6 px when its flag is set; variant 1 (state 10) sets up the window
 * (winOut 0x3F / 0x2F) and three Collision_SetBlock calls at its screen
 * position.
 */
void Object35Create(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram = gEwramData;
    u32 box = 0x0C10E8F8;
    s32 x;
    s32 y;

    if ((entity->spawnParam0 != 0
            && OBJECT_FLAG(ewram, entity->spawnParam1))
        || !Object35_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    if (entity->spawnParam0 == 0)
    {
        Hit_Enable(entity, 8, NULL, 0);
        HitBox_SetBody(entity, &box);
        HitBox_SetAttack(entity, &box);
    }
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    entity->spriteLayer = 0x12;
    if (entity->spawnParam0 == 0)
    {
        entity->animFrame = 2;
        Object35_SpawnChild(entity);
    }
    else
    {
        entity->animFrame = 1;
    }
    entity->state = 0;
    if (entity->spawnParam0 == 0)
    {
        entity->subState = 0;
        if (OBJECT_FLAG(gEwramData, entity->spawnParam1))
        {
            entity->posY.whole += 0x60000;
            entity->subState = 2;
        }
    }
    else
    {
        entity->subState = 10;
        gDisplayRegisters.winOut_L = 0x3F;
        gDisplayRegisters.winOut_H = 0x2F;
        x = (s16)entity->posX.part16.integer;
        {
            struct BgLayerState *bg = &gEwramData->bgInfo[1];

            x += (s16)bg->xPos.part16.integer;
            y = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer;
        }
        Collision_SetBlock(x, y - 8);
        Collision_SetBlock(x, y - 0x18);
        Collision_SetBlock(x, y - 0x28);
    }
    entity->scrollWithBg = 1;
    entity->flipX = 0;
    entity->ext.object35.homeX = entity->posX.whole;
    entity->ext.object35.homeY = entity->posY.whole;
}
