#include "code_08009A0.h"
#include "room_object_entity.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern u8 sObject07ParallaxSpriteGfx[], sObject07ParallaxSpritePalette[], sObject07ParallaxSpriteSpriteSheet[];
/**
 * @brief Load the semi-transparent background sprite, then drift with its
 * velocity and 1/12 of the camera motion (parallax).
 */
void Object07ParallaxSprite_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
    {
        s32 graphics = ObjGfx_LoadResource(sObject07ParallaxSpriteGfx, 0);
        s32 palette;
        if (graphics < 0 || (palette = (s8)ObjPalette_FindOrLoad(sObject07ParallaxSpritePalette)) < 0)
        {
            entity->deleteRequested = 1;
            break;
        }
        Sprite_Attach(entity, sObject07ParallaxSpriteSpriteSheet, graphics, palette);
        entity->animFrame = entity->ext.object07.spriteFrame;
        entity->spriteLayer = 0x1E;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->objMode = 1;
        entity->state++;
    }
    /* fall through */
    case 1:
    {
        s32 x, y;
        struct BgLayerState *camera;
        entity->velX.whole = entity->accelX.whole;
        x = entity->posX.whole += entity->velX.whole;
        y = entity->posY.whole += entity->velY.whole;
        camera = &gEwramData->bgInfo[1];
        entity->posX.whole = x - camera->xVel.whole / 12;
        entity->posY.whole = y - camera->yVel.whole / 12;
        break;
    }
    }
}
