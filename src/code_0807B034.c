/* Select an animation and emit debris at its impact frame. */
#include "code/code_08039340.h"
#include "code_08009A0.h"
extern void Zombie_SpawnFallingDebris(struct EwramData_EntityData *, s32, s32);
/**
 * @brief Choose animation 1 or 4 after emergence and emit frame-13 debris at the animation event; no movement.
 */
void Zombie_UpdateStationaryAnimation(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        if (entity->animDone)
        {
            entity->subState = 1;
            if (RandomNumberGenerator() & 1) Anim_StartIndexed(entity, 1, 0, 1);
            else Anim_StartIndexed(entity, 4, 0, 1);
        }
        break;
    case 1:
        if (entity->animStep == 0 && entity->animTimer == 0)
        {
            s32 x, y;
            if (entity->animScriptIndex == 4)
            {
                x = entity->flipX ? -0x90000 : 0x90000;
                y = -0x180000;
            }
            else
            {
                x = entity->flipX ? 0x70000 : -0x70000;
                y = -0x160000;
            }
            x += entity->posX.whole;
            y += entity->posY.whole;
            Zombie_SpawnFallingDebris(entity, x, y);
        }
        break;
    }
}
