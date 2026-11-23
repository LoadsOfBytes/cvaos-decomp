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
extern void SkillCockatriceEffectUpdate(struct EwramData_EntityData *);
extern void SkillCockatriceDraw(struct EwramData_EntityData *);
#include "code_08013960.h"

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


#define FIXED_TO_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)

/**
 * @brief 2968C | Cockatrice red soul: an affine breath (graphics set 27,
 * frame 1, layer 0xD, drawn by SkillCockatriceDraw) at pixel offset (8 mirrored,
 * -0x1A) from the caster, with the SkillCockatriceEffectUpdate child, Hit_SetTimerOverride(1, 60),
 * box slot 5, sound 0xCD while active. For 33 frames the angle unk_510
 * (0x2800, mirrored) turns by unk_512 = -0x40, the vertical scale is
 * sin(timer << 10) and the box (16 x 16) is moved along the angle by
 * (timer & 7) * 32 * sin/cos.
 */
void SkillCockatriceUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    u16 slot;
    u16 bx, by;

    switch (entity->state)
    {
    case 0:
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
            EntityDelete(entity);
            return;
        }
        entity->affineIndex = slot;
        entity->affine = 1;
    {
        struct SoulGraphicsResources *t = GetSet(27);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        entity->scrollWithBg = 1;
        entity->animFrame = 1;
        entity->drawFunc = (u32 *)SkillCockatriceDraw;
        entity->spriteLayer = 0xD;
        SKILL_COCKATRICE_ANGLE(entity) = 0x2800;
        SKILL_COCKATRICE_TURN(entity) = -0x40;
        entity->velX.part16.integer = 8;
        entity->velY.part16.integer = -0x1A;
        if (parent->flipX)
        {
            entity->velX.part16.integer = -entity->velX.part16.integer;
            SKILL_COCKATRICE_ANGLE(entity) = -0x8000 - SKILL_COCKATRICE_ANGLE(entity);
            SKILL_COCKATRICE_TURN(entity) = -SKILL_COCKATRICE_TURN(entity);
        }
        entity->flipX = 0;
        Hit_SetTimerOverride(1, 60);
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags |= 4;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        Sound_PlayTracked(0xCD);
        entity->timer = 0x20;
        entity->state++;
        RedSoul_CreateEntity(entity, (u32 *)SkillCockatriceEffectUpdate);
    case 1:
        entity->ext.skillCockatrice.scaleY = Sin(entity->timer << 10);
        SKILL_COCKATRICE_ANGLE(entity) += SKILL_COCKATRICE_TURN(entity);
        entity->posX.whole = parent->posX.whole + entity->velX.whole;
        entity->posY.whole = parent->posY.whole + entity->velY.whole;
        entity->accelX.whole = Sin(SKILL_COCKATRICE_ANGLE(entity) + 0x4000) << 5;
        entity->accelY.whole = Sin(SKILL_COCKATRICE_ANGLE(entity)) << 5;
        bx = FIXED_TO_INT(entity->accelX.whole) * (entity->timer & 7);
        by = FIXED_TO_INT(entity->accelY.whole) * (entity->timer & 7);
        HitBox_SetAttackRaw(entity, (((s16)bx - 8) & 0xFF) + ((((s16)by - 8) & 0xFF) << 8) + 0x10100000);
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
    case 2:
        Sound_StopTracked(0xCD);
        ObjAffine_Free(entity->affineIndex);
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    ObjAffine_Set(SKILL_COCKATRICE_ANGLE(entity), 0x10000, entity->ext.skillCockatrice.scaleY, 0, 0, entity->affineIndex);
    if (EntityIsOutsideScreen(entity, 2) || !entity->roomActive)
    {
        entity->roomActive = 1;
        Sound_StopTracked(0xCD);
        ObjAffine_Free(entity->affineIndex);
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
    }
}
