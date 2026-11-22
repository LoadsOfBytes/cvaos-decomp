/* Chaos second form: summoned part update (537C). */
#include "code_08009A0.h"
#include "chaos_entity.h"
#include "syscalls.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

struct Chaos2Data {
    s32 graphics;
    s32 palette;
    s32 palette2;
    s32 timer;
    s32 count;
    s32 summoned;
};

extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern void EntityMove(struct EwramData_EntityData *);
extern s32 Entity_GetVisibilityPattern(s32);
extern void Chaos2_SpawnSpark(struct EwramData_EntityData *);

#define TO_PIXELS(d) ((d) < 0 ? -(-(d) >> 16) : (d) >> 16)
#define LIFETIME(e) ((e)->unk_4F8.whole)

/* Summoned part: home in on the target (enemyTarget), become harmless once
 * slow, break after 1500 frames or when the core is gone; state 9 bursts
 * into sparks, state 10 blinks out. */
void Chaos2SummonedPart_Update(struct EwramData_EntityData *part)
{
    u8 collision[16];
    struct PlayerContactDamage fx;
    struct EwramData_EntityData *core = CHAOS_LINK(part);
    struct EwramData **ewram = &gEwramData;
    struct Chaos2Data *data = (struct Chaos2Data *)CHAOS_DATA(core);

    data->summoned = 1;
    if (part->state == 0)
    {
        Hit_CheckWithPlayer(collision, part);
        if (core != NULL && core->updateFunc != NULL
            && core->unk_518.part8.enemyId == part->unk_518.part8.enemyId
            && core->state != 3)
        {
            struct EwramData_EntityData *target = (*ewram)->playerState.enemyTarget;
            s32 dx = target->posX.whole - part->posX.whole;
            s32 ty = target->posY.whole;
            s32 py = part->posY.whole + 0x100000;
            s32 dy = ty - py;
            u16 angle = ArcTan2(TO_PIXELS(dx), TO_PIXELS(dy));

            dx = Sin(angle + 0x4000);
            dy = Sin(angle);
            dx = FixedMultiply(dx, 0x3000);
            dy = FixedMultiply(dy, 0x3000);
            part->posX.whole += dx;
            part->posY.whole += dy;
            EntityDampVelocityX(part, 0x18);
            EntityDampVelocityY(part, 0x18);
            EntityMove(part);
            if (part->stage == 0
                && (part->velX.whole < 0 ? -part->velX.whole : part->velX.whole) <= 0x1FF)
            {
                part->stage++;
                part->hit.flags &= ~4;
                part->hit.flags &= ~2;
            }
            if (RandomNumberGenerator() % 16)
                Anim_Update(part);
            if (LIFETIME(part)++ > 0x5DB)
            {
                part->state = 10;
                part->subState = 0;
                part->unk_518.part16.hp = 0;
                part->palette = part->unk_510.part8.basePalette;
                part->timer = 40;
                part->unk_510.part8.hitFlashTimer = 0;
            }
            return;
        }
        part->state = 10;
        part->subState = 0;
        part->unk_518.part16.hp = 0;
        part->palette = part->unk_510.part8.basePalette;
        part->timer = 40;
        part->unk_510.part8.hitFlashTimer = 0;
    }
    else if (part->state == 9)
    {
        if ((part->timer & 3) == 0)
            Chaos2_SpawnSpark(part);
        if ((s8)--part->timer <= 0)
        {
            part->deleteRequested = 1;
            fx.attack = part->attack;
            fx.attributes = 0;
            fx.unk_4 = 0;
            fx.reaction = 0;
            fx.sourceX = part->posX.whole;
            fx.sourceY = part->posY.whole;
            Player_ApplyContactDamage(&fx);
            if (core != NULL && core->updateFunc != NULL
                && core->unk_518.part8.enemyId == part->unk_518.part8.enemyId)
                data->summoned = 0;
        }
    }
    else if (part->state == 10)
    {
        part->hidden = Entity_GetVisibilityPattern(part->timer);
        if ((s8)--part->timer <= 0)
        {
            part->deleteRequested = 1;
            if (core != NULL && core->updateFunc != NULL
                && core->unk_518.part8.enemyId == part->unk_518.part8.enemyId)
                data->summoned = 0;
        }
        return;
    }
    Anim_Update(part);
}
