#include "code_08009A0.h"
#include "soul_skill_entity.h"
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
extern void SkillBiphronAttackUpdate(struct EwramData_EntityData *);
extern u8 sUnk_084F13B4[];

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
 * @brief 28800 | Biphron red soul: graphics set 15 at the caster (0x14 /
 * -0xA), needs Entity_DropToFloor; Hit_SetTimerOverride(1, 12), song 0x1A4. For 65
 * frames it moves with Entity_Walk (probe sUnk_084F13B4) at a speed
 * unk_508 growing from 0x8000 by 0x1000 up to 0x20000, turning around on
 * walls, and spawns a SkillBiphronAttackUpdate attack at its position every 8 frames.
 */
void SkillBiphronUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    struct EwramData_EntityData *child;

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(15);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        entity->scrollWithBg = 1;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        Entity_PlaceRelativeTo(entity, parent, 0x14, -0xA);
        if (Entity_DropToFloor(entity) == 0)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            break;
        }
        Hit_SetTimerOverride(1, 12);
        entity->state = 1;
        entity->ext.skillWalker.speed = 0x8000;
        PlaySong(0x1A4);
        entity->timer = 0x40;
    case 1:
        if (entity->ext.skillWalker.speed <= 0x1FFFF)
            entity->ext.skillWalker.speed += 0x1000;
        if (Entity_Walk(entity, -entity->ext.skillWalker.speed, sUnk_084F13B4, 4) & 3)
            entity->flipX ^= 1;
        if (!(entity->timer & 7))
        {
            child = RedSoul_CreateEntity(entity, (u32 *)SkillBiphronAttackUpdate);
            if (child != NULL)
            {
                child->posX.whole = entity->posX.whole;
                child->posY.whole = entity->posY.whole;
                child->flipX = entity->flipX;
            }
        }
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        break;
    }
}
