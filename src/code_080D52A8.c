#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, s32);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern void GrahamSpiralProjectile_Update(struct EwramData_EntityData *);
extern u8 sGrahamGfx1[], sGrahamSpriteSheet1[], sGrahamShotPalette[];

/**
 * @brief Create three hidden affine projectiles with indexed orbit phase
 * and optional reversed rotation. Return success, preserving original
 * allocation and failure-cleanup behavior.
 */
s32 Graham_SpawnSpiralProjectiles(struct EwramData_EntityData *owner, s32 reverse)
{
    u8 *animation = sGrahamSpriteSheet1;
    s32 success = 1;
    s32 i;
    struct EwramData_EntityData *parts[6];
    struct EwramData_EntityData **p;
    owner->unk_4F8.part8.unk_4F9 = 3;
    i = 0;
    p = parts;
    while (i < owner->unk_4F8.part8.unk_4F9)
    {
        *p = Entity_CreateObject((u32 *)GrahamSpiralProjectile_Update);
        if (!*p) { success = 0; break; }
        if (!Entity_LoadGraphics(*p, sGrahamGfx1, sGrahamShotPalette, 0, animation))
        { success = 0; goto fail; }
        (*p)->unk_4F4.part8.unk_4F5 = owner->unk_4F4.part8.unk_4F4;
        (*p)->spriteLayer = owner->spriteLayer - 1;
        (*p)->flipX = owner->flipX;
        (*p)->posX.whole = owner->posX.whole;
        (*p)->posY.whole = owner->posY.whole - 0x200000;
        (*p)->scrollWithBg = 1;
        (*p)->unk_4F8.part8.unk_4F9 = owner->unk_4F8.part8.unk_4F9;
        (*p)->attack = 0x50;
        (*p)->unk_4F8.part8.unk_4F8 = i;
        (*p)->unk_510.whole = 0xAA;
        (*p)->unk_500.part16.unk_502 = 0x2C0;
        if (reverse)
        {
            (*p)->unk_510.whole = -(*p)->unk_510.whole;
            (*p)->unk_500.part16.unk_502 = -(*p)->unk_500.part16.unk_502;
        }
        {
            s32 matrix = ObjAffine_Alloc(1);
            if (matrix < 0)
                success = 0;
            else
            {
                (*p)->affineIndex = matrix;
                (*p)->affine = 1;
                ObjAffine_Set(0x10000, 0, 0, 0, 0, (*p)->affineIndex);
            }
        }
        Anim_StartIndexed(*p, 3, 0, 1);
        (*p)->hidden = 1;
        Anim_Update(*p);
        Entity_EnableBoxes(*p, Enemy_OnContact, 0);
        (*p++)->hit.flags |= 4;
        i++;
    }
    if (!success)
    {
    fail:
        i = 0;
        if (i < owner->unk_4F8.part8.unk_4F9)
        {
            p = parts;
            do
            {
                if (*p != NULL) EntityDelete(*p);
                p++;
                i++;
            } while (i < owner->unk_4F8.part8.unk_4F9);
        }
    }
    return success;
}
