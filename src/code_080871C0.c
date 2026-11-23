/* Triton spear creation. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern struct EwramData_EntityData *Entity_CreateObject(u32 *updateFunc);
extern void TritonSpear_Update(struct EwramData_EntityData *);
extern void TritonSpear_DamagePlayerOnContact(struct EwramData_EntityData *);
extern u8 sTritonGfx[];
extern u8 sTritonSpriteSheet[];
#define PART_ANGLE(e) (e->ext.tritonSpear.angle)
/* Create the Triton's spear (TritonSpear_Update) with its own affine slot,
 * hit box -3, -15, 6x6 and contact callback TritonSpear_DamagePlayerOnContact. */
struct EwramData_EntityData *Triton_CreateSpear(struct EwramData_EntityData *owner)
{
    struct HitBoxRecord box;
    struct EwramData_EntityData *part = Entity_CreateObject((u32 *)TritonSpear_Update);
    if (part != NULL)
    {
        s32 graphics;
        u16 slot;
        part->posX.whole = owner->posX.whole;
        part->posY.whole = owner->posY.whole;
        part->spriteLayer = owner->spriteLayer - 1;
        graphics = ObjGfx_Find(sTritonGfx);
        Sprite_Attach(part, sTritonSpriteSheet, graphics, owner->basePalette);
        Anim_StartIndexed(part, 10, 0, 0);
        Anim_Update(part);
        if ((s32)((slot = ObjAffine_Alloc(1)) << 16) < 0)
        {
            EntityDelete(part);
            return; /* the original leaves the return value undefined here */
        }
        part->affineIndex = slot;
        part->affine = 1;
        PART_ANGLE(part) = 0x1C000;
        ObjAffine_SetBiased(PART_ANGLE(part), 0x10000, 0x10000, part->flipX, part->flipY, part->affineIndex);
        part->attack = owner->attack;
        if (!gEwramData->gameState.bestiaryPreview)
        {
            s32 zero = 0;
            box.x = -3;
            box.y = -15;
            box.width = 6;
            box.height = 6;
            HitBox_SetAttack(part, &box);
            /* an OR with a zero variable survives in the original */
            part->hit.flags |= 4;
            part->hit.flags |= zero;
            part->hit.flags &= ~2;
            Hit_Enable(part, 8, TritonSpear_DamagePlayerOnContact, 0);
        }
        part->ext.tritonSpear.owner = owner;
        part->scrollWithBg = 0;
        part->drawFunc = (u32 *)Sprite_DrawEntity;
    }
    return part;
}
