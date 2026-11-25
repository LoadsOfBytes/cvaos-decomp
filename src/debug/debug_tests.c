#include "debug_tests.h"
#include "core.h"
#include "bg_and_collision.h"
#include "in_game.h"

#include "room_entities.h"
#include "map_screen.h"
#include "debug_menu.h"
#include "graphics.h"
#include "gba.h"

#include "constants/main.h"

#include "data/interrupt_table.h"
#include "data/debug_font.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "enemy_common.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "palette.h"
#include "data/data_0E5364.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"

/* Called with an argument, which the function ignores (bg_effects.h is not
 * included: it declares the function without parameters). */
extern void Bg_UpdateScrollModes(s32 unused);

extern int sprintf(char *, const char *, ...);

const u8 sUnk_080e202c[] = "../graph/dr_objhit.h";
const u8 sUnk_080e2044[] = "obj_";
const u8 sUnk_080e204c[] = "../graph/dr_objhit.h";
const u8 sUnk_080e2064[] = "obj_";

static struct DebugDisplayTestState sDebugDisplayTestState;

struct DebugDisplayTestCallbacks {
    void (*updateFunc)(struct DebugDisplayTestState *test);
    void (*initFunc)(struct DebugDisplayTestState *test);
    void (*exitFunc)(struct DebugDisplayTestState *test);
}; /* size = 0xC */
struct DebugDisplayTestCallbacks sDebugDisplayTestCallbacks[] = {
    [0] = {
        .updateFunc = DebugBgDispTest_Update,
        .initFunc = DebugBgDispTest_Init,
        .exitFunc = DebugBgDispTest_Exit
    },
    [1] = {
        .updateFunc = DebugLinkTest_Update,
        .initFunc = DebugLinkTest_Init,
        .exitFunc = DebugLinkTest_Exit
    },
    [2] = {
        .updateFunc = DebugHitTest_Update,
        .initFunc = DebugHitTest_Init,
        .exitFunc = DebugHitTest_Exit
    }
};

const u16 sDebugDispTestPalette[] = {
    0x7FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000
};

const u8 sUnk_080e207a[0x080e226c - 0x080e207a] = { 0 };

/**
 * @brief Run BG test 0 (DISP), 1 (LINK) or 2 (VRAM HIT) through its
 * init/update/exit callbacks; Select+B exits. Return 1 when done.
 */
s32 DebugDispTest_Run(s32 testIndex)
{
    s32 done;
    struct DebugDisplayTestCallbacks *def;

    done = 0;
    def = &sDebugDisplayTestCallbacks[testIndex];

    switch (DebugTest_GetStage())
    {
        case 0:
            DMA_FILL_32(3, 0, &sDebugDisplayTestState, sizeof(sDebugDisplayTestState));
            BgCmdBuffer_WriteString(2, 0x13, 0xF, "PUSH SELECT + B TO EXIT.");
            def->initFunc(&sDebugDisplayTestState);
            DMA_COPY_16(3, &sDebugDispTestPalette, &gEwramData->bgPalette.base[0xF], 0x10);
            gEwramData->bgPalette.uploaded = 0;
            DebugTest_SetStage(1);
            break;

        case 1:
            Entity_UpdateAll();
            def->updateFunc(&sDebugDisplayTestState);
            if ((gEwramData->inputData.heldInput & (4 | 2)) == (4 | 2))
            {
                DebugTest_SetStage(2);
            }
            break;

        case 2:
            def->exitFunc(&sDebugDisplayTestState);
            DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
            done = 1;
            break;
    }

    return done;
}

/**
 * @brief Scroll the test BG with the D-pad (faster with R).
 */
void DebugBgDispTest_Update(struct DebugDisplayTestState *test)
{
    u8 pad[0x14];
    s32 speed;
    s32 deltaY;
    s32 deltaX;

    deltaX = 0;
    deltaY = 0;

    speed = 0x20000;
    if (gEwramData->inputData.playerHeldInput & 0x100)
    {
        speed *= 2;
    }

    if (gEwramData->inputData.playerHeldInput & 0x20)
    {
        deltaX = -speed;
    }
    else if (gEwramData->inputData.playerHeldInput & 0x10)
    {
        deltaX = speed;
    }

    if (gEwramData->inputData.playerHeldInput & 0x40)
    {
        deltaY -= speed;
    }
    else if (gEwramData->inputData.playerHeldInput & 0x80)
    {
        deltaY += speed;
    }

    BgMap_ScrollWithMargin(test->target.bgIndex, deltaX, deltaY);
    Entity_ScrollWithBg(test->target.bgIndex);
    Bg_UpdateScrollModes(0);
}

/**
 * @brief Load the object graphics and room sDebugEventMenuRoom and create the
 * DebugLinkTestWalker player stand-in.
 */
void DebugLinkTest_Init(struct DebugDisplayTestState *test)
{
    Display_SetDefaultBlend();
    Room_ResetTransientState();
    InGame_LoadCommonObjectPalettes();
    InGame_LoadCommonObjectGraphics();

    test->player = EntityCreate(DebugLinkTestWalker_Update);
    test->target.roomDefinition = (void*)sDebugEventMenuRoom;
    test->clearBgIndex = 1;
    Room_Load((struct RoomDefinition *)sDebugEventMenuRoom, 0, 0);
    gEwramData->playerState.player = test->player;
    gEwramData->playerState.unk_131EA = 0x100;
    gEwramData->playerState.unk_131EC = 0x100;

    gEwramData->playerState.currentStats[0] = 0x40;
    gEwramData->playerState.currentDEF = 0x40;
    gEwramData->playerState.cameraAnchorX = 0x78;
    gEwramData->playerState.cameraAnchorY = 0x60;
}

/**
 * @brief Load room sDebugHitTestRoom and create the DebugHitTestCursor.
 */
void DebugHitTest_Init(struct DebugDisplayTestState *test)
{
    test->player = EntityCreate(DebugHitTestCursor_Update);
    test->target.roomDefinition = (void *)sDebugHitTestRoom;
    test->clearBgIndex = 1;
    Room_Load((struct RoomDefinition *)sDebugHitTestRoom, 0, 0);
    gEwramData->playerState.player = test->player;
    gEwramData->playerState.unk_131EA = 0x100;
    gEwramData->playerState.unk_131EC = 0x100;

    gEwramData->playerState.currentStats[0] = 0x40;
    gEwramData->playerState.currentDEF = 0x40;
    gEwramData->playerState.cameraAnchorX = 0x78;
    gEwramData->playerState.cameraAnchorY = 0x60;
}

void * const sDebugTestExitArgs[4] = {
    VRAM_BASE + 0xE000, VRAM_BASE + 0xE800, VRAM_BASE + 0xF000, VRAM_BASE + 0xF800
};

// extern void Bg_UpdateScrollModes(); // implicit (should have no arguments)
// extern s32 Collision_IsPointSolid(s32 param_0, s32 param_1); // implicit (should return s16)

const char sDebugHexByteFormat[] = "0x%02x";

/**
 * @brief Move freely with the D-pad, scroll BG1 to follow and print the
 * collision tile, position and push-out values at the cursor.
 */
void DebugHitTestCursor_Update(struct EwramData_EntityData *entity)
{
    char subroutine_arg0[0x10];
    s32 gfx;
    s32 speed;
    s16 roomX;
    s16 roomY;

    roomY = GetEntityRoomYPositionInteger(entity);
    roomX = GetEntityRoomXPositionInteger(entity);

    switch (entity->state)
    {
        case 0:
            gfx = ObjGfx_LoadResource((u8 *)sDebugJitterSpriteGfx, 0);
            ObjPalette_LoadRange((u8 *)sDebugJitterSpritePalette, 0, 1, 1);
            Sprite_Attach(entity, (void *)sDebugJitterSpriteSheet, gfx, 1);
            Anim_SelectScript(entity, 0, 1);
            entity->posX.part16.integer = 0x64;
            entity->posY.part16.integer = 0x90;
            entity->scrollWithBg = 1;
            entity->velX.whole = 0x18000;
            entity->velY.whole = 0;
            entity->state += 1;
            entity->spriteLayer = 0x10;
            break;

        case 1:
            entity->accelX.whole = 0;
            if (gEwramData->inputData.playerHeldInput & 0x20)
            {
                entity->posX.whole -= entity->velX.whole;
                entity->accelX.whole = -entity->velX.whole;
                entity->flipX = 1;
            }
            else if (gEwramData->inputData.playerHeldInput & 0x10)
            {
                entity->posX.whole += entity->velX.whole;
                entity->accelX.whole = entity->velX.whole;
                entity->flipX = 0;
            }

            entity->accelY.whole = 0;
            if (gEwramData->inputData.playerHeldInput & 0x40)
            {
                entity->posY.whole -= entity->velY.whole;
                entity->accelY.whole = -entity->velY.whole;                
            }
            else
            {
                if (gEwramData->inputData.playerHeldInput & 0x80)
                {
                    entity->posY.whole += entity->velY.whole;
                    entity->accelY.whole = entity->velY.whole;
                }
            }
            break;
    }
    
    if (gEwramData->inputData.playerHeldInput & 0x100)
    {
        speed = 0x40000;
    }
    else
    {
        speed = 0x18000;
    }
    entity->velX.whole = speed;
    entity->velY.whole = speed;

    entity->timer += 1;
    BgMap_ScrollWithMargin(1, ((s16) entity->posX.part16.integer - 0x78) << 0x10, (entity->posY.part16.integer - 0x60) << 0x10);
    Entity_ScrollWithBg(1);
    Bg_UpdateScrollModes(0);
    Entity_ScrollWithBg(1);
    Sprite_Draw(entity);

    sprintf(subroutine_arg0, sDebugHexByteFormat, Collision_GetTile(roomX, roomY));
    BgCmdBuffer_WriteString(2, 2, 0xF, subroutine_arg0);

    sprintf(subroutine_arg0, "0x%02x,0x%02x      ", roomX, roomY);
    BgCmdBuffer_WriteString(7, 2, 0xF, subroutine_arg0);

    sprintf(subroutine_arg0, "R:%d    ", Collision_GetPushLeftFiltered(roomX, roomY, 0));
    BgCmdBuffer_WriteString(3, 3, 0xF, subroutine_arg0);

    sprintf(subroutine_arg0, "L:%d    ", Collision_GetPushRightFiltered(roomX, roomY, 0));
    BgCmdBuffer_WriteString(3, 4, 0xF, subroutine_arg0);

    sprintf(subroutine_arg0, "U:%d    ", Collision_GetPushDownFiltered(roomX, roomY, 0));
    BgCmdBuffer_WriteString(3, 5, 0xF, subroutine_arg0);

    sprintf(subroutine_arg0, "D:%d    ", Collision_GetPushUpFiltered(roomX, roomY, 0));
    BgCmdBuffer_WriteString(3, 6, 0xF, subroutine_arg0);

    sprintf(subroutine_arg0, "%d", Collision_IsPointSolid(roomX, roomY - 8));
    BgCmdBuffer_WriteString(3, 7, 0xF, subroutine_arg0);
}

/**
 * @brief Scroll BG1 to centre the entity, clamped near the top and bottom
 * map edges.
 */
void DebugLinkTest_CenterCamera(struct EwramData_EntityData *entity)
{
    s32 deltaY;
    s32 deltaX;
    s32 bgIndex;
    struct BgLayerState *bg1Info;
    struct BgMapMetadata *metadata;
    u32 scrollY;
    u32 scrollPx;
    u32 maxY;

    bgIndex = 1;
    deltaX = ((s16) entity->posX.part16.integer - 0x78) << 0x10;
    deltaY = (entity->posY.part16.integer - 0x60) << 0x10;

    bg1Info = &gEwramData->bgInfo[1];
    metadata = bg1Info->pBgMetadata;
    if (metadata->heightCells > 1)
    {
        scrollY = bg1Info->yPos.whole;
        scrollPx = scrollY >> 0x10;
        if (scrollPx < 0x20)
        {
            deltaY = 0x200000 - scrollY;
        }
        else
        {
            maxY = (metadata->heightCells << 5) - 0x40;
            if (scrollPx > maxY)
            {
                deltaY = maxY << (0x10 - scrollY);
            }
        }
    }

    entity->posY.part16.fraction += deltaY;
    BgMap_ScrollWithMargin(bgIndex, deltaX, deltaY);
    Entity_ScrollWithBg(bgIndex);
    Bg_UpdateScrollModes(0);
}

const char sDebugPairFormat[] = "%d, %d";
const char sDebugTripleFormat[] = "%d,%d,%d";

// (99.63%) https://decomp.me/scratch/UoIYN
/* DebugLinkTestWalker_Update: was NONMATCH, matched (same case-1 dead store as DebugTestWalkerUnused_Update). */
static inline s32 DebugWalker_TestBit1(s32 val)
{
    return val & 0x2;
}

static inline s32 DebugWalker_TestSlopeBits(s32 val)
{
    return val & 0xC0;
}

/**
 * @brief Simple walking/jumping player stand-in for the link test that
 * prints the collision tile and floor values under it.
 */
void DebugLinkTestWalker_Update(struct EwramData_EntityData *entity)
{
    char text[13];
    s16 ground;
    s16 groundLeft;
    s16 groundRight;
    s16 ceilingLeft;
    s32 filter;
    s16 wallLeft;
    s16 wallRightBody;
    s16 wallLeftBody;
    s16 wallRight;
    s16 roomX;
    s32 gfx;
    s32 blockKind;
    s32 sloped;
    s32 solid;
    s32 slopeDir;
    s16 roomY;
    s16 ceilingRight;
    u8 tile;
    s32 slopeType;
    struct EwramData *e;

    entity->roomActive = 1;
    roomY = GetEntityRoomYPositionInteger(entity);
    roomX = GetEntityRoomXPositionInteger(entity);

    filter = gEwramData->inputData.playerHeldInput >> 1;
    filter &= 1;
    ground = Collision_GetPushUpFiltered(roomX, roomY, filter);
    groundLeft = Collision_GetPushUpFiltered(roomX - 8, roomY + 1, filter);
    groundRight = Collision_GetPushUpFiltered(roomX + 8, roomY + 1, filter);
    ceilingLeft = Collision_GetPushDownFiltered(roomX - 8, roomY - 0x20, 0);
    ceilingRight = Collision_GetPushDownFiltered(roomX + 8, roomY - 0x20, 0);

    roomY = GetEntityRoomYPositionInteger(entity);
    roomX = GetEntityRoomXPositionInteger(entity);
    tile = Collision_GetTile(roomX, roomY + 1);

    sprintf(text, sDebugHexByteFormat, tile);
    BgCmdBuffer_WriteString(2, 2, 0xF, (u8 *) text);

    sprintf(text, sDebugPairFormat, roomX, roomY);
    BgCmdBuffer_WriteString(3, 3, 0xF, (u8 *) text);
    BgCmdBuffer_WriteString(3, 4, 0xF, (u8 *) text);

    solid = 0;
    if (DebugWalker_TestBit1(tile) || DebugWalker_TestSlopeBits(tile))
    {
        if (tile & 8)
        {
            if (tile & 1)
            {
                blockKind = 2;
            }
            else
            {
                blockKind = 1;
            }
        }
        else
        {
            blockKind = 0;
        }

        if (blockKind != 1)
        {
            solid = 1;
        }
    }

    if (tile & 0xC0)
    {
        if (tile & 2)
        {
            sloped = 1;
        }
        else
        {
            sloped = 1;
        }
    }
    else
    {
        sloped = 0;
    }

    if (sloped != 0)
    {
        if (tile & 4)
        {
            slopeDir = 3;
        }
        else
        {
            slopeDir = 4;
        }
    }
    else
    {
        slopeDir = 0;
    }

    slopeType = tile >> 6;
    sprintf(text, sDebugTripleFormat, solid, slopeDir, slopeType);
    BgCmdBuffer_WriteString(3, 4, 0xF, text);

    switch (entity->state)
    {
        case 0:
            gfx = ObjGfx_LoadResource((u8 *)sDebugLinkTestWalkerGfx, 0x10000);
            ObjPalette_LoadRange((u8 *)sDebugLinkTestWalkerPalette, 0, 1, 1);
            Sprite_Attach(entity, (void *)sDebugLinkTestWalkerSpriteSheet, gfx, 1);
            Anim_StartIndexed(entity, 0, 0, 1);

            entity->posX.part16.integer = 0x64;
            entity->posY.part16.integer = 0x90;
            entity->scrollWithBg = 1;
            entity->velX.whole = 0x18000;
            entity->velY.whole = 0;
            entity->state += 1;
            entity->spriteLayer = 0x10;
            DebugLinkTest_CenterCamera(entity);
            break;

        case 1:
            e = gEwramData;
            if (e)
                slopeType = 0;
            entity->accelX.whole = 0;
            if (e->inputData.playerHeldInput & 0x20)
            {
                entity->posX.whole -= entity->velX.whole;
                entity->accelX.whole = -entity->velX.whole;
                entity->flipX = 1;
            }
            if (gEwramData->inputData.playerHeldInput & 0x10)
            {
                entity->posX.whole += entity->velX.whole;
                entity->accelX.whole = entity->velX.whole;
                entity->flipX = 0;
            }

            // TODO: is double check real?
            if (entity->ext.debugWalker.airState == 1)
            {
                if ((entity->ext.debugWalker.airState == 1) && (gEwramData->inputData.playerNewInput & 1))
                {
                    entity->ext.debugWalker.airState = 2;
                    entity->velY.whole = -0x40000;
                    entity->accelY.whole = 0;           
                }
            }
            if ((gEwramData->inputData.playerNewInput & 1) && (gEwramData->inputData.playerHeldInput & 0x40))
            {
                entity->ext.debugWalker.airState = 1;
                entity->velY.whole = -0xA0000;
                entity->accelY.whole = 0;
            }

            if (entity->ext.debugWalker.airState == 0)
            {
                if (groundLeft == 0)
                {
                    if (groundRight == 0)
                    {
                        entity->ext.debugWalker.airState = 1;
                        entity->velY.whole = 0;
                        entity->accelY.whole = -0x1000;
                    }
                }
                if (entity->ext.debugWalker.airState == 0)
                {
                    if (gEwramData->inputData.playerNewInput & 1)
                    {
                        entity->ext.debugWalker.airState = 1;
                        entity->velY.whole = -0x52000;
                        entity->accelY.whole = 0;
                    }
                    if (entity->ext.debugWalker.airState == 0)
                    {
                        goto here;
                    }
                }
            }

            if (entity->velY.whole < 0)
            {
                if ((ceilingLeft != 0) || (ceilingRight != 0))
                {
                    s32 sum = entity->posY.part16.integer + ((ceilingLeft == 0) ? ceilingRight : ceilingLeft);
                    register s32 z_ asm("r2") = 0; /* FAKEMATCH: zero constant in r2, loaded before the store */
                    entity->posY.part16.integer = sum;
                    entity->velY.whole = z_;
                }
            }

            if (entity->velY.whole < -0x4000)
            {
                if (!(gEwramData->inputData.playerHeldInput & 1))
                {
                    entity->velY.whole = -0x4000;
                    entity->accelY.whole = -0x2000;
                }
            }
            if (entity->velY.whole < 0x2000)
            {
                entity->velY.whole += 0x2000;
            }
            if (entity->velY.whole > -0x20000 && entity->velY.whole < 0x2000)
            {
                if (gEwramData->inputData.playerHeldInput & 1)
                {
                    entity->accelY.whole -= 0x800;
                    if (entity->accelY.whole < -0x2800)
                    {
                        entity->accelY.whole = -0x2800;
                    }
                }
            }

            entity->velY.whole += 0x2000;
            if (entity->velY.whole > 0)
            {
                entity->velY.whole += entity->accelY.whole;
                entity->accelY.whole += 0x400;
                if (entity->accelY.whole > 0x1000)
                {
                    entity->accelY.whole = 0x1000;
                }
            }
            if (entity->velY.whole > 0x80000)
            {
                entity->velY.whole = 0x80000;
            }
            entity->posY.whole += entity->velY.whole;

            roomY = GetEntityRoomYPositionInteger(entity);
            roomX = GetEntityRoomXPositionInteger(entity);
            ground = Collision_GetPushUpFiltered(roomX, roomY, filter);
            if ((gUnk_03002CB0.blockFlipFlags & 3) == 3)
            {
                if ((Collision_GetPushDown(roomX, roomY - 0x20) == 0) && (Collision_GetPushDown(roomX, roomY - 0x10) == 0))
                {
                    entity->posY.part16.integer += ground;
                    entity->posY.part16.fraction = 0;
                }
                else
                {
                    ground = 0;
                }
            }

            if (ground == 0)
            {
                if (entity->accelX.whole > 0)
                {
                    wallRight = Collision_GetPushLeftFiltered(roomX + 8, roomY, filter);
                    if (wallRight != 0)
                    {
                        entity->posX.part16.integer = (s16)entity->posX.part16.integer + wallRight;
                        entity->posX.part16.fraction = 0;
                    }
                }
                if (entity->accelX.whole < 0)
                {
                    wallLeft = Collision_GetPushRightFiltered(roomX - 8, roomY, filter);
                    if (wallLeft != 0)
                    {
                        entity->posX.part16.integer = (s16)entity->posX.part16.integer + wallLeft;
                        entity->posX.part16.fraction = 0;
                    }
                }
            }

            if (((ground << 0x10) != 0) && (entity->velY.part16.integer >= 0))
            {
                entity->posY.part16.fraction = 0;
                entity->velY.whole = 0;
                entity->accelY.whole = 0;
                entity->ext.debugWalker.airState = 0;
            }
            break;
    }

    goto skip;
    here:
    entity->posY.whole += entity->velX.whole;
    skip:

    roomY = GetEntityRoomYPositionInteger(entity);
    roomX = GetEntityRoomXPositionInteger(entity);

    if (entity->ext.debugWalker.airState == 0)
    {
        if ((Collision_GetPushDown(roomX, roomY - 0x20) == 0) && (Collision_GetPushDown(roomX, roomY - 0x10) == 0))
        {
            ground = Collision_GetPushUpFiltered(roomX, roomY, filter);
        }
        entity->posY.part16.integer += ground;
    }

    if (entity->accelX.whole > 0)
    {
        wallRightBody = Collision_GetPushLeftFiltered(roomX + 8, roomY - 0x10, filter);
        if (wallRightBody != 0)
        {
            entity->posX.part16.integer = (s16)entity->posX.part16.integer + wallRightBody;
            entity->posX.part16.fraction = 0;
        }
    }
    if (entity->accelX.whole < 0)
    {
        wallLeftBody = Collision_GetPushRightFiltered(roomX - 8, roomY - 0x10, filter);
        if (wallLeftBody != 0)
        {
            entity->posX.part16.integer = (s16)entity->posX.part16.integer + wallLeftBody;
            entity->posX.part16.fraction = 0;
        }
    }

    if (gEwramData->inputData.playerHeldInput & 0x100)
    {
        entity->velX.whole = 0x40000;
    }
    else
    {
        entity->velX.whole = 0x18000;
    }

    entity->timer += 1;
    BgMap_ScrollWithMargin(1, ((s16) entity->posX.part16.integer - 0x78) << 0x10, (entity->posY.part16.integer - 0x70) << 0x10);
    Entity_ScrollWithBg(1);
    Bg_UpdateScrollModes(0);
    Anim_Update(entity);
    Sprite_Draw(entity);
}
// #endif

// (99.63%) https://decomp.me/scratch/GYFCy
/* DebugTestWalkerUnused_Update: was NONMATCH, matched (case 1: dead store in if (e) gives the PRE block boundary). */
// Bg_UpdateScrollModes(); implicit (should have no arguments)

/**
 * @brief Unreferenced variant of DebugLinkTestWalker_Update.
 */
void DebugTestWalkerUnused_Update(struct EwramData_EntityData *entity)
{
    char text[13];
    s16 groundLeft;
    s16 ceilingLeft;
    s16 ceilingRight;
    s32 filter;
    s16 wallRight;
    s16 wallLeft;
    s16 wallRightBody;
    s16 wallLeftBody;
    s16 roomX;
    s32 gfx;
    s32 blockKind;
    s32 sloped;
    s32 solid;
    s32 slopeDir;
    s16 ground;
    s16 roomY;
    s16 groundRight;
    u8 tile;
    s32 slopeType;
    struct EwramData *e;

    entity->roomActive = 1;
    roomY = GetEntityRoomYPositionInteger(entity);
    roomX = GetEntityRoomXPositionInteger(entity);

    filter = gEwramData->inputData.playerHeldInput >> 1;
    filter &= 1;
    Collision_GetPushDownFiltered(roomX, roomY, 0);
    groundLeft = Collision_GetPushDownFiltered(roomX - 8, roomY - 1, 0);
    groundRight = Collision_GetPushDownFiltered(roomX + 8, roomY - 1, 0);
    ceilingLeft = Collision_GetPushUpFiltered(roomX - 8, roomY + 0x20, filter);
    ceilingRight = Collision_GetPushUpFiltered(roomX + 8, roomY + 0x20, filter);

    roomY = GetEntityRoomYPositionInteger(entity);
    roomX = GetEntityRoomXPositionInteger(entity);
    tile = Collision_GetTile(roomX, roomY - 1);

    sprintf(text, sDebugHexByteFormat, tile);
    BgCmdBuffer_WriteString(2, 2, 0xF, text);

    sprintf(text, sDebugPairFormat, roomX, roomY);
    BgCmdBuffer_WriteString(3, 3, 0xF, text);

    solid = 0;
    if (DebugWalker_TestBit1(tile) || DebugWalker_TestSlopeBits(tile))
    {
        if (tile & 8)
        {
            if (tile & 1)
            {
                blockKind = 2;
            }
            else
            {
                blockKind = 1;
            }
        }
        else
        {
            blockKind = 0;
        }

        if (blockKind != 1)
        {
            solid = 1;
        }
    }

    if (tile & 0xC0)
    {
        if (tile & 2)
        {
            sloped = 1;
        }
        else
        {
            sloped = 1;
        }
    }
    else
    {
        sloped = 0;
    }

    if (sloped != 0)
    {
        if (tile & 4)
        {
            slopeDir = 3;
        }
        else
        {
            slopeDir = 4;
        }
    }
    else
    {
        slopeDir = 0;
    }

    slopeType = tile >> 6;
    sprintf(text, sDebugTripleFormat, solid, slopeDir, slopeType);
    BgCmdBuffer_WriteString(3, 4, 0xF, text);

    switch (entity->state)
    {
        case 0:
            gfx = ObjGfx_LoadResource((u8 *)sDebugLinkTestWalkerGfx, 0x10000);
            ObjPalette_LoadRange((u8 *)sDebugLinkTestWalkerPalette, 0, 1, 1);
            Sprite_Attach(entity, (void *)sDebugLinkTestWalkerSpriteSheet, gfx, 1);
            Anim_StartIndexed(entity, 0, 0, 1);

            entity->posX.part16.integer = 0x64;
            entity->posY.part16.integer = 0x80;
            entity->scrollWithBg = 1;
            entity->velX.whole = 0x18000;
            entity->velY.whole = 0;
            entity->state += 1;
            entity->spriteLayer = 0x10;
            entity->flipY = 1;
            DebugLinkTest_CenterCamera(entity);
            break;

        case 1:
            e = gEwramData;
            // Dead store in a real branch: the block boundary makes gcse PRE put the
            // &gEwramData copy (r8) right after this load; flow deletes the store and
            // jump2 the empty branch, so no code is left.
            if (e)
                slopeType = 0;
            entity->accelX.whole = 0;
            if (e->inputData.playerHeldInput & 0x20)
            {
                entity->posX.whole -= entity->velX.whole;
                entity->accelX.whole = -entity->velX.whole;
                entity->flipX = 1;
            }
            if (gEwramData->inputData.playerHeldInput & 0x10)
            {
                entity->posX.whole += entity->velX.whole;
                entity->accelX.whole = entity->velX.whole;
                entity->flipX = 0;
            }

            // TODO: is double check real?
            if (entity->ext.debugWalker.airState == 1)
            {
                if ((entity->ext.debugWalker.airState == 1) && (gEwramData->inputData.playerNewInput & 1))
                {
                    entity->ext.debugWalker.airState = 2;
                    entity->velY.whole = -0x40000;
                    entity->accelY.whole = 0;
                }
            }
            if ((gEwramData->inputData.playerNewInput & 1) && (gEwramData->inputData.playerHeldInput & 0x80))
            {
                entity->ext.debugWalker.airState = 1;
                entity->velY.whole = -0xA0000;
                entity->accelY.whole = 0;
            }
            
            if (entity->ext.debugWalker.airState == 0)
            {
                if (groundLeft == 0)
                {
                    if (groundRight == 0)
                    {
                        entity->ext.debugWalker.airState = 1;
                        entity->velY.whole = 0;
                        entity->accelY.whole = -0x1000;
                    }
                }
                if (entity->ext.debugWalker.airState == 0)
                {
                    if (gEwramData->inputData.playerNewInput & 1)
                    {
                        entity->ext.debugWalker.airState = 1;
                        entity->velY.whole = -0x52000;
                        entity->accelY.whole = 0;
                    }
                    if (entity->ext.debugWalker.airState == 0)
                    {
                        goto here; // should be deleted when rest of code figured out
                        entity->posY.whole -= entity->velX.whole;
                        break;
                    }
                }
            }

            if (entity->velY.whole < 0)
            {
                if ((ceilingLeft != 0) || (ceilingRight != 0))
                {
                    if (ceilingLeft == 0)
                    {
                        entity->posY.part16.integer += ceilingRight;
                    }
                    else
                    {
                        entity->posY.part16.integer += ceilingLeft;
                    }
                    { register s32 z_ asm("r2") = 0; entity->velY.whole = z_; } /* FAKEMATCH: zero constant in r2 */
                }
            }

            if (entity->velY.whole < -0x4000)
            {
                if (!(gEwramData->inputData.playerHeldInput & 1))
                {
                    entity->velY.whole = -0x4000;
                    entity->accelY.whole = -0x2000;
                }
            }
            if (entity->velY.whole < 0x2000)
            {
                entity->velY.whole += 0x2000;
            }
            if (entity->velY.whole > -0x20000 && entity->velY.whole < 0x2000)
            {
                if (gEwramData->inputData.playerHeldInput & 1)
                {
                    entity->accelY.whole -= 0x800;
                    if (entity->accelY.whole < -0x2800)
                    {
                        entity->accelY.whole = -0x2800;
                    }
                }
            }

            entity->velY.whole += 0x2000;
            if (entity->velY.whole > 0)
            {
                entity->velY.whole += entity->accelY.whole;
                entity->accelY.whole += 0x400;
                if (entity->accelY.whole > 0x1000)
                {
                    entity->accelY.whole = 0x1000;
                }
            }
            if (entity->velY.whole > 0x80000)
            {
                entity->velY.whole = 0x80000;
            }
            entity->posY.whole -= entity->velY.whole;

            roomY = GetEntityRoomYPositionInteger(entity);
            roomX = GetEntityRoomXPositionInteger(entity);
            ground = Collision_GetPushDownFiltered(roomX, roomY, 0);
            if ((gUnk_03002CB0.blockFlipFlags & 3) == 3)
            {
                entity->posY.part16.integer += ground;
                entity->posY.part16.fraction = 0;
            }

            if (ground == 0)
            {
                if (entity->accelX.whole > 0)
                {
                    wallRight = Collision_GetPushLeftFiltered(roomX + 8, roomY, filter);
                    if (wallRight != 0)
                    {
                        entity->posX.part16.integer = (s16)entity->posX.part16.integer + wallRight;
                        entity->posX.part16.fraction = 0;
                    }
                }
                if (entity->accelX.whole < 0)
                {
                    wallLeft = Collision_GetPushRightFiltered(roomX - 8, roomY, filter);
                    if (wallLeft != 0)
                    {
                        entity->posX.part16.integer = (s16)entity->posX.part16.integer + wallLeft;
                        entity->posX.part16.fraction = 0;
                    }
                }
            }

            if (((ground << 0x10) != 0) && (entity->velY.part16.integer >= 0))
            {
                entity->posY.part16.fraction = 0;
                entity->velY.whole = 0;
                entity->accelY.whole = 0;
                entity->ext.debugWalker.airState = 0;
            }
            break;
    }

    goto skip;
    here:
    entity->posY.whole -= entity->velX.whole;
    skip:

    roomY = GetEntityRoomYPositionInteger(entity);
    roomX = GetEntityRoomXPositionInteger(entity);

    if (entity->ext.debugWalker.airState == 0)
    {
        s32 var = Collision_GetPushDownFiltered(roomX, roomY, 0);
        entity->posY.part16.integer += var;
    }

    if (entity->accelX.whole > 0)
    {
        wallRightBody = Collision_GetPushLeftFiltered(roomX + 8, roomY + 0x10, filter);
        if (wallRightBody != 0)
        {
            entity->posX.part16.integer = (s16)entity->posX.part16.integer + wallRightBody;
            entity->posX.part16.fraction = 0;
        }
    }
    if (entity->accelX.whole < 0)
    {
        wallLeftBody = Collision_GetPushRightFiltered(roomX - 8, roomY + 0x10, filter);
        if (wallLeftBody != 0)
        {
            entity->posX.part16.integer = (s16)entity->posX.part16.integer + wallLeftBody;
            entity->posX.part16.fraction = 0;
        }
    }

    if (gEwramData->inputData.playerHeldInput & 0x100)
    {
        entity->velX.whole = 0x40000;
    }
    else
    {
        entity->velX.whole = 0x18000;
    }

    entity->timer += 1;
    BgMap_ScrollWithMargin(1, ((s16) entity->posX.part16.integer - 0x78) << 0x10, (entity->posY.part16.integer - 0x70) << 0x10);
    Entity_ScrollWithBg(1);
    Bg_UpdateScrollModes(0);
    Anim_Update(entity);
    Sprite_Draw(entity);
}

/**
 * @brief Load the test graphics, palettes and map into BG2.
 */
void DebugBgDispTest_Init(struct DebugDisplayTestState *test)
{
    test->target.bgIndex = 2;
    BgPalette_LoadRange((u8 *)sDebugBgDispTestBgPalette, 0x10, 0xA, 0);
    Gfx_CopyResourceBlocks((void *)sDebugBgDispTestGfx, 0, 0, 4);
    Gfx_CopyResourceBlocks((void *)sDebugBgDispTestGfx2, 4, 0, 4);
    Gfx_CopyResourceBlocks((void *)sDebugBgDispTestGfx3, 8, 0, 4);
    BgMap_Setup(test->target.bgIndex, (struct BgMapMetadata *)sDebugBgDispTestBgMap, 0, 0);
}

/**
 * @brief Clear the test BG map.
 */
void DebugBgDispTest_Exit(struct DebugDisplayTestState *test)
{
    void *subroutine_arg0[4];

    memcpy(subroutine_arg0, sDebugTestExitArgs, sizeof(sDebugTestExitArgs));
    DMA_FILL_32(3, 0, subroutine_arg0[test->target.bgIndex], 0x800);
}

/**
 * @brief Follow room exits with room transitions and recentre the camera
 * afterwards.
 */
void DebugLinkTest_Update(struct DebugDisplayTestState *test)
{
    struct EwramData_EntityData *entity;

    entity = test->player;
    switch (test->transitionState)
    {
        case 0:
            if (Room_IsPlayerInside(entity) == 0)
            {
                RoomTransition_Begin(&gEwramData->gameState);
                test->transitionState = 1;
            }
            break;

        case 1:
            if (RoomTransition_Update(&gEwramData->gameState) != 0)
            {
                DebugLinkTest_CenterCamera(entity);
                test->transitionState = 0;
            }
            break;
    }
}

/**
 * @brief Clear the BG1..BG3 maps.
 */
void DebugLinkTest_Exit(struct DebugDisplayTestState *test)
{
    void *subroutine_arg0[4];

    memcpy(subroutine_arg0, sDebugTestExitArgs, sizeof(sDebugTestExitArgs));
    DMA_FILL_32(3, 0, VRAM_BASE + 0xE800, 0x1800);
}

/**
 * @brief Follow room exits with room transitions.
 */
void DebugHitTest_Update(struct DebugDisplayTestState *test)
{
    switch (test->transitionState)
    {
        case 0:
            if (Room_IsPlayerInside(test->player) == 0)
            {
                RoomTransition_Begin(&gEwramData->gameState);
                test->transitionState = 1;
            }
            break;

        case 1:
            if (RoomTransition_Update(&gEwramData->gameState) != 0)
            {
                test->transitionState = 0;
            }
            break;
    }
}

/**
 * @brief Clear the BG map selected by the test state.
 */
void DebugHitTest_Exit(struct DebugDisplayTestState *test)
{
    void *subroutine_arg0[4];

    memcpy(subroutine_arg0, sDebugTestExitArgs, sizeof(sDebugTestExitArgs));
    DMA_FILL_32(3, 0, subroutine_arg0[test->clearBgIndex], 0x800);
}
