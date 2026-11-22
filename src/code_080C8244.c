/* KillerDollPartEffect_Update. */
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];
/* 6 bytes per child index; byte 1 is a y offset (s8). */
extern s8 sKillerDollPartOffsets[][6];

/**
 * @brief Attach cached effect graphics and follow the body part with its table-derived vertical offset while advancing animation.
 */
void KillerDollPartEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *doll = &gEwramData->entityData[entity->unk_4F4.part8.unk_4F5];

    switch (entity->state)
    {
        case 0:
        {
            s32 gfx = ObjGfx_Find(sCommonObjectGfx);

            Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, 7);
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->scrollWithBg = 1;
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
            entity->state++;
            break;
        }
        case 1:
            entity->posX.whole = doll->posX.whole;
            entity->posY.whole = doll->posY.whole - 0x100000;
            entity->posY.part16.integer += sKillerDollPartOffsets[(u8)doll->unk_500.part8.unk_501][1];
            break;
    }
    Anim_Update(entity);
}
