#include "code_08009A0.h"
#include "red_soul_entity.h"
#include "player_entity.h"
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
extern u8 sSkillLightningDollPalette[];

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
 * @brief 277B0 | Lightning Doll red soul: graphics set 9, animation 0,
 * layer 0xE, Hit_SetTimerOverride(1, 10), box slot 5, random vertical flip, placed
 * at the caster (0x10 / -0x1C) with the caster's facing kept in stage;
 * song 0x1A2 or 0x1A9 (Player_IsBossTargetLowHp) and 0xE9. Follows the caster while
 * its unk_4F4 bit 8 is set during the first frames, then the box width
 * (timer, from 0x10) grows by 8 per frame; on frame 8 the palette
 * sSkillLightningDollPalette is loaded once. Ends with the animation or off screen.
 */
void SkillLightningDollUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = RED_SOUL_OWNER(entity);

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(9);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->spriteLayer = 0xE;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        Hit_SetTimerOverride(1, 10);
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags |= 4;
        entity->flipY = RandomNumberGenerator() & 1;
        Entity_PlaceRelativeTo(entity, parent, 0x10, -0x1C);
        entity->stage = parent->flipX;
        entity->timer = 0x10;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        if (Player_IsBossTargetLowHp())
            PlaySong(0x1A2);
        else
            PlaySong(0x1A9);
        PlaySong(0xE9);
        entity->state = 1;
    case 1:
        if (entity->animStep > 3 && (s8)entity->timer >= 0)
            entity->timer += 8;
        if (entity->animStep <= 3 && (PLAYER_ACTION_FLAGS(parent) & 0x100))
            Entity_PlaceRelativeTo(entity, parent, 0x10, -0x1C);
        entity->flipX = entity->stage;
        HitBox_SetAttackRaw(entity, (entity->timer << 16) + 0x2000F000);
        if (entity->subState == 0 && entity->animStep == 8)
        {
            ObjPalette_LoadRange(sSkillLightningDollPalette, 1, 1, 2);
            entity->subState = 1;
        }
        if (Anim_Update(entity) == 3)
        {
            gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            return;
        }
    }
    if (EntityIsOutsideScreen(entity, 2) || !entity->roomActive)
    {
        entity->roomActive = 1;
        gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
    }
}
