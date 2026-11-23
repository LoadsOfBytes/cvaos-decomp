/* White Dragon dispatch, collision gating and delayed death cleanup. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *,struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern void Heap_Free(void *);
extern void WhiteDragon_UpdateIdle(struct EwramData_EntityData *);
extern void WhiteDragon_UpdateAttack(struct EwramData_EntityData *);
extern void WhiteDragon_UpdateRecoil(struct EwramData_EntityData *);
extern void WhiteDragon_UpdateDeath(struct EwramData_EntityData *);
extern void WhiteDragon_SpawnDeathPiece(struct EwramData_EntityData *,s32,s32,s32);
extern void WhiteDragon_SpawnSpark(struct EwramData_EntityData *,s32,s32,s32);
void EnemyWhiteDragonUpdate(struct EwramData_EntityData *entity)
{
    s32 bounds[4];
    if(entity->state!=2)
    {
        if(Entity_IsInRect(entity,-16,0,272,176))
            Player_TrackEntity(entity);
        if(entity->state!=2)
            Enemy_UpdateHitFlash(entity);
    }
    if(Entity_IsTimeStopped(entity) && entity->state!=2)
        goto collision;
    if(entity->hitFlashTimer && entity->state!=3 && entity->state!=2)
    {
collision:
        Hit_CheckWithPlayer(bounds,entity);
        return;
    }
    switch(entity->state)
    {
        case 0: WhiteDragon_UpdateIdle(entity); break;
        case 1: WhiteDragon_UpdateAttack(entity); break;
        case 2:
        {
            s32 phase=entity->subState;
            if(!phase)
            {
                WhiteDragon_UpdateDeath(entity);
                if(entity->deleteRequested)
                {
                    s32 offset;
                    u32 flipped;
                    entity->deleteRequested=0;
                    entity->hidden=1;
                    entity->subState=1;
                    entity->timer=phase;
                    flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(entity)) & 0x40;
                    offset=0x80000;
                    if(flipped) offset=-0x80000;
                    PlaySong(0x6B);
                    WhiteDragon_SpawnDeathPiece(entity,entity->posX.whole+offset,entity->posY.whole,10);
                    WhiteDragon_SpawnDeathPiece(entity,entity->posX.whole+offset,entity->posY.whole,11);
                    WhiteDragon_SpawnSpark(entity,entity->posX.whole,entity->posY.whole,entity->spriteLayer-1);
                    if(entity->ext.whiteDragon.history)
                        Heap_Free((void *)entity->ext.whiteDragon.history);
                }
            }
            else if(entity->timer++>49)
                entity->deleteRequested=1;
            break;
        }
        case 4: WhiteDragon_UpdateIdle(entity); return;
        case 3: WhiteDragon_UpdateRecoil(entity); break;
    }
    Enemy_CheckHitPlayer(entity);
}
