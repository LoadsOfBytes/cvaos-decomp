	.include "asm/gba_constants.inc"
    .include "asm/macros.inc"

	.syntax unified

    .section .rodata

    .global sDebugBgDispTestGfx
sDebugBgDispTestGfx: @ 0x080E5364
    .incbin "cvaos_us_baserom.gba", 0x0E5364, 0x0E536C - 0x0E5364

    .global sDebugBgDispTestGfx2
sDebugBgDispTestGfx2: @ 0x080E536C
    .incbin "cvaos_us_baserom.gba", 0x0E536C, 0x0E5374 - 0x0E536C

    .global sDebugBgDispTestGfx3
sDebugBgDispTestGfx3: @ 0x080E5374
    .incbin "cvaos_us_baserom.gba", 0x0E5374, 0x0E537C - 0x0E5374

    .global sDebugBgDispTestBgPalette
sDebugBgDispTestBgPalette: @ 0x080E537C
    .incbin "cvaos_us_baserom.gba", 0x0E537C, 0x0E5BA0 - 0x0E537C

    .global sDebugBgDispTestBgMap
sDebugBgDispTestBgMap: @ 0x080E5BA0
    .incbin "cvaos_us_baserom.gba", 0x0E5BA0, 0x0E5BB0 - 0x0E5BA0

    .global sSaveSelectGfx
sSaveSelectGfx: @ 0x080E5BB0
    .incbin "cvaos_us_baserom.gba", 0x0E5BB0, 0x0E5BB8 - 0x0E5BB0

    .global sSaveSelectGfx2
sSaveSelectGfx2: @ 0x080E5BB8
    .incbin "cvaos_us_baserom.gba", 0x0E5BB8, 0x0E5BC0 - 0x0E5BB8

    .global sSaveMenuSpriteGfx3
sSaveMenuSpriteGfx3: @ 0x080E5BC0
    .incbin "cvaos_us_baserom.gba", 0x0E5BC0, 0x0E5BC8 - 0x0E5BC0

    .global sCreditsGfx
sCreditsGfx: @ 0x080E5BC8
    .incbin "cvaos_us_baserom.gba", 0x0E5BC8, 0x0E5BD0 - 0x0E5BC8

    .global sCreditsGfx2
sCreditsGfx2: @ 0x080E5BD0
    .incbin "cvaos_us_baserom.gba", 0x0E5BD0, 0x0E5BD8 - 0x0E5BD0

    .global sCreditsGfx3
sCreditsGfx3: @ 0x080E5BD8
    .incbin "cvaos_us_baserom.gba", 0x0E5BD8, 0x0E5BE0 - 0x0E5BD8

    .global sTitleScreenGfx2
sTitleScreenGfx2: @ 0x080E5BE0
    .incbin "cvaos_us_baserom.gba", 0x0E5BE0, 0x0E5BE8 - 0x0E5BE0

    .global sTitleScreenGfx3
sTitleScreenGfx3: @ 0x080E5BE8
    .incbin "cvaos_us_baserom.gba", 0x0E5BE8, 0x0E5BF0 - 0x0E5BE8

    .global sIntroCutsceneGfx
sIntroCutsceneGfx: @ 0x080E5BF0
    .incbin "cvaos_us_baserom.gba", 0x0E5BF0, 0x0E5BF8 - 0x0E5BF0

    .global sIntroCutsceneGfx2
sIntroCutsceneGfx2: @ 0x080E5BF8
    .incbin "cvaos_us_baserom.gba", 0x0E5BF8, 0x0E5C00 - 0x0E5BF8

    .global sLicensedByNintendoGfx
sLicensedByNintendoGfx: @ 0x080E5C00
    .incbin "cvaos_us_baserom.gba", 0x0E5C00, 0x0E5C08 - 0x0E5C00

    .global sGameOverGfx
sGameOverGfx: @ 0x080E5C08
    .incbin "cvaos_us_baserom.gba", 0x0E5C08, 0x0E5C10 - 0x0E5C08

    .global sGameOverGfx2
sGameOverGfx2: @ 0x080E5C10
    .incbin "cvaos_us_baserom.gba", 0x0E5C10, 0x0E5C18 - 0x0E5C10

    .global sKonamiLogoGfx
sKonamiLogoGfx: @ 0x080E5C18
    .incbin "cvaos_us_baserom.gba", 0x0E5C18, 0x0E5C20 - 0x0E5C18

    .global sSaveSelectBgPalette
sSaveSelectBgPalette: @ 0x080E5C20
    .incbin "cvaos_us_baserom.gba", 0x0E5C20, 0x0E5E24 - 0x0E5C20

    .global sMainMenuPalette
sMainMenuPalette: @ 0x080E5E24
    .incbin "cvaos_us_baserom.gba", 0x0E5E24, 0x0E6028 - 0x0E5E24

    .global sCreditsBgPalette
sCreditsBgPalette: @ 0x080E6028
    .incbin "cvaos_us_baserom.gba", 0x0E6028, 0x0E622C - 0x0E6028

    .global sTitleLogoBlendPalette
sTitleLogoBlendPalette: @ 0x080E622C
    .incbin "cvaos_us_baserom.gba", 0x0E622C, 0x0E6430 - 0x0E622C

    .global sTitleScreenBgPalette
sTitleScreenBgPalette: @ 0x080E6430
    .incbin "cvaos_us_baserom.gba", 0x0E6430, 0x0E6634 - 0x0E6430

    .global sIntroCutsceneBgPalette
sIntroCutsceneBgPalette: @ 0x080E6634
    .incbin "cvaos_us_baserom.gba", 0x0E6634, 0x0E6838 - 0x0E6634

    .global sLicensedByNintendoBgPalette
sLicensedByNintendoBgPalette: @ 0x080E6838
    .incbin "cvaos_us_baserom.gba", 0x0E6838, 0x0E6A3C - 0x0E6838

    .global sGameOverBgPalette
sGameOverBgPalette: @ 0x080E6A3C
    .incbin "cvaos_us_baserom.gba", 0x0E6A3C, 0x0E6C40 - 0x0E6A3C

    .global sKonamiLogoBgPalette
sKonamiLogoBgPalette: @ 0x080E6C40
    .incbin "cvaos_us_baserom.gba", 0x0E6C40, 0x0E6CE4 - 0x0E6C40

    .global sSoundTestMenuBgMap
sSoundTestMenuBgMap: @ 0x080E6CE4
    .incbin "cvaos_us_baserom.gba", 0x0E6CE4, 0x0E6D74 - 0x0E6CE4

    .global sSoundTestMenuBgMap2
sSoundTestMenuBgMap2: @ 0x080E6D74
    .incbin "cvaos_us_baserom.gba", 0x0E6D74, 0x0E6E04 - 0x0E6D74

    .global sMainMenuDeleteBg2Map
sMainMenuDeleteBg2Map: @ 0x080E6E04
    .incbin "cvaos_us_baserom.gba", 0x0E6E04, 0x1 @ TODO: size

    .incbin "cvaos_us_baserom.gba", 0x0E6E05, 0x0E6E94 - 0x0E6E05

    .global sMainMenuBgMap
sMainMenuBgMap: @ 0x080E6E94
    .incbin "cvaos_us_baserom.gba", 0x0E6E94, 0x0E6F24 - 0x0E6E94

    .global sSoulTradeMenuBgMap
sSoulTradeMenuBgMap: @ 0x080E6F24
    .incbin "cvaos_us_baserom.gba", 0x0E6F24, 0x0E6FB4 - 0x0E6F24

    .global sMainMenuCopyBg2Map
sMainMenuCopyBg2Map: @ 0x080E6FB4
    .incbin "cvaos_us_baserom.gba", 0x0E6FB4, 0x1 @ TODO: size

    .incbin "cvaos_us_baserom.gba", 0x0E6FB5, 0x0E7044 - 0x0E6FB5

    .global sMainMenuBgMap2
sMainMenuBgMap2: @ 0x080E7044
    .incbin "cvaos_us_baserom.gba", 0x0E7044, 0x0E70D4 - 0x0E7044

    .global sMainMenuBgMap3
sMainMenuBgMap3: @ 0x080E70D4
    .incbin "cvaos_us_baserom.gba", 0x0E70D4, 0x0E7164 - 0x0E70D4

    .global sSoulTradeMenuBgMap2
sSoulTradeMenuBgMap2: @ 0x080E7164
    .incbin "cvaos_us_baserom.gba", 0x0E7164, 0x0E71F4 - 0x0E7164

    .global sMainMenuDeleteBg1Map
sMainMenuDeleteBg1Map: @ 0x080E71F4
    .incbin "cvaos_us_baserom.gba", 0x0E71F4, 0x1 @ TODO: size

    .incbin "cvaos_us_baserom.gba", 0x0E71F5, 0x0E7284 - 0x0E71F5

    .global sMainMenuCopyBg1Map
sMainMenuCopyBg1Map: @ 0x080E7284
    .incbin "cvaos_us_baserom.gba", 0x0E7284, 0x1 @ TODO: size

    .incbin "cvaos_us_baserom.gba", 0x0E7285, 0x0E7314 - 0x0E7285

    .global sMainMenuBgMap4
sMainMenuBgMap4: @ 0x080E7314
    .incbin "cvaos_us_baserom.gba", 0x0E7314, 0x0E73A4 - 0x0E7314

    .global sMainMenuBgMap5
sMainMenuBgMap5: @ 0x080E73A4
    .incbin "cvaos_us_baserom.gba", 0x0E73A4, 0x0E7434 - 0x0E73A4

    .global sMainMenuBgMap6
sMainMenuBgMap6: @ 0x080E7434
    .incbin "cvaos_us_baserom.gba", 0x0E7434, 0x0E74C4 - 0x0E7434

    .global sSaveSelectBgMap
sSaveSelectBgMap: @ 0x080E74C4
    .incbin "cvaos_us_baserom.gba", 0x0E74C4, 0x0E7B54 - 0x0E74C4

    .global sCreditsBgMap
sCreditsBgMap: @ 0x080E7B54
    .incbin "cvaos_us_baserom.gba", 0x0E7B54, 0x0E81E4 - 0x0E7B54

    .global sCreditsBgMap2
sCreditsBgMap2: @ 0x080E81E4
    .incbin "cvaos_us_baserom.gba", 0x0E81E4, 0x0E8874 - 0x0E81E4

    .global sCreditsBgMap3
sCreditsBgMap3: @ 0x080E8874
    .incbin "cvaos_us_baserom.gba", 0x0E8874, 0x0E8F04 - 0x0E8874

    .global sCreditsBgMap4
sCreditsBgMap4: @ 0x080E8F04
    .incbin "cvaos_us_baserom.gba", 0x0E8F04, 0x0E8F94 - 0x0E8F04

    .global sTitleScreenBgMap
sTitleScreenBgMap: @ 0x080E8F94
    .incbin "cvaos_us_baserom.gba", 0x0E8F94, 0x0E9024 - 0x0E8F94

    .global sTitleScreenBgMap2
sTitleScreenBgMap2: @ 0x080E9024
    .incbin "cvaos_us_baserom.gba", 0x0E9024, 0x0E90B4 - 0x0E9024

    .global sTitleScreenBgMap3
sTitleScreenBgMap3: @ 0x080E90B4
    .incbin "cvaos_us_baserom.gba", 0x0E90B4, 0x0E9244 - 0x0E90B4

    .global sIntroCutsceneBgMap
sIntroCutsceneBgMap: @ 0x080E9244
    .incbin "cvaos_us_baserom.gba", 0x0E9244, 0x0E9354 - 0x0E9244

    .global sIntroCutsceneBgMap2
sIntroCutsceneBgMap2: @ 0x080E9354
    .incbin "cvaos_us_baserom.gba", 0x0E9354, 0x0E9464 - 0x0E9354

    .global sIntroCutsceneBgMap3
sIntroCutsceneBgMap3: @ 0x080E9464
    .incbin "cvaos_us_baserom.gba", 0x0E9464, 0x0E94F4 - 0x0E9464

    .global sLicensedByNintendoBgMap
sLicensedByNintendoBgMap: @ 0x080E94F4
    .incbin "cvaos_us_baserom.gba", 0x0E94F4, 0x0E9584 - 0x0E94F4

    .global sGameOverBgMap
sGameOverBgMap: @ 0x080E9584
    .incbin "cvaos_us_baserom.gba", 0x0E9584, 0x0E9614 - 0x0E9584

    .global sKonamiLogoBgMap
sKonamiLogoBgMap: @ 0x080E9614
    .incbin "cvaos_us_baserom.gba", 0x0E9614, 0x0E9644 - 0x0E9614

    .global sEnemyData
sEnemyData: @ 0x080E9644
    .incbin "cvaos_us_baserom.gba", 0x0E9644, 0x1 @ TODO: size

    .incbin "cvaos_us_baserom.gba", 0x0E9645, 0x0EA628 - 0x0E9645

    .global sEnemyNameTextIds
sEnemyNameTextIds: @ 0x080EA628
    .incbin "cvaos_us_baserom.gba", 0x0EA628, 0x1 @ TODO: size

    .incbin "cvaos_us_baserom.gba", 0x0EA629, 0x116650 - 0x0EA629

    .global sMapCellRoomInfo
sMapCellRoomInfo: @ 0x08116650
    .incbin "cvaos_us_baserom.gba", 0x116650, 0x1 @ TODO: size

    .incbin "cvaos_us_baserom.gba", 0x116651, 0x117DD0 - 0x116651

    .global sMapCellEdges
sMapCellEdges: @ 0x08117DD0
    .incbin "cvaos_us_baserom.gba", 0x117DD0, 0x1 @ TODO: size

    .incbin "cvaos_us_baserom.gba", 0x117DD1, 0x1183B0 - 0x117DD1

    .global sGameFlagMapCoordinates
sGameFlagMapCoordinates: @ 0x081183B0
    .incbin "cvaos_us_baserom.gba", 0x1183B0, 0x1

    .incbin "cvaos_us_baserom.gba", 0x1183B1, 0x1183CC - 0x1183B1

    .global sMapScreenBgPalette
sMapScreenBgPalette: @ 0x081183CC
    .incbin "cvaos_us_baserom.gba", 0x1183CC, 0x1183F4 - 0x1183CC

    .global sTextBgPalette
sTextBgPalette: @ 0x081183F4
    .incbin "cvaos_us_baserom.gba", 0x1183F4, 0x44

    .global sHudMapTiles
sHudMapTiles: @ 0x08118438
    .incbin "cvaos_us_baserom.gba", 0x118438, 0xC

    .global sHudMapPalettes
sHudMapPalettes: @ 0x08118444
    .incbin "cvaos_us_baserom.gba", 0x118444, 0xCC

    .global sUnk_08118510
sUnk_08118510: @ 0x08118510
    .incbin "cvaos_us_baserom.gba", 0x118510, 0x10

    .global sTriggerZoneSolidCallbacks
sTriggerZoneSolidCallbacks: @ 0x08118520
    .incbin "cvaos_us_baserom.gba", 0x118520, 0x1C

    .global sRoomDecorationVariant0AnimScript0
sRoomDecorationVariant0AnimScript0: @ 0x0811853C
    .incbin "cvaos_us_baserom.gba", 0x11853C, 0x8

    .global sRoomDecorationVariant0AnimScript1
sRoomDecorationVariant0AnimScript1: @ 0x08118544
    .incbin "cvaos_us_baserom.gba", 0x118544, 0x14

    .global sRoomDecorationVariant0AnimScript2
sRoomDecorationVariant0AnimScript2: @ 0x08118558
    .incbin "cvaos_us_baserom.gba", 0x118558, 0x20

    .global sRollerVariant1SolidCallbacks
sRollerVariant1SolidCallbacks: @ 0x08118578
    .incbin "cvaos_us_baserom.gba", 0x118578, 0x18

    .global sElevatorFloorY
sElevatorFloorY: @ 0x08118590
    .incbin "cvaos_us_baserom.gba", 0x118590, 0x118598 - 0x118590

    .global sElevatorSolidCallbacks
sElevatorSolidCallbacks: @ 0x08118598
    .incbin "cvaos_us_baserom.gba", 0x118598, 0x1185B0 - 0x118598

    .global sElevatorXs
sElevatorXs: @ 0x081185B0
    .incbin "cvaos_us_baserom.gba", 0x1185B0, 0x4

    .global sElevatorYs
sElevatorYs: @ 0x081185B4
    .incbin "cvaos_us_baserom.gba", 0x1185B4, 0x14

    .global sConveyorSwitchColors
sConveyorSwitchColors: @ 0x081185C8
    .incbin "cvaos_us_baserom.gba", 0x1185C8, 0x30

    .global sWarpLightPaletteSteps
sWarpLightPaletteSteps: @ 0x081185F8
    .incbin "cvaos_us_baserom.gba", 0x1185F8, 0x38

    .global sVerticalPlatformBoxes
sVerticalPlatformBoxes: @ 0x08118630
    .incbin "cvaos_us_baserom.gba", 0x118630, 0x10

    .global sHorizontalPlatformBoxes
sHorizontalPlatformBoxes: @ 0x08118640
    .incbin "cvaos_us_baserom.gba", 0x118640, 0x10

    .global sJuliusArenaPropXs
sJuliusArenaPropXs: @ 0x08118650
    .incbin "cvaos_us_baserom.gba", 0x118650, 0x6

    .global sJuliusArenaPropYs
sJuliusArenaPropYs: @ 0x08118656
    .incbin "cvaos_us_baserom.gba", 0x118656, 0x6

    .global sJuliusArenaDebrisXs
sJuliusArenaDebrisXs: @ 0x0811865C
    .incbin "cvaos_us_baserom.gba", 0x11865C, 0x8

    .global sJuliusArenaDebrisYs
sJuliusArenaDebrisYs: @ 0x08118664
    .incbin "cvaos_us_baserom.gba", 0x118664, 0x8

    .global sJuliusArenaDebrisAnims
sJuliusArenaDebrisAnims: @ 0x0811866C
    .incbin "cvaos_us_baserom.gba", 0x11866C, 0x8

    .global sUnk_08118674
sUnk_08118674: @ 0x08118674
    .incbin "cvaos_us_baserom.gba", 0x118674, 0x10

    .global sCrumblingPlatformBoxes
sCrumblingPlatformBoxes: @ 0x08118684
    .incbin "cvaos_us_baserom.gba", 0x118684, 0x8

    .global sRotatingPlatformSolidCallbacks
sRotatingPlatformSolidCallbacks: @ 0x0811868C
    .incbin "cvaos_us_baserom.gba", 0x11868C, 0x18

    .global sRotatingPlatformShapes
sRotatingPlatformShapes: @ 0x081186A4
    .incbin "cvaos_us_baserom.gba", 0x1186A4, 0x38

    .global sObjectTableGraphics
sObjectTableGraphics: @ 0x081186DC
    .incbin "cvaos_us_baserom.gba", 0x1186DC, 0xD0

    .global sEventScenePrologueBattleSpotsA
sEventScenePrologueBattleSpotsA: @ 0x081187AC
    .incbin "cvaos_us_baserom.gba", 0x1187AC, 0x28

    .global sEventScenePrologueBattleSpotsB
sEventScenePrologueBattleSpotsB: @ 0x081187D4
    .incbin "cvaos_us_baserom.gba", 0x1187D4, 0x1188E4 - 0x1187D4

    .global sEventSceneSomaAwakensPeriods
sEventSceneSomaAwakensPeriods: @ 0x081188E4
    .incbin "cvaos_us_baserom.gba", 0x1188E4, 0x11892C - 0x1188E4

    .global sEventSceneJuliusBattleStartGfx
sEventSceneJuliusBattleStartGfx: @ 0x0811892C
    .incbin "cvaos_us_baserom.gba", 0x11892C, 0x74

    .global sEventSceneEndingFragmentFrames
sEventSceneEndingFragmentFrames: @ 0x081189A0
    .incbin "cvaos_us_baserom.gba", 0x1189A0, 0xC

    .global sEventSceneEndingBurstFrames
sEventSceneEndingBurstFrames: @ 0x081189AC
    .incbin "cvaos_us_baserom.gba", 0x1189AC, 0xC

    .global sEventSceneEndingBurstAnimations
sEventSceneEndingBurstAnimations: @ 0x081189B8
    .incbin "cvaos_us_baserom.gba", 0x1189B8, 0x14

    .global sEventSceneEndingBurstPositions
sEventSceneEndingBurstPositions: @ 0x081189CC
    .incbin "cvaos_us_baserom.gba", 0x1189CC, 0x78

    .global sEventSceneEndingEmitterBSteps
sEventSceneEndingEmitterBSteps: @ 0x08118A44
    .incbin "cvaos_us_baserom.gba", 0x118A44, 0x50

    .global sEventSceneEndingEmitterCSequence
sEventSceneEndingEmitterCSequence: @ 0x08118A94
    .incbin "cvaos_us_baserom.gba", 0x118A94, 0x28

    .global sEventSceneEndingEmitterDSteps
sEventSceneEndingEmitterDSteps: @ 0x08118ABC
    .incbin "cvaos_us_baserom.gba", 0x118ABC, 0x50

    .global sEventSceneEndingEmitterESequence
sEventSceneEndingEmitterESequence: @ 0x08118B0C
    .incbin "cvaos_us_baserom.gba", 0x118B0C, 0x28

    @ 0x08118B34: callback table, .rodata of src/event/scene30_transformation.c
