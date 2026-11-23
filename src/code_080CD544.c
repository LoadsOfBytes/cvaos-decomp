/* Alastor graphics, affine transform and callbacks. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *,u8 *,void *,u8,void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern s32 Alastor_CreateLinkedPart(struct EwramData_EntityData *);
extern void Enemy_OnContactMask1(struct EwramData_EntityData *);
extern void Alastor_OnHit(struct EwramData_EntityData *);
extern void Hit_Enable(struct EwramData_EntityData *,s32,void *,s32);
extern s32 HitBox_SetBodyRaw(struct EwramData_EntityData *,s32);
extern void Alastor_UpdateHitBoxes(struct EwramData_EntityData *);
extern void Alastor_UpdateAppear(struct EwramData_EntityData *);
extern u8 sAlastorPalette[],sAlastorSpriteSheet[],sAlastorGfx[];
static inline void SetAffine(struct EwramData_EntityData *e,s32 index) { e->affineIndex=index; }
void EnemyAlastorCreate(struct EwramData_EntityData *entity)
{
    void *palette=sAlastorPalette;
    void *sprite=sAlastorSpriteSheet;
    u8 *graphics=sAlastorGfx;
    u8 display;
    u16 affine;
    s32 layer;
    s32 zero;
    u8 *layerPtr;
    if (!Entity_LoadGraphics(entity,graphics,palette,0,sprite))
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    layerPtr=&entity->spriteLayer;
    zero=0;
    layer=3;
    *layerPtr=layer;
    entity->animFrame=zero;
    display=((u8 *)&gEwramData->gameState)[0x2E] & 0x40;
    if (display)
    {
        entity->state=8;
        entity->stage=zero;
        entity->subState=zero;
        return;
    }
    affine=ObjAffine_Alloc(1);
    if ((s16)affine<0)
    {
        EntityDelete(entity);
        return;
    }
    SetAffine(entity,affine);
    entity->affine=1;
    ObjAffine_Set(0x10000,0x10000,0x10000,0,display,entity->affineIndex);
    if (!Alastor_CreateLinkedPart(entity))
        entity->deleteRequested=1;
    Hit_Enable(entity,8,Enemy_OnContactMask1,(s32)Alastor_OnHit);
    HitBox_SetBodyRaw(entity,0x1010F8F8);
    Alastor_UpdateHitBoxes(entity);
    entity->ext.alastor.attackSet=display;
    entity->ext.alastor.attackIndex=9;
    entity->ext.alastor.hitsBeforeReact=layer;
    Alastor_UpdateAppear(entity);
}

