/* Alastor linked part: following, attack pose, flash and rest. */
#include "structs/ewram.h"
extern s32 Enemy_IsStunned(struct EwramData_EntityData *);
extern void Entity_FaceTarget(struct EwramData_EntityData *);
/**
 * @brief Follow the owner in appearance, preparation and reaction poses,
 * then blink and hide at rest. State transitions coordinate with the
 * owner attack-selection and recoil helpers. Exact visual identity unresolved.
 */
void AlastorPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=&gEwramData->entityData[entity->unk_4F4.part8.unk_4F5];
    if (Enemy_IsStunned(entity))
        return;
    switch (entity->state)
    {
        case 0:
            entity->hidden=1;
            break;
        case 1:
            entity->hidden=0;
            entity->animFrame=1;
            entity->posX.whole=owner->posX.whole;
            entity->posY.whole=owner->posY.whole;
            Entity_FaceTarget(entity);
            break;
        case 2:
            switch(entity->subState)
            {
                case 0:
                    entity->posX.whole=owner->posX.whole;
                    Entity_FaceTarget(entity);
                    entity->hidden=0;
                    entity->animFrame=1;
                    entity->timer=10;
                    entity->subState++;
                case 1:
                    entity->posX.whole=owner->posX.whole;
                    entity->posY.whole=owner->posY.whole;
                    if (entity->flipX)
                        entity->posX.whole=owner->posX.whole+0x60000;
                    else
                        entity->posX.whole=owner->posX.whole-0x60000;
                    entity->posY.whole=owner->posY.whole+0xA0000;
                    if (--entity->timer==0xFF)
                    {
                        entity->state=4;
                        entity->stage=0;
                        entity->subState=0;
                    }
                    break;
            }
            break;
        case 3:
            switch(entity->subState)
            {
                case 0:
                    entity->hidden=0;
                    entity->animFrame=2;
                    entity->timer=0;
                    entity->subState++;
                    if (gEwramData->playerState.player->posX.whole<owner->posX.whole)
                        entity->flipX=0;
                    else
                        entity->flipX=1;
                case 1:
                    if (!entity->flipX)
                        entity->posX.whole=owner->posX.whole-0xA0000;
                    else
                        entity->posX.whole=owner->posX.whole+0xA0000;
                    entity->posY.whole=owner->posY.whole-0x40000;
                    if (entity->timer++>39)
                    {
                        entity->state=4;
                        entity->stage=0;
                        entity->subState=0;
                    }
                    break;
            }
            break;
        case 4:
            switch(entity->subState)
            {
                case 0:
                    entity->timer=0;
                    entity->subState++;
                case 1:
                    entity->hidden=((u32)-(u8)((u32)entity->timer%3))>>31;
                    if (entity->timer++>30)
                    {
                        entity->state=0;
                        entity->stage=0;
                        entity->subState=0;
                        entity->timer=0;
                    }
                    break;
            }
            break;
        case 5:
            entity->posY.part16.integer=owner->posY.part16.integer+12;
            if (owner->flipX)
                entity->posX.part16.integer=owner->posX.part16.integer+14;
            else
                entity->posX.part16.integer=owner->posX.part16.integer-14;
            break;
    }
}

