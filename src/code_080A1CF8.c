/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   GladiatorPart_OnHit = ManticoreTail_OnHit */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_080211F0.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void Gladiator_OnDamaged(struct EwramData_EntityData *entity, s32 unused1, u16 *unused2, s32 damage);
extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern void Gladiator_UpdateCharge(struct EwramData_EntityData *);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
void Gladiator_UpdateDeath(struct EwramData_EntityData *entity);

/**
 * @brief A1CF8 | Same as ManticoreTail_OnHit.
 */
void GladiatorPart_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    struct EwramData_EntityData *gladiator = ENTITY_OWNER(entity);
    s32 oldX = gladiator->posX.whole;
    s32 oldY = gladiator->posY.whole;
    s16 damage;
    s16 attackPower;

    gladiator->posX.whole = entity->posX.whole;
    gladiator->posY.whole = entity->posY.whole;
    attackPower = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(ENTITY_OWNER(entity), attacker, hit, attackPower);
    gladiator->posX.whole = oldX;
    gladiator->posY.whole = oldY;
    Entity_ShowDamage(entity, damage, 0);
    Gladiator_OnDamaged(ENTITY_OWNER(entity), attacker, hit, damage);
}

/* Gladiator: dispatch the AI (0), the death (1) and a frozen state (2). */
void EnemyGladiatorUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[48];

    if (entity->state != 1)
    {
        if (Entity_IsInRect(entity, -16, 0, 272, 176)) Player_TrackEntity(entity);
        if (entity->state != 1) Enemy_UpdateHitFlash(entity);
    }
    if (Entity_IsTimeStopped(entity) && entity->state != 1)
    {
        Hit_CheckWithPlayer(collision, entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        Gladiator_UpdateCharge(entity);
        break;
    case 1:
        Gladiator_UpdateDeath(entity);
        break;
    case 2:
        Anim_Update(entity);
        return;
    }
    if (entity->ext.gladiator.flinch == 0) Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}

/* Gladiator death: slide to a stop and fall, sound once. */
void Gladiator_UpdateDeath(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        EntityDampVelocityX(entity, 0x18);
        EntityAccelerateAndMove(entity, 0x50000);
        Enemy_UpdateFragmentDeath(entity);
        if (entity->stage == 0)
        {
            entity->stage++;
            PlaySong(0x6B);
        }
    }
}
