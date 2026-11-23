#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"

struct SoulGraphicsResources {
    void *graphics;
    void *palette;
    void *spriteSheet;
};
extern struct SoulGraphicsResources sSkillGraphicsSets[];

static inline void SkillLoadGraphicsInline(struct EwramData_EntityData *entity, u8 paletteSlot, void *gfxSrc, void *palette, void *sheet)
{
    gEwramData->playerState.redSoulGfx = ObjGfx_LoadResource(gfxSrc, 0x10000);
    ObjPalette_LoadRange(palette, paletteSlot, 1, 2);
    Sprite_Attach(entity, sheet, gEwramData->playerState.redSoulGfx, 2);
}

static inline struct SoulGraphicsResources *GetSet(s32 index)
{
    return &sSkillGraphicsSets[index];
}

/* Inlined Entity_SetAttackBoxFromFrame (src/code_0802D510.c). */
static inline void SetFrameBox(struct EwramData_EntityData *entity)
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


/**
 * @brief 26C54 | Bat red soul: graphics set 3, animation 0, layer 0xD,
 * placed at the caster (0 / -0x1A), Hit_SetTimerOverride(1, 15), box slot 5 (no
 * callback), song 0xE2; then animate with the current frame's hit box until
 * the animation ends, then free the red soul slot.
 */
void SkillBatUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;

    if (entity->state == 0)
    {
        {
            struct SoulGraphicsResources *t = GetSet(3);
            void *gfx = t->graphics;
            void *pal = t->palette;
            void *oam = t->spriteSheet;
            SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
        }
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        Entity_PlaceRelativeTo(entity, parent, 0, -0x1A);
        entity->scrollWithBg = 1;
        entity->state++;
        Hit_SetTimerOverride(1, 15);
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        PlaySong(0xE2);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
    }
    entity->roomActive = 1;
    if (Anim_Update(entity) == 3)
    {
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    SetFrameBox(entity);
}
