/* Belmont subweapon attack: telegraph, timed effects, throw, recovery. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern s32 Entity_GetTargetDistanceY(struct EwramData_EntityData *);
extern s32 Entity_IsTargetInCenteredBox(struct EwramData_EntityData *,s32,s32);
extern struct EwramData_EntityData *Belmont_CreateSubweapon(struct EwramData_EntityData *,s32);
/**
 * @brief Subweapon state (6): select a variant from target position,
 * telegraph with timed effects and spawn at frame 32 before recovery.
 */
void Belmont_UpdateSubweaponAttack(struct EwramData_EntityData *entity)
{
    s32 state=entity->subState;
    switch(state)
    {
        case 0:
            Anim_StartIndexed(entity,7,1,0);
            if((s16)Entity_GetTargetDistanceY(entity)>16)
                entity->stage=state;
            else if(Entity_IsTargetInCenteredBox(entity,160,32))
                entity->stage=2;
            else
                entity->stage=1;
            entity->subState++;
            entity->timer=20;
            break;
        case 1:
            if(entity->animStep==4)
                entity->animFlags|=2;
            if(--entity->timer==255)
            {
                entity->animFlags&=0xFD;
                entity->subState++;
            }
            if((entity->timer & 3)==0)
            {
                s32 offset=0x80000;
                s32 variant=entity->stage;
                s32 kind=7;
                s32 position;
                u32 random;
                if(variant!=0)
                {
                    kind=9;
                    if(variant==1)
                        kind=6;
                }
                if(entity->flipX)
                    offset=-offset;
                random=RandomNumberGenerator();
                position=entity->posX.whole+offset;
                position+=random%0x60000;
                position-=0x30000;
                Effect_CreateFromTable(position,entity->posY.whole-0x200000,0,-0x2000,4,kind);
            }
            break;
        case 2:
            if(entity->animFrame==32)
            {
                switch(entity->stage)
                {
                    case 0: Belmont_CreateSubweapon(entity,0); break;
                    case 1: Belmont_CreateSubweapon(entity,1); break;
                    case 2: Belmont_CreateSubweapon(entity,2); break;
                }
                entity->subState++;
                entity->stage=0;
            }
            break;
        case 3:
            if(entity->animDone)
            {
                entity->state=2;
                entity->stage=0;
                entity->subState=0;
            }
            break;
    }
}
