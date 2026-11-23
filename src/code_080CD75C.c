/* Alastor appearance and blink sequence. */
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern u8 Entity_IsTargetWithinRange(struct EwramData_EntityData *,s32);
static inline u32 Blink(u32 timer) { return ((u32)-(u8)(timer%3))>>31; }
/**
 * @brief Appearance state (0): hide the owner and linked part, wait for
 * proximity, blink them into view and hand off to approach state 1.
 */
void Alastor_UpdateAppear(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part=&gEwramData->entityData[entity->ext.alastorPart.index];
    switch(entity->subState)
    {
        case 0:
            Entity_FaceTarget(entity);
            part->hidden=1;
            part->state=5;
            entity->hidden=1;
            entity->timer=0;
            entity->subState++;
        case 1:
            if (Entity_IsTargetWithinRange(entity,0x500000))
            {
                part->posX.part16.integer=entity->posX.part16.integer;
                part->posY.part16.integer=entity->posY.part16.integer;
                part->posX.part16.integer=entity->posX.part16.integer+10;
                part->posY.part16.integer=entity->posY.part16.integer+16;
                ObjAffine_Set(0x10000,0x10000,0x10000,entity->flipX,0,entity->affineIndex);
                entity->subState++;
            }
            break;
        case 2:
            entity->hidden=Blink(entity->timer);
            part->hidden=Blink(entity->timer);
            entity->objMode=1;
            if(entity->timer++>40)
            {
                entity->hidden=0;
                part->hidden=0;
                entity->objMode=0;
                entity->timer=0;
                entity->subState++;
            }
            break;
        case 3:
            if(entity->timer++>20)
            {
                entity->timer=0;
                entity->subState++;
            }
            break;
        case 4:
            part->hidden=Blink(entity->timer);
            if(entity->timer++>40)
            {
                part->hidden=1;
                part->state=0;
                part->stage=0;
                part->subState=0;
                entity->timer=0;
                entity->subState++;
            }
            break;
        case 5:
            if(entity->timer++>20)
            {
                entity->timer=0;
                entity->state=1;
                entity->stage=0;
                entity->subState=0;
            }
            break;
    }
}
