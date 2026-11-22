/* Homing projectile, including impact and its particle trail. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

/*
 * Homing orb (Effect_CreateHomingOrb / Effect_UpdateHomingOrb): the entity it
 * flies to and the y offset of the aim point, its flight angle, the turn rate
 * per frame (grows to 0x4000) and the 16.16 speed (grows to 0x80000).
 * Token-identical expansion.
 */
#define HOMING_ORB_TARGET(e) e->unk_4F8.unk_4F8_entity
#define HOMING_ORB_TARGET_DY(e) e->unk_508.whole
#define HOMING_ORB_ANGLE(e) e->unk_4FC.whole
#define HOMING_ORB_TURN(e) e->unk_500.whole
#define HOMING_ORB_SPEED(e) e->unk_504.whole
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
    struct EwramData_EntityData *target = HOMING_ORB_TARGET(entity);
    if (entity->state <= 1)
    {
        HOMING_ORB_ANGLE(entity) = Entity_StepAngleTowardPoint(entity, target->posX.whole, target->posY.whole + HOMING_ORB_TARGET_DY(entity),
                                               HOMING_ORB_ANGLE(entity), HOMING_ORB_TURN(entity));
        HOMING_ORB_TURN(entity) += 42;
        if (HOMING_ORB_TURN(entity) > 0x4000) HOMING_ORB_TURN(entity) = 0x4000;
        HOMING_ORB_SPEED(entity) += 0x1800;
        if (HOMING_ORB_SPEED(entity) > 0x80000) HOMING_ORB_SPEED(entity) = 0x80000;
        entity->velX.whole = FixedMulFast(Sin(HOMING_ORB_ANGLE(entity) + 0x4000), HOMING_ORB_SPEED(entity));
        entity->velY.whole = FixedMulFast(Sin(HOMING_ORB_ANGLE(entity)), HOMING_ORB_SPEED(entity));
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
    if (Entity_IsInRect(HOMING_ORB_TARGET(entity), (s16)(entity->posX.part16.integer - 10), (s16)(entity->posY.part16.integer - 10), 20, 20))
    {
        Effect_SpawnAttachedAnimation(HOMING_ORB_TARGET(entity), 12, 6, 0, HOMING_ORB_TARGET_DY(entity));
        PlaySong(0xBC);
        entity->deleteRequested = 1;
    }
}
