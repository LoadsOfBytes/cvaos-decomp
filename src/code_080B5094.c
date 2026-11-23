#include "code_08009A0.h"
#include "balore_entity.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern u8 sBalorePalette[], sBaloreSpriteSheet[];
extern s32 ObjPalette_FindOrLoadVariant(void *, s32);

/* Affine effect following the owner; grows and rotates, then shrinks. */
void BaloreAffineEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.balorePart.body;
    u16 matrix;
    s32 state = entity->state;
    if (state != 0 && entity->state <= 2 && (owner->ext.balore.flags & 2))
        entity->state = 3;
    switch (entity->state)
    {
    case 0:
        matrix = ObjAffine_Alloc(1);
        if ((s16)matrix < 0) goto delete;
        entity->affineIndex = matrix;
        entity->affine = 1;
        entity->spriteLayer = 0x13;
        entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sBalorePalette, 0x10);
        Sprite_Attach(entity, sBaloreSpriteSheet, (s8)owner->ext.balore.gfx, entity->unk_510.part8.basePalette);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Anim_StartIndexed(entity, 0x31, 2, 0);
        Anim_Update(entity);
        entity->drawSkip = 1;
        if (entity->subState != 0)
        {
            entity->drawSkipCounter = 1;
            entity->flipX = 1;
            entity->ext.baloreEffect.angle = 0x5000;
            entity->ext.baloreEffect.spin = 0xE0;
            entity->state = 2;
        }
        else
        {
            entity->ext.baloreEffect.angle = 0x3000;
            entity->ext.baloreEffect.spin = 0x80;
            entity->state = 1;
        }
        break;
    case 1:
        if (entity->ext.baloreEffect.scale < 0x10000)
            entity->ext.baloreEffect.scale += 0x400;
        entity->ext.baloreEffect.angle += entity->ext.baloreEffect.spin;
        break;
    case 2:
        if (entity->ext.baloreEffect.scale < 0xC000)
            entity->ext.baloreEffect.scale += 0x200;
        entity->ext.baloreEffect.angle += entity->ext.baloreEffect.spin;
        break;
    case 3:
        if (entity->ext.baloreEffect.scale > 0)
        {
            entity->ext.baloreEffect.scale += -0x400;
            break;
        }
    case 4:
        ObjAffine_Free(entity->affineIndex);
    delete:
        EntityDelete(entity);
        return;
    }
    entity->posX.part16.integer = owner->posX.part16.integer - 21;
    entity->posY.part16.integer = owner->posY.part16.integer - 9;
    ObjAffine_Set(entity->ext.baloreEffect.angle, entity->ext.baloreEffect.scale, entity->ext.baloreEffect.scale, entity->flipX, 0, entity->affineIndex);
}



