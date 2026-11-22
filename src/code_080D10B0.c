/* Belmont backward dodge with timed flashing and collision handling. */
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern void Afterimage_Create(struct EwramData_EntityData *,s32,s32,s32);
extern s32 Entity_MoveWithCollisionProfile(struct EwramData_EntityData *,s16 *,u16,s32);
extern s32 Entity_ClampXToRoom(struct EwramData_EntityData *,s32);
extern s16 sBelmontMoveProfile[];
/**
 * @brief Dodge state (9): create afterimages, move backward relative to
 * facing with timed flashing and terrain bounds handling, then return
 * to combat. Updates animation internally.
 */
void Belmont_UpdateDodge(struct EwramData_EntityData *entity)
{
    s32 state=entity->subState;
    switch(state)
    {
        case 0:
            entity->palette=entity->unk_510.part8.basePalette;
            entity->unk_510.part8.hitFlashTimer=state;
            entity->unk_4F8.part8.unk_4F9=state;
            entity->unk_504.part8.unk_504=state;
            if(entity->animScriptIndex!=3)
                Anim_StartIndexed(entity,3,1,1);
            Anim_Update(entity);
            entity->subState++;
            entity->stage=state;
            Afterimage_Create(entity,48,5,2);
            Afterimage_Create(entity,48,3,2);
            break;
        case 1:
            if(entity->animDone)
            {
                PlaySong(0x128);
                if(entity->flipX)
                {
                    entity->velX.whole=0x37000;
                    entity->accelX.whole=-0xB00;
                }
                else
                {
                    entity->velX.whole=-0x37000;
                    entity->accelX.whole=0xB00;
                }
                entity->subState++;
                entity->stage=0;
                entity->velY.whole=0x10000;
                entity->timer=80;
            }
            break;
        case 2:
        {
            s32 collision;
            Entity_FaceTarget(entity);
            { u8 value=entity->timer & 1; entity->hidden=value; }
            collision=Entity_MoveWithCollisionProfile(entity,sBelmontMoveProfile,4,0);
            if(!(collision & 8))
                entity->posY.whole+=0x10000;
            if(collision & 3)
            {
                if(entity->flipX)
                    entity->accelX.whole=entity->velX.whole=0;
                else
                    entity->accelX.whole=entity->velX.whole=0;
            }
            if(Entity_ClampXToRoom(entity,1))
            {
                if(entity->flipX)
                    entity->accelX.whole=entity->velX.whole=0;
                else
                    entity->accelX.whole=entity->velX.whole=0;
            }
            if(--entity->timer==255)
            {
                entity->state=2;
                entity->stage=0;
                entity->subState=0;
                entity->objMode=0;
                entity->timer=0;
                entity->hidden=0;
            }
            break;
        }
    }
    Anim_Update(entity);
}
