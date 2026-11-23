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
        target = entity->ext.layerObject.shuttleEnd;
        x = entity->ext.layerObject.x;
        dist = target - x;
        if (dist < 0)
            dist = -dist;
        vel = entity->velX.whole;
        entity->ext.layerObject.x = x + vel;
        if (!entity->ext.layerObject.shuttleDir)
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
        entity->ext.layerObject.shuttleDir ^= 1;
        break;
    case 2:
        target = entity->ext.layerObject.shuttleStart;
        x = entity->ext.layerObject.x;
        dist = target - x;
        if (dist < 0)
            dist = -dist;
        vel2 = entity->velX.whole;
        entity->ext.layerObject.x = x + vel2;
        if (!entity->ext.layerObject.shuttleDir)
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
        entity->ext.layerObject.shuttleDir ^= 1;
        break;
    }
    entity->posX.whole = entity->ext.layerObject.x - camera->xPos.whole;
    entity->posY.whole = entity->ext.layerObject.y - camera->yPos.whole;
    entity->ext.layerObject.drawX = entity->ext.layerObject.x - camera->xPos.whole;
    entity->ext.layerObject.drawY = entity->ext.layerObject.y - camera->yPos.whole;
    if (gEwramData->playerState.standingPlatform == entity)
        Player_SetXFraction((s16)entity->posX.part16.fraction);
}
