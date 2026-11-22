/* Projectile 0x081C15F4 / 0x0820ED60 (palette 4, frame 0x35, priority 13):
 * set up on the first frame, then fall and flag itself (deleteRequested) for
 * removal once it leaves the screen. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern u8 sCommonObjectGfx[], sCommonObjectSpriteSheet[];

/**
 * @brief Droplet spawned by SkillMermanProjectileUpdate: move with
 * acceleration and delete off-screen.
 */
void SkillMermanDropletUpdate(struct EwramData_EntityData *entity)
{
    if (entity->state == 0)
    {
        s32 graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, 4);
        entity->animFrame = 0x35;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->state++;
    }
    EntityAccelerateAndMove(entity, 0x80000);
    if (EntityIsOutsideScreen(entity, 1))
        entity->deleteRequested = 1;
}
