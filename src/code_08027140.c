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
extern void Entity_DrawWithVelocityCopies(struct EwramData_EntityData *);
#include "syscalls.h"

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
 * @brief Child of SkillDiscArmorUse: affine sprite (graphics set 7, frame 2)
 * at the owner target point, moving along the owner angle and drawn
 * rotated by it and stretched by the owner distance (at most 64 px).
 * Ends with owner state 2; the scale is uninitialised outside state 1.
 */
void SkillDiscArmorScaledSpriteUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = RED_SOUL_OWNER(entity);
    s32 scale;
    u16 slot;
    s32 dx, dy, d;

    entity->roomActive = 1;
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
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            return;
        }
        entity->animFrame = 2;
        entity->drawFunc = (u32 *)Entity_DrawWithVelocityCopies;
        entity->spriteLayer = 0xD;
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->flipX = parent->flipX ? 1 : 0;
        entity->state = 1;
    case 1:
        entity->posX.whole = parent->accelX.whole;
        entity->posY.whole = parent->accelY.whole;
        dx = (entity->posX.whole - parent->posX.whole) >> 16;
        dx *= dx;
        dy = (entity->posY.whole - parent->posY.whole) >> 16;
        dy *= dy;
        d = Sqrt(dx + dy) << 16;
        if (d > 0x400000)
            d = 0x400000;
        scale = (d >> 1) / 32;
        entity->velX.whole = Sin(SKILL_DISC_ARMOR_ANGLE(parent) + 0x4000) << 5;
        entity->velY.whole = Sin(SKILL_DISC_ARMOR_ANGLE(parent)) << 5;
        entity->velX.whole = entity->velX.whole / 0x10000 * scale;
        entity->velY.whole = entity->velY.whole / 0x10000 * scale;
        if (parent->state != 2)
            break;
    case 2:
        ObjAffine_Free(entity->affineIndex);
        gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    ObjAffine_Set(SKILL_DISC_ARMOR_ANGLE(parent), scale, 0x10000, entity->flipX, 0, entity->affineIndex);
}
