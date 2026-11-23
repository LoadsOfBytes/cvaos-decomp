/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Gargoyle_DamagePlayerOnContact = Enemy_DamagePlayerOnContact
 * Gargoyle/Triton update and creation. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern s32 Enemy_PreUpdate(struct EwramData_EntityData *);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern s32 Entity_GetVisibilityPattern(s32);
extern struct EwramData_EntityData *Triton_CreateSpear(struct EwramData_EntityData *);
extern u8 Triton_LoadGraphics(struct EwramData_EntityData *);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *, s32);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Hit_EnableContact(struct EwramData_EntityData *, s32, void *, void *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern void Gargoyle_UpdateFly(struct EwramData_EntityData *);
extern void Triton_UpdateMove(struct EwramData_EntityData *);
extern void Triton_UpdateAttack(struct EwramData_EntityData *);
extern void Triton_OnHit(struct EwramData_EntityData *);
extern void Triton_DamagePlayerOnContact(struct EwramData_EntityData *);

static inline void Entity_DamagePlayerOnContact(struct EwramData_EntityData *entity, u16 attributes, u16 unused, u16 reaction)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = attributes;
    contactDamage.unused = unused;
    contactDamage.reaction = reaction;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 86964 | Same as Enemy_DamagePlayerOnContact.
 */
void Gargoyle_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/* Gargoyle: dispatch the AI state, a knocked-back state and the display state. */
void EnemyGargoyleUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    if (Enemy_PreUpdate(entity)) goto finished;
    switch (entity->state)
    {
    case 0: Gargoyle_UpdateFly(entity); break;
    case 1:
        EntityDampVelocityX(entity, 10);
        EntityAccelerateAndMove(entity, 0x30000);
        Enemy_UpdateFragmentDeath(entity);
        break;
    case 2:
        Anim_Update(entity);
        goto finished;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
finished:
    return;
}

/* Triton: dispatch movement, attack, death fade and display states. */
void EnemyTritonUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    if (entity->state != 2) Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity) && entity->state != 2)
    {
        Hit_CheckWithPlayer(collision, entity);
        goto finished;
    }
    switch (entity->state)
    {
    case 0: Triton_UpdateMove(entity); break;
    case 1: Triton_UpdateAttack(entity); break;
    case 2:
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0)
        {
            entity->state = 3;
            entity->timer = 10;
            entity->hidden = 1;
        }
        break;
    case 3:
        entity->hidden = 1;
        if ((s8)--entity->timer <= 0) entity->deleteRequested = 1;
        break;
    case 4:
        Anim_Update(entity);
        goto finished;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
finished:
    return;
}

/* Create the Triton with its spear (Triton_CreateSpear) and contact callbacks. */
void EnemyTritonCreate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *spear;
    if (!Triton_LoadGraphics(entity)) goto fail;
    Anim_StartIndexed(entity, 1, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    spear = Triton_CreateSpear(entity);
    if (!spear)
    {
    fail:
        EntityDelete(entity);
        return;
    }
    entity->ext.triton.spear = spear;
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        return;
    }
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, Triton_DamagePlayerOnContact, Triton_OnHit);
    else
        Hit_Enable(entity, 8, Triton_DamagePlayerOnContact, (s32)Triton_OnHit);
    Entity_UpdateBoxes(entity);
    entity->state = 0;
    Entity_SnapToGround(entity);
    entity->subState = 10;
}
