/* Final Guard soul: once the parent is idle and in the right state, load the shield graphics and keep it until the state changes. */
#include "code_08009A0.h"
#include "blue_soul_entity.h"
#include "player_entity.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "macros.h"
#define GRAPHICS_CONTEXT ((struct PlayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, playerState)))
extern u8 sSkillFinalGuardGfx[], sSkillFinalGuardPalette[], sSkillFinalGuardSpriteSheet[];
extern s32 BlueSoul_ShouldEnd(struct EwramData_EntityData *entity);
extern void BlueSoul_ReleaseActiveResources(struct EwramData_EntityData *entity);

/* Inlined Entity_PlaceRelativeTo (src/code_08031D6C.c). */
static inline void FollowParent(struct EwramData_EntityData *entity, struct EwramData_EntityData *anchor, u16 offsetX, u16 offsetY)
{
    s16 dx = offsetX;
    s16 dy = offsetY;

    entity->flipX = anchor->flipX;
    if (entity->flipX)
        dx = -dx;
    entity->posX.part16.integer = (s32)dx + (s16)anchor->posX.part16.integer;
    entity->posY.part16.integer = (s32)dy + anchor->posY.part16.integer;
}

static inline void LoadGfx(struct EwramData_EntityData *entity, u8 paletteSlot, void *gfxSrc, void *palette, void *sheet)
{
    gEwramData->playerState.blueSoulGfx = ObjGfx_LoadResource(gfxSrc, 0x10000);
    ObjPalette_LoadRange(palette, paletteSlot, 1, 3);
    Sprite_Attach(entity, sheet, gEwramData->playerState.blueSoulGfx, 3);
}

void SkillFinalGuardUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = BLUE_SOUL_OWNER(entity);

    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        if (parent->state != 0)
        {
            BlueSoul_ReleaseActiveResources(entity);
            EntityDelete(entity);
            return;
        }
        if ((PLAYER_ACTION_FLAGS(parent) & 0x1001A2) != 0x100000)
            goto end;
    {
        u8 *graphics = sSkillFinalGuardGfx;
        u8 *palette = sSkillFinalGuardPalette;
        u8 *sprite = sSkillFinalGuardSpriteSheet;
        LoadGfx(entity, 0, graphics, palette, sprite);
    }
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->spriteLayer = 0xD;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        gEwramData->playerState.player->state = 10;
        Anim_StartIndexed(gEwramData->playerState.player, 0x35, 3, 1);
        PLAYER_ACTION_FLAGS(parent) &= 0xEFFFFBFF;
        PlaySong(0x1A7);
        entity->state++;
    case 1:
        entity->state++;
    default:
        if ((PLAYER_ACTION_FLAGS(parent) & 0x1001A2) != 0x100000)
        {
        end:
            BlueSoul_ReleaseActiveResources(entity);
            EntityDelete(entity);
            return;
        }
        gEwramData->playerStatus.soulStateFlags |= 0x2000;
        gEwramData->playerState.actionLockFlags |= 0x820;
        FollowParent(entity, parent, 0, 0);
        Anim_Update(entity);
    }
}
