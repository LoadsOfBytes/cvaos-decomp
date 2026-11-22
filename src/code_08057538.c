/* Object2A_UpdateMovement: horizontal moving platform; matched. */
#include "code_08009A0.h"
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
        target = entity->unk_500.whole;
        x = entity->unk_4F4.whole;
        dist = target - x;
        if (dist < 0)
            dist = -dist;
        vel = entity->velX.whole;
        entity->unk_4F4.whole = x + vel;
        if (!entity->unk_504.part8.unk_504)
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
        entity->unk_504.part8.unk_504 ^= 1;
        break;
    case 2:
        target = entity->unk_4FC.whole;
        x = entity->unk_4F4.whole;
        dist = target - x;
        if (dist < 0)
            dist = -dist;
        vel2 = entity->velX.whole;
        entity->unk_4F4.whole = x + vel2;
        if (!entity->unk_504.part8.unk_504)
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
        entity->unk_504.part8.unk_504 ^= 1;
        break;
    }
    entity->posX.whole = entity->unk_4F4.whole - camera->xPos.whole;
    entity->posY.whole = entity->unk_4F8.whole - camera->yPos.whole;
    entity->unk_508.whole = entity->unk_4F4.whole - camera->xPos.whole;
    entity->unk_50C.whole = entity->unk_4F8.whole - camera->yPos.whole;
    if (gEwramData->playerState.standingPlatform == entity)
        Player_SetXFraction((s16)entity->posX.part16.fraction);
}
