/* Delayed rotating projectile accelerates and leaves a randomized trail. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern void Effect_SpawnAt(s32,s32,s32);
extern void Effect_SpawnRisingParticle(s32,s32,s32,s32);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
#define ANGLE(e) ((e)->unk_500.part16.unk_500)
#define STEP(e) ((e)->unk_500.part16.unk_502)
/**
 * @brief Index 0 animates the firing effect; index 1 waits, then reveals
 * a rotating accelerating projectile with randomized trail effects.
 * Release the affine slot in its removal state.
 */
void GrahamShotPart_Update(struct EwramData_EntityData *entity)
{
    if((u8)entity->unk_4F8.part8.unk_4F8==0)
    {
        if(entity->animDone)
        {
            PlaySong(0x10D);
            entity->deleteRequested=1;
            return;
        }
    }
    else
    {
        switch(entity->state)
        {
            case 0:
                entity->hidden=1;
                entity->timer=26;
                if(!entity->flipX)
                    entity->posX.part16.integer-=10;
                else
                    entity->posX.part16.integer+=10;
                entity->state++;
                entity->stage=0;
                entity->subState=0;
            case 1:
                if(--entity->timer==255)
                {
                    if(entity->flipX)
                    {
                        entity->velX.whole=0x2000;
                        entity->accelX.whole=0xE00;
                    }
                    else
                    {
                        entity->velX.whole=-0x2000;
                        entity->accelX.whole=-0xE00;
                    }
                    entity->state++;
                    entity->stage=0;
                    entity->subState=0;
                    entity->hidden=0;
                }
                break;
            case 2:
            {
                s16 zero;
                s32 step,scale,angle;
                entity->posX.whole+=entity->velX.whole;
                entity->posY.whole+=entity->velY.whole;
                entity->velX.whole+=entity->accelX.whole;
                entity->velY.whole+=entity->accelY.whole;
                zero=0;
                step=zero+0x2C0;
                STEP(entity)=step;
                if(!entity->flipX)
                    STEP(entity)=-step;
                ANGLE(entity)+=STEP(entity);
                angle=(s16)ANGLE(entity);
                scale=0x10000;
                ObjAffine_Set(angle,scale,scale,entity->flipX,0,entity->affineIndex);
                if((u8)(entity->timer%10)==0)
                {
                    s32 offset=(10-RandomNumberGenerator()%20)<<16;
                    s32 x=entity->posX.whole;
                    s32 y=entity->posY.whole-offset;
                    u8 *layer=&entity->spriteLayer;
                    Effect_SpawnAt(x,y,*layer-1);
                    Effect_SpawnRisingParticle(entity->posX.whole,entity->posY.whole-offset,*layer-1,0);
                }
                entity->timer++;
                if(EntityIsOutsideScreen(entity,4))
                {
                    entity->state=0xF0;
                    entity->stage=zero;
                    entity->subState=zero;
                }
                break;
            }
            case 0xF0:
                ObjAffine_Free(entity->affineIndex);
                entity->deleteRequested=1;
                return;
        }
    }
    if(!entity->hidden)
    {
        Anim_Update(entity);
        Enemy_CheckHitPlayer(entity);
    }
}
