#include "code_08009A0.h"
#include "player_entity.h"
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

static inline void SpawnBubble(struct EwramData_EntityData *entity, s32 type)
{
    s32 angle = (RandomNumberGenerator() & 0xFF) << 8;
    s32 s = Sin(angle + 0x4000);
    s32 vx = s * 2;
    s32 c = Sin(angle);
    s32 vy = c * 2;
    s32 x = entity->posX.whole + s * 16;
    s32 y = entity->posY.whole - 0x280000;
    struct EwramData_EntityData *child;
    y += c * 16;
    child = Effect_CreateFromTable(x, y, vx / 16, vy / 16, 4, type);
    if (child != NULL)
    {
        child->velX.whole = vx;
        child->velY.whole = vy;
    }
}

/**
 * @brief 2F1A4 | Sky Fish blue soul (ends via BlueSoul_CheckEndCondition or caster state
 * unk_4F4 & 0x1A2; not started while & 0x82): graphics 0x081DD950 /
 * 0x0820B550 / 0x08236868, song 0x99. For 16 frames two Effect_CreateFromTable
 * bubbles per frame at random angles 40 px above, then the player takes
 * animation 0x34 (actionLockFlags bit 11, song 0x1A8) and a counter spawnParam1 rises
 * to 300 (song 0x95 every 60), added to STR (/ 2) and LCK every frame
 * (Player_RecalculateAttackDefense). The end state sparkles (Effect_SpawnShared every 8 of 240
 * frames) and calls BlueSoul_ReleaseActiveResources.
 */
void SkillSkyFishUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;
    s16 *str = &gEwramData->playerState.currentStats[0];

    entity->roomActive = 1;
    if ((u8)(entity->state - 1) <= 2)
    {
        if (BlueSoul_CheckEndCondition(entity))
            entity->state = 4;
        if (parent->ext.player.actionFlags & 0x1A2)
            entity->state = 4;
    }
    Follow(entity, parent, 0, 0);
    switch (entity->state)
    {
    case 0:
        if (parent->ext.player.actionFlags & 0x82)
            goto delete;
    {
        u8 *graphics = (u8 *)0x081DD950;
        u8 *palette = (u8 *)0x0820B550;
        u8 *sprite = (u8 *)0x08236868;
        LoadGfx(entity, 0, graphics, palette, sprite);
    }
        Anim_StartIndexed(entity, 1, 0, 1);
        entity->spriteLayer = 0xF;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        entity->timer = 0x10;
        entity->state = 1;
        PlaySong(0x99);
        break;
    case 1:
        gEwramData->playerStatus.inputOverrides |= 1;
        SpawnBubble(entity, 8);
        SpawnBubble(entity, 8);
        if (entity->timer == 0)
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->state = 2;
            entity->spawnParam0 = 1;
            gEwramData->playerState.actionLockFlags |= 0x800;
            Anim_StartIndexed(gEwramData->playerState.player, 0x34, 3, 0);
            PlaySong(0x1A8);
            break;
        }
        entity->timer--;
        break;
    case 2:
        gEwramData->playerState.actionLockFlags |= 0x800;
        gEwramData->playerStatus.inputOverrides |= 1;
        if ((s16)(entity->spawnParam1 % 60) == 0)
            PlaySong(0x95);
        if (entity->spawnParam1 <= 299)
            entity->spawnParam1++;
        else
            entity->state = 4;
        break;
    case 4:
        entity->state = 5;
        entity->timer = 0;
        if (entity->spawnParam0 != 0)
        {
            entity->timer = 0xF0;
            if (parent->ext.player.actionFlags & 0x20)
                PlaySong(0x1A2);
        }
        else
            entity->hidden = 1;
        entity->hidden = 1;
        break;
    case 5:
        if (entity->timer == 0)
            goto delete;
        if (!(entity->timer & 7))
        {
            s32 x = entity->posX.whole + (RandomSigned(0x10) << 16);
            Effect_SpawnShared(x, entity->posY.whole + ((RandomSigned(0x10) - 0x10) << 16), 0x10, 8);
        }
        entity->timer--;
        goto end;
    }
    goto end;
delete:
    BlueSoul_ReleaseActiveResources(entity);
    EntityDelete(entity);
    return;
end:
    *str += entity->spawnParam1 >> 1;
    gEwramData->playerState.currentStats[3] += entity->spawnParam1;
    Player_RecalculateAttackDefense();
    Anim_Update(entity);
}
