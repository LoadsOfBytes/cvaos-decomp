/* Bael patrol, obstacle jump, turn and charge selection. Only matches without
 * -f2003-patch (NO_2003_PATCH_SRCS in ../Makefile).:
 * direct posX/posY reads (loop keeps the addresses), rowOffset declared in the
 * inner loop body (hoisted by loop.c), literal zeros, ABS through an inline. */
#include "code_08009A0.h"
#include "structs/ewram.h"
extern s32 Bael_CheckCleanup(struct EwramData_EntityData *entity);
extern s32 Entity_IsOffsetOutsideRoom(struct EwramData_EntityData *entity,s32 x,s32 y);
extern void Bael_MoveAndRotate(struct EwramData_EntityData *entity,s32 mode);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *entity,s32 x,s32 y);
extern s8 Entity_GetTargetDirection(struct EwramData_EntityData *entity);
extern s32 Collision_GetPushUp(s32 x,s32 y);
extern void Bael_UpdateCharge(struct EwramData_EntityData *entity);
#define SPEED(e) (e->ext.wheel.speed)
#define ABS(x) ((x)<0?-(x):(x))
static inline s32 AbsI(s32 x) { if (x<0) x=-x; return x; }
static inline s32 RoomX(s32 pos) { struct BgLayerState *bg=&gEwramData->bgInfo[1]; return pos+(s16)bg->xPos.part16.integer; }
static inline s32 RoomY(s32 pos) { struct BgLayerState *bg=&gEwramData->bgInfo[1]; return pos+(s16)bg->yPos.part16.integer; }

/**
 * @brief Patrol, test obstacles and target proximity, and dispatch obstacle jumps, charge or timed facing changes.
 */
void Bael_UpdateActive(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part=&gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    if (Bael_CheckCleanup(entity))
        return;
    switch (entity->subState)
    {
        case 0:
        {
            s32 row;
            s32 zero;
            s32 firstX;
            if (Entity_IsOffsetOutsideRoom(entity,0,88))
                Bael_MoveAndRotate(entity,0);
            else
            {
                Bael_MoveAndRotate(entity,1);
                entity->accelY.whole=0x4000;
            }
            if (Entity_IsOffsetOutsideRoom(entity,0,32))
            {
                entity->subState=10;
                entity->stage=0;
                break;
            }
            if (!Collision_GetPushUp(RoomX((s16)entity->posX.part16.integer),RoomY((s16)entity->posY.part16.integer)+24))
                break;
            if (Target_IsNearEntity(entity,192,48))
            {
                if ((!part->flipX && Entity_GetTargetDirection(entity)==-1) ||
                    (part->flipX && Entity_GetTargetDirection(entity)==1))
                {
                    entity->subState=1;
                    entity->stage=0;
                }
            }
            row=0;
            zero=row;
            for(;row<6;row++)
            {
                s32 col;
                for(col=0;col<6;col++)
                {
                    s32 rowOffset=row*7;
                    s32 offset=col*7;
                    if (!part->flipX)
                        offset=-offset;
                    if (Collision_GetPushUp(RoomX((s16)entity->posX.part16.integer)+offset,RoomY((s16)entity->posY.part16.integer)-rowOffset+12))
                    {
                        entity->subState=10;
                        entity->stage=zero;
                        break;
                    }
                }
            }
            if (++entity->timer==0xFF)
            {
                entity->timer=0;
                if ((RandomNumberGenerator() & 7)==0)
                {
                    entity->subState=2;
                    entity->stage=0;
                }
            }
            break;
        }
        case 10:
            switch(entity->stage)
            {
                case 0:
                {
                    s32 speed;
                    entity->timer=0xFF;
                    entity->stage++;
                    speed=0x6000/((RandomNumberGenerator() & 3)+1);
                    entity->velX.whole=speed;
                    entity->velY.whole=-0x10000;
                    entity->accelY.whole=(-entity->velY.whole*2)/(s32)entity->timer;
                    SPEED(entity)=0x8000;
                    if (!part->flipX)
                    {
                        SPEED(entity)=-(s32)SPEED(entity);
                        if (!part->flipX)
                            entity->velX.whole=-ABS(speed);
                    }
                }
                case 1:
                {
                    entity->posX.whole+=entity->velX.whole;
                    entity->posY.whole+=entity->velY.whole;
                    entity->velY.whole+=entity->accelY.whole;
                    if (entity->velY.whole>0)
                    {
                        s32 adjust=Collision_GetPushUp(RoomX((s16)entity->posX.part16.integer),RoomY((s16)entity->posY.part16.integer)+26);
                        entity->posY.part16.integer+=adjust;
                        if (adjust)
                            entity->timer=0;
                    }
                    Bael_MoveAndRotate(entity,1);
                    if ((u8)(--entity->timer)==0xFF)
                    {
                        entity->subState=0;
                        entity->stage=0;
                    }
                    if (Entity_IsOffsetOutsideRoom(entity,0,32))
                        entity->stage=0;
                    break;
                }
            }
            break;
        case 1:
            Bael_UpdateCharge(entity);
            break;
        case 2:
            switch(entity->stage)
            {
                case 0:
                    entity->timer=RandomNumberGenerator() % 32 + 32;
                    entity->stage++;
                case 1:
                    part->flipX=1-part->flipX;
                    entity->stage++;
                case 2:
                {
                    if (Target_IsNearEntity(entity,96,48))
                    {
                        entity->subState=1;
                        entity->stage=0;
                    }
                    if ((u8)(--entity->timer)==0xFF)
                    {
                        if ((RandomNumberGenerator() & 1)==0)
                        {
                            entity->subState=0;
                            entity->stage=0;
                            entity->timer=0;
                            if (part->flipX)
                                SPEED(entity)=AbsI(SPEED(entity));
                            else
                                SPEED(entity)=-AbsI(SPEED(entity));
                        }
                        else
                            entity->stage=0;
                    }
                    break;
                }
            }
            break;
    }
}
