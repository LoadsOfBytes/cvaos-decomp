#include "code_08009A0.h"
#include "balore_entity.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern void Music_SetCurrent(s32);
extern void Balore_UpdateEnter(struct EwramData_EntityData *);
extern void Balore_UpdateHover(struct EwramData_EntityData *);
extern void Balore_UpdateAttack(struct EwramData_EntityData *);
extern s32 Balore_UpdateDeath(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern u8 sBaloreEyeAnimOffsets[];

void EnemyBaloreUpdate(struct EwramData_EntityData *entity)
{
    u32 box[4];
    if (entity->state != 4) Enemy_UpdateHitFlash(entity);
    switch (entity->state)
    {
    case 0:
        Music_SetCurrent(0x11);
        entity->state = 1;
        entity->subState = 0;
        break;
    case 1: Balore_UpdateEnter(entity); break;
    case 2: Balore_UpdateHover(entity); break;
    case 3: Balore_UpdateAttack(entity); break;
    case 5: Anim_Update(entity); return;
    case 4:
        if (Balore_UpdateDeath(entity))
        {
            EntityDelete(entity);
            return;
        }
        break;
    }
    *(u16 *)&entity->ext.balore.flags &= 0xFFFE;
    if (entity->state != 4 && Entity_IsInRect(entity, -8, 0, 0x100, 0xB0))
        Hit_CheckWithPlayer(box, entity);
    Anim_StartIndexed(entity, sBaloreEyeAnimOffsets[(s8)entity->ext.balore.eyeAnim], 2, 0);
    Anim_Update(entity);
}
