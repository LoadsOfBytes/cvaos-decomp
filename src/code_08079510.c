/* Allocate a rising death particle and save its screen and camera origin. */
#include "code_08009A0.h"
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
        effect->unk_4F8.whole=angle;
        effect->unk_4FC.whole=layer;
        effect->unk_500.part16.unk_500=FIXED_INTEGER(x);
        effect->unk_500.part16.unk_502=camera->xPos.part16.integer;
        effect->unk_504.part16.unk_504=FIXED_INTEGER(y);
        effect->unk_504.part16.unk_506[0]=camera->yPos.part16.integer;
    }
}
