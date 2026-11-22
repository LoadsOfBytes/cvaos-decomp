/* Run Killer Fish's rising, floating and fading death sequence. */
#include "code_08009A0.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
extern s32 Entity_MoveAndCollide(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s32 Collision_FindWaterSurfaceOffset(s32, s32);
extern s32 Entity_GetVisibilityPattern(s32);
extern void KillerFish_SpawnRisingSprite(struct EwramData_EntityData *, s32, s32, s32);
/**
 * @brief Invert the sprite, rise to the surface, oscillate and emit detached sprites; apply the visibility table before deletion.
 */
void KillerFish_UpdateDeath(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        entity->flipY = 1;
        entity->velX.whole = 1;
        entity->accelX.whole = 0;
        entity->velY.whole = 0;
        entity->accelY.whole = -0x320;
        entity->subState++;
        break;
    case 1:
        {
            s32 flags = Entity_MoveAndCollide(entity, 0, -0x70000, 1, 0x40000);
            if (entity->velY.whole < -0x4000) entity->velY.whole = -0x4000;
            if ((flags & 0x88) != 0x80)
            {
                entity->posY.part16.integer += Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, entity->posY.part16.integer - 7);
                entity->velX.whole = 0;
                entity->accelX.whole = 0;
                entity->velY.whole = 0;
                entity->accelY.whole = 0x200;
                entity->subState++;
                entity->timer = 0;
                PlaySong(0x6C);
            }
        }
        break;
    case 2:
        {
            s32 x, y;
            EntityAccelerateAndMove(entity, 0x20000);
            if (entity->velY.whole > 0x2000) entity->accelY.whole = -0x200;
            else if (entity->velY.whole < -0x2000) entity->accelY.whole = 0x200;
            x = entity->posX.whole + (RandomNumberGenerator() & 0x1FFFFF) - 0x100000;
            y = entity->posY.whole + (RandomNumberGenerator() & 0xFFFFF) - 0x80000;
            KillerFish_SpawnRisingSprite(entity, x, y, 15);
            if (entity->timer++ > 29)
                entity->hidden = Entity_GetVisibilityPattern(70 - entity->timer);
            if (entity->timer > 69) entity->deleteRequested = 1;
        }
        break;
    }
}

