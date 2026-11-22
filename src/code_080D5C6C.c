/* Mirrored fragment positions form the expanding breakup effect. */
#include <string.h>
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
struct FragmentPoint { s16 x,y; };
extern struct FragmentPoint sGrahamBreakupFragmentPoints[4][15];
extern void GrahamBreakupFragment_Update(struct EwramData_EntityData *);
extern void BossDefeat_RestoreEffectTiles(void);
/**
 * @brief Wait, emit mirrored fragments from the 4-by-15 point table with
 * palette flashes, then run the existing effect cleanup callback
 * and remove after its final countdown.
 */
void GrahamBreakup_Update(struct EwramData_EntityData *entity)
{
    struct FragmentPoint positions[4][15];
    memcpy(positions,sGrahamBreakupFragmentPoints,sizeof positions);
    switch(entity->state)
    {
        case 0:
            entity->timer=8;
            entity->accelX.whole=0x20000;
            entity->state++;
        case 1:
            if(entity->timer!=0) goto tick;
            entity->timer=128;
            entity->state+=2;
            break;
        case 2:
        case 3:
        {
            s32 column,row,mirror;
            if((entity->timer % 32)==0)
            {
                BgPalette_StartColorFade(31,31,31,0,0x10000,32);
                ObjPalette_StartColorFade(31,31,31,0x4000,0x10000,16);
            }
            for(column=0;column<15;column++)
            {
                for(row=0;row<4;row++)
                {
                    if(positions[row][column].x!=255)
                    {
                        for(mirror=0;mirror<2;mirror++)
                        {
                            if(row!=0 || mirror!=0)
                            {
                                struct EwramData_EntityData *particle=EntityCreateInRange(0x49,0xCF,(u32 *)GrahamBreakupFragment_Update);
                                if(particle)
                                {
                                    particle->scrollWithBg=1;
                                    if(mirror==0)
                                        particle->posX.whole=entity->posX.whole+(s32)((u32)positions[row][column].x<<19);
                                    else
                                        particle->posX.whole=entity->posX.whole-(s32)((u32)positions[row][column].x<<19);
                                    particle->posY.whole=entity->posY.whole+(s32)((u32)positions[row][column].y<<19);
                                    particle->velY.whole=0x4000;
                                    particle->accelY.whole-=0x2000;
                                    particle->spriteLayer=17;
                                }
                            }
                        }
                    }
                }
            }
            entity->timer=64;
            entity->state++;
            break;
        }
        case 4:
            if(entity->timer!=0) goto tick;
            BossDefeat_RestoreEffectTiles();
            EntityDelete(entity);
            return;
tick:
            entity->timer--;
            break;
    }
    if(!entity->roomActive)
        BossDefeat_RestoreEffectTiles();
}
