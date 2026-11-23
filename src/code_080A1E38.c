/* Chaos core creation. */
#include "code_08009A0.h"
#include "chaos_entity.h"
#include "code_08013960.h"
#include "code_08033CAC.h"
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

extern s32 Chaos_StartSecondForm(struct EwramData_EntityData *);
extern void *Heap_Alloc(u16);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern u8 Chaos_LoadGraphics(struct EwramData_EntityData *);
extern struct EwramData_EntityData *Chaos_CreateBackground(struct EwramData_EntityData *);
extern struct EwramData_EntityData *Chaos_CreateOuterRing(struct EwramData_EntityData *, u16, u16, u16);
extern struct EwramData_EntityData *Chaos_CreateInnerRing(struct EwramData_EntityData *, u16, u16, u16);
extern struct EwramData_EntityData *Chaos_CreateOrb(struct EwramData_EntityData *, u16, s32, s32);

/* Create the Chaos core with its background, two rings and three orbs
 * (unk_516 set: the other form Chaos_StartSecondForm); after the fight only the
 * frozen state 4. */
void EnemyChaosCreate(struct EwramData_EntityData *core)
{
    struct ChaosData *data;

    if (core->unk_514.part16.spawnParam1 != 0)
    {
        if (Chaos_StartSecondForm(core) == 0)
            return;
        goto delete;
    }
    if (core->unk_514.part16.spawnParam0 != 0 && gEwramData->gameState.bossRushMode == 0
        && (gEwramData->gameState.bossEventFlags & 0x80))
        goto delete;
    data = Heap_Alloc(10);
    CHAOS_DATA(core) = (s32)data;
    if (data == NULL) goto delete;
    if (!Chaos_LoadGraphics(core)) goto delete;
    {
        if (gEwramData->gameState.bestiaryPreview)
        {
            core->state = 4;
            return;
        }
        if (Chaos_CreateBackground(core) == NULL) goto delete;
        if (Chaos_CreateInnerRing(core, 0x4000, 0x8000, 2) == NULL) goto delete;
        if (Chaos_CreateOuterRing(core, 0, 0x4000, 3) == NULL) goto delete;
        if ((data->orbs[0] = Chaos_CreateOrb(core, 0x4000, 0, 0)) == NULL) goto delete;
        if ((data->orbs[1] = Chaos_CreateOrb(core, 0x8500, 0, 1)) == NULL) goto delete;
        if ((data->orbs[2] = Chaos_CreateOrb(core, 0xFB00, 0, 2)) == NULL)
        {
        delete:
            EntityDelete(core);
            return;
        }
        Anim_StartIndexed(core, 0, 0, 1);
        Anim_Update(core);
        core->hidden = 1;
        Enemy_ResetFromTemplate(core);
        core->hit.flags |= 4;
        core->hit.flags |= 2;
        core->state = 0;
        Player_SetSelectedTarget(core);
        gEwramData->gameState.stateFlags |= 2;
        gEwramData->gameState.stateFlags |= 1;
        gEwramData->gameState.stateFlags |= 0x200000;
        gEwramData->gameState.stateFlags |= 0x800000;
        gEwramData->startPauseEnabled = 0;
        core->timer = 0;
        CHAOS_UNUSED_RANDOM(core) = RandomNumberGenerator() % 64;
        CHAOS_ATTACK_TIMER_0(core) = (RandomNumberGenerator() & 0x7F) + 60;
        CHAOS_ATTACK_TIMER_1(core) = (RandomNumberGenerator() & 0x7F) + 120;
        CHAOS_ATTACK_TIMER_2(core) = (RandomNumberGenerator() & 0x7F) + 60;
        Music_SetCurrent(0x1F);
        data->bgScale = 0xC0;
        data->bgRadius = 0x40000;
    }
}
