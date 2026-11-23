#include "code_08009A0.h"
#include "soul_skill_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
#include "macros.h"
#define GRAPHICS_CONTEXT ((struct PlayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, playerState)))
extern u8 sSkillBuerOrbitGfx[], sSkillBuerOrbitPalette[], sSkillBuerOrbitSpriteSheet[];

#define RADIUS(e) (*(s32 *)&(e)->unk_514)

/**
 * @brief Child of SkillBuerUse: animated hit sprite (box slot 6, 0x1414F6F6)
 * orbiting the parent at angle unk_510 (step unk_512) and radius
 * parent unk_514, dropping a small rising effect every fourth update.
 */
void SkillBuerOrbitUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;
    struct EwramData_EntityData *flame;
    s32 x, y;

    switch (entity->state)
    {
    case 0:
    {
        u8 *graphics = sSkillBuerOrbitGfx;
        u8 *palette = sSkillBuerOrbitPalette;
        u8 *sprite = sSkillBuerOrbitSpriteSheet;
        GRAPHICS_CONTEXT->blueSoulGfx = ObjGfx_LoadResource(graphics, 0x10000);
        ObjPalette_LoadRange(palette, 0, 1, 3);
        Sprite_Attach(entity, sprite, GRAPHICS_CONTEXT->blueSoulGfx, 3);
    }
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->spriteLayer = 0xC;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        Hit_Enable(entity, 6, 0, 0);
        HitBox_SetAttackRaw(entity, 0x1414F6F6);
        entity->hit.flags |= 4;
        entity->state++;
    case 1:
        x = RADIUS(parent) * Sin(SKILL_BUER_ANGLE(entity) + 0x4000);
        y = RADIUS(parent) * Sin(SKILL_BUER_ANGLE(entity));
        entity->posX.whole = parent->posX.whole + x;
        entity->posY.whole = parent->posY.whole + y;
        SKILL_BUER_ANGLE(entity) += SKILL_BUER_SPEED(entity);
        entity->timer++;
        if ((entity->timer & 3) == 0)
        {
            y = -0x10000;
            flame = Effect_CreateFromTable(entity->posX.whole + RandomSigned(0x20000), entity->posY.whole - 0x40000, 0, y >> 4, 6, 7);
            if (flame != NULL)
            {
                flame->velY.whole = y;
                flame->spriteLayer = 0xD;
            }
        }
        break;
    }
    entity->roomActive = 1;
    Anim_Update(entity);
}
