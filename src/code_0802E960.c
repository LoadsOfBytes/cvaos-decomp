#include "code_08009A0.h"
#include "soul_skill_entity.h"
#include "blue_soul_entity.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern s32 BlueSoul_ShouldEnd(struct EwramData_EntityData *);
extern void BlueSoul_ReleaseActiveResources(struct EwramData_EntityData *);

static inline void LoadGfx(struct EwramData_EntityData *entity, u8 paletteSlot, void *gfxSrc, void *palette, void *sheet)
{
    gEwramData->playerState.blueSoulGfx = ObjGfx_LoadResource(gfxSrc, 0x10000);
    ObjPalette_LoadRange(palette, paletteSlot, 1, 3);
    Sprite_Attach(entity, sheet, gEwramData->playerState.blueSoulGfx, 3);
}

/* Same as BlueSoul_DeleteEntity: end a blue soul entity. */
static inline void EndBlueSoul(struct EwramData_EntityData *entity)
{
    if (entity->affineIndex != 0 || entity->affine)
        ObjAffine_Free(entity->affineIndex);
    gEwramData->playerState.activeBlueSoulEntities[BLUE_SOUL_SLOT(entity)] = NULL;
    EntityDelete(entity);
}

/**
 * @brief 2E960 | Alura Une blue soul: ends (BlueSoul_ReleaseActiveResources) when HP is full
 * or via BlueSoul_ShouldEnd; graphics 0x081DE154 / 0x0820B574 / 0x082368A8 in an
 * affine slot (ObjAffine_Alloc, freed like BlueSoul_DeleteEntity when none is free),
 * held 16 px behind and 26 px above the caster while its affine scale grows
 * (x by 0x1000, y by 0x2000 per frame up to 0x10000). Every 17 frames: song
 * 0x140 and Player_RestoreHP(10) (heal); at full HP 1 in 8 plays song 0x1AC.
 */
void SkillAluraUneUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = BLUE_SOUL_OWNER(entity);
    u16 slot;

    entity->roomActive = 1;
    if (gEwramData->playerStatus.currentHP >= gEwramData->playerStatus.maxHP)
    {
        BlueSoul_ReleaseActiveResources(entity);
        EntityDelete(entity);
        return;
    }
    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
    {
        u8 *graphics = (u8 *)0x081DE154;
        u8 *palette = (u8 *)0x0820B574;
        u8 *sprite = (u8 *)0x082368A8;
        LoadGfx(entity, 0, graphics, palette, sprite);
    }
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->spriteLayer = 0xD;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            EndBlueSoul(entity);
            return;
        }
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->state++;
    case 1:
    {
        s32 dx = -0x10;
        entity->flipX = parent->flipX;
        if (entity->flipX)
            dx = 0x10;
        entity->posX.part16.integer = parent->posX.part16.integer + dx;
        entity->posY.part16.integer = parent->posY.part16.integer - 0x1A;
    }
        if (SKILL_ALURA_UNE_SCALE_X(entity) <= 0xFFFF)
            SKILL_ALURA_UNE_SCALE_X(entity) += 0x1000;
        if (SKILL_ALURA_UNE_SCALE_Y(entity) <= 0xFFFF)
            SKILL_ALURA_UNE_SCALE_Y(entity) += 0x2000;
    }
    ObjAffine_Set(0, SKILL_ALURA_UNE_SCALE_X(entity), SKILL_ALURA_UNE_SCALE_Y(entity), entity->flipX, 0, entity->affineIndex);
    if (entity->timer++ > 15)
    {
        entity->timer = 0;
        PlaySong(0x140);
        Player_RestoreHP(10);
        if (gEwramData->playerStatus.currentHP >= gEwramData->playerStatus.maxHP && (RandomNumberGenerator() & 0x3F) <= 7)
            PlaySong(0x1AC);
    }
}
