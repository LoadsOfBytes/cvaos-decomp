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
extern void SkillRedMinotaurHitBoxUpdate(struct EwramData_EntityData *);

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
 * @brief 269BC | Red Minotaur red soul: an affine axe (graphics set 5,
 * frame 0) at the caster (0 / -0x1A) with the orbiting hit box
 * SkillRedMinotaurHitBoxUpdate; songs 0x1A0 and 0xCF. Angle unk_510 starts at 0x4000 with
 * turn speed unk_512 = -0x800 and acceleration unk_50C = 0x40 (all mirrored
 * when flipped); the scale unk_508 grows by 0x2000 up to 0x20000, the swing
 * lasts 0x50 frames, then it decelerates by 2 * unk_50C while shrinking.
 */
void SkillRedMinotaurUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    struct EwramData_EntityData *child;
    s32 zero;
    u16 slot;
    s32 angle, speed, accel;

    switch (entity->state)
    {
    case 0:
        zero = 0;
    {
        struct SoulGraphicsResources *t = GetSet(5);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        entity->animFrame = zero;
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = (void *)zero;
            EntityDelete(entity);
            return;
        }
        child = RedSoul_CreateEntity(entity, (u32 *)SkillRedMinotaurHitBoxUpdate);
        if (child == NULL)
        {
            ObjAffine_Free(entity->affineIndex);
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = child;
            EntityDelete(entity);
            return;
        }
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->doubleSize = 1;
        Entity_PlaceRelativeTo(entity, parent, 0, -0x1A);
        entity->scrollWithBg = 1;
        entity->state++;
        angle = 0x4000;
        SKILL_RED_MINOTAUR_ANGLE(entity) = angle;
        speed = 0xF800;
        SKILL_RED_MINOTAUR_SWING_SPEED(entity) = speed;
        accel = 0x40;
        entity->ext.skillRedMinotaur.swingAccel = accel;
        if (entity->flipX)
        {
            SKILL_RED_MINOTAUR_ANGLE(entity) = -angle;
            SKILL_RED_MINOTAUR_SWING_SPEED(entity) = -speed;
            entity->ext.skillRedMinotaur.swingAccel = -accel;
        }
        entity->timer = 0x50;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0x1A0);
        PlaySong(0xCF);
    case 1:
        if (entity->ext.skillRedMinotaur.scale <= 0x1FFFF)
            entity->ext.skillRedMinotaur.scale += 0x2000;
        else
            entity->state = 2;
    case 2:
        SKILL_RED_MINOTAUR_ANGLE(entity) += SKILL_RED_MINOTAUR_SWING_SPEED(entity);
        SKILL_RED_MINOTAUR_SWING_SPEED(entity) += entity->ext.skillRedMinotaur.swingAccel;
        if (entity->timer != 0)
            entity->timer--;
        else
            entity->state = 3;
        break;
    case 3:
        SKILL_RED_MINOTAUR_ANGLE(entity) += SKILL_RED_MINOTAUR_SWING_SPEED(entity);
        SKILL_RED_MINOTAUR_SWING_SPEED(entity) -= entity->ext.skillRedMinotaur.swingAccel * 2;
        if (entity->ext.skillRedMinotaur.scale > 0)
            entity->ext.skillRedMinotaur.scale -= 0x2000;
        else
            entity->state = 4;
        break;
    case 4:
        ObjAffine_Free(entity->affineIndex);
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 4) || !entity->roomActive)
    {
        entity->roomActive = 1;
        entity->state = 4;
        entity->hidden = 1;
    }
    ObjAffine_Set(SKILL_RED_MINOTAUR_ANGLE(entity), entity->ext.skillRedMinotaur.scale, entity->ext.skillRedMinotaur.scale, entity->flipX, 0, entity->affineIndex);
}
