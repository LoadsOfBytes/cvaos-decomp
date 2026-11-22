/* Create a debug-style value display at (x, y): graphics 081C15F4 /
 * 0820ED60 with the given palette, watching the 8-, 16- or 32-bit value at
 * value (bits) through NumberWatch_Update and showing it via NumberIndicator_Format. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/*
 * Number watch (NumberWatch_Create; same aliases as NumberWatch_Update):
 * width of the watched value in bits (8 / 16 / 32), its address and the last
 * shown value. Token-identical expansion.
 */
#define NUMBER_WATCH_BITS(e) e->unk_508.part8.unk_509
#define NUMBER_WATCH_ADDRESS(e) e->unk_50C.whole
#define NUMBER_WATCH_LAST(e) e->unk_510.whole
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
    NUMBER_WATCH_BITS(entity) = bits;
    if ((s8)bits == 8)
        NUMBER_WATCH_LAST(entity) = *(s8 *)value;
    else if ((s8)bits == 16)
        NUMBER_WATCH_LAST(entity) = *(s16 *)value;
    else
        NUMBER_WATCH_LAST(entity) = *(s32 *)value;
    NUMBER_WATCH_ADDRESS(entity) = (s32)value;
    NumberIndicator_Format(entity, NUMBER_WATCH_LAST(entity));
    entity->drawFunc = (u32 *)NumberIndicator_Draw;
    entity->spriteLayer = 2;
    return entity;
}
