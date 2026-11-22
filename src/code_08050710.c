#include "code_08009A0.h"
#include "code/code_08039340.h"
extern s32 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Hit_Enable(struct EwramData_EntityData *, s32, void *, void *);
extern void HitBox_SetBodyRaw(struct EwramData_EntityData *, u32);
extern u8 sObject0DGfx[], sObject0DPalette[], sObject0DSpriteSheet[];

void Object0DCreate(struct EwramData_EntityData *entity)
{
    u16 slot;
    if (!Entity_LoadGraphics(entity, sObject0DGfx, sObject0DPalette, 0, sObject0DSpriteSheet)) goto remove;
    slot = ObjAffine_Alloc(1);
    if ((s16)slot < 0)
    {
remove:
        EntityDelete(entity);
        return;
    }
    entity->affineIndex = slot;
    entity->affine = 1;
    entity->spriteLayer = 0x16;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Hit_Enable(entity, 8, NULL, NULL);
    HitBox_SetBodyRaw(entity, 0x122006F0);
    entity->hit.flags |= 2;
    entity->hit.flags &= 0xFB;
    entity->scrollWithBg = 1;
}
