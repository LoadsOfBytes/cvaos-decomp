/* Afterimage_Create: matched (halfword bitfield view for affine; anim pinned to r6
   so the copy anim2 survives - FAKEMATCH). */
/* The canonical field now supplies the halfword mode; the animation pin remains. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"

struct AfterimageFrame
{
    s16 x;
    s16 y;
    u8 frame;
    u8 flipX;
    u8 pad[2];
};

extern s32 Sprite_Attach(struct EwramData_EntityData *entity, void *sheet, s32 tileSlot, u32 palette);
extern s32 Sprite_GetTileSlot(struct EwramData_EntityData *entity);
extern void *Heap_Alloc(u16 size);
extern s16 GetEntityRoomXPositionInteger(struct EwramData_EntityData *entity);
extern s16 GetEntityRoomYPositionInteger(struct EwramData_EntityData *entity);
extern void Afterimage_Update(struct EwramData_EntityData *);

/**
 * @brief Create a history-based afterimage with the supplied countdown, history index and flags; return NULL if entity, sheet or history allocation is unavailable.
 */
struct EwramData_EntityData *Afterimage_Create(struct EwramData_EntityData *entity, u16 delay, u8 count, s32 flags)
{
    s32 gfx;
    void *anim2;
    struct EwramData_EntityData *e;
    struct AfterimageFrame *p;
    s32 i;

    gfx = Sprite_GetTileSlot(entity);
    { register void *anim asm("r6"); /* FAKEMATCH: anim in r6 */
    { u8 *ix_ = &entity->spriteSlot; anim = gEwramData->spriteSlots[*ix_].sheet; }
    anim2 = anim;
    e = EntityCreateInRange(0x49, 0xCF, (u32 *)Afterimage_Update);
    if (e == NULL)
        return NULL;
    if (anim == NULL || (e->ext.afterimage.history.value = (s32)Heap_Alloc(0x31)) == 0)
    {
        EntityDelete(e);
        return NULL;
    }
    Sprite_Attach(e, anim2, gfx, entity->palette); }
    e->animFrame = entity->animFrame;
    e->spriteLayer = entity->spriteLayer + 1;
    e->posX.part16.integer = GetEntityRoomXPositionInteger(entity);
    e->posY.part16.integer = GetEntityRoomYPositionInteger(entity);
    e->ext.afterimage.owner = entity;
    e->ext.afterimage.delay = delay;
    if (count > 0x17)
        count = 0x17;
    e->subState = count;
    if (!(flags & 8))
        e->objMode = 1;
    e->flipX = entity->flipX;
    if (flags & 1)
        e->drawSkip = 1;
    if (flags & 2)
    {
        e->affine = entity->affine;
        e->affineIndex = entity->affineIndex;
    }
    p = (struct AfterimageFrame *)e->ext.afterimage.history.value;
    for (i = 0; i < 24; i++, p++)
    {
        p->x = e->posX.part16.integer;
        p->y = e->posY.part16.integer;
        p->frame = entity->animFrame;
        p->flipX = entity->flipX;
    }
    e->ext.afterimage.flags = flags;
    if (flags & 0x20)
        e->animMode = 2;
    if (flags & 0x40)
        e->animMode = 1;
    return e;
}
