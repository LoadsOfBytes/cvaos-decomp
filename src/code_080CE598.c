/* Alastor spinning leap, conditional dive, and recovery. */
#include "code_080D73B8.h"
#include "alastor_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern struct EwramData_EntityData *Alastor_CreateAfterimage(struct EwramData_EntityData *,u8);
extern s32 Alastor_IsTargetInLowerBox(struct EwramData_EntityData *,u16,u16);
extern s32 Entity_IsOffsetOutsideRoom(struct EwramData_EntityData *,s32,s32);
extern s32 Collision_GetPushUp(s32,s32);
static inline s32 RoomX(s32 pos) { struct BgLayerState *bg=&gEwramData->bgInfo[1]; return pos+(s16)bg->xPos.part16.integer; }
static inline s32 RoomY(s32 pos) { struct BgLayerState *bg=&gEwramData->bgInfo[1]; return pos+(s16)bg->yPos.part16.integer; }
#define ANGLE(e) (ALASTOR_ANGLE((e)))
#define STEP(e) (ALASTOR_TURN((e)))
static inline void Advance(struct EwramData_EntityData *e,s32 amount,u8 zero) { e->subState+=amount; e->stage=zero; }
/**
 * @brief Attack state (6): spin through a leap while spawning copies,
 * optionally dive when the target is in the lower test box, then recover.
 */
void Alastor_UpdateSpinningLeap(struct EwramData_EntityData *entity)
{
    switch(entity->subState)
    {
        case 0:
        {
            s32 speed;
            u16 step;
            entity->unk_4F4.part8.unk_4F7=0;
            entity->subState++;
            entity->velX.whole=(speed=-0x40000);
            entity->velY.whole=-0x20000;
            entity->accelY.whole=0x4000;
            STEP(entity)=(step=0xF880);
            entity->timer=16;
            if(entity->flipX)
            {
                entity->velX.whole=-speed;
                STEP(entity)=-step;
            }
            PlaySong(0x1BB);
            PlaySong(0x86);
        }
        case 1:
        {
            s32 z;
            u16 zero;
            Alastor_CreateAfterimage(entity,5);
            entity->posX.whole+=entity->velX.whole;
            entity->posY.whole+=entity->velY.whole;
            entity->velY.whole+=entity->accelY.whole;
            {
                u16 a=ANGLE(entity)+STEP(entity);
                /* Assigning zero from z sets the two zero registers in the original order. */
                z=0; zero=z;
                ANGLE(entity)=a;
                ObjAffine_Set(ANGLE(entity),0x10000,0x10000,entity->flipX,z,entity->affineIndex);
            }
            if(--entity->timer==0xFF)
            {
                if(Alastor_IsTargetInLowerBox(entity,128,256))
                    Advance(entity,1,zero);
                else
                    Advance(entity,2,zero);

            }
            break;
        }
        case 2:
            switch(entity->stage)
            {
                case 0:
                    entity->velY.whole=0x500;
                    entity->accelY.whole=0x4000;
                    entity->timer=0;
                    entity->stage++;
                case 1:
                    entity->posY.whole+=entity->velY.whole;
                    if(entity->timer==10)
                        PlaySong(0x86);
                    if(entity->timer++>10)
                        entity->velY.whole+=entity->accelY.whole;
                    if(Entity_IsOffsetOutsideRoom(entity,0,0) || (s16)Collision_GetPushUp(RoomX((s16)entity->posX.part16.integer),RoomY((s16)entity->posY.part16.integer)+8)!=0)
                    {
                        entity->subState++;
                        entity->stage=0;
                    }
                    break;
            }
            break;
        case 3:
            if(entity->stage==0)
            {
                entity->timer=40;
                entity->stage++;
            }
            else if(--entity->timer==0xFF)
            {
                entity->timer=0;
                entity->state=1;
                entity->stage=0;
                entity->subState=0;
                entity->unk_4F4.part8.unk_4F7=0;
            }
            break;
    }
}
