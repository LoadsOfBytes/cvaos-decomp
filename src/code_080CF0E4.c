/* Mandragora intermediate part coordinates its owner and terminal part. */
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern void Effect_SpawnRandom(s32,s32,s32,s32,s32);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
#define FLAGS(e) (e->ext.mandragora.flags)
/**
 * @brief Coordinate the root owner and terminal part through activation,
 * scream timing and recovery. A defeated intermediate part drops owner
 * rewards and marks owner flags; exact visual identity remains unresolved.
 */
void MandragoraPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=&gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    struct EwramData_EntityData *tip=&gEwramData->entityData[entity->ext.mandragora.linkIndex];
    switch(entity->state)
    {
        case 0:
            if(entity->subState==0)
            {
                entity->posX.whole=owner->posX.whole;
                entity->posY.whole=owner->posY.whole;
                entity->subState++;
                entity->stage=0;
                entity->hidden=1;
            }
            break;
        case 1:
            if(entity->subState==0)
            {
                FLAGS(owner)|=2;
                entity->hp=owner->hp;
                if(entity->animScriptIndex!=3)
                    Anim_StartIndexed(entity,3,0,1);
                entity->state++;
                entity->stage=0;
                entity->subState=0;
                entity->hidden=0;
                PlaySong(0x83);
                goto activate;
            }
            goto countdown;
        case 2:
activate:
            if(entity->subState==0)
            {
                tip->state=1;
                tip->stage=0;
                tip->subState=0;
                entity->subState++;
                entity->stage=0;
                entity->timer=32;
            }
countdown:
            if(--entity->timer==0xFF)
            {
                entity->state=0xF1;
                entity->stage=0;
                entity->subState=0;
            }
            break;
        case 0xF0:
            PlaySong(0x70);
            Enemy_DropRewards(owner);
            entity->state=0xF1;
            entity->stage=0;
            entity->subState=0;
            FLAGS(owner)|=0x20;
        case 0xF1:
            if(entity->subState==0)
            {
                if(FLAGS(owner)&0x20)
                {
                    tip->state=0;
                    tip->stage=0;
                    tip->subState=0;
                }
                FLAGS(owner)|=8;
                entity->drawSkip=1;
                entity->subState++;
                entity->stage=0;
            }
            if((u8)((u32)entity->timer++%7)==0 && (FLAGS(owner)&0x20))
            {
                s32 x=-0x80000;
                if(entity->flipX)
                    x=-x;
                Effect_SpawnRandom(entity->posX.whole+x,entity->posY.whole-0x140000,0x100000,0x100000,entity->spriteLayer+1);
            }
            if(entity->animDone)
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
        if(entity->state<=0xEF)
            Enemy_CheckHitPlayer(entity);
    }
}
