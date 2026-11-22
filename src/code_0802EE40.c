/* Animated soul effect following its owner and sounding every 8 frames. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
#include "macros.h"
#define GRAPHICS_CONTEXT ((struct PlayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, playerState)))
extern u8 sUnk_081E116C[], sUnk_0820B64C[], sUnk_0823760C[];
/**
 * @brief Update of entry 2 of the blue-soul form table sBlueSoulForms used by
 * Skill_Devil_Manticore_Curly_Use (entries 0 and 1 use
 * SkillDevilManticoreSparksUpdate).
 */
void SkillBlueSoulForm2Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->unk_4F8.unk_4F8_entity;
    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
    {
        u8 *graphics = sUnk_081E116C;
        u8 *palette = sUnk_0820B64C;
        u8 *sprite = sUnk_0823760C;
        GRAPHICS_CONTEXT->blueSoulGfx = ObjGfx_LoadResource(graphics, 0x10000);
        ObjPalette_LoadRange(palette, 0, 1, 3);
        Sprite_Attach(entity, sprite, GRAPHICS_CONTEXT->blueSoulGfx, 3);
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->spriteLayer = 14;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        entity->state++;
    }
    /* fall through */
    case 1:
    {
        s32 timer;
        entity->posX.whole = owner->posX.whole;
        entity->posY.whole = owner->posY.whole;
        entity->flipX = owner->flipX;
        timer = entity->timer + 1;
        entity->timer = timer;
        if ((timer & 7) == 0) PlaySong(0x93);
        break;
    }
    }
    Anim_Update(entity);
}
