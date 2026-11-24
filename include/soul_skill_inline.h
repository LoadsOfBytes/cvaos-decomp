#ifndef SOUL_SKILL_INLINE_H
#define SOUL_SKILL_INLINE_H

/* Helpers shared by the red soul skill entities (soul/red_souls_a.c, soul/red_souls_b.c). */

#include "types.h"
#include "structs/ewram.h"
#include "hitbox.h"
#include "data/data_0E0B68.h"

static inline struct SoulGraphicsResources *SkillGraphics_GetSet(s32 index)
{
    return &sSkillGraphicsSets[index];
}

static inline void Skill_SetFrameHitBox(struct EwramData_EntityData *entity)
{
    u8 *spriteSlot = &entity->spriteSlot;
    struct SpriteBox *boxes = gEwramData->spriteSlots[*spriteSlot].sheet->frames[entity->animFrame].boxes;

    if (boxes != NULL)
    {
        entity->hit.flags &= ~2;
        HitBox_SetAttack(entity, boxes);
    }
    else
    {
        entity->hit.flags |= 2;
    }
}

static inline void Skill_AttachGraphics(struct EwramData_EntityData *entity, u8 paletteSlot, void *gfxSrc, void *palette, void *sheet)
{
    gEwramData->playerState.redSoulGfx = ObjGfx_LoadResource(gfxSrc, 0x10000);
    ObjPalette_LoadRange(palette, paletteSlot, 1, 2);
    Sprite_Attach(entity, sheet, gEwramData->playerState.redSoulGfx, 2);
}

#endif // SOUL_SKILL_INLINE_H
