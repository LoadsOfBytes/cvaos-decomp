#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08060B98.h"
extern u8 sUnk_084F13B4[];

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
 * @brief 284FC | Werewolf red soul: graphics set 14, animation 0, layer
 * 0xD at the caster (0x14 / -0xA), needs Entity_DropToFloor; Hit_SetTimerOverride(1, 12),
 * box slot 5 (no callback), song 0xD9. Dashes with Entity_Walk (speed
 * 0x4000, probe sUnk_084F13B4) with the current frame's hit box until the
 * animation ends.
 */
void SkillWerewolfUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->unk_4F8.unk_4F8_entity;

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(14);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->spriteLayer = 0xD;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        Entity_PlaceRelativeTo(entity, parent, 0x14, -0xA);
        if (Entity_DropToFloor(entity) == 0)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->unk_4FC.part16.unk_4FE[0]] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            break;
        }
        Hit_SetTimerOverride(1, 12);
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags = (entity->hit.flags | 4) & ~2;
        PlaySong(0xD9);
        entity->state = 1;
    case 1:
        Entity_Walk(entity, -0x4000, sUnk_084F13B4, 4);
        if (Anim_Update(entity) == 3)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->unk_4FC.part16.unk_4FE[0]] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            break;
        }
        SetFrameBox(entity);
        break;
    }
}
