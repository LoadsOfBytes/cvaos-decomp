/* Blue Crow flight and attack states, reconstructed from the original ROM. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Target_GetX(struct EwramData_EntityData *entity);
extern s32 Target_GetY(struct EwramData_EntityData *entity);
extern void Entity_FaceTarget(struct EwramData_EntityData *entity);
extern u8 Entity_IsTargetWithinRange(struct EwramData_EntityData *entity, s32 range);
extern s32 Entity_AimVelocityAtPoint(struct EwramData_EntityData *entity, s32 x, s32 y, s32 speed);
extern s32 Entity_GetTargetOffsetX(struct EwramData_EntityData *entity);
extern s32 Entity_GetTargetOffsetY(struct EwramData_EntityData *entity);

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define ANIM(e,n,loop) do { if ((e)->animScriptIndex != (n)) Anim_StartIndexed((e),(n),0,(loop)); } while (0)

/**
 * @brief Alive movement: activate near the target, manage flight substates
 * and aim at a point above and beside the player. Preserve the existing
 * substate reachability, division and timer-underflow behavior.
 */
void BlueCrow_UpdateMovement(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 targetX = Target_GetX(entity) + bg->xPos.whole;
    s32 targetY = Target_GetY(entity) + bg->yPos.whole - 0x2A0000;
    s32 zero = 0;

    switch (entity->subState)
    {
        case 0:
            entity->ext.blueCrow.unusedDiveCount = zero;
            Entity_FaceTarget(entity);
            ANIM(entity, 0, 1);
            entity->subState = 1;
            entity->stage = zero;
            entity->timer = zero;
        case 1:
            if (Entity_IsTargetWithinRange(entity, 0x3C0000))
            {
                entity->subState = 3;
                entity->stage = 0;
                PlaySong(0x8B);
            }
            break;
        case 2:
            switch (entity->stage)
            {
                case 0:
                    ANIM(entity, 1, 1);
                    if (!entity->flipX)
                    {
                        entity->velX.whole = -ABS(entity->velX.whole);
                        entity->accelX.whole = -ABS(entity->accelX.whole);
                    }
                    {
                        s32 duration = entity->velY.whole / entity->accelY.whole;
                        entity->timer = ABS(duration) * 2;
                    }
                    entity->stage++;
                case 1:
                    Entity_FaceTarget(entity);
                    if (Entity_AimVelocityAtPoint(entity, targetX, targetY, 0x18000))
                    {
                        entity->subState = 3;
                        entity->stage = 0;
                    }
                    break;
            }
            break;
        case 3:
            Entity_FaceTarget(entity);
            switch (entity->stage)
            {
                case 0:
                    entity->ext.blueCrow.unusedDiveCount++;
                    ANIM(entity, 2, 1);
                    entity->accelX.whole = zero;
                    entity->velX.whole = zero;
                    entity->accelY.whole = zero;
                    entity->velY.whole = zero;
                    entity->timer = 0x20;
                    entity->stage++;
                case 1:
                    if (--entity->timer == 0xFF)
                        entity->stage++;
                    break;
                case 2:
                    if (ABS(Entity_GetTargetOffsetX(entity)) > 0x480000 && ABS(Entity_GetTargetOffsetY(entity)) > 0x100000)
                        entity->stage++;
                    break;
                case 3:
                {
                    u8 *animation = &entity->animScriptIndex;
                    if (*animation != 1)
                        Anim_StartIndexed(entity, 1, 0, 1);
                    zero = -0x200000;
                    if (gEwramData->playerState.player->flipX)
                        zero = -zero;
                    if (Entity_AimVelocityAtPoint(entity, targetX + zero, targetY, 0x1A000))
                    {
                        if (*animation != 2)
                            Anim_StartIndexed(entity, 2, 0, 1);
                        entity->accelX.whole = 0;
                        entity->velX.whole = 0;
                        entity->accelY.whole = 0;
                        entity->velY.whole = 0;
                        entity->stage--;
                    }
                    break;
                }
            }
            break;
    }
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
}
