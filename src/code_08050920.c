/* Object0EPiece_Update (falling object hitting the ground). */
#include "code_08009A0.h"
#include "code_08002454.h"
#include "code/code_080211F0.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern void EntityDampVelocityX(struct EwramData_EntityData *entity, s32 factor);
/* Declared s16 in code_08001194.h; the caller here uses the full register. */
extern s32 Collision_GetGroundOffset(s32 x, s32 y);
extern void Effect_SpawnRandom(s32 x, s32 y, s32 width, s32 height, s32 layer);

/**
 * @brief 50920 | Fall; on hitting solid ground (Collision_GetGroundOffset) spawn a
 * bouncing piece (Effect_SpawnShared) and dust (Effect_SpawnRandom), play song 0x158
 * or 0x159 at random and finish (0x53D bit 3); also finish when far off
 * screen.
 */
void Object0EPiece_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg;
    s32 x;

    EntityDampVelocityX(entity, 0x3C);
    EntityAccelerateAndMove(entity, 0x80000);
    x = (s16)entity->posX.part16.integer;
    bg = &gEwramData->bgInfo[1];
    if (Collision_GetGroundOffset(x + (s16)bg->xPos.part16.integer,
                     (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer))
    {
        struct EwramData_EntityData *effect = Effect_SpawnShared(entity->posX.whole, entity->posY.whole - 0x40000, 4, 6);

        if (effect != NULL)
        {
            s32 vy;

            effect->drawSkip = 1;
            vy = RandomSigned(0x1000);
            effect->velY.whole = vy - 0x4000;
            effect->accelY.whole = effect->velY.whole / 32;
            effect->velX.whole = RandomSigned(0x2000) - entity->velX.whole;
            effect->accelX.whole = -effect->velX.whole / 64;
        }
        Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x10000, 0x40000, 0x40000, entity->spriteLayer - 1);
        if ((RandomNumberGenerator() & 0x3FF) <= 0x1FF)
            PlaySong(0x158);
        else
            PlaySong(0x159);
    }
    else if (Entity_IsInRect(entity, -0x20, -0x20, 0x130, 0xE0))
    {
        return;
    }
    entity->deleteRequested = 1;
}
