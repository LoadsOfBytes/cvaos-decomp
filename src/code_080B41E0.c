#include "code_08009A0.h"
#include "balore_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/* called with int arguments here (no re-extension of the s16 values) */
extern u16 ArcTan2(s32 x, s32 y);
extern s32 Target_GetX(struct EwramData_EntityData *entity);
extern s32 Target_GetY(struct EwramData_EntityData *entity);
extern u8 sBaloreSpriteSheet[];
extern const u8 sBaloreEyeAnimOffsets[];

#define FIXED_TO_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)

/**
 * @brief B41E0 | Balore part that follows the player: graphics
 * sBaloreSpriteSheet with the owner's tiles and palette, palette slot 0x1F
 * (state 3 when time is frozen); in state 1 it points from the owner
 * towards the player (ArcTan2 of the pixel distance, 32 / 16 px offset)
 * and is placed 3 * cos / 2 * sin + 2 px times the owner's unk_504 scale
 * from it; deleted with the owner's unk_502 bit 2. Animation 0x24 +
 * sBaloreEyeAnimOffsets[owner unk_4F6].
 */
void BaloreEye_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.balorePart.body;
    s32 dy = 0;
    s32 dx = 0;
    s32 o;
    s16 angle;

    if (owner->ext.balore.flags & 4)
        entity->state = 5;
    switch (entity->state)
    {
    case 0:
        Sprite_Attach(entity, sBaloreSpriteSheet, (s8)owner->ext.balore.gfx, owner->basePalette);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0x1F;
        if (gEwramData->gameState.bestiaryPreview)
        {
            entity->state = 3;
            break;
        }
        entity->state = 1;
    case 1:
        if (owner->state == 3)
            entity->state = 2;
        if (owner->ext.balore.flags & 2)
            entity->state = 4;
        o = (s16)owner->posX.part16.integer;
        dx = (s16)(FIXED_TO_INT(Target_GetX(owner)) - o - 0x20);
        o = owner->posY.part16.integer;
        angle = ArcTan2(dx, (s16)(FIXED_TO_INT(Target_GetY(owner)) - o - 0x10));
        dx = Sin(angle + 0x4000) * 3;
        dy = Sin(angle) * 2 + 0x20000;
        dx = FIXED_TO_INT(dx) * owner->ext.balore.scale;
        dy = FIXED_TO_INT(dy) * owner->ext.balore.scale;
        break;
    case 2:
    case 3:
    case 4:
        break;
    case 5:
        EntityDelete(entity);
        return;
    }
    entity->posX.whole = owner->posX.whole + dx;
    entity->posY.whole = owner->posY.whole + dy;
    Anim_StartIndexed(entity, sBaloreEyeAnimOffsets[(s8)owner->ext.balore.eyeAnim] + 0x24, 2, 0);
    Anim_Update(entity);
}
