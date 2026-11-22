/* Giant Ghost's graphics, collision effect and death particles. */
#include "code_08009A0.h"
#include "particle_effect.h"
#include "code/code_08039340.h"
#include "code/code_080211F0.h"
extern s8 ObjPalette_FindOrLoadVariant(void *, u8);
extern void Effect_UpdateTimedParticle(struct EwramData_EntityData *);
extern u8 sGiantGhostGfx[], sGiantGhostPalette[], sGiantGhostPaletteAnim[], sGiantGhostSpriteSheet[];
extern u8 sCommonObjectGfx[], sCommonObjectSpriteSheet[];
/**
 * @brief Spawn a falling fragment at (x, y) moving with (vx, vy) under gravity on
 * layer priority, frame chosen by type (four on death).
 */
void GiantGhost_SpawnFragment(s32 x, s32 y, s32 vx, s32 vy, s32 priority, s32 type)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateTimedParticle);
    if (part)
    {
        s32 zero;
        s32 graphics;
        u8 *priorityField;
        part->posX.whole = x;
        part->posY.whole = y;
        part->velX.whole = vx;
        part->velY.whole = vy;
        part->accelY.whole = 0x2000;
        priorityField = &part->spriteLayer;
        zero = 0;
        *priorityField = priority;
        part->timer = 200;
        PARTICLE_FLAGS(part) = zero;
        PARTICLE_FLIP_FLAGS(part) = 3;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(part, sCommonObjectSpriteSheet, graphics, 6);
        if (type == 0) part->animFrame = 64;
        else part->animFrame = 65;
        part->objMode = 1;
        part->scrollWithBg = 1;
        part->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}
