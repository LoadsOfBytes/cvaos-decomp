#include "code_08009A0.h"
#include "code/code_08039340.h"
extern s32 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Hit_Enable(struct EwramData_EntityData *, s32, void *, void *);
extern void HitBox_SetBodyRaw(struct EwramData_EntityData *, u32);
extern u8 sObject1BGfx[], sUnk_08209FD8[], sObject1BSpriteSheet[];

void Object1BCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sObject1BGfx, sUnk_08209FD8, 1, sObject1BSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 2, 0, 0);
    Anim_Update(entity);
    entity->spriteLayer = 3;
    Hit_Enable(entity, 11, NULL, NULL);
    HitBox_SetBodyRaw(entity, 0x1010F0F8);
    entity->hit.flags |= 2;
    entity->hit.flags |= 4;
    gEwramData->gameState.conveyorSpeed = -0x14000;
}
