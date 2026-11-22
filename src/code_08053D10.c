/* Object 19: move along a timed path of steps, then despawn. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

struct Object19Step
{
    s32 time;
    s32 velX;
    s32 velY;
};

extern const struct Object19Step sObject19Steps[4];
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);

static inline s32 Object19NextStep(struct EwramData_EntityData *entity, s32 step)
{
    const struct Object19Step *next;
    s32 time;

    entity->state = step;
    next = &sObject19Steps[step];
    entity->velX.whole = next->velX;
    entity->velY.whole = next->velY;
    if (entity->unk_514.part16.spawnParam1 != 0)
        entity->velX.whole = -entity->velX.whole;
    time = sObject19Steps[step].time;
    if (time == 0)
        time = entity->unk_514.part16.spawnParam0 << 16;
    return time;
}

#define OBJ19_STEP(entity, step)                                           \
    {                                                                      \
        s32 time_;                                                         \
        entity->posX.whole += entity->velX.whole;                          \
        entity->posY.whole += entity->velY.whole;                          \
        if ((s32)entity->unk_4FC.whole > 0)                           \
            { time_ = entity->unk_4FC.whole - 0x10000; asm(""); } /* FAKEMATCH: empty asm keeps jump2 from cross-jumping the copies */ \
        else                                                               \
            time_ = Object19NextStep(entity, step);                        \
        entity->unk_4FC.whole = time_;                                \
    }

void Object19Update(struct EwramData_EntityData *entity)
{
    u8 hit[0x10];

    if (Entity_IsInRect(entity, -0x10, 0, 0x110, 0xB0) == 0)
    {
        entity->hidden = 1;
        return;
    }
    entity->hidden = 0;
    entity->unk_4F8.part16.unk_4F8 += entity->unk_4F8.part16.unk_4FA;
    switch (entity->state)
    {
    case 0:
        OBJ19_STEP(entity, 1);
        break;
    case 1:
        OBJ19_STEP(entity, 2);
        break;
    case 2:
        OBJ19_STEP(entity, 3);
        break;
    case 3:
        OBJ19_STEP(entity, 0);
        break;
    }
    Hit_CheckWithPlayer(hit, entity);
    ObjAffine_Set((s16)entity->unk_4F8.part16.unk_4F8, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
}
