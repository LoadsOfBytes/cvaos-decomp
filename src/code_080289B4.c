#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08060B98.h"
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
 * @brief 289B4 | Beam Skeleton red soul: graphics set 19, animation 0,
 * layer 0xD, Hit_SetTimerOverride(1, 16), song 0x1A9, box slot 5 (no callback,
 * disabled) with the current frame's hit box; facing like the caster and
 * kept at pixel offset (0x14 mirrored, -0x10) from it, stored in the
 * integer halves of velX / velY. Sound 0xCE plays from frame 10 until the
 * animation ends or it leaves the screen.
 */
void SkillBeamSkeletonUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(19);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
        Hit_SetTimerOverride(1, 16);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        entity->flipX = parent->flipX;
        entity->velX.part16.integer = 0x14;
        entity->velY.part16.integer = -0x10;
        if (entity->flipX)
            entity->velX.part16.integer = -entity->velX.part16.integer;
        PlaySong(0x1A9);
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        entity->state++;
    case 1:
        entity->posX.part16.integer = parent->posX.part16.integer + entity->velX.part16.integer;
        entity->posY.part16.integer = parent->posY.part16.integer + entity->velY.part16.integer;
        if (*(u16 *)&entity->animStep == 10)
            Sound_PlayTracked(0xCE);
        if (Anim_Update(entity) == 3)
            entity->state = 2;
        SetFrameBox(entity);
        break;
    case 2:
        Sound_StopTracked(0xCE);
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
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
