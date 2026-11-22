/* Spawn the two child parts of this enemy (one plain, one with an affine matrix); delete both on failure. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, s32);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern void GrahamShotPart_Update(struct EwramData_EntityData *);
extern u8 sGrahamGfx1[], sGrahamSpriteSheet1[], sGrahamShotPalette[];

/**
 * @brief Create a plain firing effect and delayed affine shot at the
 * selected height. Return success; retain all existing cleanup bounds
 * and array-entry assumptions, including the failure loop as written.
 */
s32 Graham_CreateShotPair(struct EwramData_EntityData *owner, s32 low)
{
    u8 *graphics = sGrahamGfx1;
    u8 *pal = sGrahamSpriteSheet1;
    s32 success = 1;
    s32 i = 0;
    struct EwramData_EntityData *parts[2];
    struct EwramData_EntityData **p = parts;
    while (i <= 1)
    {
        { struct EwramData_EntityData *spawned = Entity_CreateObject((u32 *)GrahamShotPart_Update);
        *p = spawned;
        if (!spawned) { success = 0; break; }
        if (!Entity_LoadGraphics(spawned, graphics, sGrahamShotPalette, 0, pal))
        { success = 0; goto fail; } }
        (*p)->unk_4F4.part8.unk_4F5 = owner->unk_4F4.part8.unk_4F4;
        (*p)->spriteLayer = owner->spriteLayer - 1;
        (*p)->flipX = owner->flipX;
        (*p)->posX.whole = owner->posX.whole;
        if (low)
            (*p)->posY.whole = owner->posY.whole - 0x100000;
        else
            (*p)->posY.whole = owner->posY.whole - 0x200000;
        (*p)->scrollWithBg = 1;
        (*p)->attack = 0x50;
        (*p)->unk_4F8.part8.unk_4F8 = i;
        if ((u8)(*p)->unk_4F8.part8.unk_4F8 == 0)
            Anim_StartIndexed(*p, 4, 0, 0);
        else
        {
            s32 matrix = ObjAffine_Alloc(1);
            register s32 scale asm("r2"); /* FAKEMATCH: keeps 0x10000 from being hoisted out of the loop by loop.c */
            if (matrix < 0)
                success = 0;
            else
            {
                (*p)->affineIndex = matrix;
                (*p)->affine = 1;
                scale = 0x10000;
                ObjAffine_Set(scale, scale, scale, 0, 0, (*p)->affineIndex);
            }
            Anim_StartIndexed(*p, 3, 0, 1);
            (*p)->hidden = 1;
        }
        Anim_Update(*p);
        Entity_EnableBoxes(*p, Enemy_OnContact, 0);
        (*p++)->hit.flags |= 4;
        i++;
    }
     if (!success)
    {
    fail:
        p = parts;
        i = 2;
        do
        {
            if (*p != NULL) EntityDelete(*p);
            p++;
            i--;
        } while (i >= 0);
    }
    return success;
}
