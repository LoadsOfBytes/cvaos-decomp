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
extern void SkillSkullMillioneSkullUpdate(struct EwramData_EntityData *);

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
 * @brief 2B1B8 | Skull Millione red soul: graphics set 7, Hit_SetTimerOverride(1,
 * 12), song 0xC9; moves away from the caster (offset subState + 0x14 /
 * -0x14, subState growing by 1 per frame) and spawns a SkillSkullMillioneSkullUpdate child
 * at its position every second frame, three times; ends then or off screen.
 */
void SkillSkullMillioneUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = RED_SOUL_OWNER(entity);
    struct EwramData_EntityData *child;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(7);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        entity->scrollWithBg = 1;
        Hit_SetTimerOverride(1, 12);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0xC9);
        entity->state++;
    case 1:
        Entity_PlaceRelativeTo(entity, parent, entity->subState + 0x14, -0x14);
        entity->timer++;
        entity->subState++;
        if (entity->timer > 1)
        {
            entity->timer = 0;
            child = RedSoul_CreateEntity(entity, (u32 *)SkillSkullMillioneSkullUpdate);
            if (child != NULL)
            {
                child->posX.whole = entity->posX.whole;
                child->posY.whole = entity->posY.whole;
                child->flipX = entity->flipX;
            }
            if (++entity->stage > 2)
                entity->state = 2;
        }
        break;
    case 2:
        gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 3) || !entity->roomActive)
    {
        entity->roomActive = 1;
        gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
    }
}
