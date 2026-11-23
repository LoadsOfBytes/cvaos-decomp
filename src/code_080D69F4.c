/* An animated particle spirals inward as its angular speed increases. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];
extern struct AnimScript sRecoveryPickupParticleAnimScript[];
#define ANGLE(e) (e->ext.bossPickupParticle.angle)
#define STEP(e) (e->ext.bossPickupParticle.step)
#define RADIUS(e) (e->ext.bossPickupParticle.radius)
#define SPEED(e) (e->ext.bossPickupParticle.speed)
/**
 * @brief Animate a particle around a stored room-space center with
 * accelerating angular motion and shrinking radius. Remove when
 * the radius becomes nonpositive; preserve shifted multiplication.
 */
void BossRecoveryPickupParticle_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
    switch(entity->state)
    {
        case 0:
        {
            s32 graphics=ObjGfx_Find(sCommonObjectGfx);
            Sprite_Attach(entity,sCommonObjectSpriteSheet,graphics,7);
            Anim_Start(entity,sRecoveryPickupParticleAnimScript,0,0);
            entity->drawFunc=(u32 *)Sprite_DrawEntity;
            entity->spriteLayer=10;
            entity->scrollWithBg=1;
            entity->state++;
        }
        case 1:
        {
            s32 sine,offset,radius;
            RADIUS(entity)+=SPEED(entity);
            SPEED(entity)-=0xC00;
            if(RADIUS(entity)<=0)
            {
                EntityDelete(entity);
                return;
            }
            sine=Sin((s16)ANGLE(entity)+0x4000);
            radius=RADIUS(entity);
            offset=(radius>>8)*(sine>>8);
            entity->posX.whole=entity->accelX.whole-camera->xPos.whole+offset;
            sine=Sin((s16)ANGLE(entity));
            radius=RADIUS(entity);
            offset=(radius>>8)*(sine>>8);
            entity->posY.whole=entity->accelY.whole-camera->yPos.whole+offset;
            ANGLE(entity)+=STEP(entity);
            if(!entity->flipX)
            {
                STEP(entity)+=0x40;
                if((s16)STEP(entity)>0x1800)
                    STEP(entity)=0x1800;
            }
            else
            {
                STEP(entity)-=0x40;
                if((s16)STEP(entity)<-0x1800)
                    STEP(entity)=-0x1800;
            }
            break;
        }
    }
    Anim_Update(entity);
}

