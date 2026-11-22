#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
#include "entity_hit.h"

/*
 * Soul pickup (flies to the player like the homing orb): flight angle, turn
 * rate per frame (starts at the stationary argument, grows to 0x4000) and
 * 16.16 speed (grows to 0x80000). Token-identical expansion.
 */
#define SOUL_PICKUP_ANGLE(e) e->unk_4FC.whole
#define SOUL_PICKUP_TURN(e) e->unk_500.whole
#define SOUL_PICKUP_SPEED(e) e->unk_504.whole
extern s32 Entity_StepAngleTowardPoint(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s32 RandomSigned(s32);
extern struct EwramData_EntityData *Effect_CreateFromTable(s32, s32, s32, s32, s32, u8);
static inline s32 FixedMulFast(s32 a, s32 b) { return (a >> 8) * (b >> 8); }

/**
 * @brief Home toward the point sixteen pixels above the player in states zero/one, check collection, emit a particle trail and expire under the existing timer states.
 */
void SoulPickup_Update(struct EwramData_EntityData *entity)
{
    struct HitResult res;

    if (entity->state <= 1)
    {
        struct EwramData_EntityData *player = gEwramData->playerState.player;
        SOUL_PICKUP_ANGLE(entity) = Entity_StepAngleTowardPoint(entity, player->posX.whole, player->posY.whole - 0x100000,
                                               SOUL_PICKUP_ANGLE(entity), SOUL_PICKUP_TURN(entity));
        SOUL_PICKUP_TURN(entity) += 42;
        if (SOUL_PICKUP_TURN(entity) > 0x4000) SOUL_PICKUP_TURN(entity) = 0x4000;
        SOUL_PICKUP_SPEED(entity) += 0x1800;
        if (SOUL_PICKUP_SPEED(entity) > 0x80000) SOUL_PICKUP_SPEED(entity) = 0x80000;
        entity->velX.whole = FixedMulFast(Sin(SOUL_PICKUP_ANGLE(entity) + 0x4000), SOUL_PICKUP_SPEED(entity));
        entity->velY.whole = FixedMulFast(Sin(SOUL_PICKUP_ANGLE(entity)), SOUL_PICKUP_SPEED(entity));
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
    }
    Hit_CheckWithPlayer(&res, (u8 *)entity);
    if ((entity->timer & 1) == 0)
    {
        s32 x = entity->posX.whole + RandomSigned(0x200000);
        s32 y = entity->posY.whole + RandomSigned(0x200000);
        struct EwramData_EntityData *particle = Effect_CreateFromTable(x, y, entity->velX.whole / 64, entity->velY.whole / 64, 8, entity->palette);
        if (particle != NULL)
        {
            particle->velX.whole = -entity->velX.whole / 4;
            particle->velY.whole = -entity->velY.whole / 4;
            particle->spriteLayer = entity->spriteLayer;
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
