/* Vertical platform: accelerate between two room-space endpoints,
 * slow near each endpoint and wait for the per-variant delay. */
#include "code_08009A0.h"
#include "structs/ewram.h"
struct PlatformVariant { u8 *graphics; void *palette; void *sprites; u8 animation; u8 paletteIndex; u16 delay; };
extern struct PlatformVariant sObject29Variants[];
#define DIRECTION(e) (e->ext.layerObject.shuttleDir)
void Object29_UpdateMovement(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    register s32 y asm("r1"); /* FAKEMATCH: y in r1 */
    s32 v, k;
    

    entity->ext.layerObject.drawX = entity->ext.layerObject.x - camera->xPos.whole;

    y = entity->ext.layerObject.y;
    entity->ext.layerObject.drawY = y - camera->yPos.whole;
    switch (entity->subState)
    {
    case 0:
    {
        s32 target = entity->ext.layerObject.shuttleEnd;
        register s32 current asm("r2") = y; /* FAKEMATCH: current in r2 */
        s32 distance = target - current;
        
        if (distance < 0) distance = -distance;
        asm("" : "+r"(target)); /* FAKEMATCH: two more references raise target priority */
        if (DIRECTION(entity) == 0)
        {
            if (current >= target)
            {
                entity->subState = 1;
                entity->velY.whole = 0;
                goto wait;
            }
            if (distance > 0xFFFFF)
            {
                if (entity->velY.whole > 0xFFFF)
                    goto move;
                entity->velY.whole += 0x800;
            }
            else { v = (distance / 0x10000) << 12; k = 0x1000; goto add; }
        }
        else
        {
            if (current <= target)
            {
                entity->subState = 1;
                entity->velY.whole = 0;
                goto wait;
            }
            if (distance > 0xFFFFF)
            {
                if (entity->velY.whole <= -0x10000)
                    goto move;
                entity->velY.whole -= 0x800;
            }
            else { { s32 q_ = distance; asm("" : "+r"(q_)); /* FAKEMATCH: keep the test on distance */ if (distance < 0) q_ = distance + 0xFFFF; v = -((q_ >> 16) << 16) / 16; } k = -0x1000; add: entity->velY.whole = v + k; }
        }
        goto move;
    }
    case 1:
        if ((s8)--entity->timer <= 0)
        {
            entity->subState++;
            DIRECTION(entity) ^= 1;
        }
        break;
    case 2:
    {
        s32 target = entity->ext.layerObject.shuttleStart;
        register s32 current asm("r2") = y; /* FAKEMATCH: current in r2 */
        s32 distance = target - current;
        if (distance < 0) distance = -distance;
        if (DIRECTION(entity) == 0)
        {
            if (current >= target)
            {
                entity->subState = 3;
                entity->velY.whole = 0;
                asm(""); /* FAKEMATCH: keep this copy apart from the case 0 block (cross-jump) */
                goto wait;
            }
            if (distance > 0xFFFFF)
            {
                if (entity->velY.whole > 0xFFFF)
                    goto move;
                entity->velY.whole += 0x800;
            }
            else entity->velY.whole = ((distance / 0x10000) << 12) + 0x1000;
        }
        else
        {
            if (current <= target)
            {
                { u8 three = 3; entity->subState = three; entity->velY.whole = 0; }
            wait:
                { register struct PlatformVariant *tbl asm("r1") = sObject29Variants; /* FAKEMATCH: table base in r1 */ entity->timer = tbl[entity->spawnParam1].delay; }
                goto move;
            }
            if (distance > 0xFFFFF)
            {
                if (entity->velY.whole <= -0x10000)
                    goto move;
                entity->velY.whole -= 0x800;
            }
            else entity->velY.whole = -((distance / 0x10000) << 16) / 16 - 0x1000;
        }
    move:
        entity->ext.layerObject.y += entity->velY.whole;
        break;
    }
    case 3:
        if ((s8)--entity->timer <= 0)
        {
            entity->subState = 0;
            DIRECTION(entity) ^= 1;
        }
        break;
    }
    entity->posX.whole = entity->ext.layerObject.x - camera->xPos.whole;
    entity->posY.whole = entity->ext.layerObject.y - camera->yPos.whole;
}
