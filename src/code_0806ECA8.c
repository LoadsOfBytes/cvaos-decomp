/* Valkyrie hit effect: full strength during animation 1, frames 4..6. */
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"

extern s32 Enemy_IsStunned(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Valkyrie_UpdateApproach(struct EwramData_EntityData *);
extern void Valkyrie_UpdateAttack(struct EwramData_EntityData *);
extern void Valkyrie_UpdateRecoil(struct EwramData_EntityData *);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);

/**
 * @brief Contact: Player_ApplyContactDamage with half the attack, the full attack
 * during the thrust frames (state 1, animation 1, steps 4..6).
 */
void Valkyrie_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage effect;
    effect.attack = entity->attack >> 1;
    if (entity->state == 1 && entity->animScriptIndex == 1
        && (u8)(entity->animStep - 4) <= 2)
        effect.attack = entity->attack;
    effect.attributes = 0;
    effect.unused = 0;
    effect.reaction = 0;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
}

void EnemyValkyrieUpdate(struct EwramData_EntityData *entity)
{
    u32 box[4];
    if (Entity_IsInRect(entity, -0x10, 0, 0x110, 0xB0))
        Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 2)
            Hit_CheckWithPlayer(box, entity);
        return;
    }
    switch (entity->state)
    {
    case 0: Valkyrie_UpdateApproach(entity); break;
    case 1: Valkyrie_UpdateAttack(entity); break;
    case 2:
        EntityDampVelocityX(entity, 0xA);
        EntityAccelerateAndMove(entity, 0x30000);
        Enemy_UpdateFragmentDeath(entity);
        break;
    case 3: Valkyrie_UpdateRecoil(entity); break;
    case 4:
        Anim_Update(entity);
        return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}
