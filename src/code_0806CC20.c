/* Common pre-update entity culling and collision callback. */
#include "structs/ewram.h"
#include "code/code_080211F0.h"
#include "structs/entity_template.h"

extern void Entity_HideAboveY(struct EwramData_EntityData *, s32);
extern s32 Entity_IsInRect(struct EwramData_EntityData *, s16, s16, u16, u16);
extern s32 Enemy_IsStunned(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);

s32 Enemy_PreUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    u32 templateOffset = entity->unk_518.part8.enemyId * sizeof(struct EntityTemplate);
    struct EntityTemplate *entry = (struct EntityTemplate *)(templateOffset + (u32)sEnemyData);
    if (entry->previewOffsetY == 0)
        Entity_HideAboveY(entity, -16);
    if (entity->enemyFlags & 2)
        return 0;
    if (Entity_IsInRect(entity, -128, -64, 496, 288))
    {
        if (Entity_IsInRect(entity, -16, 0, 272, 176))
            Player_TrackEntity(entity);
        if (!Enemy_IsStunned(entity))
            return 0;
        Hit_CheckWithPlayer(collision, entity);
    }
    return 1;
}
