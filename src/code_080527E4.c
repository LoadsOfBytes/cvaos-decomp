/* Object 0x14 and helpers 0x080527E4..0x080528DF. */
#include "code_08001194.h"
#include "room_object_entity.h"
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, u8 *resource, void *palette, u8 variant, void *sheet);
extern void EntityDampVelocityX(struct EwramData_EntityData *entity, s32 factor);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *entity, s32 maxSpeed);
extern s32 EntityIsOutsideScreen(struct EwramData_EntityData *entity, s32 margin);

/* Graphics of object 0x14 and its update functions by variant (unk_514). */
extern u8 sObject14Gfx[];
extern u8 sObject14Palette[];
extern u8 sObject14SpriteSheet[];
extern void (*const sObject14VariantFuncs[])(struct EwramData_EntityData *entity);

/**
 * @brief 527E4 | Draw at the room position in unk_500 / unk_502 (made
 * screen relative to BG 1), then store the current room position there.
 */
void Object14_Draw(struct EwramData_EntityData *entity)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;
    struct BgLayerState *bg = &gEwramData->bgInfo[1];

    entity->posX.part16.integer = entity->ext.object14.prevX - bg->xPos.part16.integer;
    entity->posY.part16.integer = entity->ext.object14.prevY - bg->yPos.part16.integer;
    Sprite_DrawEntity(entity);
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
    entity->ext.object14.prevX = GetEntityRoomXPositionInteger(entity);
    entity->ext.object14.prevY = GetEntityRoomYPositionInteger(entity);
}

/**
 * @brief 52834 | Slow down sideways, fall (max 0x80000), end (0x53D bit 3)
 * outside the screen.
 */
void Object14Debris_Update(struct EwramData_EntityData *entity)
{
    EntityDampVelocityX(entity, 0x80);
    EntityAccelerateAndMove(entity, 0x80000);
    if (EntityIsOutsideScreen(entity, 4))
        entity->deleteRequested = 1;
}

/**
 * @brief Ceiling correction slot of the solid table sObject14Variant1SolidCallbacks (object 0x14
 * variant 1): none, return 0.
 */
s32 Object14_CeilingCorrection(void)
{
    return 0;
}

/**
 * @brief Floor correction slot of the solid table sObject14Variant1SolidCallbacks: none, return
 * 0.
 */
s32 Object14_FloorCorrection(void)
{
    return 0;
}

/**
 * @brief 52870 | Object 0x14 create: load the graphics (delete the object
 * when that fails), hidden.
 */
void Object14Create(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sObject14Gfx, sObject14Palette, 0, sObject14SpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    entity->flipX = 0;
    entity->hidden = 1;
    entity->scrollWithBg = 1;
}

/**
 * @brief 528C4 | Object 0x14 update: run the update of its variant.
 */
void Object14Update(struct EwramData_EntityData *entity)
{
    sObject14VariantFuncs[entity->unk_514.part16.spawnParam0](entity);
}
