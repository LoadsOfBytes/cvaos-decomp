/* Emit four accelerating fragments at four-frame intervals. */
#include "code_08009A0.h"
#include "event_scene30.h"
#include "structs/ewram.h"
#include <string.h>
extern u8 sEventScene30PieceEmitterAltFrames[];
extern struct EwramData_EntityData *EventScene30_CreateEffectSprite(s32);
extern void EventScene30Effect_UpdateSpinningKeepSlot(struct EwramData_EntityData *);
/**
 * @brief Unreferenced variant of EventScene30PieceEmitter_Update using
 * EventScene30Effect_UpdateSpinningKeepSlot.
 */
void EventScene30PieceEmitterAlt_Update(struct EwramData_EntityData *entity)
{
    u8 frames[5];
    u32 variant;
    memcpy(frames,sEventScene30PieceEmitterAltFrames,5);
    variant=RandomNumberGenerator()%3;
    switch(entity->state)
    {
    case 0:
        if(entity->timer==0)entity->state=1;
        else entity->timer--;
        break;
    case 1:
    {
        struct EwramData_EntityData *effect=EventScene30_CreateEffectSprite(1);
        if(effect)
        {
            s32 zero,xSpeed,ySpeed,step;
            effect->updateFunc=(u32 *)EventScene30Effect_UpdateSpinningKeepSlot;
            effect->posX.part16.integer=(u16)(entity->posX.part16.integer+8)-(RandomNumberGenerator()%16);
            zero=0;
            effect->posY.part16.integer=(u16)entity->posY.part16.integer+8-(RandomNumberGenerator()%16);
            xSpeed=0x18000; ySpeed=-0x4000;
            effect->accelX.whole=0x800;
            effect->accelY.whole=-0x100;
            step=variant+1;
            effect->velX.whole=step*xSpeed;
            effect->velY.whole=step*ySpeed;
            effect->spriteLayer=17;
            effect->animFrame=frames[entity->stage];
            effect->ext.event30Effect.spin=0x200;
            effect->ext.event30Effect.scale=step*0x9000;
            entity->stage++;
            entity->state=zero;
            entity->timer=16;
        }
        break;
    }
    }
    if(entity->stage==5)entity->deleteRequested=1;
}
