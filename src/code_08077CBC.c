/* Quezlcoatl dispatch and staged destruction of the parent entity. */
#include "code_08009A0.h"
#include "quezlcoatl_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
extern s32 Enemy_IsStunned(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *,struct EwramData_EntityData *);
extern void Quezlcoatl_UpdateFly(struct EwramData_EntityData *);
extern void Quezlcoatl_UpdatePreview(struct EwramData_EntityData *);
extern void Quezlcoatl_SpawnDeathPiece(struct EwramData_EntityData *,s32,s32,s32);
extern void Effect_SpawnAt(s32,s32,s32);
extern void Effect_SpawnRandom(s32,s32,s32,s32,s32);
extern void Heap_Free(void *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
void EnemyQuezlcoatlUpdate(struct EwramData_EntityData *entity)
{
    s32 bounds[4];
    if(entity->state!=2 && Entity_IsInRect(entity,-16,0,272,176))
        Player_TrackEntity(entity);
    if(Enemy_IsStunned(entity))
    {
        if(entity->state!=2) Hit_CheckWithPlayer(bounds,entity);
        return;
    }
    switch(entity->state)
    {
        case 0: Quezlcoatl_UpdateFly(entity); break;
        case 1: break;
        case 2:
            if(entity->subState==0)
            {
                u32 flipped;
                s32 offset;
                entity->subState++;
                flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(entity))&0x40;
                offset=0x80000;
                if(flipped) offset=-0x80000;
                Quezlcoatl_SpawnDeathPiece(entity,entity->posX.whole+offset,entity->posY.whole-0x80000,18);
                Quezlcoatl_SpawnDeathPiece(entity,entity->posX.whole+offset,entity->posY.whole-0x80000,11);
                Effect_SpawnAt(entity->posX.whole,entity->posY.whole,entity->spriteLayer-1);
                Effect_SpawnRandom(entity->posX.whole,entity->posY.whole,0x80000,0x80000,entity->spriteLayer-1);
                Effect_SpawnRandom(entity->posX.whole,entity->posY.whole,0x80000,0x80000,entity->spriteLayer-1);
                Effect_SpawnRandom(entity->posX.whole,entity->posY.whole,0x80000,0x80000,entity->spriteLayer-1);
                entity->hidden=1;
            }
            else if(entity->subState==1)
            {
                entity->subState++;
                if(QUEZLCOATL_HISTORY(entity)) Heap_Free((void *)QUEZLCOATL_HISTORY(entity));
            }
            else entity->deleteRequested=1;
            break;
        case 3: Quezlcoatl_UpdatePreview(entity); return;
    }
    Enemy_CheckHitPlayer(entity);
}

