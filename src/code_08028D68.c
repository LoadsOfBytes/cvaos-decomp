#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"

struct SoulGraphicsResources {
    void *graphics;
    void *palette;
    void *spriteSheet;
};
extern struct SoulGraphicsResources sSkillGraphicsSets[];
extern void SkillSkullArcherArrowUpdate(struct EwramData_EntityData *);

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

/**
 * @brief 28D68 | Skull Archer red soul: graphics set 18, animation 0, layer
 * 0xD, placed at the caster (Entity_PlaceRelativeTo, -0xC / -0x18), Hit_SetTimerOverride(1, 10);
 * when the animation reaches frame 14 (animStep read as a halfword) spawn the
 * arrow SkillSkullArcherArrowUpdate at its position; ends when the animation ends or the
 * entity leaves the screen, then frees the red soul slot.
 */
void SkillSkullArcherUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->unk_4F8.unk_4F8_entity;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(18);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
        Entity_PlaceRelativeTo(entity, parent, -0xC, -0x18);
        Hit_SetTimerOverride(1, 10);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        entity->state++;
    case 1:
        if (*(u16 *)&entity->animStep == 0xE)
        {
            struct EwramData_EntityData *child = RedSoul_CreateEntity(entity, (u32 *)SkillSkullArcherArrowUpdate);
            if (child != NULL)
                Entity_PlaceRelativeTo(child, entity, 0, 0);
        }
        if (Anim_Update(entity) == 3)
            entity->state = 2;
        break;
    case 2:
        gEwramData->playerState.activeRedSoulEntities[entity->unk_4FC.part16.unk_4FE[0]] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 4) || !entity->roomActive)
    {
        entity->roomActive = 1;
        entity->state = 2;
    }
}
