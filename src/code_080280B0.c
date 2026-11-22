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
extern void SoulAttack_MarkHit(struct EwramData_EntityData *);
extern void SkillNeedlesDraw(struct EwramData_EntityData *);

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


struct Unk_08028018 {
    s16 radius;
    s16 angle;
};

/* Four (radius 8.8, angle) pairs at entity offset 0x20, drawn by
 * SkillNeedlesDraw (src/code_08028018.c). */
struct Unk_08028018_Block {
    u8 pad_0[0x10];
    struct Unk_08028018 parts[4];
};

/**
 * @brief 280B0 | Needles red soul: graphics set 12, animation 0, layer
 * 0xD, drawn by SkillNeedlesDraw, at the caster (0x14 / -0x1A), box 0x1010F8F8
 * in slot 5 with SoulAttack_MarkHit on hit, Hit_SetTimerOverride(1, 15), song 0xE0. On a
 * hit or after 128 frames it bursts (song 0xD7, Effect_SpawnShared flame): hops
 * up with a random velY and gravity 0x2000 while four needles spread out
 * at angles 0xF000 + i * 0x3333 (+ random), radius growing by timer * 32
 * for 33 frames.
 */
void SkillNeedlesUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->unk_4F8.unk_4F8_entity;
    struct Unk_08028018_Block *block;
    s32 i;
    s32 angle;

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(12);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->spriteLayer = 0xD;
        entity->drawFunc = (u32 *)SkillNeedlesDraw;
        Hit_SetTimerOverride(1, 15);
        Entity_PlaceRelativeTo(entity, parent, 0x14, -0x1A);
        entity->scrollWithBg = 1;
        Hit_Enable(entity, 5, SoulAttack_MarkHit, 0);
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->hit.flags |= 4;
        entity->timer = 0x80;
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0xE0);
        entity->state = 1;
    case 1:
        Anim_Update(entity);
        if (--entity->timer == 0)
            entity->unk_4FC.part8.unk_4FC = 1;
        if ((u8)entity->unk_4FC.part8.unk_4FC == 0)
            break;
        entity->velY.whole = -0x10000 - (RandomNumberGenerator() & 0x7FFF);
        entity->accelY.whole = 0x2000;
        Effect_SpawnShared(entity->posX.whole, entity->posY.whole, 0xF, 7);
        block = (struct Unk_08028018_Block *)&entity->unk_4F4;
        for (i = 0, angle = 0; i <= 3; i++)
        {
            block->parts[i].radius = 0;
            block->parts[i].angle = angle + 0xF000 + (RandomNumberGenerator() & 0x3FF);
            angle += 0x3333;
        }
        PlaySong(0xD7);
        entity->timer = 0x20;
        entity->state = 2;
        break;
    case 2:
        EntityAccelerateAndMoveY(entity, 0x80000);
        block = (struct Unk_08028018_Block *)&entity->unk_4F4;
        for (i = 0; i <= 3; i++)
            block->parts[i].radius += entity->timer << 5;
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
    case 3:
        gEwramData->playerState.activeRedSoulEntities[entity->unk_4FC.part16.unk_4FE[0]] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        break;
    }
    if (EntityIsOutsideScreen(entity, 3) || !entity->roomActive)
    {
        entity->roomActive = 1;
        entity->state = 3;
    }
}
