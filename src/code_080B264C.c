#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

/* tested as a full word here (code_08001194.h declares it returning s16) */
extern s32 Collision_GetGroundOffset(s32 x, s32 y);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern u8 sBomberArmorRockArmorGfx[], sBomberArmorRockArmorPalette[], sRockArmorSpriteSheet[];

/**
 * @brief B264C | Debris piece (file 0x080B243C): graphics sBomberArmorRockArmorGfx /
 * sBomberArmorRockArmorPalette / sRockArmorSpriteSheet (palette 1, deleted when they cannot be
 * loaded), random animation 4..6, random velX -0x4000..0xC000 mirrored by
 * the facing and velY -1..-2 px; falls with gravity 0x4000 until it hits
 * the floor (Collision_GetGroundOffset 4 px below), bounces once at half speed and
 * keeps falling; finished outside the screen margin.
 */
void RockArmorRockPiece_Update(struct EwramData_EntityData *entity)
{
    s32 r, vx, x;
    struct BgLayerState *bg;

    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sBomberArmorRockArmorGfx, sBomberArmorRockArmorPalette, 1, sRockArmorSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        r = RandomNumberGenerator() & 3;
        if (r > 2)
            r = 2;
        Anim_StartIndexed(entity, r + 4, 0, 1);
        Anim_Update(entity);
        vx = RandomSigned(0x10000) - 0x4000;
        entity->velX.whole = vx;
        if (entity->flipX)
            entity->velX.whole = -vx;
        entity->velY.whole = -0x10000 - ((RandomNumberGenerator() & 0x1FFF) << 4);
        entity->state++;
        break;
    case 1:
        x = (s16)entity->posX.part16.integer;
        bg = &gEwramData->bgInfo[1];
        if (Collision_GetGroundOffset(x + (s16)bg->xPos.part16.integer,
                         (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer + 4))
        {
            entity->velY.whole = -(entity->velY.whole >> 1);
            entity->state++;
            break;
        }
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += 0x4000;
        break;
    case 2:
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += 0x4000;
        break;
    }
    if (!Entity_IsInRect(entity, -16, -16, 0x110, 0xC0))
        entity->deleteRequested = 1;
}
