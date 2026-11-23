#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void EventScene03Debris_Update(struct EwramData_EntityData *);
extern const s8 sEventScene03Debris[];
extern u8 sWingedSkeletonSpriteSheet[];

/* Seven debris pieces using the owner's graphics and palette. */
void EventScene03_SpawnPartDebris(struct EwramData_EntityData *owner)
{
    s32 offset = 0;
    s32 i, x, vx, graphics;
    struct EwramData_EntityData *part;
    const s8 *info;
    for (i = 6; i >= 0; offset += 6, i--)
    {
        part = EntityCreateInRange(0x49, 0xCF, (u32 *)EventScene03Debris_Update);
        if (part != NULL)
        {
            info = sEventScene03Debris + offset;
            ENTITY_PARENT_INDEX(part) = ENTITY_INDEX(owner);
            part->spriteLayer = owner->spriteLayer;
            part->basePalette = owner->basePalette;
            part->flipX = owner->flipX;
            graphics = Sprite_GetTileSlot(owner);
            Sprite_Attach(part, sWingedSkeletonSpriteSheet, graphics, part->basePalette);
            part->scrollWithBg = 1;
            part->drawFunc = (u32 *)Sprite_DrawEntity;
            part->posX.whole = owner->posX.whole;
            part->posY.whole = owner->posY.whole;
            x = info[0];
            part->posY.part16.integer += info[1];
            part->animFrame = info[2];
            vx = info[3] << 12;
            part->velX.whole = vx;
            part->velY.whole = -(info[4] << 12);
            part->timer = info[5] / 2;
            if (part->flipX)
            {
                part->velX.whole = -vx;
                x = -x;
            }
            part->posX.part16.integer += x;
        }
    }
}

