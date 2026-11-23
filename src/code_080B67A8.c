/* Row of 32 child entities. */
#include "code_08009A0.h"
#include "balore_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 ObjPalette_FindOrLoadVariant(void *, s32);
extern void BaloreIntro_Update(struct EwramData_EntityData *);
extern void BaloreIntroPiece_Update(struct EwramData_EntityData *);
extern u8 sUnk_0811940C[], sBalorePalette[], sBaloreSpriteSheet2[];

/* palette slot + sprite set variant 0x11; same shape as Balore_LoadGraphics */
static inline u8 LoadGfx(struct EwramData_EntityData *entity)
{
    s32 slot = ObjGfx_LoadResource(sUnk_0811940C, 0);

    if (slot < 0)
        return 0;
    entity->ext.balore.palette = ObjPalette_FindOrLoadVariant(sBalorePalette, 0x11);
    entity->ext.balore.gfx = slot;
    entity->basePalette = entity->ext.balore.palette;
    Sprite_Attach(entity, sBaloreSpriteSheet2, slot, entity->basePalette);
    entity->scrollWithBg = 1;
    return 1;
}

/**
 * @brief B67A8 | Create: update becomes BaloreIntro_Update, graphics
 * (sUnk_0811940C / sBalorePalette variant 0x11 / sBaloreSpriteSheet2) or delete,
 * animation 4, priority 0x14, hidden.  Then up to 32 children
 * BaloreIntroPiece_Update (slots 0x49..0xCF) with owner unk_4FC, index i in unk_504,
 * X = own X - 0x80 + 8 * i and Y = 0x20; stops at the first failure.
 */
void Balore_CreateIntro(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *e;
    s32 i, offset;
    s16 *x;

    entity->updateFunc = (u32 *)BaloreIntro_Update;
    if (!LoadGfx(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 4, 2, 1);
    Anim_Update(entity);
    entity->spriteLayer = 0x14;
    entity->hidden = 1;
    /* the index starts before the X pointer is taken (register order) */
    i = 0;
    x = &entity->posX.part16.integer;
    for (offset = -0x80; i < 32; offset += 8, i++)
    {
        e = EntityCreateInRange(0x49, 0xCF, (u32 *)BaloreIntroPiece_Update);
        if (e == NULL)
            return;
        e->ext.balorePiece.owner = entity;
        e->ext.balorePiece.index = i;
        e->posX.part16.integer = *x + offset;
        e->posY.part16.integer = 0x20;
        e->scrollWithBg = 1;
    }
}
