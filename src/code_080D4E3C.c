/* Three launch slopes share acceleration and cleanup when their owner dies. */
#include "code_08009A0.h"
#include "graham_projectile.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern void Effect_SpawnAt(s32,s32,s32);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
/**
 * @brief Launch along one of three slopes and check player contact.
 * On owner defeat, hit state or screen exit, spawn an effect and
 * request removal.
 */
void GrahamSpreadProjectile_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=&gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 state;
    if((s16)owner->unk_518.part16.hp<=0)
    {
        entity->hit.flags|=2;
        entity->state=0xF0;
    }
    state=entity->state;
    switch(state)
    {
        case 0:
        {
            s32 velocity=0x18000;
            entity->velX.whole=velocity;
            if(entity->flipX)
                entity->velX.whole=-velocity;
            switch((u8)GRAHAM_SPREAD_INDEX(entity))
            {
                case 0: entity->velY.whole=-0x3800; break;
                case 1: entity->velY.whole=state; break;
                case 2: entity->velY.whole=0x3800; break;
            }
            entity->state++;
            entity->stage=0;
            entity->subState=0;
        }
        case 1:
            entity->posX.whole+=entity->velX.whole;
            entity->posY.whole+=entity->velY.whole;
            entity->velX.whole+=entity->accelX.whole;
            entity->velY.whole+=entity->accelY.whole;
            if(EntityIsOutsideScreen(entity,4))
            {
                entity->state=0xF0;
                entity->stage=0;
                entity->subState=0;
            }
            break;
        case 0xF0:
            Effect_SpawnAt(entity->posX.whole,entity->posY.whole,entity->spriteLayer-1);
            entity->deleteRequested=1;
            return;
    }
    Anim_Update(entity);
    Enemy_CheckHitPlayer(entity);
}
