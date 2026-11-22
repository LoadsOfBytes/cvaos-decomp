/* Mudman begins its spawn animation or the display-mode idle animation. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 Mudman_LoadGraphics(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *,void *,void *);
extern void Mudman_DamagePlayerOnContact(void);
extern void Mudman_OnHit(void);
void EnemyMudmanCreate(struct EwramData_EntityData *entity)
{
    u8 demo;
    if(!Mudman_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity,0,0,2);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    demo=*(u8 *)((u8 *)gEwramData+0x8E) & 0x40;
    if(demo)
    {
        entity->state=4;
        Anim_StartIndexed(entity,2,0,1);
        Anim_Update(entity);
    }
    else
    {
        Entity_EnableBoxes(entity,Mudman_DamagePlayerOnContact,Mudman_OnHit);
        Entity_SnapToGround(entity);
        entity->state=demo;
    }
}
