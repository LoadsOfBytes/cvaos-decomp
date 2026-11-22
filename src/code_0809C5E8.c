#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code/code_080211F0.h"
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Legion_UpdateCircle(struct EwramData_EntityData *);
extern void Legion_UpdateBounce(struct EwramData_EntityData *);
extern void Legion_UpdateDeath(struct EwramData_EntityData *);
extern void Legion_UpdatePreview(struct EwramData_EntityData *);
extern void Heap_Free(void *);

void EnemyLegionUpdate(struct EwramData_EntityData *entity)
{
    /* Original scratch area is 0x30 bytes. */
    u32 box[12];
    if (entity->state != 2)
    {
        if (Entity_IsInRect(entity, -16, 0, 0x110, 0xB0)) Player_TrackEntity(entity);
        if (entity->state != 2) Enemy_UpdateHitFlash(entity);
    }
    if (Entity_IsTimeStopped(entity) && entity->state != 2)
    {
        Hit_CheckWithPlayer(box, entity);
        return;
    }
    switch (entity->state)
    {
    case 0: Legion_UpdateCircle(entity); break;
    case 1: Legion_UpdateBounce(entity); break;
    case 2:
        Legion_UpdateDeath(entity);
        if (entity->deleteRequested && entity->unk_4F8.unk_4F8_entity != NULL)
            Heap_Free(entity->unk_4F8.unk_4F8_entity);
        break;
    case 3: Legion_UpdatePreview(entity); Anim_Update(entity); return;
    }
    Anim_Update(entity);
    if (entity->state != 2 && !(entity->unk_504.part8.unk_506 & 1))
        Hit_CheckWithPlayer(box, entity);
}
