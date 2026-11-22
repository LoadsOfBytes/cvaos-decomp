/* Attached part: mirrors its parent's flags, follows joint 0 / table point and switches animation. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"

extern s32 Enemy_IsStunned(struct EwramData_EntityData *entity);
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
 * @brief Follow the Skeleton owner at joint 0, then table-selected joints
 * and offsets. Mirror owner visibility, palette and lifetime; select the
 * part animation from owner flags. Distinct from SkeletonAttachedPart_Update.
 */
void SkeletonLinkedPart_Update(struct EwramData_EntityData *entity)
{
    struct { s16 x; s16 y; } pt;
    struct EwramData_EntityData *parent = &gEwramData->entityData[(u8)entity->unk_4F8.part8.unk_4F8];
    const struct PartPoint *t;

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
    if (!(parent->unk_4FC.part8.unk_4FE & 3))
    {
        entity->deleteRequested = 1;
        return;
    }
    if (entity->state && Enemy_IsStunned(entity))
        return;
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
        Anim_StartIndexed(entity, 0x10, 0, 1);
        Anim_Update(entity);
        entity->state++;
        entity->hidden = 0;
    case 1:
        Sprite_GetPartCornerOffset(parent, 0, &pt.x);
        entity->flipX = parent->flipX;
        entity->posX.part16.integer = parent->posX.part16.integer + pt.x;
        entity->posY.part16.integer = parent->posY.part16.integer + pt.y;
        if (!(parent->unk_4FC.part8.unk_4FE & 1))
            entity->state++;
        break;
    case 2:
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
    if (parent->unk_4FC.part8.unk_4FE & 0x20)
    {
        if (entity->animScriptIndex != 7)
            Anim_StartIndexed(entity, 7, 0, 1);
    }
    else
    {
        if (entity->animScriptIndex != 0x10)
            Anim_StartIndexed(entity, 0x10, 0, 1);
    }
    entity->palette = parent->palette;
    Anim_Update(entity);
}
