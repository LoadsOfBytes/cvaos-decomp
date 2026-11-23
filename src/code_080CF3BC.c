/* MandragoraTerminalPart_Update. */
/* Mandragora terminal part controls the scream's palette flashes. */
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern void HitBox_SetAttackRaw(struct EwramData_EntityData *,s32);
extern s32 sMandragoraTerminalPartAttackBox[];
#define FLAGS(e) (e->ext.mandragora.flags)
static inline s32 Tick(struct EwramData_EntityData *e) { s32 timer=e->timer; if(timer) e->timer--; return timer; }
/**
 * @brief Sequence the scream animation, BG/OBJ palette flashes, expanded
 * attack box and root owner flag 0x10. Reset when the intermediate
 * part finishes; preserve existing fade and parent-validity handling.
 */
void MandragoraTerminalPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part=&gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    struct EwramData_EntityData *owner=&gEwramData->entityData[ENTITY_PARENT_INDEX(part)];
    if(!entity->roomActive)
    {
        BgPalette_StopColorFade();
        ObjPalette_StopColorFade();
        return;
    }
    if(part->updateFunc==NULL || owner->updateFunc==NULL)
        return;
    switch(entity->state)
    {
        case 0:
        {
            s32 state=entity->subState;
            if(state==0)
            {
                BgPalette_StopColorFade();
                ObjPalette_StopColorFade();
                entity->posX.whole=part->posX.whole;
                entity->posY.whole=part->posY.whole;
                entity->subState++;
                entity->stage=state;
                entity->hidden=1;
                FLAGS(owner)&=0xEF;
            }
            break;
        }
        case 1:
        {
            s32 state=entity->subState;
            if(state==0)
            {
                entity->hidden=0;
                if(entity->animScriptIndex!=4)
                    Anim_StartIndexed(entity,4,0,0);
                entity->subState++;
                entity->stage=state;
            }
            if(entity->animDone)
            {
                BgPalette_StartColorFade(31,31,31,0x10000,0,4);
                ObjPalette_StartColorFade(31,31,31,0x10000,0,4);
                entity->timer=4;
                entity->state++;
                entity->stage=0;
                entity->subState=0;
            }
            break;
        }
        case 2:
        {
            s32 timer; s16 t = entity->timer; timer=t; if(timer) entity->timer=t-1; else
            {
                BgPalette_StartColorFade(31,31,31,0,0x10000,32);
                ObjPalette_StartColorFade(31,31,31,0,0x10000,32);
                entity->state++;
                entity->stage=timer;
                entity->subState=timer;
            }
            break;
        }
        case 3:
        {
            s32 state=entity->subState;
            if(state==0)
            {
                if(entity->animScriptIndex!=5)
                    Anim_StartIndexed(entity,5,0,1);
                HitBox_SetAttackRaw(entity,sMandragoraTerminalPartAttackBox[0]);
                entity->subState++;
                entity->stage=state;
                FLAGS(owner)|=0x10;
            }
            if(part->state>0xEF)
            {
                entity->state=0xF0;
                entity->stage=0;
                entity->subState=0;
            }
            break;
        }
        case 0xF0:
            if(entity->subState==0)
            {
                if(part->state==0)
                {
                    entity->drawSkip=2;
                    entity->timer=16;
                    entity->subState++;
                    entity->stage=0;
                }
                else
                    entity->drawSkip=1;
            }
            else if(--entity->timer==0xFF)
            {
                entity->state=0;
                entity->stage=0;
                entity->subState=0;
            }
            break;
    }
    if(entity->state!=0)
    {
        Anim_Update(entity);
        if(entity->state!=0xF0)
            Enemy_CheckHitPlayer(entity);
    }
}
