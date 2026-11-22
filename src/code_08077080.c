/* Draw the head from two oscillators and the shared body angle. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#define FIXED_INTEGER(x) ((x)<0 ? -(-(x)>>16) : (x)>>16)
/**
 * @brief Draw function of the head: place it on the neck path from its anchor
 * (unk_4FC) with the sway and wave angles, then draw the sprite.
 */
void WhiteDragon_Draw(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
    s32 wave, sway, x, y;
    entity->posX.part16.integer=entity->unk_4FC.part16.unk_4FC-camera->xPos.part16.integer+entity->unk_4F8.part16.unk_4F8;
    entity->posY.part16.integer=entity->unk_4FC.part16.unk_4FE[0]-camera->yPos.part16.integer+entity->unk_4F8.part16.unk_4FA;
    wave=(Sin((u16)entity->velX.part16.fraction)*entity->unk_500.part16.unk_500)/4096;
    sway=Sin((u16)entity->unk_500.part16.unk_502)/16;
    x=FIXED_INTEGER(Sin(entity->velX.part16.integer+wave+sway+0x4000)*entity->velY.part16.integer);
    y=FIXED_INTEGER(Sin(entity->velX.part16.integer+wave+sway)*entity->velY.part16.integer);
    entity->posX.part16.integer+=x;
    entity->posY.part16.integer+=y;
    Sprite_DrawEntity(entity);
}
