/* Chaos core: fight loop (1FEC). */
#include "code_08009A0.h"
#include "chaos_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/* allocation of 10 words in the Chaos core's unk_504 */
struct ChaosData {
    s32 graphics;
    s32 palette;
    s32 palette2;
    s32 bgRotation;
    s32 bgAngle;
    s32 bgScale;
    s32 bgRadius;
    struct EwramData_EntityData *orbs[3];
};

extern void Player_StartScriptedAnimation(s32, s32, s32, s32);
extern void Effect_CreateHomingOrb(struct EwramData_EntityData *, s32, s32, s32, s32);
extern void Chaos_CreateMark(struct EwramData_EntityData *, s32);

#define ACTIVE(e) (CHAOS_ACTIVE_ORBS((e)))  /* orbs currently attacking */
#define BROKEN(e) (CHAOS_BROKEN_ORBS((e)))  /* orbs destroyed */

/* Fight: turn the background, let each living orb attack after a random
 * pause; when all three orbs are broken switch to state 3. */
void Chaos_UpdateFight(struct EwramData_EntityData *core)
{
    struct ChaosData *data = (struct ChaosData *)CHAOS_DATA(core);

    if (core->subState != 0)
        return;
    data->bgRotation += 0x80;
    data->bgAngle += 0x200;
    if (!(BROKEN(core) & 1) && !(ACTIVE(core) & 1))
    {
        if (CHAOS_ATTACK_TIMER_0(core) != 0)
            CHAOS_ATTACK_TIMER_0(core)--;
        else
        {
            CHAOS_ATTACK_TIMER_0(core) = RandomNumberGenerator() % 128 + 60;
            ACTIVE(core) |= 1;
            CHAOS_BUSY(core) = 0;
        }
    }
    if (!(BROKEN(core) & 2) && !(ACTIVE(core) & 2))
    {
        if (CHAOS_ATTACK_TIMER_1(core) != 0)
            CHAOS_ATTACK_TIMER_1(core)--;
        else
        {
            CHAOS_ATTACK_TIMER_1(core) = RandomNumberGenerator() % 128 + 120;
            ACTIVE(core) |= 2;
            CHAOS_BUSY(core) = 0;
        }
    }
    if (!(BROKEN(core) & 4) && !(ACTIVE(core) & 4))
    {
        if (CHAOS_ATTACK_TIMER_2(core) != 0)
            CHAOS_ATTACK_TIMER_2(core)--;
        else
        {
            CHAOS_ATTACK_TIMER_2(core) = RandomNumberGenerator() % 128 + 60;
            ACTIVE(core) |= 4;
            CHAOS_BUSY(core) = 0;
        }
    }
    if ((BROKEN(core) & 7) == 7)
    {
        core->state = 3;
        core->subState = 0;
        core->stage = 0;
    }
}
