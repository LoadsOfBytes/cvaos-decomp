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
extern void RedSoul_AnchorToHitTargetTimed(struct EwramData_EntityData *, struct EwramData_EntityData *);

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
 * @brief 282DC | Dryad red soul: a seed (graphics set 13, frame 0, layer
 * 0xD) thrown from the caster (0x14 / -0x1A) with velX 0x10000 (mirrored),
 * velY -0x20000 and gravity 0x2000, box 0x0202FFFF in slot 5 with
 * RedSoul_AnchorToHitTargetTimed on hit (linked entity in unk_504), Hit_SetTimerOverride(1, 20), song
 * 0xE5. After a hit it plays animation 0 (song 0xD5 and Player_RestoreHP(8) on
 * the first frame) for up to 21 frames while the link holds and the target
 * keeps its box, then stays 16 frames with drawSkip set.
 */
void SkillDryadUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    struct EwramData_EntityData *link;
    struct EwramData_EntityData *target;
    s32 zero;
    s32 r;

    switch (entity->state)
    {
    case 0:
        zero = 0;
    {
        struct SoulGraphicsResources *t = GetSet(13);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        entity->animFrame = zero;
        entity->spriteLayer = 0xD;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        entity->roomActive = 1;
        Hit_SetTimerOverride(1, 20);
        Hit_Enable(entity, 5, RedSoul_AnchorToHitTargetTimed, 0);
        HitBox_SetAttackRaw(entity, 0x0202FFFF);
        entity->hit.flags |= 4;
        Entity_PlaceRelativeTo(entity, parent, 0x14, -0x1A);
        entity->velY.whole = -0x20000;
        entity->accelY.whole = 0x2000;
        entity->velX.whole = 0x10000;
        if (entity->flipX)
            entity->velX.whole = -entity->velX.whole;
        entity->state = 1;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0xE5);
    case 1:
        EntityAccelerateAndMove(entity, 0x40000);
        if (entity->ext.redSoul.anchor == NULL)
            break;
        entity->state = 2;
        Anim_StartIndexed(entity, 0, 0, 0);
    case 2:
        link = entity->ext.redSoul.anchor;
        if (link == NULL)
            goto animate;
        if (link->ext.redSoulAnchor.targetDead != 0)
            goto stop;
        target = link->ext.redSoulAnchor.target;
        if (entity->ext.redSoul.anchorTimer == 0)
        {
            PlaySong(0xD5);
            Player_RestoreHP(8);
        }
        if (entity->ext.redSoul.anchorTimer++ > 0x14)
            goto stop;
        if (!(target->hit.flags & 4))
            goto animate;
        entity->state = 4;
        break;
    stop:
        entity->state = 4;
        break;
    animate:
        r = Anim_Update(entity);
        if (r == 3)
        {
            entity->timer = 0x10;
            entity->state = r;
        }
        break;
    case 3:
        entity->drawSkip = 1;
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
    case 4:
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 4) || !entity->roomActive)
    {
        entity->roomActive = 1;
        entity->state = 4;
    }
}
