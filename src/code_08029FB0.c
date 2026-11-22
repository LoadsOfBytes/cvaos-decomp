/* SkillTinyDevilUse; graphics
 * loading as SkillFlameDemonProjectileUpdate, spawning as SkillHarpyUse. */
#include "code_08009A0.h"
#include "soul_skill_entity.h"
#include "red_soul_entity.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"

struct SoulGraphicsResources {
    void *graphics;
    void *palette;
    void *spriteSheet;
};
extern struct SoulGraphicsResources sSkillGraphicsSets[];
extern void SkillTinyDevilUpdate(struct EwramData_EntityData *);

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
 * @brief 29FB0 | Tiny Devil red soul: graphics set 21, position at the
 * caster (Entity_PlaceRelativeTo, 8 / -0x1A), Hit_SetTimerOverride(1, 20), song 0x1A3; spawn
 * up to three SkillTinyDevilUpdate children at the caster's position with
 * angles -0x1800, 0, 0x1800 in unk_510 (mirrored as -0x8000 - angle when
 * facing left); then free the red soul slot.
 */
void SkillTinyDevilUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = RED_SOUL_OWNER(entity);
    s32 i;
    s32 angle;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(21);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        entity->scrollWithBg = 1;
        Entity_PlaceRelativeTo(entity, parent, 8, -0x1A);
        Hit_SetTimerOverride(1, 20);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0x1A3);
        for (i = 0, angle = 0; i <= 2; angle += 0x1800, i++)
        {
            struct EwramData_EntityData *child = RedSoul_CreateEntity(entity, (u32 *)SkillTinyDevilUpdate);
            s32 a;
            if (child == NULL)
                break;
            child->posX.whole = entity->posX.whole;
            child->posY.whole = entity->posY.whole;
            a = angle - 0x1800;
            SKILL_TINY_DEVIL_HEADING(child) = a;
            if (entity->flipX)
                SKILL_TINY_DEVIL_HEADING(child) = -0x8000 - a;
        }
        entity->state = 1;
        break;
    case 1:
        gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        break;
    }
}
