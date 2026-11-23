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
extern void SkillFlameDemonProjectileUpdate(struct EwramData_EntityData *);

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
 * @brief 293C4 | Flame Demon red soul: graphics set 17 at the caster (8 /
 * -0x10), Hit_SetTimerOverride(1, 4), song 0xD8 and a Effect_SpawnShared flame; fans out
 * 3 (1 in 8: 5) SkillFlameDemonProjectileUpdate fireballs at speed 2 px, starting angle
 * velX = -(0x1000 / n) * (n / 2) and step velY = 0x1000 / n (both kept in
 * the entity's velocity fields), mirrored with the facing, each with
 * acceleration velocity / 64. The original then sets roomActive through the
 * last child pointer (NULL or uninitialised in some paths).
 */
void SkillFlameDemonUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    struct EwramData_EntityData *child;
    s32 i, n;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(17);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        entity->scrollWithBg = 1;
        Entity_PlaceRelativeTo(entity, parent, 8, -0x10);
        Hit_SetTimerOverride(1, 4);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        entity->state++;
        n = 3;
        if ((RandomNumberGenerator() & 7) == 0)
            n = 5;
        entity->velY.whole = -0x1000 / n;
        entity->velX.whole = entity->velY.whole * (n >> 1);
        entity->velY.whole = 0x1000 / n;
        for (i = 0; i < n; i++)
        {
            child = RedSoul_CreateEntity(entity, (u32 *)SkillFlameDemonProjectileUpdate);
            if (child == NULL)
                break;
            Entity_PlaceRelativeTo(child, entity, 0, 0);
            child->velX.whole = Sin(entity->velX.whole + 0x4000) << 1;
            child->velY.whole = Sin(entity->velX.whole) << 1;
            if (child->flipX)
                child->velX.whole = -child->velX.whole;
            child->accelX.whole = child->velX.whole / 64;
            child->accelY.whole = child->velY.whole / 64;
            entity->velX.whole += entity->velY.whole;
        }
        PlaySong(0xD8);
        Effect_SpawnShared(entity->posX.whole, entity->posY.whole, 0xC, 6);
        break;
    case 1:
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    child->roomActive = 1;
}
