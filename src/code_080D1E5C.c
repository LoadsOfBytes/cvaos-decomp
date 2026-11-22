/* Belmont subweapon variant 1: accelerating rotating projectile with a trail. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
#define ANGLE(e) ((e)->unk_518.part16.hp)
#define STEP(e) ((e)->unk_518.part16.unk_51A)
/**
 * @brief Variant 1: rotate and accelerate horizontally with a trail, then
 * release the affine slot and remove after leaving the screen.
 */
void BelmontSubweapon1_Update(struct EwramData_EntityData *entity)
{
    switch(entity->state)
    {
        case 0:
        {
            s32 affine=ObjAffine_Alloc(1);
            s32 speed;
            s32 acceleration;
            if(affine<0)
            {
                EntityDelete(entity);
                return;
            }
            entity->affineIndex=affine;
            entity->affine=1;
            entity->posY.part16.integer-=16;
            entity->velX.whole=(speed=0x34000);
            entity->accelX.whole=(acceleration=-0x1000);
            if(!entity->flipX)
            {
                entity->velX.whole=-speed;
                entity->accelX.whole=-acceleration;
            }
            entity->hidden=0;
            entity->state++;
            PlaySong(0x1E5);
        }
        case 1:
        {
            s32 speed;
            s32 acceleration;
            u16 step;
            entity->posX.whole+=entity->velX.whole;
            speed=entity->velX.whole;
            acceleration=entity->accelX.whole;
            if((acceleration<0 && speed>-0x80000) || (acceleration>0 && speed<0x80000))
                entity->velX.whole=speed+acceleration;
            if((entity->timer++ & 1)!=0)
            {
                s32 x=0x80000;
                s32 jitter;
                s32 y;
                struct EwramData_EntityData *trail;
                if(entity->velX.whole>0)
                    x=-x;
                jitter=((RandomNumberGenerator()%0x200000)>>1)-0x80000;
                y=entity->posY.whole-jitter;
                trail=Effect_CreateFromTable(entity->posX.whole+x,y,0,0,4,6);
                if(trail)
                    trail->spriteLayer=entity->spriteLayer+1;
            }
            STEP(entity)=(step=0x580);
            if(!entity->flipX)
                STEP(entity)=-step;
            ANGLE(entity)+=STEP(entity);
            ObjAffine_Set((s16)ANGLE(entity),0x10000,0x10000,entity->flipX,0,entity->affineIndex);
            entity->unk_50C.part16.unk_50E+=entity->unk_508.part16.unk_508;
            if(EntityIsOutsideScreen(entity,4))
                entity->state=2;
            break;
        }
        case 2:
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
    }
    Enemy_CheckHitPlayer(entity);
}
