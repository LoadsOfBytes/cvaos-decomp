#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code_08013960.h"

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
 * @brief 2C0D4 | Legion red soul: graphics set 29, animation 0, layer 0xE,
 * Hit_SetTimerOverride(1, 15), box slot 5 (no callback, disabled), song 0x1A9;
 * follows the caster keeping the initial facing (stage) with the current
 * frame's hit box; starts sound 0xCE (Sound_PlayTracked) on frame 12 and stops
 * it (Sound_StopTracked) when the caster's unk_4F4 bit 7 is set or the animation
 * ends.
 */
void SkillLegionUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->unk_4F8.unk_4F8_entity;

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(29);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->spriteLayer = 0xE;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        Hit_SetTimerOverride(1, 15);
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        Entity_PlaceRelativeTo(entity, parent, 0, 0);
        entity->stage = parent->flipX;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0x1A9);
        entity->state = 1;
    case 1:
        Entity_PlaceRelativeTo(entity, parent, 0, 0);
        entity->flipX = entity->stage;
        if (*(u16 *)&entity->animStep == 0xC)
            Sound_PlayTracked(0xCE);
        if (parent->unk_4F4.whole & 0x80)
        {
            Sound_StopTracked(0xCE);
            gEwramData->playerState.activeRedSoulEntities[entity->unk_4FC.part16.unk_4FE[0]] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            return;
        }
    }
    if (Anim_Update(entity) == 3)
    {
        Sound_StopTracked(0xCE);
        gEwramData->playerState.activeRedSoulEntities[entity->unk_4FC.part16.unk_4FE[0]] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    SetFrameBox(entity);
}
