#include "code_08009A0.h"
#include "soul_skill_entity.h"
#include "player_entity.h"
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
    gEwramData->playerState.activeBlueSoulEntities[entity->ext.blueSoul.slot] = NULL;
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
 * @brief 2F548 | Shadow Knight blue soul (ends via BlueSoul_ShouldEnd): graphics
 * 0x081DF15C / 0x0820B5BC / 0x0823717C, an affine slot growing for 32 frames
 * (x by 0x1000, y by 0x800 up to 0x10000), box slot 6, Hit_SetTimerOverride(2, 60),
 * skillBaseAttack/skillAttackAttributes = 0x23 / 1, song 0x1A6. Then it alternates animation 0 while
 * the caster's weaponEntities[unk_50C] slot is taken and animation 1 when that
 * ends; always at the caster, facing the other way (Anim_UpdateWithFrameAttackBox).
 */
void SkillShadowKnightUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;
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
        u8 *graphics = (u8 *)0x081DF15C;
        u8 *palette = (u8 *)0x0820B5BC;
        u8 *sprite = (u8 *)0x0823717C;
        LoadGfx(entity, 0, graphics, palette, sprite);
    }
        Anim_StartIndexed(entity, 1, 0, 1);
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
            u16 a = 0x23;
            u16 b = 1;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = b;
        }
        PlaySong(0x1A6);
    case 1:
        if (entity->ext.skillGrowScale.scaleX <= 0xFFFF)
            entity->ext.skillGrowScale.scaleX += 0x1000;
        if (SKILL_SHADOW_KNIGHT_SCALE_Y(entity) <= 0xFFFF)
            SKILL_SHADOW_KNIGHT_SCALE_Y(entity) += 0x800;
        ObjAffine_Set(0, entity->ext.skillGrowScale.scaleX, SKILL_SHADOW_KNIGHT_SCALE_Y(entity), entity->flipX, 0, entity->affineIndex);
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
        if (gEwramData->playerState.weaponEntities[parent->ext.player.weaponSlot] != NULL)
        {
            Anim_StartIndexed(entity, 0, 0, 0);
            entity->state = 3;
        }
        break;
    case 3:
        if (entity->animFlags & 4)
        {
            entity->state = 2;
            Anim_StartIndexed(entity, 1, 0, 1);
        }
        break;
    }
    Follow(entity, parent, 0, 0);
    entity->flipX ^= 1;
    Anim_UpdateWithFrameAttackBox(entity);
}
