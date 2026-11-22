#include "code_08009A0.h"
#include "code/code_08039340.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, u8, void *);
extern void Hit_Enable(struct EwramData_EntityData *, s32, void *, void *);
extern void Object18_OnHit(void);
extern void HitBox_SetBodyRaw(struct EwramData_EntityData *, u32);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern u8 sObject18Gfx[], sObject18Palette[], sObject18SpriteSheet[];

void Object18Create(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sObject18Gfx, sObject18Palette, 0, sObject18SpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    entity->spriteLayer = 0x14;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Hit_Enable(entity, 0, NULL, Object18_OnHit);
    entity->hit.flags |= 2;
    HitBox_SetBodyRaw(entity, 0x1820E8F0);
    Entity_SnapToGround(entity);
    entity->scrollWithBg = 1;
}
