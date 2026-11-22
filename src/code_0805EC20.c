/* Homing projectile, including impact and its particle trail. */
#include "code_08009A0.h"
#include "event_scene03.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern s32 Entity_StepAngleTowardPoint(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s32 RandomSigned(s32);
extern struct EwramData_EntityData *Effect_CreateFromTable(s32, s32, s32, s32, s32, u8);
static inline s32 FixedMulFast(s32 a, s32 b) { return (a >> 8) * (b >> 8); }
/**
 * @brief Steer the soul orb towards the player with growing turn rate and speed
 * and leave a particle trail.
 */
void EventScene03SoulOrb_Update(struct EwramData_EntityData *entity)
{
    u32 collision[4];
    if (entity->state <= 1)
    {
        struct EwramData_EntityData *target = gEwramData->playerState.player;
        EVENT03_ORB_HEADING(entity) = Entity_StepAngleTowardPoint(entity, target->posX.whole, target->posY.whole - 0x100000,
                                               EVENT03_ORB_HEADING(entity), EVENT03_ORB_TURN_RATE(entity));
        EVENT03_ORB_TURN_RATE(entity) += 42;
        if (EVENT03_ORB_TURN_RATE(entity) > 0x4000) EVENT03_ORB_TURN_RATE(entity) = 0x4000;
        EVENT03_ORB_SPEED(entity) += 0x1800;
        if (EVENT03_ORB_SPEED(entity) > 0x80000) EVENT03_ORB_SPEED(entity) = 0x80000;
        entity->velX.whole = FixedMulFast(Sin(EVENT03_ORB_HEADING(entity) + 0x4000), EVENT03_ORB_SPEED(entity));
        entity->velY.whole = FixedMulFast(Sin(EVENT03_ORB_HEADING(entity)), EVENT03_ORB_SPEED(entity));
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
    }
    Hit_CheckWithPlayer(collision, entity);
    if ((entity->timer & 3) == 0)
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
