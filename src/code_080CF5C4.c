/* Mandragora proximity trigger, scream animation and linked-part timing. */
#include "code_080D73B8.h"
#include "mandragora_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Target_IsNearEntity(struct EwramData_EntityData *,s32,s32);
/**
 * @brief Waiting state (1): play animation 0, then select scream state 2
 * after the target enters the proximity box.
 */
void Mandragora_UpdateWait(struct EwramData_EntityData *entity)
{
    s32 state=entity->subState;
    switch(state)
    {
        case 0:
            if(entity->animScriptIndex!=0)
                Anim_StartIndexed(entity,0,0,1);
            entity->subState=1;
            entity->stage=state;
        case 1:
            if(Target_IsNearEntity(entity,256,128))
            {
                entity->subState=2;
                entity->stage=0;
            }
            break;
        case 2:
            entity->state=state;
            entity->stage=0;
            entity->subState=0;
            break;
    }
}
/**
 * @brief Scream state (2): sequence root animations and sound, activate
 * the intermediate part and wait for its timing/owner flags before
 * entering death state 0xF0.
 */
void Mandragora_UpdateScream(struct EwramData_EntityData *entity)
{
    struct EwramData *ram=gEwramData;
    struct EwramData_EntityData *part=&ram->entityData[MANDRAGORA_LINK_INDEX(entity)];
    switch(entity->subState)
    {
        case 0:
            switch(entity->stage)
            {
                case 0:
                    entity->timer=80;
                    if(entity->animScriptIndex!=1)
                        Anim_StartIndexed(entity,1,0,0);
                    entity->stage++;
                case 1:
                    if(entity->timer==1)
                        PlaySong(0x119);
                    if(entity->timer!=0)
                        entity->timer--;
                    if(entity->animDone)
                    {
                        entity->timer=0;
                        entity->subState=1;
                        entity->stage=0;
                    }
                    break;
            }
            break;
        case 1:
        {
            s32 state=entity->stage;
            switch(state)
            {
                case 0:
                    if(entity->animScriptIndex!=2)
                        Anim_StartIndexed(entity,2,0,0);
                    part->state++;
                    part->stage=state;
                    part->subState=state;
                    entity->stage++;
                case 1:
                    if(entity->animDone)
                    {
                        entity->animFlags|=2;
                        entity->animFrame=10;
                        entity->stage++;
                    }
                    break;
                case 2:
                    if(part->state==2 && part->timer<=7)
                        entity->animFrame=10;
                    if(MANDRAGORA_FLAGS(entity) & 0x18)
                    {
                        entity->state=0xF0;
                        entity->stage=0;
                        entity->subState=0;
                    }
                    break;
            }
            break;
        }
    }
}
