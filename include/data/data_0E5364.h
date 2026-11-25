#ifndef DATA_0E5364_H
#define DATA_0E5364_H

/* Data of asm/data/data_0E5364.s used by C files. */

#include "types.h"

/* Spawn spots of EventScene03_SpawnPartsA: room x and flags for EventScene03_SpawnRisingPart. */
struct Unk_081187AC {
    s32 x;
    s32 flags;
};

struct FragmentFrames { u32 frame[3]; };

struct BurstFrames {u32 frame[3];};

struct BurstAnimations {u32 frame[5];};

struct BurstPosition {s16 x,y;u8 delay;u8 pad[3];};

struct EffectStep { u16 x,y; u8 delay; u8 padding[3]; };

struct EffectSequence { struct EffectStep steps[5]; };

struct Unk_08118510
{
    s32 unk[4];
};

struct FourBoxes { u32 box[4]; };

/* Object 0x31 per variant (spawnParam0): start x, start y, animation, unk_4F8. */
struct Object31Words {
    s32 v[4];
};

/* Two hit boxes of object 0x32 (by spawnParam1). */
struct CrumblingPlatformBoxes {
    u32 box[2];
};

/* Twelve resource tuples in palette, sprite-sheet, graphics order. */
struct ObjectTableGraphics {
    void *palette;
    void *spriteSheet;
    u8 *graphics;
};

extern u8 sDebugBgDispTestGfx[];
extern u8 sDebugBgDispTestGfx2[];
extern u8 sDebugBgDispTestGfx3[];
extern u8 sDebugBgDispTestBgPalette[];
extern u8 sDebugBgDispTestBgMap[];
extern u8 sSaveSelectGfx[];
extern u8 sSaveSelectGfx2[];
extern u8 sSaveMenuSpriteGfx3[];
extern u8 sCreditsGfx[];
extern u8 sCreditsGfx2[];
extern u8 sCreditsGfx3[];
extern u8 sTitleScreenGfx2[];
extern u8 sTitleScreenGfx3[];
extern u8 sIntroCutsceneGfx[];
extern u8 sIntroCutsceneGfx2[];
extern u8 sLicensedByNintendoGfx[];
extern u8 sGameOverGfx[];
extern u8 sGameOverGfx2[];
extern u8 sKonamiLogoGfx[];
extern u8 sSaveSelectBgPalette[];
extern u8 sMainMenuPalette[];
extern u8 sCreditsBgPalette[];
extern u8 sTitleLogoBlendPalette[];
extern u8 sTitleScreenBgPalette[];
extern u8 sIntroCutsceneBgPalette[];
extern u8 sLicensedByNintendoBgPalette[];
extern u8 sGameOverBgPalette[];
extern u8 sKonamiLogoBgPalette[];
extern u8 sSoundTestMenuBgMap[];
extern u8 sSoundTestMenuBgMap2[];
extern u32 *sMainMenuDeleteBg2Map;
extern u8 sMainMenuBgMap[];
extern u8 sSoulTradeMenuBgMap[];
extern u32 *sMainMenuCopyBg2Map;
extern u8 sMainMenuBgMap2[];
extern u8 sMainMenuBgMap3[];
extern u8 sSoulTradeMenuBgMap2[];
extern u32 *sMainMenuDeleteBg1Map;
extern u32 *sMainMenuCopyBg1Map;
extern u8 sMainMenuBgMap4[];
extern u8 sMainMenuBgMap5[];
extern u8 sMainMenuBgMap6[];
extern u8 sSaveSelectBgMap[];
extern u8 sCreditsBgMap[];
extern u8 sCreditsBgMap2[];
extern u8 sCreditsBgMap3[];
extern u8 sCreditsBgMap4[];
extern u8 sTitleScreenBgMap[];
extern u8 sTitleScreenBgMap2[];
extern u8 sTitleScreenBgMap3[];
extern u8 sIntroCutsceneBgMap[];
extern u8 sIntroCutsceneBgMap2[];
extern u8 sIntroCutsceneBgMap3[];
extern u8 sLicensedByNintendoBgMap[];
extern u8 sGameOverBgMap[];
extern u8 sKonamiLogoBgMap[];
extern u16 sMapCellRoomInfo[]; // map info
extern u8 sMapCellEdges[];
extern s8 sGameFlagMapCoordinates[][2];
extern u8 sMapScreenBgPalette[];
extern u8 sTextBgPalette[];
extern const s32 sHudMapTiles[3];
extern const s32 sHudMapPalettes[3];
extern const struct Unk_08118510 sUnk_08118510;
extern u8 sTriggerZoneSolidCallbacks[];
extern u8 sRoomDecorationVariant0AnimScript0[];
extern u8 sRoomDecorationVariant0AnimScript1[];
extern u8 sRoomDecorationVariant0AnimScript2[];
extern u8 sRollerVariant1SolidCallbacks[];
extern const s16 sElevatorFloorY[];
extern u8 sElevatorSolidCallbacks[];
extern const u16 sElevatorXs[];
extern const u16 sElevatorYs[];
extern u16 sObject1BColors[];
extern struct FourBoxes sVerticalPlatformBoxes;
extern struct FourBoxes sHorizontalPlatformBoxes;
/* Start x / y of object 0x2F per variant (spawnParam0). */
extern const u16 sObject2FXs[3];
extern const u16 sObject2FYs[3];
extern const u16 sObject31Xs[4];
extern const u16 sObject31Ys[4];
extern const u16 sObject31Anims[4];
extern const struct Object31Words sUnk_08118674;
extern const struct CrumblingPlatformBoxes sCrumblingPlatformBoxes;
extern u8 sRotatingPlatformSolidCallbacks[];
/* Object 0x36 per variant: { rotation speed * 2 / scale step, rotation speed }. */
extern const s16 sRotatingPlatformShapes[9][2];
extern struct ObjectTableGraphics sObjectTableGraphics[12];
extern const struct Unk_081187AC sEventScene03SpotsA[5];
extern const struct Unk_081187AC sEventScene03SpotsB[2];
extern u8 sEventSceneSomaAwakensPeriods[];
extern u8 sEventSceneJuliusBattleStartGfx[];
extern struct FragmentFrames sEventSceneEndingFragmentFrames;
extern struct BurstFrames sEventSceneEndingBurstFrames;
extern struct BurstAnimations sEventSceneEndingBurstAnimations;
extern struct BurstPosition sEventSceneEndingBurstPositions[15];
extern struct EffectStep sEventSceneEndingEmitterBSteps[];
extern struct EffectSequence sEventSceneEndingEmitterCSequence;
extern struct EffectStep sEventSceneEndingEmitterDSteps[];
extern struct EffectSequence sEventSceneEndingEmitterESequence;

#endif /* DATA_0E5364_H */
