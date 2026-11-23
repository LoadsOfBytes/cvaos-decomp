/* Alastor death: smoke along the blade, then falling out of view. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void Effect_SpawnRandom(s32,s32,s32,s32,s32);
/**
 * @brief Death state (0xFF): emit effects along the blade and fall; free
 * the owner affine slot and request deletion of both owner and linked
 * part on the existing bounds/timer condition.
 */
void Alastor_UpdateDeath(struct EwramData_EntityData *entity)
{
    s32 state=entity->subState;
    switch(state)
    {
        case 0:
            PlaySong(0x6E);
            entity->palette=entity->basePalette;
            entity->subState++;
            entity->stage=state;
        case 1:
            switch(entity->stage)
            {
                case 0:
                    entity->timer=10;
                    entity->stage++;
                case 1:
                {
                    s32 angle=(entity->ext.alastor.angle+0xC000)&0xFFFF;
                    s32 radius=0x2A0000-entity->timer*0x54000;
                    s32 x=FixedMultiply(Sin(angle+0x4000),radius);
                    s32 y=FixedMultiply(Sin(angle),radius);
                    u8 *layer=&entity->spriteLayer;
                    s32 scale=0x100000;
                    s32 i;
                    for(i=2;i>=0;i--)
                        Effect_SpawnRandom(entity->posX.whole+x,entity->posY.whole+y+0x60000,scale,scale,*layer-1);
                    if(--entity->timer==0xFF)
                        entity->stage++;
                    break;
                }
                case 2:
                    entity->subState++;
                    entity->stage=0;
                    break;
            }
            break;
        case 2:
            if(entity->stage==0)
            {
                entity->velY.whole=0x8000;
                entity->accelY.whole=0x800;
                entity->timer=64;
                entity->stage++;
            }
            entity->posY.whole+=entity->velY.whole;
            entity->velY.whole+=entity->accelY.whole;
            if((s16)entity->posY.part16.integer>170 || entity->timer++>64)
            {
                ObjAffine_Free(entity->affineIndex);
                entity->deleteRequested=1;
                {
                    s32 index=entity->ext.alastorPart.index;
                    index[gEwramData->entityData].deleteRequested=1;
                }
            }
            break;
    }
}
