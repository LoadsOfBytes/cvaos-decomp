/* Hit callback of a two-part enemy. */
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern u16 Player_CalculateAttackPower(u16);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s16, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern void Nemesis_SetAfterimagesHidden(struct EwramData_EntityData *, s32);

/* the partner entity, slot unk_4F6 (see Nemesis_CreatePartner) */
#define LINKED(e) (*(gEwramData->entityData + e->ext.nemesis.partnerIndex))

/**
 * @brief Apply damage and flash; when the hit counter or health condition requires it, start partner reaction and state 6, then reveal self and afterimages.
 */
void Nemesis_OnHit(struct EwramData_EntityData *entity, s32 hit, u16 *data)
{
    s32 damage = Hit_ResolveDamage(entity, hit, data, (s16)Player_CalculateAttackPower(data[2]));

    /* hp read before the sign extension, as in Werewolf_OnHit */
    {
        u16 hp = entity->unk_518.part16.hp;
        damage <<= 16;
        damage >>= 16;
        entity->unk_518.part16.hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    entity->unk_510.part8.hitFlashTimer = 8;
    if (entity->ext.nemesis.hitsBeforeReact != 0)
    {
        if ((s16)entity->unk_518.part16.hp > 0)
        {
            entity->ext.nemesis.hitsBeforeReact--;
            goto done;
        }
        goto dead;
    }
    if ((s16)entity->unk_518.part16.hp <= 0)
    {
    dead:
        Enemy_DropRewards(entity);
        PlaySong(0x1C3);
    }
    if (LINKED(entity).state != 3)
    {
        LINKED(entity).state = 3;
        LINKED(entity).subState = LINKED(entity).stage = 0;
    }
    entity->state = 6;
    entity->subState = 0;
    entity->ext.nemesis.hitsBeforeReact = 3;
done:
    entity->hidden = 0;
    Nemesis_SetAfterimagesHidden(entity, 0);
}
