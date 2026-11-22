/* Belmont subweapon variant 0: rotating ballistic projectile. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void EntityAccelerateAndMoveX(struct EwramData_EntityData *,s32);
extern void EntityAccelerateAndMoveY(struct EwramData_EntityData *,s32);
extern s32 EntityIsOutsideScreen(struct EwramData_EntityData *,s32);
extern void Afterimage_Create(struct EwramData_EntityData *,s32,s32,s32);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
#define ANGLE(e) ((e)->unk_518.part16.hp)
#define STEP(e) ((e)->unk_518.part16.unk_51A)
/**
 * @brief Variant 0: allocate an affine slot, launch a rotating ballistic
 * projectile with afterimages and release the slot after leaving the screen.
 */
void BelmontSubweapon0_Update(struct EwramData_EntityData *entity)
{
    switch(entity->state)
    {
        case 0:
        {
            u16 affine=ObjAffine_Alloc(1);
            s32 speed;
            u16 step;
            if((s16)affine<0)
            {
                EntityDelete(entity);
                return;
            }
            entity->affineIndex=affine;
            entity->affine=1;
            STEP(entity)=(step=0xB00);
            if(!entity->flipX)
                STEP(entity)=-step;
            entity->unk_514.unk_514_32=(void *)(u32)(speed=0x16000);
            entity->velX.whole=speed;
            if(!entity->flipX)
                entity->velX.whole=-speed;
            entity->velY.whole=-0x38000;
            entity->accelY.whole=0x2000;
            entity->hidden=0;
            entity->state++;
            Afterimage_Create(entity,48,5,2);
            Afterimage_Create(entity,48,3,2);
            PlaySong(0x1EC);
            PlaySong(0xC5);
            break;
        }
        case 1:
            EntityAccelerateAndMoveX(entity,0x80000);
            EntityAccelerateAndMoveY(entity,0x80000);
            break;
        case 2:
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
    }
    ANGLE(entity)+=STEP(entity);
    ObjAffine_Set((s16)ANGLE(entity),0x10000,0x10000,entity->flipX,0,entity->affineIndex);
    if(EntityIsOutsideScreen(entity,4))
    {
        entity->state=2;
        entity->hidden=1;
    }
    Enemy_CheckHitPlayer(entity);
}

