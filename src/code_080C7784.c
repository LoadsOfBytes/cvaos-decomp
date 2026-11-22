/* Linked part (NemesisPartner_Update, cf. Alastor part AlastorPart_Update): follows its
 * owner, two attack poses with timed return to the blinking rest state. */
#include "structs/ewram.h"
extern s32 Enemy_IsStunned(struct EwramData_EntityData *);
extern void Entity_FaceTarget(struct EwramData_EntityData *);

static inline void SetState(struct EwramData_EntityData *entity, u8 state)
{
    if (entity->state != state)
    {
        entity->state = state;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/**
 * @brief Follow the owner in the initial pose or execute owner-triggered attack/reaction poses and a timed blinking return to hidden state 0.
 */
void NemesisPartner_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=&gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
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
                    entity->flipX=1-entity->flipX;
                    entity->hidden=0;
                    entity->animFrame=2;
                    entity->timer=0;
                    entity->subState++;
                case 1:
                    if (entity->flipX)
                        entity->posX.whole=owner->posX.whole-0x90000;
                    else
                        entity->posX.whole=owner->posX.whole+0x90000;
                    entity->posY.whole=owner->posY.whole+0x250000;
                    if (entity->timer++>9)
                        SetState(entity, 4);
                    break;
            }
            break;
        case 3:
            switch(entity->subState)
            {
                case 0:
                    entity->hidden=0;
                    entity->animFrame=3;
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
                        SetState(entity, 4);
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
                        SetState(entity, 0);
                        entity->timer=0;
                    }
                    break;
            }
            break;
        case 5:
            break;
    }
}
