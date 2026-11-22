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
extern void SkillDiscArmorScaledSpriteUpdate(struct EwramData_EntityData *);

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
 * @brief 27340 | Disc Armor red soul: affine disc (graphics set 7, frame 1,
 * layer 0xD, box 0x2020F0F0 in slot 5, Hit_SetTimerOverride(1, 10), song 0xD4) with
 * the SkillDiscArmorScaledSpriteUpdate trail. For 27 frames it circles a point 0x18 px above the
 * caster: angle unk_510 from 0x2000 (0x6000 flipped) turning by unk_512 =
 * -0x276 (mirrored), radius unk_508 = 60 * sin(timer * 1260) px; it spins
 * by unk_514 += 0x1B00 and is scaled by the same sine (unk_50C).
 */
void SkillDiscArmorUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = RED_SOUL_OWNER(entity);
    u16 slot;
    s32 dx, dy;
    s32 scale, rot;

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
        entity->animFrame = 1;
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
            EntityDelete(entity);
            return;
        }
        entity->scrollWithBg = 1;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->flipX = parent->flipX ? 1 : 0;
        SKILL_DISC_ARMOR_ANGLE(entity) = 0x2000;
        SKILL_DISC_ARMOR_TURN(entity) = -0x276;
        if (entity->flipX)
        {
            SKILL_DISC_ARMOR_ANGLE(entity) = 0x6000;
            SKILL_DISC_ARMOR_TURN(entity) = -SKILL_DISC_ARMOR_TURN(entity);
        }
        Hit_SetTimerOverride(1, 10);
        Hit_Enable(entity, 5, 0, 0);
        HitBox_SetAttackRaw(entity, 0x2020F0F0);
        entity->hit.flags |= 4;
        entity->timer = 0x1A;
        entity->state = 1;
        PlaySong(0xD4);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        RedSoul_CreateEntity(entity, (u32 *)SkillDiscArmorScaledSpriteUpdate);
    case 1:
        entity->accelX.whole = parent->posX.whole;
        entity->accelY.whole = parent->posY.whole - 0x180000;
        SKILL_DISC_ARMOR_RADIUS(entity) = FIXED_TO_INT(Sin(entity->timer * 1260) * 60);
        SKILL_DISC_ARMOR_ANGLE(entity) += SKILL_DISC_ARMOR_TURN(entity);
        entity->unk_514.part16.spawnParam0 += 0x1B00;
        dx = SKILL_DISC_ARMOR_RADIUS(entity) * Sin(SKILL_DISC_ARMOR_ANGLE(entity) + 0x4000);
        dy = SKILL_DISC_ARMOR_RADIUS(entity) * Sin(SKILL_DISC_ARMOR_ANGLE(entity));
        entity->posX.whole = entity->accelX.whole + dx;
        entity->posY.whole = entity->accelY.whole + dy;
        if (entity->timer != 0)
            entity->timer--;
        else
            entity->state = 2;
        break;
    case 2:
        ObjAffine_Free(entity->affineIndex);
        gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    SKILL_DISC_ARMOR_SCALE(entity) = Sin(entity->timer * 1260);
    rot = entity->unk_514.part16.spawnParam0;
    if (entity->flipX)
        rot = -rot;
    ObjAffine_Set(rot, SKILL_DISC_ARMOR_SCALE(entity), SKILL_DISC_ARMOR_SCALE(entity), entity->flipX, 0, entity->affineIndex);
}
