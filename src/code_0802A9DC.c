#include "code_08009A0.h"
#include "soul_skill_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"

#include "structs/entity_render_flags.h"


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
 * @brief 2A9DC | Nightmare red soul: affine graphics set 23 (layer 0xD,
 * flipped against the caster, offset 8 px), moving at 0x4000 / 0x2000 with
 * acceleration 0x80000; sounds 0x19C and 0xE1. Every frame a Effect_SpawnShared
 * (type 7) particle appears at a random spot and drifts at a random angle;
 * the affine scale unk_508 grows by 0x1000 per frame until 0x10000, then the
 * slot is freed and two RedSoul_CreateTrailCopy children (types 7 and 3) are released.
 */
void SkillNightmareUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    struct EwramData_EntityData *child;
    u16 slot;

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
        struct SoulGraphicsResources *t = GetSet(23);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
        Entity_PlaceRelativeTo(entity, parent, 8, 0);
    {
        s32 vx = 0x4000;
        s32 vy;
        entity->velX.whole = vx;
        vy = 0x2000;
        entity->accelX.whole = vy;
        if (entity->flipX)
        {
            entity->velX.whole = -vx;
            entity->accelX.whole = -vy;
        }
    }
        entity->flipX ^= 1;
        Hit_SetTimerOverride(1, 20);
        Hit_Enable(entity, 5, 0, 0);
    {
        u8 zero = 0;
        entity->hit.flags |= 4;
        entity->hit.flags |= zero;
        entity->hit.flags &= ~2;
    }
        PlaySong(0x19C);
        PlaySong(0xE1);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        entity->state = 1;
    case 1:
    {
        s32 r = (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
        s32 x = entity->posX.whole + r;
        s32 y;
        r = (RandomNumberGenerator() & 0x7FFFF) - 0x140000;
        y = entity->posY.whole + r;
        child = Effect_SpawnShared(x, y, 0, 7);
    }
        if (child != NULL)
        {
            u16 angle = (RandomNumberGenerator() & 0x3FFF) - 0x2000;
            if (entity->flipX)
                angle = (u32)((angle << 16) + 0x80000000) >> 16;
            child->accelX.whole = Sin((s16)angle + 0x4000) / 32;
            child->accelY.whole = Sin((s16)angle) / 32;
        }
        if (entity->ext.skillNightmare.scaleX <= 0xFFFF)
        {
            entity->ext.skillNightmare.scaleX += 0x1000;
            ObjAffine_Set(0, entity->ext.skillNightmare.scaleX, 0x10000, entity->flipX, 0, entity->affineIndex);
            EntityAccelerateAndMove(entity, 0x80000);
            break;
        }
        ObjAffine_Free(entity->affineIndex);
        entity->affineIndex = 0;
        entity->affine = 0;
        entity->state = 2;
    {
        struct EwramData_EntityData *c = RedSoul_CreateTrailCopy(entity, 7);
        if (c != NULL)
        {
            c->drawSkip = 1;
            c->objMode = 1;
        }
        c = RedSoul_CreateTrailCopy(entity, 3);
        if (c != NULL)
            c->objMode = 1;
    }
    case 2:
        EntityAccelerateAndMove(entity, 0x80000);
        break;
    }
    Anim_Update(entity);
    SetFrameBox(entity);
    if (EntityIsOutsideScreen(entity, 3) || !entity->roomActive)
    {
        entity->roomActive = 1;
        if (ENTITY_RENDER_FLAGS16(entity) & 0x101F)
            ObjAffine_Free(entity->affineIndex);
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
    }
}
