	.include "asm/gba_constants.inc"
    .include "asm/macros.inc"

	.syntax unified

    .section .rodata

    .incbin "cvaos_us_baserom.gba", 0x0E5364, 0x0E6E04 - 0x0E5364

    .global sMainMenuDeleteBg2Map
sMainMenuDeleteBg2Map: @ 0x080E6E04
    .incbin "cvaos_us_baserom.gba", 0x0E6E04, 0x1 @ TODO: size

    .incbin "cvaos_us_baserom.gba", 0x0E6E05, 0x0E6FB4 - 0x0E6E05

    .global sMainMenuCopyBg2Map
sMainMenuCopyBg2Map: @ 0x080E6FB4
    .incbin "cvaos_us_baserom.gba", 0x0E6FB4, 0x1 @ TODO: size

    .incbin "cvaos_us_baserom.gba", 0x0E6FB5, 0x0E71F4 - 0x0E6FB5

    .global sMainMenuDeleteBg1Map
sMainMenuDeleteBg1Map: @ 0x080E71F4
    .incbin "cvaos_us_baserom.gba", 0x0E71F4, 0x1 @ TODO: size

    .incbin "cvaos_us_baserom.gba", 0x0E71F5, 0x0E7284 - 0x0E71F5

    .global sMainMenuCopyBg1Map
sMainMenuCopyBg1Map: @ 0x080E7284
    .incbin "cvaos_us_baserom.gba", 0x0E7284, 0x1 @ TODO: size

    .incbin "cvaos_us_baserom.gba", 0x0E7285, 0x0E9644 - 0x0E7285

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

    .incbin "cvaos_us_baserom.gba", 0x1183B1, 0x43

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

    .global sObject0CSolidCallbacks
sObject0CSolidCallbacks: @ 0x08118520
    .incbin "cvaos_us_baserom.gba", 0x118520, 0x1C

    .global sObject12Variant0AnimScript0
sObject12Variant0AnimScript0: @ 0x0811853C
    .incbin "cvaos_us_baserom.gba", 0x11853C, 0x8

    .global sObject12Variant0AnimScript1
sObject12Variant0AnimScript1: @ 0x08118544
    .incbin "cvaos_us_baserom.gba", 0x118544, 0x14

    .global sObject12Variant0AnimScript2
sObject12Variant0AnimScript2: @ 0x08118558
    .incbin "cvaos_us_baserom.gba", 0x118558, 0x20

    .global sObject14Variant1SolidCallbacks
sObject14Variant1SolidCallbacks: @ 0x08118578
    .incbin "cvaos_us_baserom.gba", 0x118578, 0x18

    .global sUnk_08118590
sUnk_08118590: @ 0x08118590
    .incbin "cvaos_us_baserom.gba", 0x118590, 0x20

    .global sObject17Xs
sObject17Xs: @ 0x081185B0
    .incbin "cvaos_us_baserom.gba", 0x1185B0, 0x4

    .global sObject17Ys
sObject17Ys: @ 0x081185B4
    .incbin "cvaos_us_baserom.gba", 0x1185B4, 0x14

    .global sObject1BColors
sObject1BColors: @ 0x081185C8
    .incbin "cvaos_us_baserom.gba", 0x1185C8, 0x30

    .global sWarpLightPaletteSteps
sWarpLightPaletteSteps: @ 0x081185F8
    .incbin "cvaos_us_baserom.gba", 0x1185F8, 0x38

    .global sObject29Boxes
sObject29Boxes: @ 0x08118630
    .incbin "cvaos_us_baserom.gba", 0x118630, 0x10

    .global sObject2ABoxes
sObject2ABoxes: @ 0x08118640
    .incbin "cvaos_us_baserom.gba", 0x118640, 0x10

    .global sObject2FXs
sObject2FXs: @ 0x08118650
    .incbin "cvaos_us_baserom.gba", 0x118650, 0x6

    .global sObject2FYs
sObject2FYs: @ 0x08118656
    .incbin "cvaos_us_baserom.gba", 0x118656, 0x6

    .global sObject31Xs
sObject31Xs: @ 0x0811865C
    .incbin "cvaos_us_baserom.gba", 0x11865C, 0x8

    .global sObject31Ys
sObject31Ys: @ 0x08118664
    .incbin "cvaos_us_baserom.gba", 0x118664, 0x8

    .global sObject31Anims
sObject31Anims: @ 0x0811866C
    .incbin "cvaos_us_baserom.gba", 0x11866C, 0x8

    .global sUnk_08118674
sUnk_08118674: @ 0x08118674
    .incbin "cvaos_us_baserom.gba", 0x118674, 0x10

    .global sObject32Boxes
sObject32Boxes: @ 0x08118684
    .incbin "cvaos_us_baserom.gba", 0x118684, 0x8

    .global sObject36SolidCallbacks
sObject36SolidCallbacks: @ 0x0811868C
    .incbin "cvaos_us_baserom.gba", 0x11868C, 0x18

    .global sObject36Shapes
sObject36Shapes: @ 0x081186A4
    .incbin "cvaos_us_baserom.gba", 0x1186A4, 0x38

    .global sObjectTableGraphics
sObjectTableGraphics: @ 0x081186DC
    .incbin "cvaos_us_baserom.gba", 0x1186DC, 0xD0

    .global sEventScene03SpotsA
sEventScene03SpotsA: @ 0x081187AC
    .incbin "cvaos_us_baserom.gba", 0x1187AC, 0x28

    .global sEventScene03SpotsB
sEventScene03SpotsB: @ 0x081187D4
    .incbin "cvaos_us_baserom.gba", 0x1187D4, 0x158

    .global sEventScene19Gfx
sEventScene19Gfx: @ 0x0811892C
    .incbin "cvaos_us_baserom.gba", 0x11892C, 0x74

    .global sEventScene30FragmentFrames
sEventScene30FragmentFrames: @ 0x081189A0
    .incbin "cvaos_us_baserom.gba", 0x1189A0, 0xC

    .global sEventScene30BurstFrames
sEventScene30BurstFrames: @ 0x081189AC
    .incbin "cvaos_us_baserom.gba", 0x1189AC, 0xC

    .global sEventScene30BurstAnimations
sEventScene30BurstAnimations: @ 0x081189B8
    .incbin "cvaos_us_baserom.gba", 0x1189B8, 0x14

    .global sEventScene30BurstPositions
sEventScene30BurstPositions: @ 0x081189CC
    .incbin "cvaos_us_baserom.gba", 0x1189CC, 0x78

    .global sEventScene30EmitterBSteps
sEventScene30EmitterBSteps: @ 0x08118A44
    .incbin "cvaos_us_baserom.gba", 0x118A44, 0x50

    .global sEventScene30EmitterCSequence
sEventScene30EmitterCSequence: @ 0x08118A94
    .incbin "cvaos_us_baserom.gba", 0x118A94, 0x28

    .global sEventScene30EmitterDSteps
sEventScene30EmitterDSteps: @ 0x08118ABC
    .incbin "cvaos_us_baserom.gba", 0x118ABC, 0x50

    .global sEventScene30EmitterESequence
sEventScene30EmitterESequence: @ 0x08118B0C
    .incbin "cvaos_us_baserom.gba", 0x118B0C, 0x28

    @ 0x08118B34: callback table, .rodata of src/event/scene30_transformation.c
