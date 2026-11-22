/* Allocate a rising death particle and save its screen and camera origin. */
#include "code_08009A0.h"
#include "werejaguar_entity.h"
#include "structs/ewram.h"
extern void WerejaguarDeathEffect_Update(struct EwramData_EntityData *);
#define FIXED_INTEGER(x) ((x)<0 ? -(-(x)>>16) : (x)>>16)
/**
 * @brief Spawn a death effect at (x, y) on layer that circles from angle
 * (WerejaguarDeathEffect_Update).
 */
void Werejaguar_SpawnDeathEffect(s32 x,s32 y,s32 layer,s32 angle)
{
    struct EwramData_EntityData *effect=EntityCreateInRange(0x49,0xCF,(u32 *)WerejaguarDeathEffect_Update);
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
    if(effect)
    {
        effect->posX.whole=x; effect->posY.whole=y;
        effect->flipX=RandomNumberGenerator()%2;
        effect->spriteLayer=layer;
        WEREJAGUAR_EFFECT_ANGLE(effect)=angle;
        WEREJAGUAR_EFFECT_LAYER(effect)=layer;
        WEREJAGUAR_EFFECT_X(effect)=FIXED_INTEGER(x);
        WEREJAGUAR_EFFECT_CAMERA_X(effect)=camera->xPos.part16.integer;
        WEREJAGUAR_EFFECT_Y(effect)=FIXED_INTEGER(y);
        WEREJAGUAR_EFFECT_CAMERA_Y(effect)=camera->yPos.part16.integer;
    }
}
