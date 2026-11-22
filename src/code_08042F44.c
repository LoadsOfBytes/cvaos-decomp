/* Create a debug-style value display at (x, y): graphics 081C15F4 /
 * 0820ED60 with the given palette, watching the 8-, 16- or 32-bit value at
 * value (bits) through NumberWatch_Update and showing it via NumberIndicator_Format. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void NumberIndicator_Format(struct EwramData_EntityData *, s32);
extern void NumberWatch_Update(struct EwramData_EntityData *);
extern void NumberIndicator_Draw(struct EwramData_EntityData *);
extern u8 sCommonObjectGfx[], sCommonObjectSpriteSheet[];

/**
 * @brief Create a numeric display at fixed-point x/y watching a signed 8-, 16- or default 32-bit memory value with the supplied palette.
 */
struct EwramData_EntityData *NumberWatch_Create(void *value, s32 x, s32 y, u8 palette, u8 bits)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)NumberWatch_Update);
    s32 graphics = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, (s8)palette);
    entity->posX.whole = x;
    entity->posY.whole = y;
    entity->unk_508.part8.unk_509 = bits;
    if ((s8)bits == 8)
        entity->unk_510.whole = *(s8 *)value;
    else if ((s8)bits == 16)
        entity->unk_510.whole = *(s16 *)value;
    else
        entity->unk_510.whole = *(s32 *)value;
    entity->unk_50C.whole = (s32)value;
    NumberIndicator_Format(entity, entity->unk_510.whole);
    entity->drawFunc = (u32 *)NumberIndicator_Draw;
    entity->spriteLayer = 2;
    return entity;
}
