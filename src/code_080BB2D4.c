/* KillerMantle_UpdateHover (state 1 of the enemy created at 0x080BB70C); only matches without -f2003-patch
 * (NO_2003_PATCH_SRCS in ../Makefile). (Target_IsNearEntity test through an inline with two
 * returns, state 1 constant in a block variable). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Entity_Walk(struct EwramData_EntityData *entity, s32 speed, void *profile, s32 flags);
extern s32 Entity_GetTargetDirection(struct EwramData_EntityData *entity);
extern s32 Player_GetFocusedActionFlags(void);
extern s32 Target_GetPixelX(struct EwramData_EntityData *entity);
extern s32 Target_GetFlipX(struct EwramData_EntityData *entity);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *entity, u16 width, u16 height);
extern u8 sKillerMantleWalkProfile[];

#define ABS(x) ((x) < 0 ? -(x) : (x))

/**
 * @brief BB2D4 | Hover (state 1): sprite 1, then sprite 2 with a random
 * speed factor unk_4F8 (0x100..0x1FF). Sways back and forth (sine of the
 * timer unk_4FC, direction stage); when an input of Player_GetFocusedActionFlags (mask
 * 0x360) comes while the target is within 80 pixels in front, it dodges
 * (timer 0x30, timer). After 48 frames it attacks (state 2) when the
 * target is within 0xA0 x 0x46, otherwise turns and picks a new speed
 * (faster towards the target and after a dodge).
 */
static inline u32 InRange(struct EwramData_EntityData *e) { if (Target_IsNearEntity(e, 0xA0, 0x46)) return 1; return 0; }

void KillerMantle_UpdateHover(struct EwramData_EntityData *entity)
{
    s32 speed;
    u8 back;

    switch (entity->subState)
    {
        case 0:
            Anim_StartIndexed(entity, 1, 0, 0);
            {
                u8 one = 1;
                entity->velY.whole = 0;
                entity->accelY.whole = 0;
                entity->stage = 0;
                entity->ext.killerMantle.swayFrame = 0;
                entity->subState = one;
            }
        case 1:
            if (!(entity->animFlags & 4))
                break;
            Anim_StartIndexed(entity, 2, 0, 0);
            entity->subState = 2;
            entity->ext.killerMantle.speed = RandomNumberGenerator() % 256 + 0x100;
            break;
        case 2:
            entity->ext.killerMantle.swayFrame++;
            entity->accelY.whole = Sin(entity->ext.killerMantle.swayFrame << 11) / 48;
            speed = entity->ext.killerMantle.speed * (Sin(entity->ext.killerMantle.swayFrame * 3 << 8) / 256);
            if (entity->stage)
                speed = -speed;
            entity->timer = 0;
            if ((Player_GetFocusedActionFlags() & 0x360)
                && ABS((s16)Target_GetPixelX(entity) - (s16)entity->posX.part16.integer) <= 79)
            {
                back = entity->stage;
                if (entity->flipX)
                    back ^= 1;
                if (((!back && (s8)Entity_GetTargetDirection(entity) == -1) || (back && (s8)Entity_GetTargetDirection(entity) == 1))
                    && ((!Target_GetFlipX(entity) && (s8)Entity_GetTargetDirection(entity) == -1)
                        || (Target_GetFlipX(entity) && (s8)Entity_GetTargetDirection(entity) == 1)))
                {
                    entity->ext.killerMantle.swayFrame = 0x30;
                    entity->timer = 1;
                }
            }
            Entity_Walk(entity, speed, sKillerMantleWalkProfile, 0);
            if (entity->ext.killerMantle.swayFrame <= 47)
                break;
            if (InRange(entity) && entity->timer == 0)
            {
                entity->state = 2;
                entity->stage = 0;
                entity->subState = 0;
                break;
            }
            entity->stage ^= 1;
            if (entity->stage == 0)
                Anim_StartIndexed(entity, 2, 0, 0);
            else
                Anim_StartIndexed(entity, 7, 0, 0);
            back = entity->stage;
            if (entity->flipX)
                back ^= 1;
            entity->ext.killerMantle.speed = RandomNumberGenerator() % 256 + 0x100;
            if ((!back && (s8)Entity_GetTargetDirection(entity) == -1) || (back && (s8)Entity_GetTargetDirection(entity) == 1))
                entity->ext.killerMantle.speed += 0x100 + RandomNumberGenerator() % 64;
            if (entity->timer)
                entity->ext.killerMantle.speed += 0x100;
            entity->ext.killerMantle.swayFrame = 0;
            break;
    }
}
