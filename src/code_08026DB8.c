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
extern void SkillMermanDropletUpdate(struct EwramData_EntityData *);

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
 * @brief Child of SkillMermanUse: projectile (graphics set 0, animation 6,
 * box 0x1010F8F8) accelerating horizontally for 16 updates and dropping
 * one falling spark, then moving without its box until off screen.
 */
void SkillMermanProjectileUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;

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
        Anim_StartIndexed(entity, 6, 0, 1);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        Hit_Enable(entity, 5, 0, 0);
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->hit.flags |= 4;
        entity->timer = 0x10;
        entity->state++;
    case 1:
        EntityAccelerateAndMoveX(entity, 0x80000);
        if (entity->timer != 0)
        {
            if (entity->timer == entity->subState)
            {
                child = EntityCreateInRange(0x49, 0xCF, (u32 *)SkillMermanDropletUpdate);
                if (child != NULL)
                {
                    child->scrollWithBg = 1;
                    child->posX.whole = entity->posX.whole;
                    child->posY.whole = entity->posY.whole;
                    child->velX.whole = entity->velX.whole;
                    child->velY.whole = ((RandomNumberGenerator() & 0xF) << 14) - 0x38000;
                    child->accelY.whole = 0x2000;
                }
            }
            entity->timer--;
            break;
        }
        entity->hit.flags |= 2;
        entity->state = 2;
    case 2:
        EntityAccelerateAndMove(entity, 0x80000);
        break;
    case 3:
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 3) || !entity->roomActive)
    {
        entity->roomActive = 1;
        entity->state = 3;
        entity->hidden = 1;
    }
    Anim_Update(entity);
}
