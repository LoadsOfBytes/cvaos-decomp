/* Object 0x07 (room effects) and the room table sWarpRoomEntries,
 * 0x0804F690..0x0804F89F. */
#include "syscalls.h"
#include "room_object_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void Music_StartOverride(s32 song);
/* Declared here with an int result: Warp_ActivateCurrentRoom uses the value without
 * re-extending it from s16 (see code_08001194.h for the real type). */
extern s32 GetEntityRoomXPositionInteger(struct EwramData_EntityData *entity);
extern s32 GetEntityRoomYPositionInteger(struct EwramData_EntityData *entity);
extern void BgParticle_SpawnAll(struct EwramData_EntityData *owner);
extern void Object07_SpawnParallaxSprites(struct EwramData_EntityData *entity);

/* Palette animation frame: palette data, palette slot, duration. */
struct BgPaletteCycleFrame {
    u8 *paletteResource;
    u16 sourcePalette;
    u16 duration;
};

/* Palette animation (variant spawnParam1 of object 0x07 type 1). */
struct BgPaletteCycle {
    u16 frameCount; // number of frames
    u16 destinationPalette;
    struct BgPaletteCycleFrame *frames;
};

extern struct BgPaletteCycle sBgPaletteCycles[];
extern struct WarpRoomEntry sWarpRoomEntries[];

/**
 * @brief 4F690 | Step the palette animation of variant spawnParam1: frame
 * unk_4F4, timer unk_4F6. (Non-void in the original, which returns nothing
 * explicitly: `pop {r1}` epilogue.)
 */
s32 BgPaletteAnim_Step(struct EwramData_EntityData *entity)
{
    struct BgPaletteCycle *anim = &sBgPaletteCycles[entity->spawnParam1];
    struct BgPaletteCycleFrame *frame = &anim->frames[(u16)entity->ext.bgPaletteAnim.frame];

    entity->ext.bgPaletteAnim.tick = Mod((u16)entity->ext.bgPaletteAnim.tick + 1, frame->duration);
    if (entity->ext.bgPaletteAnim.tick == 0)
    {
        entity->ext.bgPaletteAnim.frame = Mod((u16)entity->ext.bgPaletteAnim.frame + 1, anim->frameCount);
        frame = &anim->frames[(u16)entity->ext.bgPaletteAnim.frame];
        BgPalette_LoadRange(frame->paletteResource, frame->sourcePalette, 1, anim->destinationPalette);
    }
}

/**
 * @brief 4F6E4 | Object 0x07 create, by type spawnParam0.
 */
void Object07Create(struct EwramData_EntityData *entity)
{
    switch (entity->spawnParam0)
    {
        case 0:
            BgParticle_SpawnAll(entity);
            Object07_SpawnParallaxSprites(entity);
            break;
        case 1:
            entity->ext.object07.paletteAnim = 0;
            break;
        case 2:
            BgParticle_SpawnAll(entity);
            break;
        case 3:
            gEwramData->gameState.conveyorSpeed = 0;
            gEwramData->gameState.stateFlags |= 0x800;
            BgPalette_ApplyTintedGrayscale();
            break;
        case 4:
            if (entity->spawnParam1 != 0)
                Music_StartOverride(entity->spawnParam1);
            break;
    }
}

/**
 * @brief 4F770 | Object 0x07 update: type 1 palette animation, types 0 and
 * 2 keep bldCnt at 0xE40.
 */
void Object07Update(struct EwramData_EntityData *entity)
{
    switch (entity->spawnParam0)
    {
        case 1:
            BgPaletteAnim_Step(entity);
            break;
        case 0:
        case 2:
            gDisplayRegisters.bldCnt = 0xE40;
            break;
    }
}

/* Index of the room (x, y) in sWarpRoomEntries, -1 when not listed. */
static inline s8 FindRoom(u16 x, u16 y)
{
    struct WarpRoomEntry *entry = sWarpRoomEntries;
    s8 i = 0;

    for (; entry->roomDefinition != NULL; i++, entry++)
    {
        if ((u16)entry->mapX == x && (u16)entry->mapY == y)
            return i;
    }
    return -1;
}

/**
 * @brief If the map cell under the player is listed in the warp-room table,
 * set its bit in activatedWarpFlags (activated warp rooms). Called by Room_Load.
 */
void Warp_ActivateCurrentRoom(void)
{
    u32 x;
    u32 y;
    s8 warp;

    x = (u16)(gEwramData->gameState.roomMapXPos + (GetEntityRoomXPositionInteger(gEwramData->playerState.player) >> 8));
    y = (u16)(gEwramData->gameState.roomMapYPos + (GetEntityRoomYPositionInteger(gEwramData->playerState.player) >> 8));
    warp = FindRoom(x, y);
    if (warp >= 0)
        gEwramData->gameState.activatedWarpFlags[warp >> 5] |= 1 << (warp & 0x1F);
}

/**
 * @brief Return entry n of the warp-room table sWarpRoomEntries.
 */
struct WarpRoomEntry *Warp_GetRoomEntry(u8 index)
{
    return &sWarpRoomEntries[index];
}

/**
 * @brief Return the index of map cell (x, y) in the warp-room table, or -1.
 */
s32 Warp_FindRoomIndex(u16 x, u16 y)
{
    struct WarpRoomEntry *entry = sWarpRoomEntries;
    s8 i = 0;

    for (; entry->roomDefinition != NULL; i++, entry++)
    {
        if ((u16)entry->mapX == x && (u16)entry->mapY == y)
            return i;
    }
    return -1;
}
