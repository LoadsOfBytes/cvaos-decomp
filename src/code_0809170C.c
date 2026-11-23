/* DevilPart_OnHit, decompiled from the original assembly. The damage is kept
 * in an s16 variable (copied from r0 right after the call and sign-extended
 * in place at use); the new HP is tested on its own s32 value. */
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u16 Player_CalculateAttackPower(u16);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s16, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);

/* Apply a hit to the Devil's owner at the struck part's position. */
void DevilPart_OnHit(struct EwramData_EntityData *input, s32 source, u16 *collision)
{
    struct EwramData_EntityData *entity = input;
    s32 attacker = source;
    u16 *hit = collision;
    register struct EwramData_EntityData *owner asm("r5") = entity->ext.jointPart.owner; /* FAKEMATCH: pinned register */
    s32 oldX = owner->posX.whole;
    s32 oldY = owner->posY.whole;
    s16 damage;
    s16 power;
    owner->posX.whole = entity->posX.whole;
    owner->posY.whole = entity->posY.whole;
    power = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(entity->ext.jointPart.owner, attacker, hit, power);
    owner->posX.whole = oldX;
    owner->posY.whole = oldY;
    Entity_ShowDamage(entity, damage, 0);
    owner = entity->ext.jointPart.owner;
    owner->hitFlashTimer = 8;
    {
    s32 hp = owner->hp - damage;
    owner->hp = hp;
    if ((s16)hp <= 0)
    {
        Enemy_DropRewards(owner);
        PlaySong(0x72);
        owner->palette = owner->basePalette;
        owner->timer = 40;
        owner->state = 2;
        owner->subState = 0;
    }
}
}
