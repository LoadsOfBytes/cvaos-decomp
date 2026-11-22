/* Spawn the object 0x22's attached falling part. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern void Object22FallingPart_Update(struct EwramData_EntityData *);
extern void Object22FallingPart_OnPlayerContact(struct EwramData_EntityData *);
extern s32 ObjPalette_FindOrLoadVariant(void *, s32);
extern u8 sObject22Gfx[], sUnk_08209FD8[], sObject22SpriteSheet[];
void Object22_SpawnFallingPart(struct EwramData_EntityData *owner)
{
    struct HitBoxRecord box;
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Object22FallingPart_Update);
    s32 graphics;
    if (child != NULL)
    {
        child->posX.whole = owner->posX.whole;
        child->posY.whole = 0;
        child->velY.whole = 0x60000;
        child->spriteLayer = owner->spriteLayer + 1;
        graphics = ObjGfx_Find(sObject22Gfx);
        child->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sUnk_08209FD8, 1);
        Sprite_Attach(child, sObject22SpriteSheet, graphics, child->unk_510.part8.basePalette);
        Anim_StartIndexed(child, 4, 0, 0);
        Anim_Update(child);
        child->attack = owner->attack;
        box.x = -3;
        box.y = -15;
        box.width = 6;
        box.height = 15;
        HitBox_SetAttack(child, &box);
        child->hit.flags |= 4;
        child->hit.flags &= ~2;
        Hit_Enable(child, 8, Object22FallingPart_OnPlayerContact, 0);
        child->unk_518.part8.enemyId = owner->unk_518.part8.enemyId;
        child->unk_4FC.unk_4FC_entity = owner;
        child->scrollWithBg = 1;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}
