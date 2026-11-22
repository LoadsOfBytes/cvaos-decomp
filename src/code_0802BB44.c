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
 * @brief 2BB44 | Kyoma Demon red soul: affine sprite (graphics set 27,
 * frame 0, layer 0xD, objMode = 1) on the caster, song 0x97; for 33
 * frames scaled vertically by sin(timer << 10) (unk_508), then calls
 * Player_StartContactImmunity(180) on the player, sets hidden and waits 180 frames before freeing its
 * affine slot and red soul slot.
 */
void SkillKyomaDemonUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = RED_SOUL_OWNER(entity);
    u16 slot;

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
            EntityDelete(entity);
            break;
        }
        entity->affineIndex = slot;
        entity->affine = 1;
    {
        struct SoulGraphicsResources *t = GetSet(27);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        entity->animFrame = 0;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->objMode = 1;
        entity->scrollWithBg = 1;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0x97);
        entity->timer = 0x20;
        entity->state++;
    case 1:
        Entity_PlaceRelativeTo(entity, parent, 0, 0);
        SKILL_KYOMA_DEMON_SCALE_X(entity) = Sin(entity->timer << 10);
        ObjAffine_Set(0, SKILL_KYOMA_DEMON_SCALE_X(entity), 0x10000, 0, 0, entity->affineIndex);
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        SKILL_KYOMA_DEMON_TIMER(entity) = 0xB4;
        Player_StartContactImmunity(0xB4);
        entity->hidden = 1;
        entity->state++;
    case 2:
        if (SKILL_KYOMA_DEMON_TIMER(entity) != 0)
        {
            SKILL_KYOMA_DEMON_TIMER(entity)--;
            break;
        }
        ObjAffine_Free(entity->affineIndex);
        gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        break;
    }
}
