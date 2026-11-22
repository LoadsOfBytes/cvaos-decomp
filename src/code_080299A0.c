#include "code_08009A0.h"
#include "red_soul_entity.h"
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
 * @brief 299A0 | Fleaman red soul: a hopping flea (graphics set 20, layer
 * 0xD) from the caster (8 / -0x1A) facing the other way, gravity 0x2000,
 * box slot 5 (no callback) with the current frame's hit box,
 * Hit_SetTimerOverride(1, 20), song 0x19C. For 160 frames it moves with
 * Entity_Walk (speed 0x18000, probe sUnk_084F13B4 + 0xA): frame by velY, turns
 * on walls, hops again on landing (velY -0x10000, or 1 in 4 a random
 * -5..-4 px). Then animation 1 with drawSkip set and random Effect_SpawnShared
 * flames every 4 frames for 16 frames.
 */
void SkillFleamanUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = RED_SOUL_OWNER(entity);
    s32 r;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(20);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
        Entity_PlaceRelativeTo(entity, parent, 8, -0x1A);
        entity->accelY.whole = 0x2000;
        entity->flipX ^= 1;
        Hit_SetTimerOverride(1, 20);
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags |= 4;
        entity->timer = 0xA0;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0x19C);
        entity->state = 1;
    case 1:
        r = Entity_Walk(entity, 0x18000, sUnk_084F13B4 + 0xA, 8);
        if (entity->velY.whole < -0x8000)
            entity->animFrame = 0;
        else if (entity->velY.whole > 0x8000)
            entity->animFrame = 2;
        else
            entity->animFrame = 1;
        if (r & 3)
            entity->flipX ^= 1;
        if (r & 8)
        {
            if (RandomNumberGenerator() & 3)
                entity->velY.whole = -0x10000;
            else
                entity->velY.whole = (RandomNumberGenerator() & 0xFFFF) + ({ s32 k = 0x50000; -k; });
        }
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->drawSkip = 1;
        entity->state = 2;
        entity->timer = 0x10;
        break;
    case 2:
        if (!(entity->timer & 3))
        {
            s32 x, y;
            {
                s32 d = (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
                x = entity->posX.whole + d;
            }
            {
                /* -k keeps the offset out of CSE (reloaded from the pool as in the original) */
                s32 d = (RandomNumberGenerator() & 0x7FFFF) + ({ s32 k = 0x40000; -k; });
                y = entity->posY.whole + d;
            }
            Effect_SpawnShared(x, y, 0, 7);
        }
        Anim_Update(entity);
        if (entity->timer == 0)
        {
            gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            return;
        }
        entity->timer--;
        break;
    }
    if (EntityIsOutsideScreen(entity, 2) || !entity->roomActive)
    {
        entity->roomActive = 1;
        gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    SetFrameBox(entity);
}
