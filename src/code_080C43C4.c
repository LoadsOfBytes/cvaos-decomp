/* Attached part: follows its parent's joint (case 2), mirrors its flags and deletes itself with it. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, u8 *resource, void *palette, u8 variant, void *sheet);
extern void Sprite_GetPartCornerOffset(struct EwramData_EntityData *entity, s32 index, s16 *offset);

struct PartPoint {
    u16 point;
    s8 dx;
    s8 dy;
};

extern u8 sSkeletonGfx[];
extern u8 sSkeletonWaiterSkeletonPalette[];
extern u8 sSkeletonSpriteSheet[];
extern const struct PartPoint sSkeletonPartPoints[];

/**
 * @brief Follow the parent animation joint and palette; delete when the parent identity or attachment flags are invalid.
 */
void SkeletonAttachedPart_Update(struct EwramData_EntityData *entity)
{
    struct { s16 x; s16 y; } pt;
    struct EwramData_EntityData *parent = &gEwramData->entityData[(u8)entity->unk_4F8.part8.unk_4F8];
    const struct PartPoint *t;

    if (!entity->hidden && parent->hidden)
        entity->hidden = parent->hidden;
    entity->deleteRequested = parent->deleteRequested;
    if (entity->deleteRequested)
        return;
    if (entity->unk_4F4.part16.unk_4F4 != parent->unk_4F4.part16.unk_4F4)
    {
        entity->deleteRequested = 1;
        return;
    }
    if (parent->updateFunc == NULL)
    {
        entity->deleteRequested = 1;
        return;
    }
    if (parent->deleteRequested)
    {
        entity->deleteRequested = 1;
        return;
    }
    if (!(parent->unk_4FC.part8.unk_4FE & 4))
    {
        entity->deleteRequested = 1;
        return;
    }
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sSkeletonGfx, sSkeletonWaiterSkeletonPalette, 0, sSkeletonSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        entity->scrollWithBg = 1;
        entity->spriteLayer = parent->spriteLayer;
        Anim_StartIndexed(entity, 8, 0, 1);
        entity->state++;
        break;
    case 1:
        entity->state = 2;
    case 2:
        if ((*(u32 *)&parent->animFlags & 0xFFFF00) == 0x30600)
        {
            if (entity->hidden)
                entity->hidden = 0;
            else
                goto check;
        }
        if (!entity->hidden)
        {
        check:
            if (parent->hidden)
                entity->deleteRequested = 1;
        }
        t = &sSkeletonPartPoints[parent->animScriptIndex];
        Sprite_GetPartCornerOffset(parent, t->point, &pt.x);
        entity->flipX = parent->flipX;
        entity->posX.part16.integer = parent->posX.part16.integer + pt.x;
        entity->posY.part16.integer = parent->posY.part16.integer + pt.y;
        if (entity->flipX)
            entity->posX.part16.integer -= t->dx;
        else
            entity->posX.part16.integer += t->dx;
        entity->posY.part16.integer += t->dy;
        break;
    }
    entity->palette = parent->palette;
    Anim_Update(entity);
}
