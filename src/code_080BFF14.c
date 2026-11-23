#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"

extern s32 Enemy_PreUpdate(struct EwramData_EntityData *);
extern void EvilButcher_UpdateWalk(struct EwramData_EntityData *);
extern void EvilButcher_UpdateThrow(struct EwramData_EntityData *);
extern s32 EvilButcher_UpdateDeath(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Player_SetKnockbackVelocity(s32, s32);

void EnemyEvilButcherUpdate(struct EwramData_EntityData *entity)
{
    if (Enemy_PreUpdate(entity)) return;
    switch (entity->state)
    {
    case 0: EvilButcher_UpdateWalk(entity); break;
    case 1: EvilButcher_UpdateThrow(entity); break;
    case 2:
        if (EvilButcher_UpdateDeath(entity))
        {
            EntityDelete(entity);
            return;
        }
        break;
    case 3:
        Anim_Update(entity);
        return;
    }
    Enemy_CheckHitPlayer(entity);
    if (entity->state != 2) Entity_AnimateWithBoxes(entity);
}

/**
 * @brief Apply stored contact damage; while kick flag 1 is set, add one
 * quarter damage, use effect flags 3 and apply directional player knockback.
 */
void KickerSkeleton_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage effect;
    s32 speed;
    effect.attributes = 0;
    effect.unused = 0;
    if (entity->ext.kickerSkeleton.flags & 1)
    {
        effect.attack = entity->attack + (entity->attack >> 2);
        effect.reaction = 3;
    }
    else
    {
        effect.attack = entity->attack;
        effect.reaction = 0;
    }
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
    if (effect.reaction == 3)
    {
        speed = 0x58000;
        if (entity->velX.whole < 0) speed = -speed;
        Player_SetKnockbackVelocity(speed, -0x18000);
    }
}
