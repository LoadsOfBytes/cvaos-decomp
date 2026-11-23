/* Enemy helpers 0x080C45E8..0x080C467B. */
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s16 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);
extern void Skeleton_UpdateDeath(struct EwramData_EntityData *entity);

/**
 * @brief Start animation 0, clear hidden and advance animation. The gameplay meaning of state 5 remains unresolved.
 */
void Skeleton_UpdateState5Animation(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->hidden = 0;
            entity->subState++;
        case 1:
            Anim_Update(entity);
            break;
    }
}

/**
 * @brief Resolve damage, show it and flash; on lethal damage play the death sound and start the death sequence.
 */
void Skeleton_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    u16 hp = entity->hp; // HP, loaded first
    damage <<= 16;
    damage >>= 16;
    entity->hp = hp - damage;
    Entity_ShowDamage(entity, damage, 0);
    entity->hitFlashTimer = 8;
    if ((s16)entity->hp <= 0)
    {
        PlaySong(0x6B);
        entity->subState = 0;
        Skeleton_UpdateDeath(entity);
    }
}
