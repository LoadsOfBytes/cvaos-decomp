/* Red Crow flight and attack states, reconstructed from the original ROM. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void Entity_FaceTarget(struct EwramData_EntityData *entity);
extern u8 Entity_IsTargetWithinRange(struct EwramData_EntityData *entity, s32 range);
#define ABS(x) ((x) < 0 ? -(x) : (x))
#define ANIM(e,n,loop) do { if ((e)->animScriptIndex != (n)) Anim_StartIndexed((e),(n),0,(loop)); } while (0)

/**
 * @brief Alive movement: activate near the target, alternate timed waits
 * and accelerating flight, optionally play a reset animation when out
 * of range. Preserve facing changes and timer-underflow completion.
 */
void RedCrow_UpdateMovement(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
        {
            s32 zero = 0;
            entity->ext.redCrow.phase = zero;
            Entity_FaceTarget(entity);
            ANIM(entity, 0, 1);
            entity->subState = 1;
            entity->stage = zero;
            entity->timer = zero;
        }
        case 1:
            if (Entity_IsTargetWithinRange(entity, 0x3C0000))
            {
                entity->subState = 3;
                entity->stage = 0;
            }
            break;
        case 2:
            switch (entity->stage)
            {
                case 0:
                    ANIM(entity, 1, 1);
                    entity->velX.whole = 0x8000;
                    entity->velY.whole = 0x20000;
                    entity->accelX.whole = 0x1000;
                    entity->accelY.whole = -0x1000;
                    if (!entity->flipX)
                    {
                        entity->velX.whole = -entity->velX.whole;
                        entity->accelX.whole = -entity->accelX.whole;
                    }
                    {
                        s32 duration = entity->velY.whole / entity->accelY.whole;
                        entity->timer = ABS(duration) * 2;
                    }
                    entity->stage++;
                    PlaySong(0x8B);
                case 1:
                    if (--entity->timer == 0xFF)
                    {
                        entity->subState = 3;
                        entity->stage = 0;
                    }
                    break;
            }
            break;
        case 3:
            switch (entity->stage)
            {
                case 0:
                    entity->ext.redCrow.phase++;
                    ANIM(entity, 2, 1);
                    entity->accelX.whole = 0;
                    entity->velX.whole = 0;
                    entity->accelY.whole = 0;
                    entity->velY.whole = 0;
                    entity->timer = 0x80;
                    entity->stage++;
                case 1:
                    if (entity->timer == 0x40)
                    {
                        if (entity->ext.redCrow.phase & 1)
                            Entity_FaceTarget(entity);
                        else
                            entity->flipX = 1 - entity->flipX;
                    }
                    if (--entity->timer == 0xFF)
                    {
                        entity->subState = 2;
                        entity->stage = 0;
                        if ((entity->ext.redCrow.phase & 1) && entity->ext.redCrow.phase != 1)
                        {
                            s32 inRange = (s8)Entity_IsTargetWithinRange(entity, 0x780000);
                            if (inRange == 0)
                            {
                                entity->subState = 4;
                                entity->stage = inRange;
                                entity->animDone = 0;
                            }
                        }
                    }
                    break;
            }
            break;
        case 4:
            switch (entity->stage)
            {
                case 0:
                    if (entity->animDone)
                        entity->stage = 1;
                    break;
                case 1:
                    ANIM(entity, 4, 0);
                    entity->stage++;
                case 2:
                    if (entity->animDone)
                    {
                        entity->subState = 0;
                        entity->stage = 0;
                    }
                    break;
            }
            break;
    }
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
}
