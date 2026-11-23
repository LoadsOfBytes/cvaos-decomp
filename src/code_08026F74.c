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
extern void SkillMermanProjectileUpdate(struct EwramData_EntityData *);

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


static inline void FreeRedSoul(struct EwramData_EntityData *entity)
{
    gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
    gEwramData->playerState.nbrActiveRedSoulEntities--;
    EntityDelete(entity);
}

/**
 * @brief 26F74 | Merman red soul: graphics set 0, animation 0, layer 0xD,
 * Hit_SetTimerOverride(1, 30), placed at the caster (6 / -0x1A), song 0xED; spawns
 * five SkillMermanProjectileUpdate projectiles with speeds 1..5 px (mirrored with the
 * caster), accelX = -velX / 128, gravity 0x2000 and spark frame i + 2; then
 * follows the caster (0 / -0x1A) until its animation ends.
 */
void SkillMermanUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    struct EwramData_EntityData *child;
    s32 i;

    entity->roomActive = 1;
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
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
        Hit_SetTimerOverride(1, 30);
        Entity_PlaceRelativeTo(entity, parent, 6, -0x1A);
        PlaySong(0xED);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        entity->state++;
        for (i = 0; i <= 4; i++)
        {
            child = RedSoul_CreateEntity(entity, (u32 *)SkillMermanProjectileUpdate);
            if (child == NULL)
                continue;
            child->posX.whole = entity->posX.whole;
            child->posY.whole = entity->posY.whole;
            child->velX.whole = (i + 1) << 16;
            child->accelY.whole = 0x2000;
            child->flipX = entity->flipX;
            if (child->flipX)
                child->velX.whole = -child->velX.whole;
            child->accelX.whole = -child->velX.whole / 128;
            child->subState = i + 2;
        }
    case 1:
        Entity_PlaceRelativeTo(entity, parent, 0, -0x1A);
        break;
    case 2:
        FreeRedSoul(entity);
        return;
    }
    if (Anim_Update(entity) == 3)
        FreeRedSoul(entity);
}
