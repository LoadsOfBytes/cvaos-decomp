/* Chaos second form: part damage (4FA0). */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u16 Player_CalculateAttackPower(u16);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s16);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s32, s32);

/* Part hit: flash (ext.chaos2Part.flash = 30), lose HP; at 0 break (state 10)
 * and lower the core's ext.chaos2.partCount. */
void Chaos2Part_OnHit(struct EwramData_EntityData *part, s32 attacker, u16 *hit)
{
    struct EwramData_EntityData *core = part->ext.chaosChild.link;
    s32 damage = Hit_ResolveDamage(part, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));
    s32 zero;

    part->hitFlashTimer = 8;
    zero = 0;
    part->ext.chaos2Part.flash = 30;
    damage <<= 16;
    damage >>= 16;
    Entity_ShowDamage(part, damage, 0);
    part->hp -= damage;
    Anim_StartIndexed(part, 1, 2, 0);
    if ((s16)part->hp <= 0)
    {
        PlaySong(0x6E);
        part->palette = part->basePalette;
        part->timer = 40;
        part->hitFlashTimer = zero;
        part->state = 10;
        part->subState = zero;
        if (*(s8 *)&core->ext.chaos2.partCount != 0)
            core->ext.chaos2.partCount--;
    }
}
