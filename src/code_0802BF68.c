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
extern void SkillFlameDemonUpdate(struct EwramData_EntityData *);

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
 * @brief 2BF68 | Demon Lord red soul: graphics set 28, animation 0, layer
 * 0xD, Hit_SetTimerOverride(1, 15), song 0x1A4, placed at the caster (8 / -0x10);
 * on the 29th frame spawn a SkillFlameDemonUpdate fireball (song 0x12A,
 * offset 0xA / -2, velX 0x18000, accelX 0x400, both negated when flipped);
 * ends with the animation. The original sets roomActive through the child
 * pointer, which is uninitialised when no fireball was spawned this frame.
 */
void SkillDemonLordUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = RED_SOUL_OWNER(entity);
    struct EwramData_EntityData *child;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(28);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->spriteLayer = 0xD;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        Hit_SetTimerOverride(1, 15);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0x1A4);
        entity->state++;
        Entity_PlaceRelativeTo(entity, parent, 8, -0x10);
    case 1:
        if (++entity->timer == 0x1D)
        {
            child = RedSoul_CreateEntity(entity, (u32 *)SkillFlameDemonUpdate);
            if (child != NULL)
            {
                s32 speed, accel;
                PlaySong(0x12A);
                Entity_PlaceRelativeTo(child, entity, 0xA, -2);
                speed = 0x18000;
                child->velX.whole = speed;
                accel = 0x400;
                child->accelX.whole = accel;
                if (child->flipX)
                {
                    child->velX.whole = -speed;
                    child->accelX.whole = -accel;
                }
            }
        }
        if (Anim_Update(entity) == 3)
        {
            gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            return;
        }
    }
    child->roomActive = 1;
}
