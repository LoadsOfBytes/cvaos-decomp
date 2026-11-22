#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void BlueSoulProjectile_IgnoreContact(struct EwramData_EntityData *);

/* Inlined BlueSoul_DeleteEntity (src/code_08031D6C.c). */
static inline void EndBlueSoul(struct EwramData_EntityData *entity)
{
    if (entity->affineIndex != 0 || entity->affine)
        ObjAffine_Free(entity->affineIndex);
    gEwramData->playerState.activeBlueSoulEntities[entity->unk_508.part16.unk_508] = NULL;
    EntityDelete(entity);
}

/* Blue soul projectile spawned with a contact child by SkillWitchGiantGhost_OnContact: moves by
 * its velocity with effect Effect_CreateFromTable and deletes itself off screen. */
/**
 * @brief Blue soul projectile spawned on contact: move by its velocity with a
 * trail effect and delete itself off screen.
 */
void SkillWitchGiantGhostProjectileUpdate(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        Hit_Enable(entity, 6, BlueSoulProjectile_IgnoreContact, 0);
        HitBox_SetAttackRaw(entity, 0x0808FCFC);
        entity->hit.flags |= 4;
        entity->scrollWithBg = 1;
        entity->state++;
        PlaySong(0x98);
    case 1:
        Effect_CreateFromTable(entity->posX.whole, entity->posY.whole, 0, 0, 5, 6);
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        break;
    case 2:
        EndBlueSoul(entity);
        return;
    }
    entity->roomActive = 1;
    if (EntityIsOutsideScreen(entity, 2))
        entity->state = 2;
}
