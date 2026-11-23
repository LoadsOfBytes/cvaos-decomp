/* Chaos second form: summoned part damage (55F0). */
#include "code_08009A0.h"
#include "syscalls.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u16 Player_CalculateAttackPower(u16);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s16);

#define TO_PIXELS(d) ((d) < 0 ? -(-(d) >> 16) : (d) >> 16)

/* Summoned part hit: bounce away from the attacker (speed 2 px), lose HP;
 * at 0 break (state 10). */
void Chaos2SummonedPart_OnHit(struct EwramData_EntityData *part, struct EwramData_EntityData *attacker, u16 *hit)
{
    u16 damage = Hit_ResolveDamage(part, (s32)attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));
    s32 dx, dy;
    u16 angle;
    s32 zero;

    dx = part->posX.whole - attacker->posX.whole;
    dy = part->posY.whole - attacker->posY.whole;
    angle = ArcTan2(TO_PIXELS(dx), TO_PIXELS(dy));
    dx = Sin(angle + 0x4000);
    dy = Sin(angle);
    part->velX.whole = FixedMultiply(dx, 0x20000);
    part->velY.whole = FixedMultiply(dy, 0x20000);
    {
        s32 hp = part->hp;
        s32 d = (s16)damage;

        hp -= d;
        zero = 0;
        part->hp = hp;
        if ((s16)hp > 0)
            return;
    }
    {
        part->palette = part->basePalette;
        part->timer = 40;
        part->hitFlashTimer = zero;
        part->state = 10;
        part->subState = zero;
    }
}
