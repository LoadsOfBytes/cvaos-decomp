/* Belmont_UpdateCombat. */
/* Belmont combat decisions: proximity, health phase, turns and attacks. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/entity_template.h"
extern s32 Entity_TryWalk(struct EwramData_EntityData *,s32,s32,s32,s32);
extern s32 Entity_IsOffsetOutsideRoom(struct EwramData_EntityData *,s32,s32);
extern s32 Entity_IsTargetInCenteredBox(struct EwramData_EntityData *,s32,s32);
#define FLAGS(e) ((e)->ext.raw.unk_4F8.part8.unk_4F8)
#define COOLDOWN(e) ((e)->ext.raw.unk_504.part16.unk_506[0])
/**
 * @brief Combat state (2): update movement/cooldown and choose attacks,
 * dodges or the low-HP powered phase from proximity, player state,
 * room bounds and randomness. Preserve the existing health threshold tests.
 */
void Belmont_UpdateCombat(struct EwramData_EntityData *entity)
{
    s32 facing=entity->flipX;
    u32 work;
    s32 next;
    switch(entity->subState)
    {
        case 0:
        case 1:
            if((u16)COOLDOWN(entity)!=0)
                COOLDOWN(entity)--;
            if(entity->animScriptIndex!=1)
                Anim_StartIndexed(entity,1,1,1);
            if(!(Entity_TryWalk(entity,0x20000,0x200000,0x40000,0)&4))
                entity->posY.whole+=entity->velY.whole;
            if((s16)entity->hp < (u16)((u32)sEnemyData[entity->enemyId].hp/3))
            {
                if(Entity_IsOffsetOutsideRoom(entity,128,0) || Entity_IsOffsetOutsideRoom(entity,-128,0))
                {
                    if(Entity_IsOffsetOutsideRoom(entity,128,0))
                        entity->flipX=0;
                    else
                        entity->flipX=1;
                    entity->state=9;
                    entity->stage=0;
                    entity->subState=0;
                    return;
                }
                {
u8 mask=FLAGS(entity)&8; if(mask==0){ entity->state=8; entity->stage=mask; entity->subState=mask; FLAGS(entity)|=8;
                        gEwramData->gameState.stateFlags|=0x1000;
                        break;
                    }
                }
            }
            if(Entity_IsTargetInCenteredBox(entity,32,48))
            {
                work=RandomNumberGenerator()%12;
                if(work==0) { entity->state=9; entity->stage=work; entity->subState=work; return; }
                work=RandomNumberGenerator()%8;
                if(work==0) { entity->state=4; entity->stage=work; entity->subState=work; return; }
                work=RandomNumberGenerator()%4;
                if(work==0) { entity->state=10; entity->stage=work; entity->subState=work; return; }
            }
            if(entity->subState==1)
            {
                if(entity->flipX!=facing)
                {
                    entity->subState++;
                    entity->stage=0;
                    break;
                }
            }
            else
                entity->subState=1;
            { s32 nearby=Entity_IsTargetInCenteredBox(entity,128,64);
            if(nearby)
            {
                struct EwramData_EntityData *player=gEwramData->playerState.player;
                if(player->state==1 && player->flipX==entity->flipX && player->animStep==0)
                {
                    work=RandomNumberGenerator()%8;
                    if(work==0) { entity->state=9; entity->stage=work; entity->subState=work; return; }
                    work=RandomNumberGenerator()%6;
                    if(work==0) { entity->state=4; entity->stage=work; entity->subState=work; return; }
                }
                work=RandomNumberGenerator()%10;
                if(work==0) { entity->state=7; entity->stage=work; entity->subState=work; return; }
                work=RandomNumberGenerator()%50;
                if(work==0)
                {
                    entity->state=10;
                    entity->stage=work;
                    entity->subState=work;
                    return;
                }
            }
            else if((Player_GetFocusedActionFlags()&2) && RandomNumberGenerator()%60==20)
            {
                entity->state=4;
                entity->stage=nearby;
                entity->subState=5;
                break;
            }
            }
            if((FLAGS(entity)&8) && Entity_IsTargetInCenteredBox(entity,256,128) && RandomNumberGenerator()%30==0 && (u16)COOLDOWN(entity)==0)
            {
                COOLDOWN(entity)=RandomNumberGenerator()%4;
                entity->state=6;
                entity->stage=0;
                entity->subState=0;
            }
            break;
        case 2:
        {
            s32 roll;
            if(entity->stage==0)
            {
                entity->timer=10;
                entity->stage++;
            }
            Entity_TryWalk(entity,0x4000,0x10000,0x10000,0);
            roll=RandomNumberGenerator()&3;
            if(roll==0 && Entity_IsTargetInCenteredBox(entity,32,256) && (Player_GetFocusedActionFlags()&4))
            {
                entity->state=4;
                entity->stage=roll;
                entity->subState=roll;
                break;
            }
            if(--entity->timer==0xFF)
            {
                entity->subState--;
                entity->stage=0;
            }
            break;
        }
    }

}
