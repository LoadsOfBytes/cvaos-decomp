/* Belmont subweapon variants 2 and 3: ground impact and repeated flames. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern s32 Entity_Walk(struct EwramData_EntityData *,s32,void *,s32);
extern struct EwramData_EntityData *Belmont_CreateSubweapon(struct EwramData_EntityData *,s32);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern s16 sBelmontMoveProfile[];
/**
 * @brief Variant 2: throw along terrain, stop horizontal motion at walls
 * and on ground impact repeatedly create variant 3 until the countdown
 * ends. Preserve lifetime and player-hit checks.
 */
void BelmontSubweapon2_Update(struct EwramData_EntityData *entity)
{
    switch(entity->state)
    {
        case 0:
        {
            s32 speed=0x24000;
            entity->unk_514.unk_514_32=(void *)(u32)speed;
            entity->velX.whole=speed;
            if(!entity->flipX)
                entity->velX.whole=-speed;
            entity->velY.whole=-0x6000;
            entity->accelY.whole=0x4000;
            entity->hidden=0;
            entity->state++;
            break;
        }
        case 1:
        {
            s32 collision=Entity_Walk(entity,(s32)entity->unk_514.unk_514_32,sBelmontMoveProfile,0);
            if(collision & 3)
                entity->unk_514.unk_514_32=NULL;
            if(collision & 8)
                entity->state=3;
            break;
        }
        case 2:
            EntityDelete(entity);
            return;
        case 3:
            if(entity->subState==0)
            {
                PlaySong(0x129);
                entity->hidden=1;
                entity->timer=60;
                entity->subState++;
            }
            if((u8)((u32)entity->timer%10)==0)
                Belmont_CreateSubweapon(entity,3);
            if(--entity->timer==0xFF)
                entity->state=2;
            break;
    }
    if(EntityIsOutsideScreen(entity,4))
    {
        entity->state=2;
        entity->hidden=1;
    }
    Enemy_CheckHitPlayer(entity);
}
/**
 * @brief Variant 3: animate and move along terrain, stopping at walls;
 * remove when animation completes or outside the screen, otherwise
 * update hit boxes and check player contact.
 */
void BelmontSubweapon3_Update(struct EwramData_EntityData *entity)
{
    if(entity->state==0)
    {
        Anim_StartIndexed(entity,2,0,0);
        entity->unk_514.unk_514_32=(void *)0x12000;
        entity->hidden=0;
        entity->state++;
        PlaySong(0x91);
    }
    if(Entity_Walk(entity,(s32)entity->unk_514.unk_514_32,sBelmontMoveProfile,0) & 3)
        entity->unk_514.unk_514_32=NULL;
    if(entity->animDone || EntityIsOutsideScreen(entity,4))
        EntityDelete(entity);
    else
    {
        Entity_AnimateWithBoxes(entity);
        entity->hit.flags|=4;
        Enemy_CheckHitPlayer(entity);
    }
}
