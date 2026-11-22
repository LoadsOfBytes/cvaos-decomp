/* EnemyCockatriceUpdate. */
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
 * @brief BC164 | Cockatrice update. Outside state 2: stop when outside the
 * screen and only run Hit_CheckWithPlayer while Enemy_IsStunned says so. States: 0
 * CockatriceFamily_UpdatePatrol, 1 CockatriceFamily_UpdateAttack, 2 CockatriceFamily_UpdateDeath (delete when nonzero), 3 only
 * animate; then Hit_CheckWithPlayer (not in state 2) and animate.
 */
void EnemyCockatriceUpdate(struct EwramData_EntityData *entity)
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

extern void CockatriceFamily_Create(struct EwramData_EntityData *entity, s32 paletteSlot);

/**
 * @brief BC1F8 | Basilisk create: CockatriceFamily_Create(entity, 1).
 */
void EnemyBasiliskCreate(struct EwramData_EntityData *entity)
{
    CockatriceFamily_Create(entity, 1);
}
