/* Graham attack selection, animation turns and timed projectile volleys. */
#include "code_08009A0.h"
#include "graham_entity.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_IsFacingTarget(struct EwramData_EntityData *);
extern void Graham_SpawnSpreadProjectiles(struct EwramData_EntityData *);
extern void Graham_CreateShotPair(struct EwramData_EntityData *,s32);
/**
 * @brief Attack state (6): turn toward the target, choose spread or paired
 * projectile volleys with timed recovery and optional mid-volley turns,
 * then return to teleport state 3.
 */
void GrahamFirstForm_UpdateAttacks(struct EwramData_EntityData *entity)
{
    s32 remaining;
    switch(entity->subState)
    {
        case 0:
        {
            s32 facing=Entity_IsFacingTarget(entity);
            if(!facing)
            {
                entity->subState=1;
                entity->stage=facing;
            }
            else
            {
                entity->subState=2;
                entity->stage=0;
            }
            break;
        }
        case 1:
            if(entity->animScriptIndex!=7)
                Anim_StartIndexed(entity,7,0,0);
            if(entity->animDone)
            {
                entity->flipX=1-entity->flipX;
                entity->subState=2;
                entity->stage=0;
            }
            break;
        case 2:
            if(entity->animScriptIndex!=2)
                Anim_StartIndexed(entity,2,0,0);
            if(entity->animDone)
            {
                entity->subState=3;
                entity->stage=0;
            }
            break;
        case 3:
        {
            s32 random;
            if(entity->animScriptIndex!=1)
                Anim_StartIndexed(entity,1,0,1);
            random=RandomNumberGenerator() & 1;
            if(!random)
            {
                entity->timer=16;
                PlaySong(0x1D9);
                entity->subState=4;
                entity->stage=random;
            }
            else
            {
                random=RandomNumberGenerator()%3;
                if(!random)
                {
                    PlaySong(0x1D7);
                    entity->timer=random;
                    entity->subState=5;
                    entity->stage=4;
                }
                else
                {
                    PlaySong(0x1D8);
                    entity->timer=0;
                    entity->subState=6;
                    entity->stage=2;
                    if(!(RandomNumberGenerator() & 1))
                        entity->ext.grahamTurn.volleySide=0;
                    else
                        entity->ext.grahamTurn.volleySide=1;
                }
            }
            break;
        }
        case 4:
            if(--entity->timer==255)
            {
                PlaySong(0x7A);
                Graham_SpawnSpreadProjectiles(entity);
                entity->timer=64;
                entity->subState=8;
                entity->stage=0;
            }
            break;
        case 5:
            if(--entity->timer==255)
            {
                remaining=entity->stage;
                if(!remaining) goto finish_volley;
                Graham_CreateShotPair(entity,0);
                entity->timer=24;
                entity->stage--;
            }
            else if(entity->timer<=7 && entity->stage && !Entity_IsFacingTarget(entity))
            {
                entity->ext.grahamTurn.resume=5;
                entity->subState=7;
            }
            break;
        case 6:
            if(--entity->timer==255)
            {
                remaining=entity->stage;
                if(!remaining)
                {
finish_volley:
                    entity->subState=8;
                    entity->stage=remaining;
                    entity->timer=32;
                    break;
                }
                if(entity->ext.grahamTurn.volleySide)
                {
                    if(remaining==2) goto shot_one;
                    Graham_CreateShotPair(entity,0);
                    goto shot_done;
                }
                if(remaining==2) goto shot_zero;
shot_one:
                Graham_CreateShotPair(entity,1);
                goto shot_done;
shot_zero:
                Graham_CreateShotPair(entity,0);
shot_done:
                entity->timer=48;
                entity->stage--;
            }
            else if(entity->timer<=7 && entity->stage && !Entity_IsFacingTarget(entity))
            {
                entity->ext.grahamTurn.resume=6;
                entity->subState=7;
            }
            break;
        case 7:
        {
            u8 *animation=&entity->animScriptIndex;
            if(*animation!=7)
                Anim_StartIndexed(entity,7,0,1);
            if(entity->animDone)
            {
                if(*animation!=1)
                    Anim_StartIndexed(entity,1,0,1);
                entity->flipX=1-entity->flipX;
                entity->subState=entity->ext.grahamTurn.resume;
            }
            break;
        }
        case 8:
            if(--entity->timer==255)
            {
                entity->state=3;
                entity->stage=0;
                entity->subState=0;
            }
            break;
    }
}
