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
 * @brief 27A04 | Altair red soul: graphics set 10, animation 1, layer 0xD,
 * Hit_SetTimerOverride(1, 30), placed at the caster (0x14 / -0x1A) facing the other
 * way, velX -0x28000 (mirrored), box 0x1020F8F0 in slot 5 disabled, song
 * 0xD6. When the animation ends (animFlags bit 2) it switches to animation 2
 * with blinking palette (2 / 4), then to the looping animation 0 with the
 * box enabled, flying off at 0x80000 acceleration (song 0xEE on frame 2).
 */
void SkillAltairUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    s32 state;
    s32 timer;

    entity->roomActive = 1;
    state = entity->state;
    switch (state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(10);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->spriteLayer = 0xD;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Hit_SetTimerOverride(1, 30);
        Entity_PlaceRelativeTo(entity, parent, 0x14, -0x1A);
        entity->flipX ^= 1;
        entity->velX.whole = -0x28000;
        if (entity->flipX)
            entity->velX.whole = -entity->velX.whole;
        entity->scrollWithBg = 1;
        entity->state = 1;
        Hit_Enable(entity, 5, 0, 0);
        HitBox_SetAttackRaw(entity, 0x1020F8F0);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0xD6);
    case 1:
        if (entity->animFlags & 4)
        {
            entity->palette = 4;
            Anim_StartIndexed(entity, 2, 0, 0);
            entity->state = 2;
        }
        break;
    case 2:
        timer = entity->timer + 1;
        entity->timer = timer;
        if (timer & state) /* state == 2 here */
            entity->palette = 2;
        else
            entity->palette = 4;
        if (entity->animFlags & 4)
        {
            entity->palette = 2;
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->state = 3;
            entity->hit.flags &= ~2;
        }
        break;
    case 3:
        EntityAccelerateAndMoveX(entity, 0x80000);
        if (*(u16 *)&entity->animStep == 2)
            PlaySong(0xEE);
        break;
    }
    if (EntityIsOutsideScreen(entity, 2) || !entity->roomActive)
    {
        entity->roomActive = 1;
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    Anim_Update(entity);
}
