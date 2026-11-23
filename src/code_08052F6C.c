/* Object 0x16 helpers 0x08052F6C..0x0805305B. */
#include "code_08009A0.h"
#include "room_object_entity.h"
#include "code/code_08039340.h"
#include "code/code_08050A3C.h"
#include "structs/ewram.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, void *resource, void *palette, u8 variant, void *sheet);

extern u8 sObject16Gfx[];
extern u8 sObject16Palette[];
extern u8 sObject16SpriteSheet[];
extern u8 sObject16PaletteAnim[];

/**
 * @brief 52F6C | Object 0x16 create: load the graphics, palette
 * sObject16PaletteAnim, layer 0x12, unk_500 = 0xF00, unk_504 = 0, unk_508 = 0x40,
 * unk_4F8 = 0x100.
 */
void Object16Create(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sObject16Gfx, sObject16Palette, 0, sObject16SpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    PaletteAnim_Start(sObject16Palette, sObject16PaletteAnim, entity->unk_510.part8.basePalette);
    entity->animFrame = 0;
    entity->spriteLayer = 0x12;
    entity->ext.object16.scale = 0xF00;
    entity->ext.object16.angle = 0;
    entity->ext.object16.spin = 0x40;
    entity->ext.object16.timer = 0x100;
}

/**
 * @brief Follow the parent and mirror its states: show with animation 2 when
 * it enters state 1, animation 3 at state 3, hide again at state 0.
 */
void Object17Overlay_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);

    entity->posX.whole = owner->posX.whole;
    entity->posY.whole = owner->posY.whole;
    switch (entity->state)
    {
        case 0:
            if (owner->state == 1)
            {
                entity->hidden = 0;
                Anim_StartIndexed(entity, 2, 0, 0);
                entity->state = 1;
            }
            break;
        case 1:
            if (owner->state == 3)
            {
                Anim_StartIndexed(entity, 3, 0, 0);
                entity->state = 2;
            }
            break;
        case 2:
            if (owner->state == 0)
            {
                entity->hidden = 1;
                entity->state = 0;
            }
            break;
    }
    Anim_Update(entity);
}
