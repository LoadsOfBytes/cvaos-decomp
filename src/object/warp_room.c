/*
 * Warp room: update, gate parts, side lights and room objects 0x1F and 0x21.
 */
#include "warp_room.h"
#include "core.h"
#include "in_game.h"
#include "bg_effects.h"
#include "room_entities.h"
#include "map_screen.h"
#include "map_flags.h"
#include "music.h"
#include "sound.h"
#include "player_core.h"
#include "graphics.h"
#include "entity_util.h"
#include "gba/dma.h"
#include "structs/main.h"
#include "structs/ewram.h"
#include "gameplay.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "palette.h"
#include "data/data_118B48.h"
#include "obj_palette.h"
#include "player_soma.h"
#include "entity_hit.h"

void WarpRoom_UpdateGatePart(struct EwramData_EntityData *);
void WarpRoom_ResetGatePart(struct EwramData_EntityData *);
void WarpRoom_UpdateSideLight(struct EwramData_EntityData *);
u8 Crusher_LoadGraphics(struct EwramData_EntityData *entity);
void Crusher_OnPlayerContact(struct EwramData_EntityData *entity);
s32 WarpRoom_LoadGraphics(struct EwramData_EntityData *entity);

struct WarpLightPaletteStep
{
    u16 sourcePalette;
    u16 unk_2;
};

extern const struct WarpLightPaletteStep sWarpLightPaletteSteps[12];

/* WarpRoom_Update. */

/**
 * @brief Warp room (object 0x1F): on Up at the warp spot open the warp menu;
 * a different destination starts the warp sequence. With
 * stateFlags bit 0x40 start in the arrival state.
 */
void WarpRoom_Update(struct EwramData_EntityData *e)
{
    u16 input = gEwramData->inputData.newInput;
    struct GameState *gameState = &gEwramData->gameState;

    switch (e->state)
    {
    case 0:
        if (gEwramData->gameState.stateFlags & 0x40)
        {
            gUnk_03002CB0.dispCnt |= 0x2000;
            e->ext.warpRoom.windowBottom = 0x74;
            gDisplayRegisters.winIn_L = 0x1B;
            gDisplayRegisters.winOut_L = 0x3F;
            gDisplayRegisters.winOut_H = 0x1E;
            gEwramData->playerState.player->posY.part16.integer = 0x60;
            e->state = 5;
            gEwramData->startPauseEnabled = 0;
        }
        else
        {
            gUnk_03002CB0.dispCnt &= 0xDFFF;
            gEwramData->gameState.stateFlags &= ~1;
            e->state = 1;
            gEwramData->startPauseEnabled = 1;
        }
        e->timer = 0;
        break;
    case 1:
        switch (e->subState)
        {
        case 0:
            if (!(input & 0x40))
                break;
            if (!Entity_IsInRect(gEwramData->playerState.player, 0x55, 0x7F, 0x49, 2))
                break;
            Game_SetMenusEnabled(0);
            e->subState++;
            e->timer = 3;
            break;
        case 1:
        {
            s32 r = Player_IsBusy();
            if (r == 0)
            {
                PlaySong(0xF3);
                BgPalette_StopColorFade();
                ObjPalette_StopColorFade();
                RedSoul_CancelAll();
                gEwramData->gameState.stateFlags |= 0x20000;
                gEwramData->startPauseEnabled = 0;
                e->timer = r;
                e->subState = r;
                e->state++;
                WarpMenu_Open(gameState);
            }
            else if (--e->timer == 0xFF)
            {
                Game_SetMenusEnabled(1);
                e->subState--;
            }
            break;
        }
        }
        break;
    case 2:
        if (gEwramData->gameState.selectedWarpIndex == -1)
        {
            e->state = 0;
            e->subState = 0;
            break;
        }
        if (gEwramData->gameState.currentRoomDefinition != gEwramData->gameState.roomJumpDestination)
        {
            PlaySong(0x1010);
            PlaySong(0xF9);
            e->subState = 0;
            e->state++;
            break;
        }
        e->subState = 0;
        e->state = 0;
        break;
    case 3:
        if (e->timer & 3)
            gEwramData->playerState.player->palette = 4;
        else
            gEwramData->playerState.player->palette = 0;
        if (e->timer++ > 0x80)
        {
            BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x10000, 0, 0x40);
            ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x10000, 0, 0x40);
            e->timer = 0;
            e->state++;
        }
        break;
    case 4:
        if (e->timer++ > 0x3E)
        {
            gEwramData->gameState.stateFlags |= 0x40;
            GameFlag_Set(0x39);
            Room_ClearForTransition(gameState);
            DMA_FILL_32(3, 0, VRAM_BASE + 0xE800, 0x800);
            DMA_FILL_32(3, 0, VRAM_BASE + 0xF000, 0x800);
            DMA_FILL_32(3, 0, VRAM_BASE + 0xF800, 0x800);
            gEwramData->playerState.player->posX.part16.integer = gEwramData->gameState.roomJumpPlayerX;
            gEwramData->playerState.player->posY.part16.integer = gEwramData->gameState.roomJumpPlayerY;
            gEwramData->playerState.player->velX.whole = 0;
            gEwramData->playerState.player->velY.whole = 0;
            gEwramData->gameState.currentRoomDefinition = gEwramData->gameState.roomJumpDestination;
            Room_LoadAtCamera(gEwramData->gameState.roomJumpCameraX, gEwramData->gameState.roomJumpCameraY);
            Room_StoreEntryPosition(gameState);
        }
        break;
    case 5:
        BgPalette_StopColorFade();
        ObjPalette_StopColorFade();
        BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x40);
        ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x40);
        e->timer = 0;
        e->state++;
        gEwramData->playerState.player->palette = 0;
        break;
    case 6:
        if (e->timer++ > 0x3F)
        {
            BgPalette_StopColorFade();
            ObjPalette_StopColorFade();
            e->state++;
            e->timer = 0;
            gEwramData->gameState.stateFlags &= ~0x40;
            Music_PlayAreaQuiet();
        }
        break;
    case 8:
        gEwramData->startPauseEnabled = 1;
        if ((u8)(e->timer++ % 3) == 0)
        {
            if ((s16)--e->ext.warpRoom.windowBottom < 0x24)
            {
                e->ext.warpRoom.windowBottom = 0x24;
                e->timer = 0;
                e->state = 0;
            }
        }
        break;
    }

    if (e->state >= 3 && e->state <= 5)
    {
        gEwramData->startPauseEnabled = 0;
        Player_StartScriptedAnimation(0x20, 1, 0, 0xFFFFE000);
    }
    if (e->state >= 2 && e->state <= 7)
        SetPlayerInput(1, 0);
    if (e->state > 4)
    {
        s32 h = (s16)e->ext.warpRoom.windowBottom;
        gDisplayRegisters.win0H = 0x5898;
        h |= 0x2400;
        gDisplayRegisters.win0V = h;
    }
}

/**
 * @brief Create the nine gate part sprites (update WarpRoom_UpdateGatePart)
 * and reset each. Return 1 when any was created.
 */
s32 WarpRoom_CreateGateParts(struct EwramData_EntityData *owner)
{
    s32 created = 0;
    s32 graphics = ObjGfx_Find(sWarpRoomGfx);
    s32 i;
    ObjPalette_LoadRange(sWarpRoomPalette, 2, 1, 0xA);
    for (i = 0, i = created; i <= 8; i++)
    {
        struct EwramData_EntityData *entity = EntityCreateInRange(0x1B, 0x48, EntityUpdateNothing);
        if (entity != NULL)
        {
            created = 1;
            Sprite_Attach(entity, sWarpRoomSpriteSheet, graphics, 0xA);
            entity->scrollWithBg = 1;
            entity->animFrame = i;
            ENTITY_PARENT_INDEX(entity) = ENTITY_INDEX(owner);
            if (i == 0)
            {
                register u8 *layer asm("r0") = &entity->spriteLayer; /* FAKEMATCH: pinned register */
                *layer = 0x1C;
            }
            if (i >= 1 && i <= 4)
                entity->spriteLayer = 0x1D;
            if (i >= 5 && i <= 8)
                entity->spriteLayer = 0x1E;
            entity->updateFunc = WarpRoom_UpdateGatePart;
            entity->affineIndex = ObjAffine_Alloc(1);
            entity->drawFunc = Sprite_DrawEntity;
            WarpRoom_ResetGatePart(entity);
        }
    }
    return created;
}

#define DIST(e) (e->ext.warpGate.dist)
#define SPEED(e) (e->ext.warpGate.speed)
#define ACCEL(e) (e->ext.warpGate.accel)
#define POS(e) (e->ext.warpGate.pos)
#define STEP(e) (e->ext.warpGate.step)

/**
 * @brief Reset a gate part to its closed animation; on arrival (stateFlags bit
 * 0x40) fast-forward it to the open state 6.
 */
void WarpRoom_ResetGatePart(struct EwramData_EntityData *entity)
{
    s32 limit;

    entity->posX.part16.integer = 0x78;
    entity->posY.part16.integer = 0x48;
    entity->state = 1;
    if (entity->animFrame == 0)
        entity->timer = entity->animFrame * 5;
    else
        entity->timer = entity->animFrame * 5 + 15;
    DIST(entity) = 0xFFFF;
    SPEED(entity) = 10;
    ACCEL(entity) = 0x3C;
    POS(entity) = 0;
    STEP(entity) = 0x800;
    ObjAffine_Set(0, 0x10000, 0x10000, 0, 0, entity->affineIndex);
    entity->shrinkPartOffsets = 0;
    if (!(gEwramData->gameState.stateFlags & 0x40))
        return;
    entity->timer = 0;
    do
    {
        DIST(entity) -= SPEED(entity);
        SPEED(entity) += ACCEL(entity);
        if (entity->animFrame > 3)
        {
            limit = 20;
            if (entity->animFrame > 5)
                limit = 10;
        }
        else
            limit = 30;
        if (entity->timer++ > limit)
            POS(entity) += STEP(entity);
    } while (DIST(entity) > 0);
    entity->state = 6;
    STEP(entity) *= -1;
    SPEED(entity) *= -1;
    ACCEL(entity) *= -1;
    if (entity->animFrame == 0)
        entity->timer = (9 - entity->animFrame) * 5 + 15;
    else
        entity->timer = (9 - entity->animFrame) * 5;
    ObjAffine_Set(POS(entity), DIST(entity) + 0x1800, DIST(entity) + 0x1800, 0, 0, entity->affineIndex);
    entity->affine = 1;
}

#define HOMEX(e) (e->ext.warpGate.homeX)
#define HOMEY(e) (e->ext.warpGate.homeY)

/**
 * @brief Animate a gate part through its opening and closing states, driven
 * by the warp room states 3 and 5.
 */
void WarpRoom_UpdateGatePart(struct EwramData_EntityData *entity)
{
    struct EwramData *ew = gEwramData;
    struct EwramData_EntityData *parent = &ew->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 limit;

    switch (entity->state)
    {
    case 0:
        WarpRoom_ResetGatePart(entity);
        break;
    case 1:
        if (parent->state == 3)
            entity->state++;
        break;
    case 2:
        if (--entity->timer == 0xFF)
        {
            entity->affine = 1;
            HOMEX(entity) = entity->posX.part16.integer;
            HOMEY(entity) = entity->posY.part16.integer;
            entity->state++;
            entity->timer = 0;
        }
        break;
    case 3:
        if (entity->animFrame == 0)
        {
            if (entity->timer & 1)
            {
                u32 r = RandomNumberGenerator();
                s16 *p = (s16 *)&entity->posX.part16.integer;
                *p -= r % 3 - 1;
                r = RandomNumberGenerator();
                p = (s16 *)&entity->posY.part16.integer;
                *p -= r % 3 - 1;
            }
            else
            {
                entity->posX.part16.integer = HOMEX(entity);
                entity->posY.part16.integer = HOMEY(entity);
            }
        }
        if (entity->timer++ > 0x50)
        {
            entity->posX.part16.integer = HOMEX(entity);
            entity->posY.part16.integer = HOMEY(entity);
            entity->timer = 0;
            entity->state++;
        }
        break;
    case 4:
        entity->shrinkPartOffsets = 1;
        DIST(entity) -= SPEED(entity);
        SPEED(entity) += ACCEL(entity);
        if (entity->animFrame > 3)
        {
            limit = 20;
            if (entity->animFrame > 5)
                limit = 10;
        }
        else
            limit = 30;
        if (entity->timer++ > limit)
            POS(entity) += STEP(entity);
        ObjAffine_Set(POS(entity), DIST(entity) + 0x1800, DIST(entity) + 0x1800, 0, 0, entity->affineIndex);
        if (DIST(entity) <= 0)
            entity->state++;
        break;
    case 5:
        entity->hidden = 1;
        break;
    case 6:
        if (parent->state == 5)
            entity->state++;
        break;
    case 7:
        if (--entity->timer == 0xFF)
        {
            entity->affine = 1;
            entity->hidden = 0;
            entity->timer = 0;
            entity->state++;
        }
        break;
    case 8:
        entity->shrinkPartOffsets = 1;
        DIST(entity) -= SPEED(entity);
        SPEED(entity) += ACCEL(entity);
        if (entity->animFrame > 3)
        {
            limit = 0;
            if (entity->animFrame > 5)
                limit = 5;
        }
        else
            limit = 0;
        if (entity->timer++ > limit)
            POS(entity) += STEP(entity);
        ObjAffine_Set(POS(entity), DIST(entity) + 0x1800, DIST(entity) + 0x1800, 0, 0, entity->affineIndex);
        if (POS(entity) == 0)
        {
            if (entity->animFrame != 0)
            {
                entity->state = 0;
            }
            else
            {
                entity->state = 0;
                parent->state++;
            }
            gEwramData->gameState.stateFlags &= ~0x40;
        }
        break;
    }
}

/**
 * @brief Create the two side light sprites (update WarpRoom_UpdateSideLight)
 * at x 0x54 and 0x9C. Return 1 when any was created.
 */
s32 WarpRoom_CreateSideLights(struct EwramData_EntityData *owner)
{
    s32 created = 0;
    s32 graphics = ObjGfx_Find(sWarpRoomGfx);
    s32 i;
    ObjPalette_LoadRange(sWarpRoomPalette, 4, 1, 0xB);
    for (i = 0, i = created; i <= 1; i++)
    {
        struct EwramData_EntityData *entity = EntityCreateInRange(0x1B, 0x48, EntityUpdateNothing);
        if (entity != NULL)
        {
            created = 1;
            Sprite_Attach(entity, sWarpRoomSpriteSheet, graphics, 0xB);
            if (i == 0)
                entity->posX.part16.integer = 0x54;
            else
                entity->posX.part16.integer = 0x9C;
            entity->posY.part16.integer = 0x80;
            entity->scrollWithBg = 1;
            entity->animFrame = 9;
            entity->spriteLayer = 0x15;
            entity->updateFunc = WarpRoom_UpdateSideLight;
            ENTITY_PARENT_INDEX(entity) = ENTITY_INDEX(owner);
            entity->ext.warpRoomSideLight.frame = 0;
            entity->state = 0;
            entity->drawFunc = Sprite_DrawEntity;
        }
    }
    return created;
}

static inline s32 Rem(s32 a, s32 b) { return a % b; }
#define FRAME(e) (*(s8 *)&(e)->ext.warpRoomSideLight.frame)

/**
 * @brief While the idle player stands on the warp spot, cycle the light
 * palette through sWarpLightPaletteSteps; restore it when the player leaves.
 */
void WarpRoom_UpdateSideLight(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *other = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    struct WarpLightPaletteStep frames[12];
    s32 found;
    s32 m;

    memcpy(frames, sWarpLightPaletteSteps, sizeof(frames));
    switch (entity->state)
    {
    case 0:
        if (Entity_IsInRect(gEwramData->playerState.player, 0x55, 0x7F, 0x49, 2) == 0)
            break;
        if (Player_IsBusy() != 0)
            break;
        FRAME(entity) = 0;
        entity->state++;
        break;
    case 1:
        found = Entity_IsInRect(gEwramData->playerState.player, 0x55, 0x7F, 0x49, 2);
        if (found == 0 && other->state <= 2)
        {
            ObjPalette_LoadRange(sWarpRoomPalette, 4, 1, 0xB);
            entity->state = found;
            break;
        }
        ObjPalette_LoadRange(sWarpRoomPalette, frames[FRAME(entity)].sourcePalette, 1, 0xB);
        m = entity->timer++;
        if ((m = Rem(m, 8)) == 0)
        {
            if (++FRAME(entity) > 11)
                FRAME(entity) = m;
        }
        break;
    }
}

void WarpRoom_UpdateFadeExit(struct EwramData_EntityData *entity)
{
    struct GameState *g = &gEwramData->gameState;
    struct EwramData_EntityData *player = gEwramData->playerState.player;
    u8 t;

    switch (entity->state)
    {
    case 0:
        if (!(gEwramData->inputData.heldInput & 0x40))
            return;
        if (g->subMode != 1)
            return;
        if (Entity_IsInRect(player, 0x55, 0x7F, 0x49, 2) == 0)
            return;
        Game_SetMenusEnabled(0);
        player->objMode = 1;
        gDisplayRegisters.bldAlpha = 0x10;
        entity->ext.warpRoomFadeExit.fadeTimer = 0x80;
        break;
    case 1:
        gEwramData->gameState.stateFlags |= 1;
        SetPlayerInput(1, 0);
        t = entity->timer;
        if ((s8)entity->timer >= 0)
        {
            entity->timer = t + 1;
        }
        else
        {
            player->hidden = 1;
            player->objMode = 0;
            gDisplayRegisters.bldAlpha = 0x808;
        }
        gDisplayRegisters.bldAlpha = (0x10 - (entity->timer >> 3)) | ((entity->timer >> 3) << 8);
        if (--entity->ext.warpRoomFadeExit.fadeTimer != 0xFFFF)
            return;
        RoomFadeTransition_Begin(g);
        break;
    default:
        return;
    }
    entity->state++;
}

/**
 * @brief Unreferenced part next to the warp room code: once its parent leaves
 * state 0 it rises, spins and converges on the screen centre while
 * shrinking; states 10/11 drop it back to y 0x84 and re-enable the
 * menus.
 */
void WarpRoomUnusedPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.warpRoomUnusedPart.parent;
    s32 limit;

    entity->drawFunc = Sprite_DrawEntity;
    switch (entity->state)
    {
    case 0:
        if (parent->state == 0)
            break;
        entity->timer = (RandomNumberGenerator() & 0xF) + 8;
        entity->state++;
        break;
    case 1:
        if (entity->timer != 0)
        {
            entity->timer--;
            entity->posY.whole -= 0x10000;
            break;
        }
        entity->timer = (RandomNumberGenerator() & 0xF) + 0x10;
        entity->state++;
        break;
    case 2:
        entity->ext.warpRoomUnusedPart.angle += 0x400;
        if (--entity->timer != 0)
            break;
        entity->velX.whole = -(entity->posX.whole >> 6);
        entity->velY.whole = (entity->posX.whole - 0xA00000) >> 7;
        entity->state++;
        break;
    case 3:
        limit = 0x30000 - (entity->ext.warpRoomUnusedPart.riseFrame << 9);
        if (limit < 0)
            limit = 0;
        if (entity->posX.part16.integer < 0x80)
        {
            entity->velX.whole += 0x4000;
            if (entity->velX.whole > limit)
                entity->velX.whole = limit;
        }
        else
        {
            entity->velX.whole -= 0x4000;
            if (entity->velX.whole < -limit)
                entity->velX.whole = -limit;
        }
        if ((u16)entity->posY.part16.integer < 0x50)
        {
            entity->velY.whole += 0x3000;
            if (entity->velY.whole > limit)
                entity->velY.whole = limit;
        }
        else
        {
            entity->velY.whole -= 0x3000;
            if (entity->velY.whole < -limit)
                entity->velY.whole = -limit;
        }
        entity->ext.warpRoomUnusedPart.riseFrame++;
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->ext.warpRoomUnusedPart.scale -= 0x200;
        if (entity->ext.warpRoomUnusedPart.scale == 0)
            entity->state = 0x63;
        break;
    case 10:
        entity->ext.warpRoomUnusedPart.angle += 0x400;
        if (++entity->timer > 0x2F)
            entity->state++;
        break;
    case 11:
        entity->velY.whole += 0x6000;
        entity->posY.whole += entity->velY.whole;
        if ((u16)entity->posY.part16.integer > 0x83)
        {
            if (entity->spawnIndex == 0)
                Game_SetMenusEnabled(1);
            entity->posY.part16.integer = 0x84;
            entity->state = 0;
        }
        break;
    case 0x63:
        entity->deleteRequested = 1;
        ObjAffine_Free(entity->ext.warpRoomUnusedPart.affine);
        return;
    }
    ObjAffine_Set(entity->ext.warpRoomUnusedPart.angle, entity->ext.warpRoomUnusedPart.scale, entity->ext.warpRoomUnusedPart.scale, 0, 0, entity->ext.warpRoomUnusedPart.affine);
}

/* Objects 0x1F / 0x21 create, 0x080564F8..0x08056617. */

/**
 * @brief 564F8 | Object 0x1F create.
 */
void WarpRoomCreate(struct EwramData_EntityData *entity)
{
    WarpRoom_LoadGraphics(entity);
    WarpRoom_CreateGateParts(entity);
    WarpRoom_CreateSideLights(entity);
}

/**
 * @brief Get (or load) the graphics set sWarpRoomGfx into unk_4FC and switch
 * the update to WarpRoom_Update; delete the entity when that fails
 * (no return value then).
 */
s32 WarpRoom_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_Find(sWarpRoomGfx);

    if (gfx < 0)
    {
        gfx = ObjGfx_LoadResource(sWarpRoomGfx, 0);
        if (gfx < 0)
        {
            EntityDelete(entity);
            return;
        }
    }
    entity->ext.warpRoom.unusedGfx = gfx;
    entity->updateFunc = WarpRoom_Update;
    return 1;
}

/**
 * @brief 56550 | Object 0x21 create: hitbox with callback Crusher_OnPlayerContact,
 * strength attack from the player stats (equipmentDefenseBonus * 3 / 4 + maxHP / 16),
 * fixed room position (0x40, 0x18).
 */
void CrusherCreate(struct EwramData_EntityData *entity)
{
    struct EwramData **ewramRef = &gEwramData;
    struct BgLayerState *bg = &(*ewramRef)->bgInfo[1];
    struct SpriteBox box = { -30, 41, 0x3C, 0x14 };

    if (!Crusher_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Hit_Enable(entity, 8, Crusher_OnPlayerContact, 0);
    HitBox_SetAttack(entity, &box);
    entity->hit.flags |= 4;
    entity->hit.flags &= ~2;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    {
        s32 power = ((*ewramRef)->playerState.equipmentDefenseBonus >> 1) + ((*ewramRef)->playerState.equipmentDefenseBonus >> 2);

        power += (*ewramRef)->playerStatus.maxHP >> 4;
        entity->attack = power;
    }
    entity->spriteLayer = 0x12;
    entity->state = 0;
    entity->posX.part16.integer = 0x40 - bg->xPos.part16.integer;
    entity->posY.part16.integer = 0x18 - bg->yPos.part16.integer;
    entity->accelY.whole = 0x2800;
}

/**
 * @brief Main behaviour of object 0x21, called by CrusherUpdate.
 */
void Crusher_UpdateState(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    switch (entity->subState)
    {
    case 0:
        EntityAccelerateAndMoveY(entity, 0x70000);
        if (entity->animScriptIndex == 0 && (s16)entity->posY.part16.integer >= 0x24 - (s16)bg->yPos.part16.integer)
            Anim_StartIndexed(entity, 3, 0, 1);
        if ((s16)entity->posY.part16.integer >= 0x7F - (s16)bg->yPos.part16.integer)
        {
            entity->posY.part16.integer = 0x7F - bg->yPos.part16.integer;
            entity->velY.whole = -0x14000;
            entity->subState++;
            entity->timer = 0;
            PlaySong(0x7F);
        }
        break;
    case 1:
        EntityAccelerateAndMoveY(entity, 0x70000);
        if ((s16)entity->posY.part16.integer >= 0x7F - (s16)bg->yPos.part16.integer)
        {
            entity->posY.part16.integer = 0x7F - bg->yPos.part16.integer;
            entity->velY.whole = 0;
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 2:
        if (entity->timer++ > 0x3B)
        {
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 3:
        entity->velY.whole = -0x10000;
        entity->posY.whole += entity->velY.whole;
        if (entity->animScriptIndex == 3 && (s16)entity->posY.part16.integer <= 0x24 - (s16)bg->yPos.part16.integer)
            Anim_StartIndexed(entity, 0, 0, 1);
        if ((s16)entity->posY.part16.integer <= 0x18 - (s16)bg->yPos.part16.integer)
        {
            entity->posY.part16.integer = 0x18 - bg->yPos.part16.integer;
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 4:
        if (entity->timer++ > 0x3B)
        {
            entity->subState = 0;
            entity->timer = 0;
            entity->velY.whole = 0;
        }
        break;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   CrusherUpdate = Object28Update
 *   Crusher_LoadGraphics = CrumblingPlatform_LoadGraphics
 *   Crusher_OnPlayerContact = DevilPart_DamagePlayerOnContact */

/**
 * @brief 56784 | Same as Object28Update.
 */
void CrusherUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;

    if (entity->state == 0)
        Crusher_UpdateState(entity);
    Anim_Update(entity);
    Hit_CheckWithPlayer(&result, entity);
}

/**
 * @brief 567AC | Same as CrumblingPlatform_LoadGraphics.
 */
u8 Crusher_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sCrusherObject23Gfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sUnk_08209FD8, 1);
    Sprite_Attach(entity, sCrusherObject23SpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 567F8 | Same as DevilPart_DamagePlayerOnContact.
 */
void Crusher_OnPlayerContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 1;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}
