/* Homing projectile, including impact and its particle trail. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
extern s32 Entity_StepAngleTowardPoint(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s32 RandomSigned(s32);
extern struct EwramData_EntityData *Effect_CreateFromTable(s32, s32, s32, s32, s32, u8);
static inline s32 FixedMulFast(s32 a, s32 b) { return (a >> 8) * (b >> 8); }
/**
 * @brief Steer the orb towards its target with growing turn rate and speed;
 * delete it with sound 0xBC when it reaches the target.
 */
void EventScene03TargetOrb_Update(struct EwramData_EntityData *entity)
{
    u32 unused[4]; /* The original reserves 16 unused stack bytes. */
    struct EwramData_EntityData *target = entity->unk_508.unk_508_entity;
    if (entity->state <= 1)
    {
        entity->unk_4FC.whole = Entity_StepAngleTowardPoint(entity, target->posX.whole, target->posY.whole - 0x100000,
                                               entity->unk_4FC.whole, entity->unk_500.whole);
        entity->unk_500.whole += 42;
        if (entity->unk_500.whole > 0x4000) entity->unk_500.whole = 0x4000;
        entity->unk_504.whole += 0x1800;
        if (entity->unk_504.whole > 0x80000) entity->unk_504.whole = 0x80000;
        entity->velX.whole = FixedMulFast(Sin(entity->unk_4FC.whole + 0x4000), entity->unk_504.whole);
        entity->velY.whole = FixedMulFast(Sin(entity->unk_4FC.whole), entity->unk_504.whole);
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
    }
    if (Entity_IsInRect(target, (s16)(entity->posX.part16.integer - 16), (s16)(entity->posY.part16.integer - 16), 32, 32))
    {
        entity->deleteRequested = 1;
        PlaySong(0xBC);
        return;
    }
    if ((entity->timer & 1) == 0)
    {
        s32 palette = entity->palette;
        s32 x = entity->posX.whole + RandomSigned(0x200000);
        s32 y = entity->posY.whole + RandomSigned(0x200000);
        struct EwramData_EntityData *particle = Effect_CreateFromTable(x, y, entity->velX.whole / 64, entity->velY.whole / 64, 8, palette);
        if (particle != NULL)
        {
            particle->velX.whole = -entity->velX.whole / 4;
            particle->velY.whole = -entity->velY.whole / 4;
        }
    }
    switch (entity->state)
    {
    case 0:
        if (--entity->timer == 0)
        {
            entity->drawSkip = 1;
            entity->timer = 64;
            entity->state++;
        }
        break;
    case 1:
        if (--entity->timer == 0) entity->deleteRequested = 1;
        break;
    default: entity->timer++; break;
    }
    if (entity->stage != 0) Anim_Update(entity);
}
