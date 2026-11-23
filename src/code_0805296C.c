#include "code_08009A0.h"
#include "room_object_entity.h"
#include "code_0800B700.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Object15Orbiter_Update(struct EwramData_EntityData *);
extern u8 sObject15Gfx[], sUnk_0820A1C4[], sObject15SpriteSheet[];
void Object15Create(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;
    if (!Entity_LoadGraphics(entity, sObject15Gfx, sUnk_0820A1C4, 8, sObject15SpriteSheet)) goto fail;
    entity->animFrame = 0;
    entity->spriteLayer = 31;
    child = EntityCreateInRange(0x1B, 0x48, (u32 *)Object15Orbiter_Update);
    if (child == NULL)
    {
fail:
        EntityDelete(entity);
        return;
    }
    child->scrollWithBg = 1;
    ENTITY_OWNER(child) = entity;
    entity->ext.object15.angle = 0x6000;
    entity->ext.object15.radius = 0x200000;
    if ((gEwramData->gameState.gameFlagsTail[1] & 0x0A000000) == 0x02000000)
    {
        entity->state = 4;
        gEwramData->gameState.stateFlags |= 1;
        Game_SetMenusEnabled(0);
        gEwramData->gameState.gameFlagsTail[1] |= 0x08000000;
    }
    else entity->state = 1;
}
