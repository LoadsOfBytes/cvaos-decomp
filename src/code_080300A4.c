#include "code_08009A0.h"
#include "blue_soul_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern s32 BlueSoul_ShouldEnd(struct EwramData_EntityData *);

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

static inline void Follow(struct EwramData_EntityData *entity, struct EwramData_EntityData *anchor, u16 offsetX, s32 offsetY)
{
    s16 dx = offsetX;

    entity->flipX = anchor->flipX;
    if (entity->flipX)
        dx = -dx;
    entity->posX.part16.integer = (s32)dx + (s16)anchor->posX.part16.integer;
    entity->posY.part16.integer = anchor->posY.part16.integer + offsetY;
}
extern void BlueSoul_ReleaseActiveResources(struct EwramData_EntityData *);
extern s32 BlueSoul_CheckEndCondition(struct EwramData_EntityData *);
extern void Anim_UpdateWithFrameAttackBox(struct EwramData_EntityData *);
extern void PersephoneSoul_OnHit(struct EwramData_EntityData *);
#include "code_08013960.h"

/**
 * @brief 300A4 | Persephone blue soul (ends via BlueSoul_CheckEndCondition: song 0x9C
 * stopped, 16 frames, then BlueSoul_ReleaseActiveResources): graphics 0x081E0968 / 0x0820B628
 * / 0x08237464, box slot 6 with callback PersephoneSoul_OnHit (disabled),
 * Hit_SetTimerOverride(2, 20), skillBaseAttack/skillAttackAttributes = 10 / 0; keeps song 0x9C playing and
 * stays at the caster (Anim_UpdateWithFrameAttackBox).
 */
void SkillPersephoneUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = BLUE_SOUL_OWNER(entity);

    entity->roomActive = 1;
    {
        u8 three = 3;
        if (entity->state < three && BlueSoul_CheckEndCondition(entity))
        {
            entity->state = three;
            Sound_StopTracked(0x9C);
            entity->drawSkip = 1;
            entity->timer = 0x10;
        }
    }
    switch (entity->state)
    {
    case 0:
        entity->scrollWithBg = 1;
    {
        u8 *graphics = (u8 *)0x081E0968;
        u8 *palette = (u8 *)0x0820B628;
        u8 *sprite = (u8 *)0x08237464;
        LoadGfx(entity, 0, graphics, palette, sprite);
    }
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->spriteLayer = 0xD;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Hit_Enable(entity, 6, PersephoneSoul_OnHit, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        Hit_SetTimerOverride(2, 0x14);
        {
            u16 a = 0xA;
            u16 b = 0;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = b;
        }
        entity->state++;
        Sound_PlayTrackedIfAbsent(0x9C);
    case 1:
        Sound_PlayTrackedIfAbsent(0x9C);
        entity->state++;
    case 2:
        Sound_PlayTrackedIfAbsent(0x9C);
        Follow(entity, parent, 0, 0);
        Anim_UpdateWithFrameAttackBox(entity);
        break;
    case 3:
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        BlueSoul_ReleaseActiveResources(entity);
        EntityDelete(entity);
        break;
    }
}
