#include "code_08009A0.h"
#include "soul_skill_entity.h"
#include "red_soul_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08060B98.h"
extern void SoulAttack_MarkHit(struct EwramData_EntityData *);
extern u8 sSkillUneWalkProfile[];

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
 * @brief 265C0 | Une red soul: a seed (graphics set 0, animation 3) thrown
 * from the caster (8 / -0x1A) with velY -0x18000, gravity 0x2000 and
 * horizontal speed unk_508 = 0xA000, box slot 5 (SoulAttack_MarkHit on hit,
 * disabled), Hit_SetTimerOverride(1, 20). It moves with Entity_Walk (probe
 * sSkillUneWalkProfile), halving and reversing unk_508 on wall contact (bits 0-1);
 * on landing (bit 3, song 0xE0) it waits 0x80 frames or until hit, then
 * sprouts (animation 4, song 0xE8) with the frame hit box until it ends.
 */
void SkillUneUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = RED_SOUL_OWNER(entity);
    s32 result;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(0);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 3, 0, 1);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Entity_PlaceRelativeTo(entity, parent, 8, -0x1A);
        SKILL_UNE_SPEED(entity) = 0xA000;
        entity->velY.whole = -0x18000;
        entity->accelY.whole = 0x2000;
        entity->scrollWithBg = 1;
        entity->state++;
        Hit_SetTimerOverride(1, 20);
        Hit_Enable(entity, 5, SoulAttack_MarkHit, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
    case 1:
        result = Entity_Walk(entity, -SKILL_UNE_SPEED(entity), sSkillUneWalkProfile, 0);
        if (result & 3)
            SKILL_UNE_SPEED(entity) = -SKILL_UNE_SPEED(entity) / 2;
        if (!(result & 8))
            break;
        PlaySong(0xE0);
        SetFrameBox(entity);
        entity->hit.flags &= ~2;
        entity->state = 2;
        entity->timer = 0x80;
        break;
    case 2:
        entity->timer--;
        if ((u8)RED_SOUL_HIT(entity) != 0 || entity->timer == 0)
        {
            Anim_StartIndexed(entity, 4, 0, 0);
            entity->state = 3;
            PlaySong(0xE8);
        }
        break;
    case 3:
        SetFrameBox(entity);
        if (entity->animFlags & 4)
            entity->state = 4;
        break;
    case 4:
        gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 4) || !entity->roomActive)
    {
        entity->roomActive = 1;
        entity->state = 4;
        entity->hidden = 1;
    }
    entity->roomActive = 1;
    Anim_Update(entity);
}
