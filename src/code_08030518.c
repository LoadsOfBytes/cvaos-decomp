/* SkillDeathUse; graphics
 * loading as in SkillBlueSoulForm2Update. */
#include "code_08009A0.h"
#include "blue_soul_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
#include "macros.h"
#define GRAPHICS_CONTEXT ((struct PlayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, playerState)))
extern u8 sSkillDeathGfx[], sSkillDeathPalette[], sSkillDeathSpriteSheet[];
extern s32 BlueSoul_ShouldEnd(struct EwramData_EntityData *);
extern void SkillDeathSickleUpdate(struct EwramData_EntityData *);

/**
 * @brief 30518 | Death blue soul (ends via BlueSoul_ShouldEnd): load its graphics,
 * hidden, Hit_SetTimerOverride(2, 0x14), skillBaseAttack/skillAttackAttributes = 0x14/1; follow 0x20 px above
 * the caster and every 0x38 frames (first after 8) spawn a SkillDeathSickleUpdate
 * child (BlueSoul_CreateEntity) at a random offset RandomSigned(0x40) per axis.
 */
void SkillDeathUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = BLUE_SOUL_OWNER(entity);

    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
    {
        u8 *graphics = sSkillDeathGfx;
        u8 *palette = sSkillDeathPalette;
        u8 *sprite = sSkillDeathSpriteSheet;
        GRAPHICS_CONTEXT->blueSoulGfx = ObjGfx_LoadResource(graphics, 0x10000);
        ObjPalette_LoadRange(palette, 0, 1, 3);
        Sprite_Attach(entity, sprite, GRAPHICS_CONTEXT->blueSoulGfx, 3);
    }
        entity->hidden = 1;
        Hit_SetTimerOverride(2, 0x14);
        {
            u16 a = 0x14;
            u16 b = 1;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = b;
        }
        entity->timer = 8;
        entity->state++;
    case 1:
        entity->flipX = parent->flipX;
        entity->posX.part16.integer = parent->posX.part16.integer;
        entity->posY.part16.integer = parent->posY.part16.integer - 0x20;
        if (--entity->timer == 0)
        {
            struct EwramData_EntityData *child = BlueSoul_CreateEntity(parent, SkillDeathSickleUpdate);
            if (child != NULL)
            {
                child->posX.part16.integer = entity->posX.part16.integer + RandomSigned(0x40);
                child->posY.part16.integer = entity->posY.part16.integer + RandomSigned(0x40);
                child->scrollWithBg = 1;
            }
            entity->timer = 0x38;
        }
    }
}
