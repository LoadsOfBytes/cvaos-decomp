#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern s32 BlueSoul_ShouldEnd(struct EwramData_EntityData *);
extern void Anim_UpdateWithFrameAttackBox(struct EwramData_EntityData *);

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

/**
 * @brief 31ACC | Big Golem blue soul, same frame as SkillShadowKnightUse:
 * graphics 0x0825FB1C / 0x082664B8 / 0x082677B8, skillBaseAttack/skillAttackAttributes = 0x1E / 0,
 * song 0x1A6; animation 1 while the caster's weaponEntities[unk_50C] slot is
 * taken (song 0xC5 on frame 4), back to animation 0 when it ends; held
 * 12 px behind and 4 px above the caster.
 */
void SkillBigGolemUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->unk_4F8.unk_4F8_entity;
    u16 slot;

    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
    {
        u8 *graphics = (u8 *)0x0825FB1C;
        u8 *palette = (u8 *)0x082664B8;
        u8 *sprite = (u8 *)0x082677B8;
        LoadGfx(entity, 0, graphics, palette, sprite);
    }
        Anim_StartIndexed(entity, 0, 0, 1);
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            EndBlueSoul(entity);
            return;
        }
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->shrinkPartOffsets = 1;
        entity->spriteLayer = 0xF;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        entity->state++;
        entity->timer = 0x20;
        Hit_Enable(entity, 6, 0, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        Hit_SetTimerOverride(2, 0x3C);
        {
            u16 a = 0x1E;
            u16 b = 0;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = b;
        }
        PlaySong(0x1A6);
    case 1:
        if (entity->unk_50C.whole <= 0xFFFF)
            entity->unk_50C.whole += 0x1000;
        if (entity->unk_510.whole <= 0xFFFF)
            entity->unk_510.whole += 0x800;
        ObjAffine_Set(0, entity->unk_50C.whole, entity->unk_510.whole, entity->flipX, 0, entity->affineIndex);
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        ObjAffine_Free(entity->affineIndex);
        entity->affine = 0;
        entity->affineIndex = 0;
        entity->state++;
    case 2:
        if (gEwramData->playerState.weaponEntities[parent->unk_50C.part8.unk_50C] != NULL)
        {
            Anim_StartIndexed(entity, 1, 0, 0);
            entity->state = 3;
        }
        break;
    case 3:
        if (*(u16 *)&entity->animStep == 4)
            PlaySong(0xC5);
        if (entity->animFlags & 4)
        {
            entity->state = 2;
            Anim_StartIndexed(entity, 0, 0, 1);
        }
        break;
    }
    {
        s32 dx = -0xC;
        entity->flipX = parent->flipX;
        if (entity->flipX)
            dx = 0xC;
        entity->posX.part16.integer = parent->posX.part16.integer + dx;
        entity->posY.part16.integer = parent->posY.part16.integer - 4;
    }
    Anim_UpdateWithFrameAttackBox(entity);
}
