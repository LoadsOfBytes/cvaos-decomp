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
extern void RedSoul_AnchorToHitTarget(struct EwramData_EntityData *, struct EwramData_EntityData *);

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
 * @brief Child of SkillSkullArcherUse: arrow (graphics set 18, frame 1,
 * velX 0x60000, box 0x0202FF08) whose hit callback links a follower
 * entity; after a hit it stays 16 updates hidden, also ends off screen.
 */
void SkillSkullArcherArrowUpdate(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(18);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        entity->animFrame = 1;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        Hit_Enable(entity, 5, RedSoul_AnchorToHitTarget, 0);
        HitBox_SetAttackRaw(entity, 0x0202FF08);
        entity->hit.flags |= 4;
        entity->velX.whole = 0x60000;
        if (entity->flipX)
            entity->velX.whole = -entity->velX.whole;
        PlaySong(0xC8);
        entity->state++;
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        if (entity->ext.redSoul.anchor != NULL)
        {
            entity->state = 2;
            entity->timer = 0x10;
            entity->drawSkip = 1;
        }
        break;
    case 2:
        if (entity->ext.redSoul.anchor->ext.redSoulAnchor.targetDead != 0)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            return;
        }
        if (entity->timer == 0)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
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
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
    }
}
