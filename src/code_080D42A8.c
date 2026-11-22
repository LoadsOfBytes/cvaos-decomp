/* Graham core: vanish/reappear palette sequence, reposition, then combat. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void GrahamFirstForm_UpdateTransition(struct EwramData_EntityData *);
extern void GrahamFirstForm_UpdateAttacks(struct EwramData_EntityData *);
extern void GrahamFirstForm_UpdateAlternateAttack(struct EwramData_EntityData *);
extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern u8 sGrahamPalette1[];
#define SET_PALETTE(n) ObjPalette_LoadRange(sGrahamPalette1,n,1,entity->palette)
/**
 * @brief Dispatch first-form teleport/reappearance state 3, attacks in
 * state 6/0xE0 and transition state 5. Apply palette events, target
 * facing, hit-box animation and contact checks; bestiary only animates.
 */
void GrahamFirstForm_Update(struct EwramData_EntityData *entity)
{
    if(entity->state==0xD0)
    { Anim_Update(entity); return; }
    if(entity->state==5)
    { GrahamFirstForm_UpdateTransition(entity); return; }
    switch(entity->state)
    {
        case 3:
        {
            s32 state=entity->subState;
            switch(state)
            {
                case 0:
                    if(entity->animScriptIndex!=0)
                        Anim_StartIndexed(entity,0,0,1);
                    if(entity->unk_4F8.part16.unk_4FA & 2)
                    {
                        entity->unk_518.part16.hp=1500;
                        entity->subState+=2;
                        entity->stage=0;
                    }
                    else
                    {
                        entity->timer=16;
                        entity->subState++;
                        entity->stage=state;
                    }
                    break;
                case 1:
                    if(--entity->timer==255)
                    {
                        entity->subState++;
                        entity->stage=0;
                    }
                    break;
                case 2:
                    switch(entity->stage)
                    {
                        case 0:
                            entity->posY.part16.integer-=32;
                            entity->stage++;
                        case 1:
                        {
                            u8 *frame;
                            if(entity->animScriptIndex!=6)
                                Anim_StartIndexed(entity,6,0,0);
                            frame=&entity->animStep;
                            if(*frame==2) SET_PALETTE(3);
                            if(*frame==3) SET_PALETTE(2);
                            if(*frame==4) SET_PALETTE(1);
                            if(*frame==21) SET_PALETTE(0);
                            if(entity->animDone)
                            {
                                Entity_FaceTarget(entity);
                                if(entity->unk_4F8.part16.unk_4FA & 2)
                                {
                                    struct BgLayerState *camera=&gEwramData->bgInfo[1];
                                    entity->posX.whole=0x780000-camera->xPos.whole;
                                }
                                else
                                {
                                    u32 x=RandomNumberGenerator()%160;
                                    struct BgLayerState *camera;
                                    x+=40;
                                    camera=&gEwramData->bgInfo[1];
                                    x-=camera->xPos.part16.integer;
                                    entity->posX.part16.integer=x;
                                }
                                entity->stage++;
                                SET_PALETTE(1);
                            }
                            break;
                        }
                        case 2:
                        {
                            u8 *animation=&entity->animScriptIndex;
                            u8 *frame;
                            if(*animation!=5)
                                Anim_StartIndexed(entity,5,0,0);
                            frame=&entity->animStep;
                            if(*frame<=14)
                                Entity_FaceTarget(entity);
                            if(*frame==19) SET_PALETTE(2);
                            if(*frame==20) SET_PALETTE(3);
                            if(*frame==21) SET_PALETTE(0);
                            if(entity->animDone)
                            {
                                SET_PALETTE(0);
                                if(*animation!=0)
                                    Anim_StartIndexed(entity,0,0,1);
                                entity->posY.part16.integer+=32;
                                entity->stage++;
                                entity->timer=state;
                            }
                            break;
                        }
                        case 3:
                            if(--entity->timer==255)
                            {
                                u32 flags=*(volatile u16 *)&entity->unk_4F8.part16.unk_4FA;
                                s32 mask=flags & state;
                                if(mask)
                                {
                                    entity->state=0xE0;
                                    entity->stage=0;
                                    entity->subState=0;
                                }
                                else
                                {
                                    entity->state=6;
                                    entity->stage=mask;
                                    entity->subState=mask;
                                }
                            }
                            break;
                    }
                    break;
            }
            break;
        }
        case 6:
            GrahamFirstForm_UpdateAttacks(entity);
            break;
        case 0xE0:
            GrahamFirstForm_UpdateAlternateAttack(entity);
            break;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}
