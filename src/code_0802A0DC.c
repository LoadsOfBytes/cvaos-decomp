#include "code_08009A0.h"
#include "soul_skill_entity.h"
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
 * @brief 2A0DC | Ukoback red soul: affine fireball (graphics set 22,
 * animation 1, layer 0xD) at the caster (8 / -0x1A), box 0x1010F8F8 in slot
 * 5 with SoulAttack_MarkHit on hit, Hit_SetTimerOverride(1, 20), song 0xDC. Grows (scale
 * unk_508 += 0x1000 up to 0x10000) and stays 240 frames or until hit, then
 * shrinks by 0x2000 per frame and ends; also ends off screen.
 */
void SkillUkobackUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = RED_SOUL_OWNER(entity);
    u16 slot;

    switch (entity->state)
    {
    case 0:
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
            EntityDelete(entity);
            return;
        }
        entity->affineIndex = slot;
        entity->affine = 1;
    {
        struct SoulGraphicsResources *t = GetSet(22);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 1, 0, 1);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
        Entity_PlaceRelativeTo(entity, parent, 8, -0x1A);
        Hit_SetTimerOverride(1, 20);
        Hit_Enable(entity, 5, SoulAttack_MarkHit, 0);
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->hit.flags |= 4;
        entity->timer = 0xF0;
        PlaySong(0xDC);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        entity->state = 1;
    case 1:
        if (SKILL_UKOBACK_SCALE(entity) <= 0xFFFF)
            SKILL_UKOBACK_SCALE(entity) += 0x1000;
        if ((u8)RED_SOUL_HIT(entity) != 0)
            entity->timer = 0;
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->state = 2;
    case 2:
        if (SKILL_UKOBACK_SCALE(entity) <= 0)
        {
            ObjAffine_Free(entity->affineIndex);
            gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            return;
        }
        SKILL_UKOBACK_SCALE(entity) -= 0x2000;
        break;
    }
    Anim_Update(entity);
    ObjAffine_Set(0, SKILL_UKOBACK_SCALE(entity), SKILL_UKOBACK_SCALE(entity), entity->flipX, 0, entity->affineIndex);
    if (EntityIsOutsideScreen(entity, 2) || !entity->roomActive)
    {
        entity->roomActive = 1;
        ObjAffine_Free(entity->affineIndex);
        gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
    }
}
