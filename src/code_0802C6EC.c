#include "code_08009A0.h"
#include "red_soul_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
/* Man Eater soul skill: frames since the bite started (ends after 0x1D). */
#define SKILL_MAN_EATER_FRAMES(e) e->unk_508.part16.unk_508

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
 * @brief 2C6EC | Man Eater red soul: a seed (graphics set 32, one of three
 * random variants, frame base stage = 5 * variant) from the caster (8 /
 * -0x1A) with velX 0x12000 (mirrored), box 0x0C0CFAFA in slot 5 with
 * SoulAttack_MarkHit on hit, Hit_SetTimerOverride(1, 20), song 0x15E. The frame advances
 * every 10 frames for 30 frames, then it flies on for 48 frames; on a hit
 * it stops (frame base + 4) for 16 frames. Either way it then shows drawSkip
 * for 16 frames and ends; also ends off screen.
 */
void SkillManEaterUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = RED_SOUL_OWNER(entity);

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(32);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        RandomNumberGenerator();
        entity->subState = RandomNumberGenerator() % 3;
        entity->stage = entity->subState * 5;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
        Entity_PlaceRelativeTo(entity, parent, 8, -0x1A);
        entity->velX.whole = 0x12000;
        if (entity->flipX)
            entity->velX.whole = -entity->velX.whole;
        Hit_SetTimerOverride(1, 20);
        Hit_Enable(entity, 5, SoulAttack_MarkHit, 0);
        HitBox_SetAttackRaw(entity, 0x0C0CFAFA);
        entity->hit.flags |= 4;
        PlaySong(0x15E);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        entity->state++;
    case 1:
        entity->animFrame = entity->stage + SKILL_MAN_EATER_FRAMES(entity) / 10;
        EntityAccelerateAndMove(entity, 0x80000);
        if (++SKILL_MAN_EATER_FRAMES(entity) > 0x1D)
        {
            entity->timer = 0x30;
            entity->state = 2;
        }
        break;
    case 2:
        EntityAccelerateAndMove(entity, 0x80000);
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->drawSkip = 1;
        entity->timer = 0x10;
        entity->state = 3;
        break;
    case 3:
        EntityAccelerateAndMove(entity, 0x80000);
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->state = 5;
        break;
    case 4:
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->drawSkip = 1;
        entity->timer = 0x10;
        entity->state = 3;
        break;
    case 5:
        gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (entity->state <= 2 && (u8)RED_SOUL_HIT(entity) != 0)
    {
        entity->velX.whole = 0;
        entity->animFrame = entity->stage + 4;
        entity->state = 4;
        entity->timer = 0x10;
    }
    if (EntityIsOutsideScreen(entity, 2) || !entity->roomActive)
    {
        entity->roomActive = 1;
        gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
    }
}
