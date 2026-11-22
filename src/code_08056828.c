#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern u8 Object22_LoadGraphics(struct EwramData_EntityData *entity);

/**
 * @brief 56828 | Object 0x22 init: deleted unless Object22_LoadGraphics accepts it;
 * box 0x0C20FFF0 in slot 8 (HitBox_SetBody / HitBox_SetAttack, disabled),
 * animation 3, layer 0x12, attack = equipmentDefenseBonus / 2 + equipmentDefenseBonus / 4 +
 * maxHP / 16, fixed screen position (0x40, 0x9D) and gravity 0x2800.
 */
void Object22Create(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    u8 state = 0; /* the original keeps this zero in its own register */
    u32 box = 0x0C20FFF0;

    if (!Object22_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Hit_Enable(entity, 8, 0, 0);
    HitBox_SetBody(entity, &box);
    HitBox_SetAttack(entity, &box);
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    Anim_StartIndexed(entity, 3, 0, 1);
    Anim_Update(entity);
    {
        s32 strength = (gEwramData->playerState.equipmentDefenseBonus >> 1) + (gEwramData->playerState.equipmentDefenseBonus >> 2);
        strength += gEwramData->playerStatus.maxHP >> 4;
        entity->attack = strength;
    }
    entity->spriteLayer = 0x12;
    entity->state = state;
    entity->posX.part16.integer = 0x40 - bg->xPos.part16.integer;
    entity->posY.part16.integer = 0x9D - bg->yPos.part16.integer;
    entity->accelY.whole = 0x2800;
}
