/* Harpy setup randomizes its animation phase before enabling movement. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 Harpy_LoadGraphics(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Hit_EnableContact(struct EwramData_EntityData *,s32,void *,void *);
extern void Harpy_DamagePlayerOnContact(void);
extern void Harpy_OnHit(void);
void EnemyHarpyCreate(struct EwramData_EntityData *entity)
{
    u8 demo;
    if(!Harpy_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity,0,0,1);
    entity->animTimer=RandomNumberGenerator()%16;
    entity->animStep=RandomNumberGenerator()%5;
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    demo=*(u8 *)((u8 *)gEwramData+0x8E) & 0x40;
    if(demo)
        entity->state=4;
    else
    {
        entity->spriteLayer=3;
        Hit_EnableContact(entity,8,Harpy_DamagePlayerOnContact,Harpy_OnHit);
        Entity_UpdateBoxes(entity);
        entity->state=demo;
        entity->subState=10;
        entity->stage=demo;
    }
}
