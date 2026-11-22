/* SkillBiphronAttackUpdate; graphics
 * loading as in SkillFlameDemonProjectileUpdate (src/code_080292B4.c). */
#include "code_08009A0.h"
#include "red_soul_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"

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
 * @brief Child of SkillBiphronUse: graphics set 15, animation 0, box slot 5
 * and sound 0xD9; animate with frame hit boxes until the animation
 * ends, then free its red soul slot and delete.
 */
void SkillBiphronAttackUpdate(struct EwramData_EntityData *entity)
{
    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(15);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->spriteLayer = 0xF;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags = (entity->hit.flags | 4) & ~2;
        PlaySong(0xD9);
        entity->state++;
    case 1:
        if (Anim_Update(entity) == 3)
        {
            gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            return;
        }
        SetFrameBox(entity);
    }
}
