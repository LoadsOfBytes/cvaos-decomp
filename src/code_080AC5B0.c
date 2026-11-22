/* Big Golem damage and contact callbacks. */
#include "code_08009A0.h"
#include "big_golem_entity.h"
#include "joint_entity.h"
#include "code_080D73B8.h"
#include "code_08033CAC.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern void BossDefeatEffect_Spawn(s32, s32);
extern void BossRecoveryPickup_Spawn(s32, s32, s32, s32, s32);
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s16);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s32, s32);

/* Flash and reduce the shared HP; end the boss encounter at zero HP. */
void BigGolem_OnDamaged(struct EwramData_EntityData *entity, s32 attacker, u16 *hit, s32 damage)
{
    entity->unk_510.part8.hitFlashTimer = 8;
    entity->unk_518.part16.hp -= damage;
    if ((s8)BIG_GOLEM_HIT_COUNT(entity) < 10)
        BIG_GOLEM_HIT_COUNT(entity)++;
    BIG_GOLEM_HIT(entity) = 1;
    if ((s16)entity->unk_518.part16.hp <= 0)
    {
        Enemy_DropRewards(entity);
        Anim_StartIndexed(entity, 3, 0, 1);
        PlaySong(0x6E);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->timer = 60;
        entity->state = 2;
        entity->subState = 0;
        if (entity->unk_514.part16.spawnParam0 != 0)
        {
            BossDefeatEffect_Spawn(entity->posX.whole, entity->posY.whole - 0x200000);
            BossRecoveryPickup_Spawn((s16)entity->posX.part16.integer, 32, 12, 96, 0);
            Player_SetSelectedTarget(NULL);
        }
    }
}

/* Contact from parts 12/13 during the parent's early attack substates
 * also pushes the player horizontally away and upwards. */
void BigGolemPart_OnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contact;
    struct EwramData_EntityData *parent = JOINT_PART_OWNER(entity);
    contact.attack = entity->attack;
    contact.attributes = 0;
    contact.unk_4 = 0;
    if (parent->state == 1 && parent->subState <= 19 && (u8)(*(u8 *)&entity->unk_504 - 12) <= 1)
    {
        s32 vx;
        contact.reaction = 3;
        /* Original call occurs before the contact coordinates are assigned
         * below. Preserve this order; revisit when making a portable runtime. */
        Player_ApplyContactDamage(&contact);
        vx = -0x40000;
        if (gEwramData->playerState.player->posX.whole > entity->posX.whole)
            vx = -vx;
        Player_SetKnockbackVelocity(vx, -0x24000);
    }
    else
        contact.reaction = 0;
    contact.sourceX = entity->posX.whole;
    contact.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contact);
}
