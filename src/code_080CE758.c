/* Alastor recoil after its linked part is hit. */
#include "code_08009A0.h"
#include "structs/ewram.h"
#define FLAGS(e) (e->ext.alastor.flags)
#define PHASE(e) (e->ext.alastor.phase)
static inline s32 Wave(s32 angle,s32 amplitude) { return (amplitude>>8)*(Sin(angle)>>8); }
static inline s32 Multiply(s32 a,s32 b) { return a*b; }
static inline s32 Scale(s32 a,s32 b) { return (a>>8)*(b>>8); }
/**
 * @brief Reaction state (7): move away while coordinating the linked
 * part reaction; when it rests, return to approach or death state 0xFF
 * according to HP. Mark flag 0x20 while defeated.
 */
void Alastor_UpdateRecoil(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part=&gEwramData->entityData[entity->ext.alastorPart.index];
    switch(entity->subState)
    {
        case 0:
            entity->timer=0;
            PHASE(entity)=0;
            if(gEwramData->playerState.player->posX.whole<entity->posX.whole)
                FLAGS(entity)|=8;
            else
                FLAGS(entity)&=~8;
            entity->subState++;
        case 1:
        {
            s32 angle=0xD000;
            s32 speed=0x20000;
            s32 amplitude=0x18000;
            u16 phase=PHASE(entity);
            if(!(FLAGS(entity)&8))
                speed=-speed;
            entity->posX.whole+=Wave(angle+0x4000,speed);
            entity->posY.whole+=Scale(amplitude,Sin(angle))+Multiply((s16)phase,0xF00);
            PHASE(entity)++;
            if(part->state==4)
                entity->subState++;
            break;
        }
        case 2:
        {
            s32 state=part->state;
            if(state==0)
            {
                if((s16)entity->hp<=0)
                    entity->state=0xFF;
                else
                    entity->state=1;
                entity->stage=state;
                entity->subState=state;
            }
            break;
        }
    }
    if((s16)entity->hp<=0)
        FLAGS(entity)|=0x20;
}
