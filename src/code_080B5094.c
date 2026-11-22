#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern u8 sBalorePalette[], sBaloreSpriteSheet[];
extern s32 ObjPalette_FindOrLoadVariant(void *, s32);

/* Affine effect following the owner; grows and rotates, then shrinks. */
void BaloreAffineEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->unk_4FC.unk_4FC_entity;
    u16 matrix;
    s32 state = entity->state;
    if (state != 0 && entity->state <= 2 && (owner->unk_500.part16.unk_502 & 2))
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
        Sprite_Attach(entity, sBaloreSpriteSheet, (s8)owner->unk_4F8.part8.unk_4FB, entity->unk_510.part8.basePalette);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Anim_StartIndexed(entity, 0x31, 2, 0);
        Anim_Update(entity);
        entity->drawSkip = 1;
        if (entity->subState != 0)
        {
            entity->drawSkipCounter = 1;
            entity->flipX = 1;
            entity->unk_500.part16.unk_500 = 0x5000;
            entity->unk_500.part16.unk_502 = 0xE0;
            entity->state = 2;
        }
        else
        {
            entity->unk_500.part16.unk_500 = 0x3000;
            entity->unk_500.part16.unk_502 = 0x80;
            entity->state = 1;
        }
        break;
    case 1:
        if (entity->unk_504.whole < 0x10000)
            entity->unk_504.whole += 0x400;
        entity->unk_500.part16.unk_500 += entity->unk_500.part16.unk_502;
        break;
    case 2:
        if (entity->unk_504.whole < 0xC000)
            entity->unk_504.whole += 0x200;
        entity->unk_500.part16.unk_500 += entity->unk_500.part16.unk_502;
        break;
    case 3:
        if (entity->unk_504.whole > 0)
        {
            entity->unk_504.whole += -0x400;
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
    ObjAffine_Set(entity->unk_500.part16.unk_500, entity->unk_504.whole, entity->unk_504.whole, entity->flipX, 0, entity->affineIndex);
}



