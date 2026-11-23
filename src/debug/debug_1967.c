#include "debug_1967.h"
#include "core.h"
#include "bg_and_collision.h"
#include "room_entities.h"
#include "player_core.h"
#include "player_level.h"
#include "graphics.h"
#include "gameplay.h"
#include "entity_util.h"
#include "sound.h"
#include "gba.h"

#include "constants/main.h"

#include "data/data_0E0334.h"
#include "data/data_0E3464.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "damage_numbers.h"
#include "hitbox.h"
#include "item_menu.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "palette.h"
#include "pause_menu.h"
#include "text12.h"
#include "in_game.h"
#include "map_flags.h"
#include "map_screen.h"
#include "entity_common.h"
extern void HudMap_WriteTiles(s32, s32, u16, s32);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s32, s32);

struct Debug1967MenuEntry {
    u8 *label; // pointer to string
    void *handler; // func ptr
}; /* size = 0x8 */
struct Debug1967MenuEntry sDebug1967MenuEntries[] = {
    [0] = {
        .label = "ENEMY",
        .handler = Debug1967_EnemyTest
    },
    [1] = {
        .label = "SDTEST",
        .handler = Debug1967_SoundTest
    },
    [2] = {
        .label = "START",
        .handler = Debug1967_StartGame
    },
    [3] = {
        .label = "OBJTEST",
        .handler = Debug1967_ObjTest
    }
};

const u8 pad_080e1f72[] = {0, 0};

// TODO: used by DebugPlatform_Update, so seemingly externed?
const u8 sUnk_080E1F74[24] = {
    0x01, 0x00, 0x00, 0x00, 0xF1, 0x20, 0x02, 0x08, 0xB9, 0x57, 0x03, 0x08, 0xF1, 0x20, 0x02, 0x08, 
    0xF1, 0x20, 0x02, 0x08, 0x6D, 0x63, 0x03, 0x08, 
};

/**
 * @brief 35930 | Handle debug 1967 game mode
 * 
 * @return s32 Game mode, -1 is no change, -2 is next game mode, else the specified game mode
 */
s32 GameModeDebug1967Update(void)
{
    s32 gameMode;
    struct Debug1967MenuEntry *option;
    u32 i;

    gameMode = GAME_MODE_SAME_MODE;
    option = sDebug1967MenuEntries;

    switch (gEwramData->gameModeUpdateStage)
    {
        case 0:
            gEwramData->bgPalette.base[0][0] = 0xFFFF;
            gEwramData->bgPalette.uploaded = 0;
            gDisplayRegisters.bgCnt[0] = 0x1C00;

            for (i = 0; i < 4; i++)
            {
                BgCmdBuffer_WriteString(8, i * 2 + 4, 1, option->label);
                option += 1;
            }

            gEwramData->gameModeUpdateStage += 1;
            break;

        case 1:
            if (gEwramData->inputData.repeatedInput & 0x80)
            {
                if (gEwramData->gameModeSubStage < 3)
                {
                    gEwramData->gameModeSubStage += 1;
                }
            }
            else if (gEwramData->inputData.repeatedInput & 0x40)
            {
                if (gEwramData->gameModeSubStage != 0)
                {
                    gEwramData->gameModeSubStage -= 1;
                }
            }

            for (i = 0; i < 4; i++)
            {
                BgCmdBuffer_WriteString(6, i * 2 + 4, 1, " ");
            }

            BgCmdBuffer_WriteString(6, gEwramData->gameModeSubStage * 2 + 4, 1, "+");
            if (gEwramData->inputData.newInput & 1)
            {
                DMA_FILL_32(3, 0, &gEwramData->debugMenuWork, sizeof(gEwramData->debugMenuWork));
                gEwramData->gameState.subMode = gEwramData->gameState.subModeStep = gEwramData->gameState.subModeCounter = 0;
                gEwramData->gameModeUpdateStage += 1;
            }
            break;

        case 2:
            DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
            gEwramData->gameModeUpdateStage += 1;
            break;

        case 3:
            gameMode = ((s32 (*)(void))sDebug1967MenuEntries[gEwramData->gameModeSubStage].handler)();
            break;
    }

    return gameMode;
}

const u8 pad_080e1f92[] = {0,0};

/**
 * @brief Unreferenced: every 100 calls create a DebugTestEnemy entity in the
 * first free of five slots, forgetting slots whose entity was freed.
 */
void Debug1967_UpdateTestEnemySpawner(struct DebugEnemySpawnerState *spawner)
{
    s32 i;
    struct EwramData_EntityData *child;

    switch (spawner->state)
    {
        case 0:
            for (i = 0; i < 5; i++)
            {
                spawner->spawnedEntities[i] = NULL;
            }
            spawner->spawnCounter = 100;
            spawner->state += 1;
            break;

        case 1:
            if (spawner->spawnCounter++ < 100)
                break;

            spawner->spawnCounter = 0;

            for (i = 0; i < 5; i++)
            {
                if (spawner->spawnedEntities[i] == NULL)
                {
                    child = EntityCreateInRange(0x1B, 0x48, (u32*)&DebugTestEnemy_Update);
                    if (child != NULL)
                    {
                        child->posX.part16.integer = 160 + i * 10;
                        child->posY.part16.integer = 0x50;
                        spawner->spawnedEntities[i] = child;
                        break;
                    }
                }
                else if (spawner->spawnedEntities[i]->updateFunc == NULL)
                {
                    spawner->spawnedEntities[i] = NULL;
                }
            }

            break;
    }
}

/**
 * @brief START entry: load room 0x0852269C with level 99 and fixed souls and
 * equipment, then run entities, HUD and room transitions.
 */
s32 Debug1967_StartGame(void)
{
    switch ((&gEwramData->debugMenuWork)->state)
    {
        case 0:
            gEwramData->gameState.currentRoomDefinition = (struct RoomDefinition *)0x0852269C;
            Player_SetLevel(0x63);
            Player_EquipSoulsAndRestore(5, 3, 2);
            Player_EquipItem(2, 0x19);
            Player_EquipItem(0, 8);
            Player_EquipItem(1, 0x24);
            return 4;

        case 1:
            Entity_UpdateAll();
            if (!gEwramData->paused)
            {
                InGame_UpdateTimerAndHud();
            }
            ScreenShake_Update();

            if (Room_IsPlayerInside(gEwramData->playerState.player) == 0)
            {
                RoomTransition_Begin(&gEwramData->gameState);
                (&gEwramData->debugMenuWork)->state = 2;
            }
            else
            {
                RoomEntity_SpawnOnScroll();
            }
            break;

        case 2:
            Entity_UpdateAll();
            if (RoomTransition_Update(&gEwramData->gameState) != 0)
            {
                Map_MarkCurrentCell(&gEwramData->gameState);
                (&gEwramData->debugMenuWork)->state = 1;
            }
            break;
    }

    ItemIconCache_InvalidateUpload();
    return -1;
}

extern const u16 sEnemyNameTextIds[];

// TODO: very similar to GameModeSoundTestMenu_inlines
// TODO: also clean all this mess up
static inline void Debug1967_WrapEnemyValue(s32 value, s32 max, s16 *dst, s32 wrapTo)
{
    if (value > max)
    {
        *dst = wrapTo;
    }
    else if (value < 0)
    {
        *dst = max;
    }
}

static inline s32 Debug1967_AdjustEnemySelection(s16 *value)
{
    s32 zero;
    s32 max;
    u16 var;
    u32 upKey;
    u32 rightKey;

    var = (&gEwramData->debugMenuWork)->selection;
    upKey = 0x40;
    do { /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        zero = 0;
    } while(0);
    max = 0x70;

    if (gEwramData->inputData.repeatedInput & upKey) // KEY_UP
    {
        *value += 1;
    }
    else if (gEwramData->inputData.repeatedInput & (upKey << 1)) // KEY_DOWN
    {
        *value -= 1;
    }

    if (gEwramData->inputData.repeatedInput & (rightKey = 0x10)) // KEY_RIGHT
    {
        *value += 10;
    }
    if (gEwramData->inputData.repeatedInput & (rightKey << 1)) // KEY_LEFT
    {
        *value -= 10;
    }

    Debug1967_WrapEnemyValue(*value, max, value, zero);
    return var;
}

/**
 * @brief Change the selected enemy id (0..0x70) with Up/Down/Left/Right,
 * reprint its name, and on A or B choose 1 or 5 enemies; return 1 on a
 * choice.
 */
s32 Debug1967_HandleEnemySelectInput(void)
{
    u8 pad[0x40];
    s16 prevEnemy;

    prevEnemy = Debug1967_AdjustEnemySelection(&(&gEwramData->debugMenuWork)->selection);

    if (prevEnemy != (&gEwramData->debugMenuWork)->selection)
    {
        Text8_SetRegion(0, 1, 0xF, 1);
        Text8_ClearRegion();
        Text8_Print((struct unk_08506B38 *) Text_GetLocalizedEntry(sEnemyNameTextIds[(&gEwramData->debugMenuWork)->selection]), 0);
    }

    if (gEwramData->inputData.newInput & 1)
    {
        (&gEwramData->debugMenuWork)->spawnCount = 1;
        return 1;
    }

    if (gEwramData->inputData.newInput & 2)
    {
        (&gEwramData->debugMenuWork)->spawnCount = 5;
        return 1;
    }

    return 0;
}

/* Local 0x24-byte template-table view: retain this TU's types and padding. */
struct EnemyTemplateView {
    void *createFunc;
    void *updateFunc;
    u8 pad_8[0xC - 0x8];
    u16 hp;
    u8 pad_E[0x24 - 0xE];
}; /* size = 0x24 */
extern const struct EnemyTemplateView sEnemyData[];

// TODO: HudMap_WriteTiles is implicitly called in other files, it may be implicit here too

/**
 * @brief ENEMY entry: choose an enemy from sEnemyData, load the test room
 * with a levelled player and spawn the chosen number of that enemy;
 * L+R returns to the selection.
 */
s32 Debug1967_EnemyTest(void)
{
    u8 pad[0x40];
    s32 flip;
    s32 i;
    struct EwramData_EntityData *child;
    struct PlayerStatus *playerStatus;
    struct GameState *gameState;
    const struct EnemyTemplateView *enemy;
    s32 x;
    s32 halfWidth;
    struct BgLayerState *bg1Info;

    gameState = &gEwramData->gameState;
    playerStatus = &gEwramData->playerStatus;

    switch ((&gEwramData->debugMenuWork)->state)
    {
        case 0:
            Text_SetTileBuffer(VRAM_BASE);
            Text_SetForegroundColor(1);
            Text_SetBackgroundColor(0);
            Text8_SetRegion(0, 1, 0x10, 1);
            Text8_ClearRegion();
            Text8_Print((struct unk_08506B38 *) Text_GetLocalizedEntry(sEnemyNameTextIds[(&gEwramData->debugMenuWork)->selection]), 0);
            HudMap_WriteTiles(8, 0xC, 0x1020, 0xA);
            (&gEwramData->debugMenuWork)->state = 1;
            /* fallthrough */
        case 1:
            if (Debug1967_HandleEnemySelectInput() == 0)
            {
                break;
            }

            (&gEwramData->debugMenuWork)->state = 2;
        case 2:
            gEwramData->gameState.currentRoomDefinition = (struct RoomDefinition *)0x085241E0;
            playerStatus->currentLevel = 0;
            InGame_InitScene();
            (&gEwramData->debugMenuWork)->state = 3;

            if (gEwramData->inputData.heldInput & 0x100)
            {
                Player_SetLevel(0x1E);
            }
            else if (gEwramData->inputData.heldInput & 0x200)
            {
                Player_SetLevel(0x14);
            }
            else if (gEwramData->inputData.heldInput & 8)
            {
                Player_SetLevel(0xA);
            }
            else if (gEwramData->inputData.heldInput & 4)
            {
                Player_SetLevel(0x32);
            }
            else
            {
                Player_SetLevel(0x63);
            }

            Player_EquipSoulsAndRestore(5, 4, 0);
            Player_EquipItem(2, 8);
            Player_EquipItem(0, 5);
            Player_EquipItem(1, 0x27);
            break;

        case 3:
            Player_RestoreHPAndMP();

            for (i = 0; i < (&gEwramData->debugMenuWork)->spawnCount; i++)
            {
                child = EntityCreateInRange(0x1B, 0x48, (u32 *) EntityUpdateNothing);
                if (child == NULL)
                    continue;

                x = (i << 5) + 0x100;
                halfWidth = ((u32)(&gEwramData->debugMenuWork)->spawnCount >> 1) << 5;
                x -= halfWidth;
                bg1Info = &gEwramData->bgInfo[1];
                child->posX.part16.integer = x - bg1Info->xPos.part16.integer;
                child->posY.part16.integer = 0x90 - bg1Info->yPos.part16.integer;

                flip = 0;
                if (child->posX.whole < gEwramData->playerState.player->posX.whole)
                {
                    flip = 1;
                }
                child->flipX = flip;
                child->scrollWithBg = 1;

                child->spawnParam0 = 0;
                child->spawnParam1 = 0;
                child->spawnIndex = 0;
                child->enemyId = (&gEwramData->debugMenuWork)->selection;
                child->drawFunc = (u32 *) Sprite_DrawEntity;
                enemy = &sEnemyData[(&gEwramData->debugMenuWork)->selection];
                child->hp = enemy->hp;
                child->updateFunc = (u32 *) enemy->updateFunc;
                ((void (*)(struct EwramData_EntityData *)) enemy->createFunc)(child);
            }

            (&gEwramData->debugMenuWork)->state = 4;
            break;

        case 4:
            Entity_UpdateAll();
            if (!(gEwramData->paused))
            {
                InGame_UpdateTimerAndHud();
            }
            ScreenShake_Update();

            if (Room_IsPlayerInside(gEwramData->playerState.player) == 0)
            {
                RoomTransition_Begin(&gEwramData->gameState);
                (&gEwramData->debugMenuWork)->state = 5;
            }
            else
            {
                RoomEntity_SpawnOnScroll();
            }

            if ((gEwramData->inputData.heldInput & (0x200 | 0x100)) == (0x200 | 0x100))
            {
                (&gEwramData->debugMenuWork)->state = 7;
                EntityDeleteAll();
                ObjTiles_Init();
                Obj_Init();
                Sprite_Init();
                Palette_Init();
                Text_Initialize();
            }
            else if ((gEwramData->inputData.newInput & 1) && gEwramData->paused)
            {
                PauseMenu_Enter(gameState);
                (&gEwramData->debugMenuWork)->state = 6;
            }
            else
            {
                ItemIconCache_InvalidateUpload();
            }
            break;

        case 5:
            Entity_UpdateAll();
            if (RoomTransition_Update(&gEwramData->gameState) != 0)
            {
                (&gEwramData->debugMenuWork)->state = 3;
            }
            break;

        case 6:
            if (PauseMenu_Update(gameState) != 0)
            {
                (&gEwramData->debugMenuWork)->state = 4;
            }
            break;

        case 7:
            DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
            DMA_FILL_32(3, 0, VRAM_BASE + 0xE800, 0x800);
            DMA_FILL_32(3, 0, VRAM_BASE + 0xF000, 0x800);
            DMA_FILL_32(3, 0, VRAM_BASE + 0xF800, 0x800);
            gEwramData->bgPalette.base[0][0] = 0xFFFF;
            gEwramData->bgPalette.uploaded = 0;
            (&gEwramData->debugMenuWork)->state = 0;
            break;
    }

    return -1;
}

// TODO: proper struct
const u8 sDebug1967Gfx[] = {
    0x02, 0x04, 0x02, 0x01, 0xA4, 0x42, 0x1F, 0x08, 0xA8, 0x52, 0x1F, 0x08, 0xAC, 0x62, 0x1F, 0x08, 
    0xB0, 0x72, 0x1F, 0x08, 
};

/**
 * @brief OBJTEST viewer entity: show the sprite at 0x08252748 and step its
 * animation index with Up/Down.
 */
void Debug1967ObjTestViewer_Update(struct EwramData_EntityData *entity)
{
    s32 gfx;

    switch (entity->state)
    {
        case 0:
            gfx = ObjGfx_LoadResource((u8 *)sDebug1967Gfx, 0);
            ObjPalette_LoadRange((u8 *)0x0820C028, 0, 1, 0xA);
            Sprite_Attach(entity, (void *)0x08252748, gfx, 0xA);
            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            entity->subState = 0;
            Anim_StartIndexed(entity, 0, 1, 1);
            entity->state += 1;
            /* fallthrough */
        case 1:
            if (gEwramData->inputData.newInput & 0x40)
            {
                entity->subState -= 1;
            }
            if (gEwramData->inputData.newInput & 0x80)
            {
                entity->subState += 1;
            }

            if (entity->animScriptIndex != entity->subState)
            {
                Anim_StartIndexed(entity, entity->subState, 1, 1);
            }
            break;
    }

    Anim_Update(entity);
}

struct DebugSoundCategory {
    u8 *label;
    s16 minSoundId;
    s16 maxSoundId;
}; /* size = 0x8 */
const struct DebugSoundCategory sDebugSoundCategories[] = {
    [0] = {
        .label = "BGM",
        .minSoundId = 0,
        .maxSoundId = 0x24
    },
    [1] = {
        .label = "EFF1",
        .minSoundId = 0x64,
        .maxSoundId = 0x170
    },
    [2] = {
        .label = "VOICE",
        .minSoundId = 0x190,
        .maxSoundId = 0x1EE
    }
};

static inline void Debug1967_WrapToRange(s32 value, s32 max, s16 *dst, s32 min)
{
    if (value > max)
    {
        *dst = min;
    }
    else if (value < min)
    {
        *dst = max;
    }
}

static inline void Debug1967_WrapValue(s32 value, s32 max, s16 *dst, s32 wrapTo)
{
    if (value > max)
    {
        *dst = wrapTo;
    }
    else if (value < 0)
    {
        *dst = max;
    }
}

static inline void Debug1967_AdjustSoundId(const struct DebugSoundCategory *entry, u32 rightKey, s16 *value)
{
    s32 min;
    s32 max;

    min = entry->minSoundId;
    max = entry->maxSoundId;

    if (gEwramData->inputData.repeatedInput & rightKey) // KEY_RIGHT
    {
        *value += 1;
    }
    else if (gEwramData->inputData.repeatedInput & (rightKey << 1)) // KEY_LEFT
    {
        *value -= 1;
    }

    if (gEwramData->inputData.repeatedInput & (rightKey << 4)) // KEY_R
    {
        *value += 10;
    }
    if (gEwramData->inputData.repeatedInput & (rightKey << 5)) // KEY_L
    {
        *value -= 10;
    }

    Debug1967_WrapToRange(*value, max, value, min);
}

static inline void Debug1967_AdjustSoundCategory(u32 upKey, s16 *value)
{
    s32 zero;
    s32 max;

    do { /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        zero = 0;
    } while(0);
    max = 2;

    if (gEwramData->inputData.repeatedInput & upKey) // KEY_UP
    {
        *value += 1;
    }
    else if (gEwramData->inputData.repeatedInput & (upKey >> 1)) // KEY_DOWN
    {
        *value -= 1;
    }

    Debug1967_WrapValue(*value, max, value, zero);
}

/**
 * @brief SDTEST entry: pick BGM/EFF1/VOICE with Up/Down, change the id within
 * the category range, A plays it and B stops audio.
 */
s32 Debug1967_SoundTest(void)
{
    u8 pad[0x40];
    s32 i;
    const struct DebugSoundCategory *entries;
    s32 selected;

    switch (gEwramData->debugMenuWork.state)
    {
        case 0:
            gEwramData->debugMenuWork.state = 1;
            for (i = 0; i < 3; i++)
            {
                (&gEwramData->debugMenuWork)->entryValues[i] = sDebugSoundCategories[i].minSoundId;
            }
            /* fallthrough */
        case 1:
            Debug1967_AdjustSoundCategory(0x80, &(&gEwramData->debugMenuWork)->selection);
            selected = (&gEwramData->debugMenuWork)->selection;
            entries = sDebugSoundCategories;
            Debug1967_AdjustSoundId(&entries[selected], 0x10, &(&gEwramData->debugMenuWork)->entryValues[(&gEwramData->debugMenuWork)->selection]);
            if (gEwramData->inputData.newInput & 1)
            {
                PlaySong((&gEwramData->debugMenuWork)->entryValues[(&gEwramData->debugMenuWork)->selection]);
            }
            if (gEwramData->inputData.newInput & 2)
            {
                PlaySong(0x1000);
            }
            break;
    }
    return -1;
}

/**
 * @brief OBJTEST entry: create the viewer entity and update entities.
 */
s32 Debug1967_ObjTest(void)
{
    u8 pad[0x40];
    struct EwramData_EntityData *child;

    switch (gEwramData->debugMenuWork.state)
    {
        case 0:
            child = EntityCreateInRange(0x1B, 0x48, (u32 *) Debug1967ObjTestViewer_Update);
            if (child != NULL)
            {
                child->posX.part16.integer = 0x80;
                child->posY.part16.integer = 0x50;
            }
            gEwramData->debugMenuWork.state = 1;
            /* fallthrough */
        case 1:
            Entity_UpdateAll();
            break;
    }

    return -1;
}

/**
 * @brief Unreferenced empty function.
 */
void Debug1967_UnusedStub(void)
{
    return;
}

/**
 * @brief Carry callback of the debug platform: when flagged, copy its x
 * fraction to the player and return its x velocity.
 */
s32 DebugPlatform_GetCarryVelocity(struct EwramData_EntityData *entity)
{
    if ((u8) entity->ext.debug.platformCarrying == 0)
        return 0;

    gEwramData->playerState.player->posX.part16.fraction = entity->posX.part16.fraction;
    entity->ext.debug.platformCarrying = 0;
    return entity->velX.whole;
}

// TODO: function declarations
// void Entity_ShowDamage(struct EwramData_EntityData *, s16, s32);
// s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);

/**
 * @brief Take player attack damage; at zero HP start the falling death state
 * 2. Show the damage number.
 */
void DebugTestEnemy_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 attackPower;

    attackPower = (s16) Player_CalculateAttackPower(hit[2]);
    Hit_ResolveDamage(entity, attacker, hit, attackPower);
    entity->hitFlashTimer = 8;

    entity->hp -= attackPower;
    if ((s16)entity->hp <= 0)
    {
        entity->timer = 0x3C;
        entity->state = 2;
        entity->velY.whole = -0x4000;
        entity->accelY.whole = 0x1000;
        entity->palette = entity->basePalette;
        Anim_SelectScript(entity, 2, 1);
    }

    Entity_ShowDamage(entity, attackPower, 0);
}

/**
 * @brief While alive and the hit-flash timer runs, alternate palette 4 and
 * return 1.
 */
s32 DebugTestEnemy_UpdateHitFlash(struct EwramData_EntityData *entity)
{
    if ((s16)entity->hp > 0)
    {
        if (entity->hitFlashTimer != 0)
        {
            entity->hitFlashTimer -= 1;
            if (entity->hitFlashTimer & 1)
            {
                entity->palette = 4;
            }
            else
            {
                entity->palette = entity->basePalette;
            }
            return 1;
        }
    }
    return 0;
}

// TODO: function declarations
// ? Hit_CheckWithPlayer(? *, struct EwramData_EntityData *);
// ? HitBox_SetBodyRaw(struct EwramData_EntityData *, s32);
// ? Effect_SpawnRandom(s32, s32, s32, s32, s32);

/**
 * @brief Debug test enemy (100 HP): circle with a knockback impulse, on death
 * fall with random effects and delete itself.
 */
void DebugTestEnemy_Update(struct EwramData_EntityData *entity)
{
    u32 hitInfo[0x4];
    s32 gfx;

    if (DebugTestEnemy_UpdateHitFlash(entity) != 0)
    {
        Hit_CheckWithPlayer(&hitInfo, entity);
        return;
    }

    switch (entity->state)
    {
        case 0:
            gfx = ObjGfx_LoadResource((u8 *)0x081CBF2C, 0);
            entity->basePalette = (u8) ObjPalette_FindOrLoad((void *)0x0820A3FC);
            Sprite_Attach(entity, (void *)0x0821285C, gfx, (u32) entity->basePalette);
            Anim_Start(entity, (struct AnimScript *)(*(s32 *)0x086391C0), 0, 1);
            entity->state = 1;
            entity->scrollWithBg = 1;

            Hit_Enable(entity, 9, &DebugTestEnemy_OnPlayerContact, (s32) DebugTestEnemy_OnHit);
            Hit_SetTimerOverride(3, 0xA);
            HitBox_SetBodyRaw(entity, 0x2014E0F6);
            HitBox_SetAttackRaw(entity, 0x2014E0F6);

            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            entity->hp = 0x64;
            entity->spriteLayer = 0x12;
            break;

        case 1:
            entity->posX.whole += entity->velX.whole;
            entity->posY.whole += entity->velY.whole;
            entity->ext.debug.testEnemyAngle += 0x400;
            entity->velY.whole = Sin(entity->ext.debug.testEnemyAngle);
            entity->velX.whole = Sin(entity->ext.debug.testEnemyAngle + 0x4000);

            if (entity->accelX.whole > 0)
            {
                entity->velX.whole += entity->accelX.whole;
                entity->accelX.whole -= 0x8000;
                if (entity->accelX.whole < 0)
                {
                    entity->accelX.whole = 0;
                }
            }
            else if (entity->accelX.whole < 0)
            {
                entity->velX.whole += entity->accelX.whole;
                entity->accelX.whole += 0x8000;
                if (entity->accelX.whole > 0)
                {
                    entity->accelX.whole = 0;
                }
            }
            break;

        case 2:
            entity->velY.whole += entity->accelY.whole;
            entity->posY.whole += entity->velY.whole;
            if (!(entity->timer & 7))
            {
                Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0x80000, 0x80000, entity->spriteLayer - 1);
            }

            entity->timer -= 1;
            if (entity->timer == 0)
            {
                entity->deleteRequested = 1;
            }
            break;
    }

    if (entity->state != 2)
    {
        Hit_CheckWithPlayer(&hitInfo, entity);
    }
    Player_TrackEntity(entity);
    Anim_Update(entity);
}

/**
 * @brief Knock the player away and apply contact damage.
 */
void DebugTestEnemy_OnPlayerContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage subroutine_arg0;
    s32 knockbackX;

    subroutine_arg0.attack = 10;
    subroutine_arg0.attributes = 2;
    subroutine_arg0.unused = 0;
    subroutine_arg0.sourceX = entity->posX.whole;
    subroutine_arg0.sourceY = entity->posY.whole;
    subroutine_arg0.reaction = 3;

    knockbackX = -0x68000;
    if (gEwramData->playerState.player->posX.whole > entity->posX.whole)
    {
        knockbackX = -knockbackX;
    }
    Player_SetVelocityWithBraking(knockbackX, -0x30000);
    Player_ApplyContactDamage(&subroutine_arg0);
}
