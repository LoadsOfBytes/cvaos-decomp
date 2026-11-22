#include "code_08009A0.h"
#include "code_080D73B8.h"
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
    gEwramData->playerState.activeBlueSoulEntities[entity->unk_508.part16.unk_508] = NULL;
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
extern void SkillWitchGiantGhost_OnContact(struct EwramData_EntityData *);
extern void BlueSoul_DrawAlternatingLayer(struct EwramData_EntityData *);

/**
 * @brief 2F96C | Witch / Giant Ghost blue soul (unk_500 = 0 / 1, ends via
 * BlueSoul_CheckEndCondition): affine slot, box slot 7 with callback SkillWitchGiantGhost_OnContact,
 * Hit_SetTimerOverride(2, 10), skillBaseAttack/skillAttackAttributes = 0x1E / 0x20. Witch: graphics
 * 0x081DE958 (layer 0xD, box 0x2010E010, song 0x97); Giant Ghost: graphics
 * 0x081E0968 drawn by BlueSoul_DrawAlternatingLayer (layer 0xE, box 0x2830D8E8, song 0x8D).
 * Grows (scale unk_50C += 0x2000 up to 0x10000) at the caster, then on the
 * end shrinks by 0x2000 per frame and calls BlueSoul_ReleaseActiveResources.
 */
void Skill_Witch_GiantGhost_Use(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->unk_4F8.unk_4F8_entity;
    u16 slot;

    entity->roomActive = 1;
    {
        u8 three = 3;
        if (entity->state < three && BlueSoul_CheckEndCondition(entity))
        {
            entity->state = three;
            entity->hit.flags |= 2;
        }
    }
    switch (entity->state)
    {
    case 0:
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            EndBlueSoul(entity);
            return;
        }
        entity->scrollWithBg = 1;
        Hit_SetTimerOverride(2, 10);
        Hit_Enable(entity, 7, SkillWitchGiantGhost_OnContact, 0);
        entity->hit.flags |= 2;
        entity->hit.flags |= 4;
        if (entity->unk_500.whole == 0)
        {
            u8 *graphics = (u8 *)0x081DE958;
            u8 *palette = (u8 *)0x0820B598;
            u8 *sprite = (u8 *)0x0823693C;
            LoadGfx(entity, 0, graphics, palette, sprite);
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
            entity->spriteLayer = 0xD;
            Anim_StartIndexed(entity, 0, 0, 1);
            HitBox_SetAttackRaw(entity, 0x2010E010);
            PlaySong(0x97);
        }
        else
        {
            u8 *graphics = (u8 *)0x081E0968;
            u8 *palette = (u8 *)0x0820B628;
            u8 *sprite = (u8 *)0x08237464;
            LoadGfx(entity, 0, graphics, palette, sprite);
            entity->drawFunc = (u32 *)BlueSoul_DrawAlternatingLayer;
            entity->spriteLayer = 0xE;
            Anim_StartIndexed(entity, 1, 0, 1);
            HitBox_SetAttackRaw(entity, 0x2830D8E8);
            PlaySong(0x8D);
        }
        entity->affineIndex = slot;
        entity->shrinkPartOffsets = 1;
        entity->affine = 1;
        {
            u16 a = 0x1E;
            u16 b = 0x20;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = b;
        }
        entity->state++;
    case 1:
        if (entity->unk_50C.whole <= 0xFFFF)
        {
            entity->unk_50C.whole += 0x2000;
            break;
        }
        entity->affine = 0;
        entity->state++;
        entity->hit.flags &= ~2;
        break;
    case 3:
        entity->affine = 1;
        if (entity->unk_50C.whole <= 0)
        {
            BlueSoul_ReleaseActiveResources(entity);
            EntityDelete(entity);
            return;
        }
        entity->unk_50C.whole -= 0x2000;
        break;
    }
    entity->palette = 3;
    Follow(entity, parent, 0, 0);
    ObjAffine_Set(0, entity->unk_50C.whole, entity->unk_50C.whole, entity->flipX, 0, entity->affineIndex);
    Anim_Update(entity);
}
