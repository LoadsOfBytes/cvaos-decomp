/* Homing projectile, including impact and its particle trail. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern void Effect_SpawnAttachedAnimation(struct EwramData_EntityData *, u8, u8, s32, s32);
extern s32 Entity_StepAngleTowardPoint(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s32 RandomSigned(s32);
extern struct EwramData_EntityData *Effect_CreateFromTable(s32, s32, s32, s32, s32, u8);
static inline s32 FixedMulFast(s32 a, s32 b) { return (a >> 8) * (b >> 8); }
/**
 * @brief Home toward the stored target, emit a trail and expire by timer or proximity; on proximity attach an effect and play sound 0xBC.
 */
void Effect_UpdateHomingOrb(struct EwramData_EntityData *entity)
{
    u32 unused[4]; /* The original reserves 16 unused stack bytes. */
    struct EwramData_EntityData *target = entity->ext.homingOrb.target;
    if (entity->state <= 1)
    {
        entity->ext.homingOrb.angle = Entity_StepAngleTowardPoint(entity, target->posX.whole, target->posY.whole + entity->ext.homingOrb.targetDy,
                                               entity->ext.homingOrb.angle, entity->ext.homingOrb.turn);
        entity->ext.homingOrb.turn += 42;
        if (entity->ext.homingOrb.turn > 0x4000) entity->ext.homingOrb.turn = 0x4000;
        entity->ext.homingOrb.speed += 0x1800;
        if (entity->ext.homingOrb.speed > 0x80000) entity->ext.homingOrb.speed = 0x80000;
        entity->velX.whole = FixedMulFast(Sin(entity->ext.homingOrb.angle + 0x4000), entity->ext.homingOrb.speed);
        entity->velY.whole = FixedMulFast(Sin(entity->ext.homingOrb.angle), entity->ext.homingOrb.speed);
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
    }
    if ((entity->timer & 1) == 0)
    {
        s32 x = entity->posX.whole + RandomSigned(0x200000);
        s32 y = entity->posY.whole + RandomSigned(0x200000);
        struct EwramData_EntityData *particle = Effect_CreateFromTable(x, y, entity->velX.whole / 64, entity->velY.whole / 64, 8, entity->palette);
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
    if (Entity_IsInRect(entity->ext.homingOrb.target, (s16)(entity->posX.part16.integer - 10), (s16)(entity->posY.part16.integer - 10), 20, 20))
    {
        Effect_SpawnAttachedAnimation(entity->ext.homingOrb.target, 12, 6, 0, entity->ext.homingOrb.targetDy);
        PlaySong(0xBC);
        entity->deleteRequested = 1;
    }
}
