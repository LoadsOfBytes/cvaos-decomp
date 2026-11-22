/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   EnemyBasiliskUpdate = EnemyCockatriceUpdate */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void Entity_HideAboveY(struct EwramData_EntityData *entity, s32 minY);
extern s32 EntityIsOutsideScreen(struct EwramData_EntityData *entity, s32 margin);
extern s32 Enemy_IsStunned(struct EwramData_EntityData *entity);
extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern void CockatriceFamily_UpdatePatrol(struct EwramData_EntityData *entity);
extern void CockatriceFamily_UpdateAttack(struct EwramData_EntityData *entity);
extern s32 CockatriceFamily_UpdateDeath(struct EwramData_EntityData *entity);

/**
 * @brief BC204 | Same as EnemyCockatriceUpdate.
 */
void EnemyBasiliskUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[0x10];

    Entity_HideAboveY(entity, -0x20);
    if (entity->state != 2)
    {
        if (EntityIsOutsideScreen(entity, 6))
            return;
        if (Enemy_IsStunned(entity))
        {
            Hit_CheckWithPlayer(collision, entity);
            return;
        }
    }
    switch (entity->state)
    {
        case 0:
            CockatriceFamily_UpdatePatrol(entity);
            break;
        case 1:
            CockatriceFamily_UpdateAttack(entity);
            break;
        case 2:
            if (CockatriceFamily_UpdateDeath(entity))
            {
                EntityDelete(entity);
                return;
            }
            break;
        case 3:
            Anim_Update(entity);
            return;
    }
    if (entity->state != 2)
        Hit_CheckWithPlayer(collision, entity);
    Anim_Update(entity);
}
