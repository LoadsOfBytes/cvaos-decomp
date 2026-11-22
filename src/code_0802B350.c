#include "code_08009A0.h"
#include "red_soul_entity.h"
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
extern void SoulAttack_MarkHit(struct EwramData_EntityData *);

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
 * @brief 2B350 | Fish Head red soul: a fireball (graphics set 17, animation
 * 0, layer 0xD) from the caster (8 / -0x1A) with velX 0x20000 (mirrored)
 * and acceleration accelX = velX / 32, box 0x0C0CFAFA in slot 5 with
 * SoulAttack_MarkHit on hit, Hit_SetTimerOverride(1, 20), song 0xD8. Ends on hit or off
 * screen.
 */
void SkillFishHeadUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = RED_SOUL_OWNER(entity);

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(17);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
        Entity_PlaceRelativeTo(entity, parent, 8, -0x1A);
        entity->velX.whole = 0x20000;
        if (entity->flipX)
            entity->velX.whole = -entity->velX.whole;
        entity->accelX.whole = entity->velX.whole / 32;
        Hit_SetTimerOverride(1, 20);
        Hit_Enable(entity, 5, SoulAttack_MarkHit, 0);
        HitBox_SetAttackRaw(entity, 0x0C0CFAFA);
        entity->hit.flags |= 4;
        PlaySong(0xD8);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        entity->state++;
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        Anim_Update(entity);
    }
    if (EntityIsOutsideScreen(entity, 2) || !entity->roomActive)
        entity->roomActive = 1;
    else if ((u8)RED_SOUL_HIT(entity) == 0)
        return;
    gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
    gEwramData->playerState.nbrActiveRedSoulEntities--;
    EntityDelete(entity);
}
