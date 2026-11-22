/* Create and retire an affine afterimage with its own matrix allocation. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern u8 sGiantGhostGfx[], sGiantGhostSpriteSheet[];
extern void GiantGhostAuraTrail_Update(struct EwramData_EntityData *);
/**
 * @brief Spawn a semi-transparent affine trail sprite (animation 6) at (x, y)
 * with the scale of parent (kept in velX / velY); GiantGhostAuraTrail_Update.
 */
void GiantGhost_SpawnAuraTrail(struct EwramData_EntityData *parent, s32 x, s32 y)
{
    struct EwramData_EntityData *part = Entity_CreateObject((u32 *)GiantGhostAuraTrail_Update);
    if (part)
    {
        s32 graphics;
        u16 matrix;
        part->posX.whole = x;
        part->posY.whole = y;
        part->velX.whole = parent->velX.whole;
        part->velY.whole = parent->velY.whole;
        part->flipX = parent->flipX;
        part->spriteLayer = parent->spriteLayer + 1;
        graphics = ObjGfx_Find(sGiantGhostGfx);
        Sprite_Attach(part, sGiantGhostSpriteSheet, graphics, parent->unk_510.part8.basePalette);
        part->hidden = parent->hidden;
        matrix = ObjAffine_Alloc(1);
        if ((s16)matrix < 0)
        {
            EntityDelete(part);
            goto finished;
        }
        part->affineIndex = matrix;
        part->affine = 1;
        part->doubleSize = 1;
        part->objMode = 1;
        if ((u16)(part->posY.part16.integer + 28) > 220) part->hidden = 1;
        else part->hidden = 0;
        Anim_StartIndexed(part, 6, 0, 1);
        Anim_Update(part);
        ObjAffine_SetBiased(0, part->velX.whole, part->velY.whole, part->flipX, part->flipY, part->affineIndex);
        part->timer = 4;
        part->scrollWithBg = 1;
        part->drawFunc = (u32 *)Sprite_DrawEntity;
    }
finished:
    return;
}
