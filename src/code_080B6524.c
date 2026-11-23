#include "code_08009A0.h"
#include "balore_entity.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 sBalorePalette[], sBaloreSpriteSheet2[];
extern const u8 sBaloreHandBSteeringParams0[], sBaloreHandBSteeringParams1[], sBaloreHandBSteeringParams2[];
extern s32 ObjPalette_FindOrLoadVariant(void *, s32);
extern s32 Entity_SteerToPoint(struct EwramData_EntityData *, const void *);
extern s32 Music_SetCurrent(s32);
#define ANIM(e) (BALORE_HAND_ANIM((e)))
#define STEP(e) (*(s32 *)&(e)->unk_514)
#define TO_PIXELS(x) ((x) < 0 ? -(-(x) >> 16) : (x) >> 16)

static inline void FadeIn(struct EwramData_EntityData *entity)
{
    u8 pal = entity->unk_510.part8.basePalette;
    /* Reload the byte counter after its decrement, as in the original. */
    s32 level = (*(volatile u8 *)&entity->timer >> 1) + 8; /* FAKEMATCH: volatile read forces the reload of the original */
    u8 index = pal - 10;
    if (gEwramData->gameState.sharedObjPaletteVariants[index] != level)
    {
        gEwramData->gameState.sharedObjPaletteVariants[index] = level;
        ObjPalette_LoadRange((void *)gEwramData->gameState.sharedObjPaletteSources[index], level, 1, pal);
    }
}

static inline void FadeOut(struct EwramData_EntityData *entity)
{
    u8 pal = entity->unk_510.part8.basePalette;
    u8 level = 15 - (*(volatile u8 *)&entity->timer >> 3); /* FAKEMATCH: volatile read forces the reload of the original */
    u8 index = pal - 10;
    if (gEwramData->gameState.sharedObjPaletteVariants[index] != level)
    {
        gEwramData->gameState.sharedObjPaletteVariants[index] = level;
        ObjPalette_LoadRange((void *)gEwramData->gameState.sharedObjPaletteSources[index], level, 1, pal);
    }
}

static inline s32 ToPixels(s32 x) { return TO_PIXELS(x); }
/* Second family of Balore hand: fade, extend, attack and retract. */
void BaloreHandB_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *body = BALORE_BODY(entity);
    u16 matrix;
    s32 scale;
    if (body->state == 10) entity->state = 5;
    switch (entity->state)
    {
    case 0:
        matrix = ObjAffine_Alloc(1);
        if ((s16)matrix < 0) goto delete;
        entity->affineIndex = matrix;
        entity->affine = 1;
        entity->spriteLayer = 0x1A;
        entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sBalorePalette, 15);
        Sprite_Attach(entity, sBaloreSpriteSheet2, (s8)BALORE_GFX(body), entity->unk_510.part8.basePalette);
        Anim_StartIndexed(entity, 7, 2, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        ANIM(entity) = 10;
        BALORE_ARM_LENGTH(entity) = 0x260000;
        STEP(entity) = 0x80000;
        entity->timer = 16;
        entity->state = 1;
    case 1:
        if (entity->timer != 0)
        {
            entity->timer--;
            FadeIn(entity);
        }
        if (Entity_SteerToPoint(entity, sBaloreHandBSteeringParams0))
        {
            Music_SetCurrent(17);
            ANIM(entity) = 9;
            entity->timer = 8;
            entity->state = 2;
        }
        break;
    case 2:
        if (--entity->timer == 0)
        {
            PlaySong(0x9F);
            ANIM(entity) = 8;
        }
        if (Entity_SteerToPoint(entity, sBaloreHandBSteeringParams1))
        {
            PlaySong(0x9F);
            entity->state = 3;
        }
        break;
    case 3:
        if (body->state == 8)
        {
            STEP(entity) = 0x8000;
            entity->timer = 64;
            entity->state = 4;
        }
        break;
    case 4:
        if (entity->timer != 0)
        {
            entity->timer--;
            FadeOut(entity);
        }
        if (Entity_SteerToPoint(entity, sBaloreHandBSteeringParams2))
        {
            entity->hidden = 1;
            entity->state = 5;
        }
        break;
    case 5:
        if (entity->affine) ObjAffine_Free(entity->affineIndex);
    delete:
        EntityDelete(entity);
        return;
    }
    if (entity->hidden) return;
    scale = (s32)((u32)ToPixels(BALORE_ARM_LENGTH(entity)) << 16) / 76;
    BALORE_ARM_SCALE(entity) = scale;
    ObjAffine_Set(BALORE_ARM_ANGLE(entity), scale, 0x10000, entity->flipX, 0, entity->affineIndex);
    Anim_Update(entity);
}
