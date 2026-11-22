/* Object2A_UpdateMovement: horizontal moving platform; matched. */
#include "code_08009A0.h"
#include "layer_object.h"
#include "structs/ewram.h"
extern void Player_SetXFraction(s32 fraction);

struct ObjectVariant { u8 *graphics; void *palette; void *sprites; u8 animation; u8 paletteIndex; u16 wait; };
extern struct ObjectVariant sObject2AVariants[];

void Object2A_UpdateMovement(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    s32 target, x, dist, vel, vel2;

    switch (entity->subState)
    {
    case 0:
        target = SHUTTLE_END(entity);
        x = BG1_OBJECT_X(entity);
        dist = target - x;
        if (dist < 0)
            dist = -dist;
        vel = entity->velX.whole;
        BG1_OBJECT_X(entity) = x + vel;
        if (!SHUTTLE_DIR(entity))
        {
            if (x >= target)
            {
                entity->subState++;
                entity->velX.whole = 0;
                entity->timer = sObject2AVariants[entity->unk_514.part16.spawnParam1].wait;
                break;
            }
            if (dist > 0xFFFFF)
            {
                if (vel > 0xFFFF)
                    break;
                entity->velX.whole = vel + 0x800;
                break;
            }
            entity->velX.whole = (dist / 0x10000) * 0x1000 + 0x1000;
        }
        else
        {
            if (x <= target)
            {
                entity->subState++;
                entity->velX.whole = 0;
                entity->timer = sObject2AVariants[entity->unk_514.part16.spawnParam1].wait;
                break;
            }
            if (dist > 0xFFFFF)
            {
                if (vel <= -0x10000)
                    break;
                entity->velX.whole = vel - 0x800;
                break;
            }
            entity->velX.whole = -((dist / 0x10000) << 16) / 16 - 0x1000;
        }
        break;
    case 1:
        if ((s8)--entity->timer > 0)
            break;
        entity->subState++;
        SHUTTLE_DIR(entity) ^= 1;
        break;
    case 2:
        target = SHUTTLE_START(entity);
        x = BG1_OBJECT_X(entity);
        dist = target - x;
        if (dist < 0)
            dist = -dist;
        vel2 = entity->velX.whole;
        BG1_OBJECT_X(entity) = x + vel2;
        if (!SHUTTLE_DIR(entity))
        {
            if (x >= target)
            {
                entity->subState++;
                entity->velX.whole = 0;
                entity->timer = sObject2AVariants[entity->unk_514.part16.spawnParam1].wait;
                break;
            }
            if (dist > 0xFFFFF)
            {
                if (vel2 > 0xFFFF)
                    break;
                entity->velX.whole = vel2 + 0x800;
                break;
            }
            entity->velX.whole = (dist / 0x10000) * 0x1000 + 0x1000;
        }
        else
        {
            if (x <= target)
            {
                entity->subState++;
                entity->velX.whole = 0;
                entity->timer = sObject2AVariants[entity->unk_514.part16.spawnParam1].wait;
                break;
            }
            if (dist > 0xFFFFF)
            {
                if (vel2 <= -0x10000)
                    break;
                entity->velX.whole = vel2 - 0x800;
                break;
            }
            entity->velX.whole = -((dist / 0x10000) << 16) / 16 - 0x1000;
        }
        break;
    case 3:
        if ((s8)--entity->timer > 0)
            break;
        entity->subState = 0;
        SHUTTLE_DIR(entity) ^= 1;
        break;
    }
    entity->posX.whole = BG1_OBJECT_X(entity) - camera->xPos.whole;
    entity->posY.whole = BG1_OBJECT_Y(entity) - camera->yPos.whole;
    BG1_OBJECT_DRAW_X(entity) = BG1_OBJECT_X(entity) - camera->xPos.whole;
    BG1_OBJECT_DRAW_Y(entity) = BG1_OBJECT_Y(entity) - camera->yPos.whole;
    if (gEwramData->playerState.standingPlatform == entity)
        Player_SetXFraction((s16)entity->posX.part16.fraction);
}
