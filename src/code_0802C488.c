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

/* Inlined Entity_SetAttackBoxFromFrame (src/code_0802D510.c). */
static inline void SetFrameBox(struct EwramData_EntityData *entity)
{
    u8 *spriteSlot = &entity->spriteSlot;
    struct SpriteBox *boxes = gEwramData->spriteSlots[*spriteSlot].sheet->frames[entity->animFrame].boxes;

    if (boxes != NULL)
    {
        entity->hit.flags &= ~2;
        HitBox_SetAttack(entity, boxes);
    }
    else
    {
        entity->hit.flags |= 2;
    }
}

/**
 * @brief 2C488 | Blue Crow red soul: graphics set 31, animation 2, layer
 * 0xD, Hit_SetTimerOverride(1, 20), song 0xEE, box slot 5 (no callback, disabled)
 * with the current frame's hit box. For 51 frames it circles a point 0x12
 * px above the caster facing the other way: angle unk_510 from 0x1000
 * (-0x9000 flipped) turning by unk_512 = -0x147 (mirrored), radius
 * unk_508 = 60 * sin(timer * 0x28F) px.
 */
void SkillBlueCrowUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->unk_4F8.unk_4F8_entity;
    s32 dx, dy;
    s32 spin, angle;

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(31);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 2, 0, 1);
        entity->spriteLayer = 0xD;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Hit_SetTimerOverride(1, 20);
        entity->flipX = parent->flipX;
        entity->unk_510.part16.unk_510 = 0x1000;
        spin = 0xFEB9;
        entity->unk_510.part16.unk_512 = spin;
        if (entity->flipX)
        {
            angle = -0x9000;
            entity->unk_510.part16.unk_510 = angle;
            entity->unk_510.part16.unk_512 = -entity->unk_510.part16.unk_512;
        }
        entity->flipX ^= 1;
        entity->timer = 0x32;
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        PlaySong(0xEE);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        entity->state++;
    case 1:
        entity->unk_508.whole = FIXED_TO_INT(Sin(entity->timer * 0x28F) * 60);
        entity->unk_510.part16.unk_510 += entity->unk_510.part16.unk_512;
        dx = entity->unk_508.whole * Sin(entity->unk_510.part16.unk_510 + 0x4000);
        dy = entity->unk_508.whole * Sin(entity->unk_510.part16.unk_510);
        entity->posX.whole = parent->posX.whole + dx;
        entity->posY.whole = parent->posY.whole + dy - 0x120000;
        if (entity->timer != 0)
            entity->timer--;
        else
            entity->state = 2;
        break;
    case 2:
        gEwramData->playerState.activeRedSoulEntities[entity->unk_4FC.part16.unk_4FE[0]] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    Anim_Update(entity);
    SetFrameBox(entity);
}
