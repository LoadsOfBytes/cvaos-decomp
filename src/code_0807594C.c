/* Aerial approach, braking, volleys and the return to ordinary flight. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern s32 Entity_AimVelocityAtPoint(struct EwramData_EntityData *,s32,s32,s32);
extern s32 Target_IsWithinDistance(struct EwramData_EntityData *,s32);
extern void EntityMove(struct EwramData_EntityData *);
extern void EntityDampVelocityX(struct EwramData_EntityData *,s32);
extern void Harpy_ThrowFeather(struct EwramData_EntityData *,s32);
extern void Harpy_ThrowWavyFeather(struct EwramData_EntityData *,s32);
#define FRAME_PAIR(e) (*(u16 *)&(e)->animStep)
static inline s32 Magnitude(s32 value)
{
    if(value<0) value=-value;
    return value;
}
/**
 * @brief Attack state machine (state 1): wind up (animation 2, sound 0x87), then
 * throw a fan of feathers - three straight ones (Harpy_ThrowFeather kinds
 * 0 / 2 / 4, sound 0x89), three wavy ones (Harpy_ThrowWavyFeather) or two
 * straight ones (kinds 1 / 3); then back to state 0.
 */
void Harpy_UpdateAttack(struct EwramData_EntityData *entity)
{
    s32 song;
    switch(entity->subState)
    {
        case 0:
        {
            u32 random=RandomNumberGenerator() & 1;
            if(random)
            {
                struct BgLayerState *camera;
                Anim_StartIndexed(entity,2,0,0);
                PlaySong(0x87);
                entity->subState++;
                camera=&gEwramData->bgInfo[1];
                entity->ext.harpy.diveX=Target_GetX(entity)+camera->xPos.whole;
                entity->ext.harpy.diveY=Target_GetY(entity)+camera->yPos.whole-0x180000;
                entity->ext.harpy.startY=entity->posY.whole;
            }
            else
            {
                Anim_StartIndexed(entity,1,0,1);
                entity->subState=10;
                entity->stage=random;
                entity->timer=random;
            }
            break;
        }
        case 1:
            if(Entity_AimVelocityAtPoint(entity,entity->ext.harpy.diveX,entity->ext.harpy.diveY,0x30000))
            {
                entity->subState++;
                Anim_StartIndexed(entity,1,0,1);
                if(entity->velY.whole>0) entity->accelY.whole=-0x1000;
                else entity->accelY.whole=0x100;
                EntityDampVelocityY(entity,8);
                entity->timer=0;
            }
            EntityMove(entity);
            break;
        case 2:
        {
            s32 result;
            EntityAccelerateAndMove(entity,0x30000);
            EntityDampVelocityX(entity,64);
            result=Target_IsWithinDistance(entity,0x780000);
            if(!result)
            {
                entity->subState++;
                entity->accelY.whole=result;
            }
            else if(++entity->timer>59)
            {
                entity->subState++;
                entity->accelY.whole=0;
            }
            break;
        }
        case 3:
            entity->flipX=Entity_IsTargetRight(entity);
            EntityDampVelocityX(entity,8);
            EntityDampVelocityY(entity,8);
            EntityMove(entity);
            if(Magnitude(entity->velX.whole)<=0x7FFF && Magnitude(entity->velY.whole)<=0x7FFF)
            {
                entity->state=0;
                entity->subState=10;
                entity->stage=0;
                Anim_StartIndexed(entity,0,0,1);
            }
            break;
        case 10:
            if(FRAME_PAIR(entity)==1 && ++entity->timer>3)
            {
                if(entity->enemyId==56)
                {
                    Harpy_ThrowFeather(entity,0);
                    Harpy_ThrowFeather(entity,2);
                    Harpy_ThrowFeather(entity,4);
                    PlaySong(0x89);
                    goto advance;
                }
                Harpy_ThrowWavyFeather(entity,0);
                Harpy_ThrowWavyFeather(entity,1);
                Harpy_ThrowWavyFeather(entity,2);
                song=0x137;
                goto play_song;
            }
            break;
        case 11:
            if(FRAME_PAIR(entity)==1)
            {
                if(entity->enemyId==56)
                {
                    Harpy_ThrowFeather(entity,1);
                    Harpy_ThrowFeather(entity,3);
                    song=0x89;
play_song:
                    PlaySong(song);
                }
advance:
                entity->subState++;
                entity->timer=0;
            }
            break;
        case 12:
            if(FRAME_PAIR(entity)==1)
            {
                s32 zero;
                u8 timer=++entity->timer;
                zero=0;
                if(timer>2)
                {
                    Anim_StartIndexed(entity,0,0,1);
                    entity->state=zero;
                    entity->subState=10;
                    entity->stage=zero;
                }
            }
            break;
    }
}
