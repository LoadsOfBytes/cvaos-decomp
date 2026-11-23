/*
 * Room objects 0x00-0x07: gates and doors, background particles, parallax sprites, palette animation and the warp room lookups.
 */
#include "core.h"
#include "graphics.h"
#include "structs/entity_render_flags.h"
#include "structs/ewram.h"
#include "in_game.h"
#include "entity_util.h"
#include "sound.h"
#include "gba.h"
#include "structs/main.h"
#include "player_core.h"
#include "music.h"
#include "syscalls.h"
#include "effects.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "objects_00_07.h"
#include "palette.h"
/* from bg_and_collision.h (not included: see the module notes) */
extern void Collision_SetBlock(s32 arg0, s32 arg1);
extern void Collision_ClearBlock(s32 arg0, s32 arg1);

/* Draw the entity twice with mirrored affine matrices (rotation from unk_4F8), restoring position, frame and flip. */
struct AffineSlot16 { u16 slot:5; u16 rest:3; };
/**
 * @brief Draw the gate entity twice with mirrored affine matrices from its
 * opening angle unk_4F8 (0x4000 = fully open draws it flipped without
 * affine), restoring position, frame and flip.
 */
void Gate_Draw(struct EwramData_EntityData *e)
{
    s32 savedX;
    u8 *p549;
    u8 saved549;
    u32 savedFlip;
    s32 d;
    s32 m;

    savedX = e->posX.whole;
    p549 = &e->animFrame;
    saved549 = *p549;
    savedFlip = e->flipX;
    if (e->ext.gate.openAngle == 0x4000)
    {
        e->affine = 0;
        e->flipX ^= 1;
    }
    else
    {
        if (e->ext.gate.openAngle != 0)
        {
            e->affine = 1;
            ObjAffine_Set(0, Sin(e->ext.gate.openAngle), 0x10000, e->flipX ^ 1, 0, e->ext.gateDraw.affine[0]);
            m = 0x1F;
            
            e->affineIndex = ({ u32 b_ = e->ext.gateDraw.affine[0]; asm("" ::: "r0"); /* FAKEMATCH: forget r0 == mask so reload copies it from r8 */ b_; }) & m;
            Sprite_Draw(e);
            ObjAffine_Set(0, Sin(e->ext.gate.openAngle + 0x4000), 0x10000, e->flipX, 0, e->ext.gateDraw.affine[1]);
            e->affineIndex = ({ u32 b_ = e->ext.gateDraw.affine[1]; b_; }) & m;
            
            asm("" :: "r"(m)); /* FAKEMATCH: lengthen mask life so zero wins r5 in local-alloc */
            d = (Sin(e->ext.gate.openAngle) << 5) - 0x10000;
            {
                s32 v;
                s32 x = e->posX.whole;
                if (e->flipX)
                    v = x - d;
                else
                    v = x + d;
                e->posX.whole = v;
                asm("" :: "r"(m)); /* FAKEMATCH: lengthen mask life so zero wins r5 in local-alloc */
            }
        }
        else
        {
            { u8 *q_ = ENTITY_RENDER_FLAGS_HIGH_ADDRESS(e); u8 f_ = *q_; register u32 m_ asm("r0") = ~0x10; m_ &= f_; *q_ = m_; /* FAKEMATCH: mask/result in r0 */ }
            *p549 = saved549 + 2;
            Sprite_DrawEntity(e);
        }
        *p549 = saved549 + 1;
    }
    Sprite_Draw(e);
    e->posX.whole = savedX;
    e->animFrame = saved549;
    e->flipX = savedFlip;
}

extern s32 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern u8 sGateGfx[], sGatePalette[], sGateSpriteSheet[], sGatePaletteAnim0[], sGatePaletteAnim1[];

/**
 * @brief 4D8F0 | Graphics setup of object 0: layer 0x11, facing left when
 * right of x 0x78, sprite sGateGfx / palette sGatePalette /
 * sGateSpriteSheet (Entity_LoadGraphics; palette slot 5 in basePalette when it fails),
 * palette slot kept in unk_4FF; frame 0 with palette data sGatePaletteAnim0 when
 * opened is 0, otherwise frame 5 with sGatePaletteAnim1 (PaletteAnim_Start); two
 * affine slots (ObjAffine_Alloc(1)) in unk_4FC / unk_4FD, drawn by
 * Gate_Draw.
 */
void Gate_SetupGraphics(struct EwramData_EntityData *entity, s32 opened)
{
    u8 pal;

    entity->spriteLayer = 0x11;
    entity->flipX = (s16)entity->posX.part16.integer > 0x78 ? 1 : 0;
    if (!Entity_LoadGraphics(entity, sGateGfx, sGatePalette, 0, sGateSpriteSheet))
        entity->basePalette = 5;
    entity->ext.gate.palette = entity->basePalette;
    pal = entity->ext.gate.palette;
    entity->roomActive = 1;
    if (opened == 0)
    {
        entity->animFrame = opened;
        PaletteAnim_Start(sGatePalette, sGatePaletteAnim0, (s8)pal);
    }
    else
    {
        entity->animFrame = 5;
        PaletteAnim_Start(sGatePalette, sGatePaletteAnim1, (s8)pal);
    }
    entity->ext.gate.affine0 = ObjAffine_Alloc(1);
    entity->ext.gate.affine1 = ObjAffine_Alloc(1);
    entity->affineIndex = entity->ext.gate.affine0;
    entity->affine = 1;
    entity->drawFunc = (u32 *)Gate_Draw;
}

/**
 * @brief 4D9DC | Object 0 init: Gate_SetupGraphics with spawnParam0; when the player
 * is inside the 0x28 x 0x28 area 0x14 px to its left (Entity_IsInRect), set
 * bits 0 and 8 of stateFlags, call Game_SetMenusEnabled(0), set unk_4F8 to 0x4000,
 * state 10 and force player input 0x10 (0x20 when flipped); otherwise
 * mark three tiles (Collision_SetBlock) 8 / 0x18 / 0x28 px above its room
 * position and reset unk_4F8 and the state.
 */
void Object00Create(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player;
    s32 found;
    s32 x, y;
    struct BgLayerState *bg;

    Gate_SetupGraphics(entity, entity->spawnParam0);
    player = gEwramData->playerState.player;
    found = Entity_IsInRect(entity, (s16)(player->posX.part16.integer - 0x14), player->posY.part16.integer, 0x28, 0x28);
    if (found)
    {
        gEwramData->gameState.stateFlags |= 0x101;
        Game_SetMenusEnabled(0);
        entity->ext.gate.openAngle = 0x4000;
        entity->state = 10;
        SetPlayerInput(1, entity->flipX ? 0x20 : 0x10);
        return;
    }
    x = (s16)entity->posX.part16.integer;
    bg = &gEwramData->bgInfo[1];
    x += (s16)bg->xPos.part16.integer;
    y = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer;
    Collision_SetBlock(x, y - 8);
    Collision_SetBlock(x, y - 0x18);
    Collision_SetBlock(x, y - 0x28);
    entity->ext.gate.openAngle = found;
    entity->state = found;
}

/**
 * @brief 4DAB4 | Gate object (object 0x00): hidden off screen, state 0x63
 * once roomActive is clear (releases the player input, frees the two affine
 * slots unk_4FC / unk_4FD). State 0 waits for the player to stand in the
 * 0x28 x 2 box facing it (not while bit 9 of soulStateFlags or bit 28 of the
 * player's unk_4F4), takes the input (stateFlags |= 0x101, sound 0x115);
 * state 1 counts unk_4F8 up to 0x4000 and clears the gate tiles
 * (Collision_ClearBlock at three heights); state 2 walks the player through.
 * States 10/11 close it again behind the player (sound 0x114,
 * Collision_SetBlock) and return to state 0.
 */
void Object00Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player;
    struct BgLayerState *bg;
    s32 near;
    /* the original keeps &gEwramData and 0x13110 in registers from the start */
    struct EwramData **ew = &gEwramData;
    u32 off = 0x13110;

    entity->hidden = Entity_IsInRect(entity, -0x20, 0, 0x130, 0xD0) ? 0 : 1;
    if (!entity->roomActive)
        entity->state = 0x63;
    if (entity->state <= 9)
    {
        player = *(struct EwramData_EntityData **)((u8 *)*ew + off);
        near = Entity_IsInRect(entity, (s16)(player->posX.part16.integer - 0x14), (s16)player->posY.part16.integer, 0x28, 2);
    }
    else
    {
        player = *(struct EwramData_EntityData **)((u8 *)*ew + off);
        near = Entity_IsInRect(entity, (s16)(player->posX.part16.integer - 0x14), (s16)player->posY.part16.integer, 0x28, 0x28);
    }
    switch (entity->state)
    {
    case 0:
        if (!near)
            break;
        if (entity->flipX == gEwramData->playerState.player->flipX)
            break;
        if (gEwramData->playerStatus.soulStateFlags & 0x200)
            break;
        if (gEwramData->playerState.player->ext.player.actionFlags & 0x10000000)
            break;
        SetPlayerInput(1, 0);
        gEwramData->gameState.stateFlags |= 0x101;
        Game_SetMenusEnabled(0);
        PlaySong(0x115);
        entity->state++;
        break;
    case 1:
    {
        s32 x1, y1;
        SetPlayerInput(1, 0);
        if ((entity->ext.gate.openAngle += 0x100) <= 0x3FFF)
            break;
        x1 = (s16)entity->posX.part16.integer;
        /* &gEwramData->bgInfo[1], loaded straight into the bg register */
        bg = (struct BgLayerState *)gEwramData;
        bg = (struct BgLayerState *)((u8 *)bg + 0xA094);
        x1 += (s16)bg->xPos.part16.integer;
        y1 = (s16)entity->posY.part16.integer;
        y1 += (s16)bg->yPos.part16.integer;
        Collision_ClearBlock(x1, y1 - 8);
        Collision_ClearBlock(x1, y1 - 0x18);
        y1 -= 0x28;
        Collision_ClearBlock(x1, y1);
        entity->state++;
        break;
    }
    case 2:
        SetPlayerInput(1, entity->flipX ? 0x10 : 0x20);
        break;
    case 10:
        Game_SetMenusEnabled(0);
        SetPlayerInput(1, entity->flipX ? 0x20 : 0x10);
        if (near)
            break;
        PlaySong(0x114);
        entity->state++;
        break;
    case 11:
    {
        s32 x2, y2;
        SetPlayerInput(1, 0);
        if ((entity->ext.gate.openAngle += -0x1A0) > 0)
            break;
        entity->ext.gate.openAngle = 0;
        gEwramData->gameState.stateFlags &= 0xFFFFFEFE;
        Game_SetMenusEnabled(1);
        x2 = (s16)entity->posX.part16.integer;
        bg = (struct BgLayerState *)gEwramData;
        bg = (struct BgLayerState *)((u8 *)bg + 0xA094);
        x2 += (s16)bg->xPos.part16.integer;
        y2 = (s16)entity->posY.part16.integer;
        y2 += (s16)bg->yPos.part16.integer;
        Collision_SetBlock(x2, y2 - 8);
        Collision_SetBlock(x2, y2 - 0x18);
        y2 -= 0x28;
        Collision_SetBlock(x2, y2);
        entity->ext.gate.openAngle = 0;
        entity->state = 0;
        break;
    }
    case 0x63:
        SetPlayerInput(1, 0);
        gEwramData->gameState.stateFlags &= 0xFFFFFEFE;
        entity->deleteRequested = 1;
        ObjAffine_Free((u8)entity->ext.gate.affine0);
        ObjAffine_Free(entity->ext.gate.affine1);
        break;
    }
}

/**
 * @brief 4DD84 | Door step: once roomActive is clear, set deleteRequested and free
 * both affine slots (unk_4FC / unk_4FD). Otherwise while flag spawnParam0 of
 * the flag words at gameFlagsTail is set open (unk_4F8 += 0x400 up to 0x4000,
 * Collision_ClearBlock on the three tiles above it), else close (unk_4F8 -= 0x400
 * down to 0, Collision_SetBlock on the tiles).
 */
void Gate_UpdateFlagControlled(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg;
    s32 x, y, px, py;

    px = (s16)entity->posX.part16.integer;
    bg = &gEwramData->bgInfo[1];
    x = px + (s16)bg->xPos.part16.integer;
    py = (s16)entity->posY.part16.integer;
    y = py + (s16)bg->yPos.part16.integer;
    if (!entity->roomActive)
    {
        entity->deleteRequested = 1;
        ObjAffine_Free((u8)entity->ext.gate.affine0);
        ObjAffine_Free(entity->ext.gate.affine1);
        return;
    }
    if (gEwramData->gameState.gameFlagsTail[entity->spawnParam0 >> 5] & (1 << (entity->spawnParam0 & 0x1F)))
    {
        entity->ext.gate.openAngle += 0x400;
        if (entity->ext.gate.openAngle > 0x4000)
            entity->ext.gate.openAngle = 0x4000;
        Collision_ClearBlock(x, y - 8);
        Collision_ClearBlock(x, y - 0x18);
        Collision_ClearBlock(x, y - 0x28);
    }
    else
    {
        entity->ext.gate.openAngle -= 0x400;
        if (entity->ext.gate.openAngle < 0)
            entity->ext.gate.openAngle = 0;
        Collision_SetBlock(x, y - 8);
        Collision_SetBlock(x, y - 0x18);
        Collision_SetBlock(x, y - 0x28);
    }
}

/* Helpers 0x0804DE60..0x0804DFAB. */
void Door_Draw(struct EwramData_EntityData *entity);

extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];

/* Event flag param_1 of the bit array gameFlagsTail (as in Object35Create). */
#define OBJECT_FLAG(ewram, flag) ((ewram)->gameState.gameFlagsTail[(flag) >> 5] & (1 << ((flag) & 0x1F)))

/**
 * @brief (Unused, no pointer in the ROM) Set up the gate graphics; start fully
 * open (unk_4F8 = 0x4000) when the object flag spawnParam0 is set.
 */
void Gate_CreateFlagControlled(struct EwramData_EntityData *entity)
{
    Gate_SetupGraphics(entity, 0);
    if (OBJECT_FLAG(gEwramData, entity->spawnParam0))
        entity->ext.gate.openAngle = 0x4000;
}

/**
 * @brief 4DEA0 | Draw; while unk_4F8 (1..0x2F) is set also draw a copy as
 * OBJ window 0x30 px higher and lower the entity by unk_4F8; above 0x2F
 * only switch the OBJ window off.
 */
void Door_Draw(struct EwramData_EntityData *entity)
{
    u8 *framePtr = &entity->animFrame;
    u32 savedFrame = *framePtr;
    u16 *yPtr = &entity->posY.part16.integer;
    u32 savedY = *yPtr;

    if (entity->ext.door.openHeight > 0x2F)
    {
        gUnk_03002CB0.dispCnt &= ~DCNT_WINOBJ;
        return;
    }
    if (entity->ext.door.openHeight != 0)
    {
        entity->objMode = 2;
        gUnk_03002CB0.dispCnt |= DCNT_WINOBJ;
        *yPtr -= 0x30;
        Sprite_Draw(entity);
        entity->objMode = 0;
        *yPtr = savedY - entity->ext.door.openHeight;
    }
    *framePtr = savedFrame;
    Sprite_Draw(entity);
    *yPtr = savedY;
}

/**
 * @brief 4DF3C | Set up the drawing: layer 0x16, facing right, unk_4FE =
 * whether it is in the right half of the screen, animation set
 * sCommonObjectSpriteSheet with palette 6, frame 0x3F, OBJ window on, drawFunc
 * Door_Draw.
 */
void Door_SetupGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx;

    entity->spriteLayer = 0x16;
    entity->flipX = 0;
    entity->ext.door.onRight = (s16)entity->posX.part16.integer > 0x78;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    entity->ext.door.palette = 6;
    Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, 6);
    entity->animFrame = 0x3F;
    gUnk_03002CB0.dispCnt |= DCNT_WINOBJ;
    entity->drawFunc = (u32 *)Door_Draw;
}

/**
 * @brief 4DFAC | Object 2 init (door, as Object06Create): deleted when
 * spawnParam0 <= 1 and bit spawnParam1 of bossEventFlags is set; Door_SetupGraphics, window
 * outside 0x3F / 0x2F, variant 7 uses palette 9 (unk_4FF, palette). When
 * the player is inside the 0x38 x 0x22 area 0x1C px to its left: unk_4F8 =
 * 0x30, state 10, clear player input, RedSoul_CancelAll, and stop the player
 * (velX, accelX) when moving towards the room centre (x 0x78); otherwise
 * mark three tiles above it and pick the state from spawnParam0.
 */
void Object02Create(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player;
    struct EwramData_EntityData **pp;
    struct BgLayerState *bg;
    s32 zero;
    s32 found;
    s32 x, y;

    if (entity->spawnParam0 <= 1
        && ((gEwramData->gameState.bossEventFlags >> entity->spawnParam1) & 1))
    {
        EntityDelete(entity);
        return;
    }
    Door_SetupGraphics(entity);
    {
        struct DisplayRegisters *regs = &gDisplayRegisters;
        u8 *win = &regs->winOut_L;
        zero = 0;
        *win = 0x3F;
        regs->winOut_H = 0x2F;
    }
    if (entity->spawnParam1 == 7)
    {
        entity->ext.door.palette = 9;
        entity->palette = entity->ext.door.palette;
    }
    found = Entity_IsInRect(entity, (s16)(gEwramData->playerState.player->posX.part16.integer - 0x1C),
                         gEwramData->playerState.player->posY.part16.integer, 0x38, 0x22);
    if (found)
    {
        entity->ext.door.openHeight = 0x30;
        entity->state = 10;
        SetPlayerInput(1, 0);
        RedSoul_CancelAll();
        pp = &gEwramData->playerState.player;
        player = *pp;
        if ((s16)player->posX.part16.integer <= 0x77)
        {
            if (player->velX.whole >= 0)
                return;
        }
        else if (player->velX.whole <= 0)
            return;
        player->velX.whole = zero;
        (*pp)->accelX.whole = zero;
        return;
    }
    x = (s16)entity->posX.part16.integer;
    bg = &gEwramData->bgInfo[1];
    x += (s16)bg->xPos.part16.integer;
    y = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer;
    Collision_SetBlock(x, y - 8);
    Collision_SetBlock(x, y - 0x18);
    Collision_SetBlock(x, y - 0x28);
    entity->ext.door.openHeight = found;
    switch (entity->spawnParam0)
    {
    case 1:
        entity->state = 0x14;
        break;
    case 3:
        entity->state = 0x63;
        break;
    default:
        entity->state = found;
        break;
    }
}

#define BG &gEwramData->bgInfo[1]

/**
 * @brief 4E0F8 | Shutter door (object 0x02): like Object04Update, but
 * variant 7 throws sparks (Effect_SpawnShared) every fourth frame while closed,
 * state 10 closes once the player is out of the 0x38 x 0x22 box and state 11
 * also clears bit 15 of DISPCNT.
 */
void Object02Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player = gEwramData->playerState.player;

    entity->hidden = Entity_IsInRect(entity, -0x20, 0, 0x130, 0xD0) ? 0 : 1;
    switch (entity->state)
    {
    case 0:
    {
        s32 ex, x, y, t;

        if (entity->spawnParam1 == 7 && entity->ext.door.openHeight == 0)
        {
            entity->timer++;
            if (!(entity->timer & 3))
            {
                s32 sx = entity->posX.whole + RandomSigned(0x80000);
                Effect_SpawnShared(sx, entity->posY.whole + ((RandomNumberGenerator() % 0x30 - 0x30) << 16), 0x10, 9);
            }
        }
        ex = (s16)entity->posX.part16.integer;
        if ((u32)(ex - (s16)player->posX.part16.integer + 0x28) <= 0x4F)
        {
            struct BgLayerState *bg = BG;
            x = (s16)bg->xPos.part16.integer;
            x = ex - -x; /* "ex + x" with ex as the first operand */
            y = (s16)entity->posY.part16.integer;
            y += (s16)bg->yPos.part16.integer;
            Collision_ClearBlock(x, y - 8);
            Collision_ClearBlock(x, y - 0x18);
            y -= 0x28;
            Collision_ClearBlock(x, y);
            if (entity->ext.door.openHeight == 0 && Entity_IsInRect(entity, -0x10, 0, 0x110, 0xC0))
                PlaySong(0x113);
            if ((entity->ext.door.openHeight += 2) > 0x2F)
                entity->ext.door.openHeight = 0x30;
            break;
        }
        t = entity->ext.door.openHeight;
        if (t <= 2)
        {
            struct BgLayerState *bg2 = BG;
            x = ex + (s16)bg2->xPos.part16.integer;
            y = (s16)entity->posY.part16.integer + (s16)bg2->yPos.part16.integer;
            if (t != 0)
            {
                Collision_SetBlock(x, y - 8);
                Collision_SetBlock(x, y - 0x18);
                Collision_SetBlock(x, y - 0x28);
            }
            entity->ext.door.openHeight = 0;
            break;
        }
        if (t == 0x30 && Entity_IsInRect(entity, -0x10, 0, 0x110, 0xC0))
            PlaySong(0x113);
        entity->ext.door.openHeight -= 2;
        break;
    }
    case 21:
    {
        struct BgLayerState *bg;
        s32 x, y;

        if (++entity->ext.door.openHeight <= 0x2F)
            break;
        x = (s16)entity->posX.part16.integer;
        bg = BG;
        x += (s16)bg->xPos.part16.integer;
        y = (s16)entity->posY.part16.integer;
        y += (s16)bg->yPos.part16.integer;
        Collision_ClearBlock(x, y - 8);
        Collision_ClearBlock(x, y - 0x18);
        y -= 0x28;
        Collision_ClearBlock(x, y);
        entity->state++;
        break;
    }
    case 2:
        SetPlayerInput(1, entity->ext.door.onRight ? 0x10 : 0x20);
        break;
    case 10:
    {
        struct BgLayerState *bg;
        s32 x, y;

        SetPlayerInput(1, entity->ext.door.onRight ? 0x20 : 0x10);
        {
            struct EwramData_EntityData *player = gEwramData->playerState.player;
            if (Entity_IsInRect(entity, (s16)(player->posX.part16.integer - 0x1C), (s16)player->posY.part16.integer, 0x38, 0x22))
                break;
        }
        x = (s16)entity->posX.part16.integer;
        bg = BG;
        x += (s16)bg->xPos.part16.integer;
        y = (s16)entity->posY.part16.integer;
        y += (s16)bg->yPos.part16.integer;
        Collision_SetBlock(x, y - 8);
        Collision_SetBlock(x, y - 0x18);
        y -= 0x28;
        Collision_SetBlock(x, y);
        PlaySong(0x113);
        entity->state++;
        break;
    }
    case 11:
        SetPlayerInput(1, 0);
        if (--entity->ext.door.openHeight != 0)
            break;
        gEwramData->gameState.stateFlags &= 0xFFFFFEFE;
        gUnk_03002CB0.dispCnt &= 0x7FFF;
        if (entity->spawnParam0 > 1)
            entity->state = 0x63;
        else
            entity->state = 0x14;
        break;
    case 20:
        if ((gEwramData->gameState.bossEventFlags >> entity->spawnParam1) & 1)
        {
            if (!(gEwramData->gameState.stateFlags & 0x400000))
                Music_SubmitCommandOnce(0x1040);
            PlaySong(0x113);
            entity->state++;
        }
        break;
    }
}

/* Initialize object 0x03's window mask, three vertical tile positions and variant state. */
void Object03Create(struct EwramData_EntityData *entity)
{
    s32 x, y;
    struct BgLayerState *bg;
    Door_SetupGraphics(entity);
    gDisplayRegisters.winOut_L = 0x3F;
    gDisplayRegisters.winOut_H = 0x2F;
    x = (s16)entity->posX.part16.integer;
    bg = &gEwramData->bgInfo[1];
    x += (s16)bg->xPos.part16.integer;
    y = (s16)entity->posY.part16.integer;
    y += (s16)bg->yPos.part16.integer;
    Collision_SetBlock(x, y - 8);
    Collision_SetBlock(x, y - 24);
    Collision_SetBlock(x, y - 40);
    entity->ext.door.openHeight = 0;
    switch (entity->spawnParam0)
    {
    case 1: entity->state = 20; break;
    case 3: entity->state = 99; break;
    default: entity->state = 0; break;
    }
}

/**
 * @brief 4E52C | Door object (object 0x03): hidden off screen; in state 0
 * it walks the player towards it (SetPlayerInput) when the event flag
 * linkPlay is set and the player (and both followers linkPartner / linkTargetProxy)
 * stand within 0x13 px, then waits 4 frames (state 1) and opens (state 2):
 * clears the door tiles (Collision_ClearBlock at three heights, sound 0x113), clears
 * stateFlags bits 0 and 8 after 48 frames and keeps the player walking (state 3).
 */
void Object03Update(struct EwramData_EntityData *entity)
{
    struct EwramData *ew;
    struct EwramData_EntityData *p2;
    struct BgLayerState *bg;
    s32 x;
    s32 y;

    entity->hidden = Entity_IsInRect(entity, -0x20, 0, 0x130, 0xD0) ? 0 : 1;
    switch (entity->state)
    {
    case 0:
        if (gEwramData->playerState.selectedTarget)
            break;
        if (!gEwramData->gameState.linkPlay)
            goto open;
        if ((u32)((s16)entity->posX.part16.integer - (s16)gEwramData->playerState.player->posX.part16.integer + 0x18) <= 0x2F)
            SetPlayerInput(1, entity->flipX ? 0x20 : 0x10);
        if ((u32)((s16)entity->posX.part16.integer - (s16)gEwramData->playerState.player->posX.part16.integer + 0x13) > 0x26)
        {
            entity->timer = 0;
            break;
        }
        ew = gEwramData;
        if (!ew->gameState.linkPlay)
            break;
        p2 = ew->playerState.linkPartner;
        if ((u32)((s16)entity->posX.part16.integer - (s16)p2->posX.part16.integer + 0x13) > 0x26)
            break;
        if ((s16)ew->playerState.linkTargetProxy->posX.part16.integer != (s16)p2->posX.part16.integer)
            break;
        entity->timer = 4;
        SetPlayerInput(1, entity->flipX ? 0x20 : 0x10);
        entity->state = 1;
        break;
    case 1:
        SetPlayerInput(1, entity->flipX ? 0x20 : 0x10);
        if (entity->timer)
        {
            entity->timer--;
            break;
        }
    open:
        entity->state = 2;
        break;
    case 2:
        if (gEwramData->gameState.linkPlay)
            SetPlayerInput(1, entity->flipX ? 0x20 : 0x10);
        if (entity->ext.door.openHeight == 0)
        {
            x = (s16)entity->posX.part16.integer;
            bg = &gEwramData->bgInfo[1];
            x += (s16)bg->xPos.part16.integer;
            y = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer;
            Collision_ClearBlock(x, y - 8);
            Collision_ClearBlock(x, y - 0x18);
            Collision_ClearBlock(x, y - 0x28);
            PlaySong(0x113);
        }
        if (++entity->ext.door.openHeight > 0x2F)
        {
            gEwramData->gameState.stateFlags &= 0xFFFFFEFE;
            if (gEwramData->gameState.linkPlay)
                entity->state++;
        }
        break;
    case 3:
        SetPlayerInput(1, entity->flipX ? 0x20 : 0x10);
        break;
    }
}

/**
 * @brief 4E770 | Object 4 init (one-way door, as Object02Create): deleted
 * when spawnParam0 <= 1 and bit spawnParam1 of bossEventFlags is set; Door_SetupGraphics, window
 * outside 0x3F / 0x2F. The player counts as passing when at most 0x18 px
 * right of it (unk_4FE == 0) or at most 0x18 px left of it (otherwise):
 * then unk_4F8 = 0x30, state 10, clear player input, RedSoul_CancelAll and stop
 * the player when moving towards the room centre; otherwise mark three
 * tiles above it and pick the state from spawnParam0.
 */
void Object04Create(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player;
    struct EwramData_EntityData **pp;
    struct BgLayerState *bg;
    s32 found;
    s32 x, y;

    if (entity->spawnParam0 <= 1
        && ((gEwramData->gameState.bossEventFlags >> entity->spawnParam1) & 1))
    {
        EntityDelete(entity);
        return;
    }
    Door_SetupGraphics(entity);
    gDisplayRegisters.winOut_L = 0x3F;
    gDisplayRegisters.winOut_H = 0x2F;
    if (entity->ext.door.onRight == 0)
    {
        if ((s16)gEwramData->playerState.player->posX.part16.integer <= (s16)entity->posX.part16.integer + 0x18)
            goto passed;
        goto blocked;
    }
    else
    {
        s32 px = (s16)gEwramData->playerState.player->posX.part16.integer;
        s32 ex = (s16)entity->posX.part16.integer - 0x18;
        found = 0;
        if (px >= ex)
        {
        passed:
            found = 1;
        }
    }
    if (found)
    {
        entity->ext.door.openHeight = 0x30;
        found = 0;
        entity->state = 10;
        SetPlayerInput(1, 0);
        RedSoul_CancelAll();
        pp = &gEwramData->playerState.player;
        player = *pp;
        if ((s16)player->posX.part16.integer <= 0x77)
        {
            if (player->velX.whole >= 0)
                return;
        }
        else if (player->velX.whole <= 0)
            return;
        player->velX.whole = found;
        (*pp)->accelX.whole = found;
        return;
    }
blocked:
    x = (s16)entity->posX.part16.integer;
    bg = &gEwramData->bgInfo[1];
    x += (s16)bg->xPos.part16.integer;
    y = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer;
    Collision_SetBlock(x, y - 8);
    Collision_SetBlock(x, y - 0x18);
    Collision_SetBlock(x, y - 0x28);
    entity->ext.door.openHeight = 0;
    switch (entity->spawnParam0)
    {
    case 1:
        entity->state = 0x14;
        break;
    case 3:
        entity->state = 0x63;
        break;
    default:
        entity->state = 0;
        break;
    }
}

static inline s32 IsBeside(struct EwramData_EntityData *entity)
{
    if (!entity->ext.door.onRight)
    {
        if ((s16)gEwramData->playerState.player->posX.part16.integer > (s16)entity->posX.part16.integer + 0x18)
            return 0;
    }
    else if ((s16)gEwramData->playerState.player->posX.part16.integer < (s16)entity->posX.part16.integer - 0x18)
        return 0;
    return 1;
}

/**
 * @brief 4E8C4 | Shutter door (object 0x04): hidden off screen. State 0
 * opens it while the player is within 0x28 px (tiles cleared with
 * Collision_ClearBlock, sound 0x113, counter unk_4F8 up to 0x30) and closes it again
 * (Collision_SetBlock) when the player has left; states 10/11 close it behind the
 * player and go to 0x63 / 0x14 (spawnParam0); state 20 waits for its boss flag
 * (bit spawnParam1 of bossEventFlags, map reveal 0x1040) and state 21 opens it for good.
 */
void Object04Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player = gEwramData->playerState.player;

    entity->hidden = Entity_IsInRect(entity, -0x20, 0, 0x130, 0xD0) ? 0 : 1;
    switch (entity->state)
    {
    case 0:
    {
        s32 ex, x, y, t;

        ex = (s16)entity->posX.part16.integer;
        if ((u32)(ex - (s16)player->posX.part16.integer + 0x28) <= 0x4F)
        {
            struct BgLayerState *bg = BG;
            x = (s16)bg->xPos.part16.integer;
            x = ex - -x; /* "ex + x" with ex as the first operand */
            y = (s16)entity->posY.part16.integer;
            y += (s16)bg->yPos.part16.integer;
            Collision_ClearBlock(x, y - 8);
            Collision_ClearBlock(x, y - 0x18);
            y -= 0x28;
            Collision_ClearBlock(x, y);
            if (entity->ext.door.openHeight == 0 && Entity_IsInRect(entity, -0x10, 0, 0x110, 0xC0))
                PlaySong(0x113);
            if ((entity->ext.door.openHeight += 2) > 0x2F)
                entity->ext.door.openHeight = 0x30;
            break;
        }
        t = entity->ext.door.openHeight;
        if (t <= 2)
        {
            struct BgLayerState *bg2 = BG;
            x = ex + (s16)bg2->xPos.part16.integer;
            y = (s16)entity->posY.part16.integer + (s16)bg2->yPos.part16.integer;
            if (t != 0)
            {
                Collision_SetBlock(x, y - 8);
                Collision_SetBlock(x, y - 0x18);
                Collision_SetBlock(x, y - 0x28);
            }
            entity->ext.door.openHeight = 0;
            break;
        }
        if (t == 0x30 && Entity_IsInRect(entity, -0x10, 0, 0x110, 0xC0))
            PlaySong(0x113);
        entity->ext.door.openHeight -= 2;
        break;
    }
    case 21:
    {
        struct BgLayerState *bg;
        s32 x, y;

        if (++entity->ext.door.openHeight <= 0x2F)
            break;
        x = (s16)entity->posX.part16.integer;
        bg = BG;
        x += (s16)bg->xPos.part16.integer;
        y = (s16)entity->posY.part16.integer;
        y += (s16)bg->yPos.part16.integer;
        Collision_ClearBlock(x, y - 8);
        Collision_ClearBlock(x, y - 0x18);
        y -= 0x28;
        Collision_ClearBlock(x, y);
        entity->state++;
        break;
    }
    case 2:
        SetPlayerInput(1, entity->ext.door.onRight ? 0x10 : 0x20);
        break;
    case 10:
    {
        struct BgLayerState *bg;
        s32 x, y;

        SetPlayerInput(1, entity->ext.door.onRight ? 0x20 : 0x10);
        if (IsBeside(entity))
            break;
        x = (s16)entity->posX.part16.integer;
        bg = BG;
        x += (s16)bg->xPos.part16.integer;
        y = (s16)entity->posY.part16.integer;
        y += (s16)bg->yPos.part16.integer;
        Collision_SetBlock(x, y - 8);
        Collision_SetBlock(x, y - 0x18);
        y -= 0x28;
        Collision_SetBlock(x, y);
        PlaySong(0x113);
        entity->state++;
        break;
    }
    case 11:
        SetPlayerInput(1, 0);
        if (--entity->ext.door.openHeight != 0)
            break;
        gEwramData->gameState.stateFlags &= 0xFFFFFEFE;
        if (entity->spawnParam0 > 1)
            entity->state = 0x63;
        else
            entity->state = 0x14;
        break;
    case 20:
        if ((gEwramData->gameState.bossEventFlags >> entity->spawnParam1) & 1)
        {
            if (!(gEwramData->gameState.stateFlags & 0x400000))
                Music_SubmitCommandOnce(0x1040);
            PlaySong(0x113);
            entity->state++;
        }
        break;
    }
}

/**
 * @brief 4EC3C | Object 6 init (as Object00Create): Door_SetupGraphics, palette
 * palette = 5, window outside 0x3F / 0x2F; when the player is inside the
 * 0x38 x 0x22 area 0x1C px to its left or bossEventFlags bit 2 is set, set unk_4F8
 * to 0x30, state 10 and clear player input; otherwise mark three tiles
 * above it (Collision_SetBlock), reset unk_4F8 and pick the state from spawnParam0
 * (1: 0x14, 3: 0x63, else 0).
 */
void Object06Create(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player;
    struct BgLayerState *bg;
    u16 flag;
    s32 x, y;

    Door_SetupGraphics(entity);
    entity->palette = 5;
    gDisplayRegisters.winOut_L = 0x3F;
    gDisplayRegisters.winOut_H = 0x2F;
    player = gEwramData->playerState.player;
    if (Entity_IsInRect(entity, (s16)(player->posX.part16.integer - 0x1C), player->posY.part16.integer, 0x38, 0x22)
        || (flag = gEwramData->gameState.bossEventFlags & 4) != 0)
    {
        entity->ext.door.openHeight = 0x30;
        entity->state = 10;
        SetPlayerInput(1, 0);
        return;
    }
    x = (s16)entity->posX.part16.integer;
    bg = &gEwramData->bgInfo[1];
    x += (s16)bg->xPos.part16.integer;
    y = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer;
    Collision_SetBlock(x, y - 8);
    Collision_SetBlock(x, y - 0x18);
    Collision_SetBlock(x, y - 0x28);
    entity->ext.door.openHeight = flag;
    switch (entity->spawnParam0)
    {
    case 1:
        entity->state = 0x14;
        break;
    case 3:
        entity->state = 0x63;
        break;
    default:
        entity->state = flag;
        break;
    }
}

/* Door transition and effects. Explicit register locals retain agbcc output;
 * they do not establish the original source spelling. */

void Object06Update(struct EwramData_EntityData *entity)
{
    struct EwramData *ew;
    struct EwramData_EntityData *p2;
    struct BgLayerState *bg;
    s32 x;
    s32 y;
    s32 speed;
    s32 vx;
    struct EwramData_EntityData *fx;

    entity->hidden = Entity_IsInRect(entity, -0x20, 0, 0x130, 0xD0) ? 0 : 1;
    switch (entity->state)
    {
    case 0: {
        register u32 flags asm("r1") = gEwramData->gameState.bossEventFlags; /* FAKEMATCH: pinned register */
        u32 scratch = 1;
        register u32 one asm("sl") = scratch; /* FAKEMATCH: pinned register */
        { u32 test = one;
          test &= flags;
        if (test)
            goto open;
        }
        if (!BgPalette_GetPendingFades())
        {
            BgPalette_StopColorFade();
            ObjPalette_StopColorFade();
        }
        { s32 counter = entity->timer + 1;
          entity->timer = counter;
        if (counter & one)
        {
            Effect_SpawnShared(entity->posX.whole + RandomSigned(0x80000), entity->posY.whole + ((RandomNumberGenerator() % 0x30 - 0x30) << 16), 0x10, 5);
        }
        }
        speed = 0x40000;
        fx = Effect_CreateFromTable(entity->posX.whole + RandomSigned(speed) + speed, entity->posY.whole + ((RandomNumberGenerator() % 0x30 - 0x30) << 16), RandomSigned(0x400) + 0x1000, RandomSigned(0x400), 7, 5);
        if (fx != NULL)
            fx->spriteLayer = 0xE;
        p2 = gEwramData->playerState.player;
        if (p2->ext.player.actionFlags & 0x80)
            break;
        if (!Entity_IsInRect(entity, (s16)(p2->posX.part16.integer - 0x1C), (s16)p2->posY.part16.integer, 0x38, 0x22))
            break;
        PlaySong(0x111);
        BgPalette_StartColorFade(1, 1, 1, 0x4000, 0x10000, 0x10);
        ObjPalette_StartColorFade(1, 1, 1, 0x4000, 0x10000, 0x10);
        ew = gEwramData;
        ew->playerState.pendingHitDamage = one;
        ew->playerState.hitReaction = 3;
        ew->playerState.hitSourceX = ew->playerState.player->posX.whole + 0x80000;
        ew->playerState.hitSourceY = ew->playerState.player->posY.whole;
        Player_SetKnockbackVelocity(speed, -0x10000);
        break;
    }
    case 1:
        SetPlayerInput(1, entity->flipX ? 0x20 : 0x10);
        if (entity->timer)
        {
            entity->timer--;
            break;
        }
    open:
        entity->state = 2;
        break;
    case 2:
        if (gEwramData->gameState.linkPlay)
            SetPlayerInput(1, entity->flipX ? 0x20 : 0x10);
        if (entity->ext.door.openHeight == 0)
        {
            x = (s16)entity->posX.part16.integer;
            bg = &gEwramData->bgInfo[1];
            x += (s16)bg->xPos.part16.integer;
            y = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer;
            Collision_ClearBlock(x, y - 8);
            Collision_ClearBlock(x, y - 0x18);
            Collision_ClearBlock(x, y - 0x28);
        }
        if (++entity->ext.door.openHeight > 0x2F)
        {
            gEwramData->gameState.stateFlags &= 0xFFFFFEFE;
            if (gEwramData->gameState.linkPlay)
                entity->state++;
        }
        break;
    case 3:
        SetPlayerInput(1, entity->flipX ? 0x20 : 0x10);
        break;
    }
}

/* Object 0x05 (door / gate opening near the player), 0x0804F00C..0x0804F15F. */
/**
 * @brief 4F00C | Object 0x05 create: drawing set up by Door_SetupGraphics, frame
 * 0x7D, palette 7, window outside 0x3F / 0x2F; start open (unk_4F8 = 0x30)
 * when the player is already in front of it.
 */
void Object05Create(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player;

    Door_SetupGraphics(entity);
    entity->animFrame = 0x7D;
    entity->palette = 7;
    gDisplayRegisters.winOut_L = 0x3F;
    gDisplayRegisters.winOut_H = 0x2F;
    player = gEwramData->playerState.player;
    if (Entity_IsInRect(entity, (s16)(player->posX.part16.integer - 0x1C), player->posY.part16.integer, 0x38, 0x22))
        entity->ext.door.openHeight = 0x30;
}

/**
 * @brief 4F084 | Object 0x05 update: hidden off screen; opens (unk_4F8 up
 * to 0x30 in steps of 2) while the player is within 0x28 px horizontally,
 * closes otherwise, with sound 0x113 when it starts moving on screen.
 */
void Object05Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player = gEwramData->playerState.player;

    entity->hidden = Entity_IsInRect(entity, -0x20, 0, 0x130, 0xD0) ? 0 : 1;
    if (entity->state != 0)
        return;

    if ((u32)((s16)entity->posX.part16.integer - (s16)player->posX.part16.integer + 0x28) <= 0x4F)
    {
        if (entity->ext.door.openHeight == 0 && Entity_IsInRect(entity, -0x10, 0, 0x110, 0xC0))
            PlaySong(0x113);
        entity->ext.door.openHeight += 2;
        if (entity->ext.door.openHeight > 0x2F)
            entity->ext.door.openHeight = 0x30;
    }
    else
    {
        if (entity->ext.door.openHeight == 0x30 && Entity_IsInRect(entity, -0x10, 0, 0x110, 0xC0))
            PlaySong(0x113);
        if (entity->ext.door.openHeight <= 2)
            entity->ext.door.openHeight = 0;
        else
            entity->ext.door.openHeight -= 2;
    }
}

extern const s32 sUnk_0852559C[];

/**
 * @brief 4F160 | Floating background particle: state 0 loads graphics and
 * palette for kind spawnParam0 (0 or 2; deleteRequested on failure) and starts
 * animation unk_500 at a random frame; state 1 drifts on a sine path
 * (kind 3 uses a second angle unk_4FC), scrolls against the camera
 * velocity scaled by sUnk_0852559C[kind] and wraps around the screen
 * edges at a random position. Movement pauses while timeStopTimer is set.
 */
void BgParticle_Update(struct EwramData_EntityData *entity)
{
    s32 gfx;
    s32 pal;
    s32 angle;
    s32 vy;
    const s32 *k;
    struct BgLayerState *bg;

    switch (entity->state)
    {
    case 0:
        if (entity->spawnParam0 == 0)
        {
            gfx = ObjGfx_LoadResource((u8 *)0x081CBE5C, 0);
            if (gfx < 0)
                goto fail;
            pal = ObjPalette_FindOrLoad((void *)0x08209EE8);
            if (pal < 0)
                goto fail;
            Sprite_Attach(entity, (void *)0x0820FE44, gfx, pal);
        }
        else if (entity->spawnParam0 == 2)
        {
            gfx = ObjGfx_LoadResource((u8 *)0x081CBEB4, 0);
            if (gfx < 0 || (pal = ObjPalette_FindOrLoad((void *)0x0820A100)) < 0)
            {
            fail:
                entity->deleteRequested = 1;
                return;
            }
            Sprite_Attach(entity, (void *)0x08210DB0, gfx, pal);
        }
        Anim_StartIndexed(entity, entity->ext.bgParticle.kind, 0, 1);
        entity->animStep = RandomNumberGenerator() & 1;
        Anim_Update(entity);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->state++;
        break;
    case 1:
        if (!gEwramData->gameState.timeStopTimer)
        {
            entity->ext.bgParticle.angleX += 0x800;
            if (entity->ext.bgParticle.kind != 3)
            {
                entity->velX.whole = entity->accelX.whole;
                vy = entity->accelY.whole * Sin(entity->ext.bgParticle.angleX) / 0x10000;
            }
            else
            {
                entity->ext.bgParticle.angleY += 0xA00;
                entity->velX.whole = Sin(entity->ext.bgParticle.angleX + 0x4000) * entity->accelX.whole / 0x1000;
                vy = entity->accelY.whole * Sin(entity->ext.bgParticle.angleY) / 0x1000;
            }
            entity->velY.whole = vy;
            entity->posX.whole += entity->velX.whole;
            entity->posY.whole += vy;
            Anim_Update(entity);
        }
        {
            const s32 *tbl = sUnk_0852559C;
            k = &tbl[entity->ext.bgParticle.kind];
        }
        bg = &gEwramData->bgInfo[1];
        entity->posX.whole -= *k * bg->xVel.whole / 256;
        entity->posY.whole -= *k * bg->yVel.whole / 256;
        if ((s16)entity->posX.part16.integer > 0x110)
        {
            entity->posY.part16.integer = (RandomNumberGenerator() & 0x7F) + 1;
            entity->posX.part16.integer = -0x20;
        }
        else if ((s16)entity->posX.part16.integer < -0x20)
        {
            entity->posY.part16.integer = (RandomNumberGenerator() & 0x7F) + 1;
            entity->posX.part16.integer = 0x110;
        }
        else if ((s16)entity->posY.part16.integer > 0xC0)
        {
            entity->posY.part16.integer = -0x10;
            entity->posX.part16.integer = RandomNumberGenerator() & 0xBF;
        }
        else if ((s16)entity->posY.part16.integer < -0x20)
        {
            entity->posY.part16.integer = 0xB0;
            entity->posX.part16.integer = RandomNumberGenerator() & 0xBF;
        }
        break;
    }
}

extern const s32 sBgParticleCounts[];

/**
 * @brief 4F374 | Spawn sBgParticleCounts[spawnParam1] particles of four sizes with
 * random positions and velocities (size 3 is placed relative to the scroll).
 * Returns no value; r0 is left as the original leaves it.
 */
s32 BgParticle_SpawnAll(struct EwramData_EntityData *owner)
{
    s32 i;
    struct EwramData_EntityData *entity;

    for (i = 0; i < sBgParticleCounts[owner->spawnParam1]; i++)
    {
                entity = EntityCreateInRange(0x49, 0xCF, (u32 *)BgParticle_Update);
        if (!entity)
            continue;
        entity->ext.bgParticle.ownerIndex = ENTITY_INDEX(owner);
        entity->ext.bgParticle.kind = RandomNumberGenerator() & 0xF;
        if (entity->ext.bgParticle.kind > 8)
        {
            entity->ext.bgParticle.kind = 2;
            entity->spriteLayer = 0x1D;
        }
        else if (entity->ext.bgParticle.kind > 5)
        {
            entity->ext.bgParticle.kind = 1;
            entity->spriteLayer = 0x1A;
        }
        else if (entity->ext.bgParticle.kind > 1)
        {
            entity->ext.bgParticle.kind = 0;
            entity->spriteLayer = 0x19;
        }
        else
        {
            entity->ext.bgParticle.kind = 3;
            entity->spriteLayer = 0x1D;
        }
        if (entity->ext.bgParticle.kind != 3)
        {
            entity->posX.part16.integer = (RandomNumberGenerator() & 0x3F) * 4 + 0xFFE0;
            entity->posY.part16.integer = (RandomNumberGenerator() & 0x7F) + 1;
            entity->accelX.whole = ((RandomNumberGenerator() & 0x3FFF) + 0xC000) >> entity->ext.bgParticle.kind;
            entity->accelY.whole = ((RandomNumberGenerator() & 0x1FFF) + 0x8000) >> entity->ext.bgParticle.kind;
        }
        else
        {
            entity->posX.part16.integer = 0x8C - ((s16)(gEwramData->bgInfo + 1)->xPos.part16.integer >> 2) + (RandomNumberGenerator() & 0x3F);
            entity->posY.part16.integer = (RandomNumberGenerator() & 0x3F) + 0x21;
            entity->accelX.whole = ((RandomNumberGenerator() & 0x7F) << 4) + 0x800;
            entity->accelY.whole = ((RandomNumberGenerator() & 0x7F) << 4) + 0x800;
            entity->ext.bgParticle.angleY = (RandomNumberGenerator() & 0x1F) << 12;
        }
        if (entity->flipX)
            entity->accelX.whole = -entity->accelX.whole;
        entity->ext.bgParticle.angleX = (RandomNumberGenerator() & 0x1F) << 12;
    }
}

extern u8 sObject07ParallaxSpriteGfx[], sObject07ParallaxSpritePalette[], sObject07ParallaxSpriteSpriteSheet[];
/**
 * @brief Load the semi-transparent background sprite, then drift with its
 * velocity and 1/12 of the camera motion (parallax).
 */
void Object07ParallaxSprite_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
    {
        s32 graphics = ObjGfx_LoadResource(sObject07ParallaxSpriteGfx, 0);
        s32 palette;
        if (graphics < 0 || (palette = (s8)ObjPalette_FindOrLoad(sObject07ParallaxSpritePalette)) < 0)
        {
            entity->deleteRequested = 1;
            break;
        }
        Sprite_Attach(entity, sObject07ParallaxSpriteSpriteSheet, graphics, palette);
        entity->animFrame = entity->ext.object07.spriteFrame;
        entity->spriteLayer = 0x1E;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->objMode = 1;
        entity->state++;
    }
    /* fall through */
    case 1:
    {
        s32 x, y;
        struct BgLayerState *camera;
        entity->velX.whole = entity->accelX.whole;
        x = entity->posX.whole += entity->velX.whole;
        y = entity->posY.whole += entity->velY.whole;
        camera = &gEwramData->bgInfo[1];
        entity->posX.whole = x - camera->xVel.whole / 12;
        entity->posY.whole = y - camera->yVel.whole / 12;
        break;
    }
    }
}

/* Object07_SpawnParallaxSprites. */
struct Offset16 { u16 x, y; };
extern const u8 sUnk_085255DC[];
extern const u16 sUnk_085255C4[];

/**
 * @brief Object 0x07 type 0: create four background sprites from
 * sUnk_085255DC, placed by the camera position scaled with the room
 * size plus the variant offset.
 */
void Object07_SpawnParallaxSprites(struct EwramData_EntityData *entity)
{
    struct BgMapMetadata *meta = gEwramData->bgInfo[1].pBgMetadata;
    s32 i;

    for (i = 0; i <= 3; i++)
    {
        struct EwramData_EntityData *e = EntityCreateInRange(0x49, 0xCF, (u32 *)Object07ParallaxSprite_Update);
        if (e != NULL)
        {
            const u8 *entry;
            struct BgLayerState *cam;
            { const u8 *tbl = sUnk_085255DC; entry = tbl + i * 4; }
            e->ext.object07.spriteFrame = entry[2];
            {
                s32 x = entry[0] + 0x50;
                cam = &gEwramData->bgInfo[1];
                e->posX.part16.integer = x - ((s16)cam->xPos.part16.integer * meta->widthCells) / 64
                    + sUnk_085255C4[entity->spawnParam1 * 2];
            }
            e->posY.part16.integer = entry[1] - ((s16)cam->yPos.part16.integer * meta->heightCells) / 64
                + (sUnk_085255C4 + 1)[entity->spawnParam1 * 2];
            e->accelX.whole = 0x80;
        }
    }
}

/* Object 0x07 (room effects) and the room table sWarpRoomEntries,
 * 0x0804F690..0x0804F89F. */

/* Declared here with an int result: Warp_ActivateCurrentRoom uses the value without
 * re-extending it from s16 (see bg_and_collision.h for the real type). */
extern s32 GetEntityRoomXPositionInteger(struct EwramData_EntityData *);
extern s32 GetEntityRoomYPositionInteger(struct EwramData_EntityData *);

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
