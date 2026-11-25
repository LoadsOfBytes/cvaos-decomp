	.include "asm/gba_constants.inc"
    .include "asm/macros.inc"

	.syntax unified

    .section .data

    .incbin "cvaos_us_baserom.gba", 0x4F15A8, 0x14414

    .global sUnk_085059BC
sUnk_085059BC: @ 0x085059BC
    .incbin "cvaos_us_baserom.gba", 0x5059BC, 0x180

    .global sConsumableInfo
sConsumableInfo: @ 0x08505B3C
    .incbin "cvaos_us_baserom.gba", 0x505B3C, 0x158

    .global sUnk_08505C94
sUnk_08505C94: @ 0x08505C94
    .incbin "cvaos_us_baserom.gba", 0x505C94, 0xA8

    .global sWeaponInfo
sWeaponInfo: @ 0x08505D3C
    .incbin "cvaos_us_baserom.gba", 0x505D3C, 0x674

    .global sArmorInfo
sArmorInfo: @ 0x085063B0
    .incbin "cvaos_us_baserom.gba", 0x5063B0, 0x384

    .global sItemNameTextIds
sItemNameTextIds: @ 0x08506734
    .incbin "cvaos_us_baserom.gba", 0x506734, 0x202

    .global sItemDescriptionTextIds
sItemDescriptionTextIds: @ 0x08506936
    .incbin "cvaos_us_baserom.gba", 0x506936, 0x202

    .global sTextEntries
sTextEntries: @ 0x08506B38
    .incbin "cvaos_us_baserom.gba", 0x506B38, 0x2D3C

    .incbin "cvaos_us_baserom.gba", 0x509874, 0x2

    .global sText8Font
sText8Font: @ 0x08509876
    .incbin "cvaos_us_baserom.gba", 0x509876, 0x8C0

    .global sText12Font
sText12Font: @ 0x0850A136
    .incbin "cvaos_us_baserom.gba", 0x50A136, 0x4832

    .global sTransitionRooms
sTransitionRooms: @ 0x0850E968
    .incbin "cvaos_us_baserom.gba", 0x50E968, 0x44

    .incbin "cvaos_us_baserom.gba", 0x50E9AC, 0x50EF08 - 0x50E9AC

    .global sAreaRoomTables
sAreaRoomTables: @ 0x0850EF08
    .incbin "cvaos_us_baserom.gba", 0x50EF08, 0x30

    .incbin "cvaos_us_baserom.gba", 0x50EF38, 0x50EF9C - 0x50EF38

    .global sDebugHitTestRoom
sDebugHitTestRoom: @ 0x0850EF9C
    .incbin "cvaos_us_baserom.gba", 0x50EF9C, 0x50F01C - 0x50EF9C

    .global sDebug4531Room
sDebug4531Room: @ 0x0850F01C
    .incbin "cvaos_us_baserom.gba", 0x50F01C, 0x50F15C - 0x50F01C

    .global sDebugEventRoom12
sDebugEventRoom12: @ 0x0850F15C
    .incbin "cvaos_us_baserom.gba", 0x50F15C, 0x50F3B4 - 0x50F15C

    .global sDebugEventRoom3
sDebugEventRoom3: @ 0x0850F3B4
    .incbin "cvaos_us_baserom.gba", 0x50F3B4, 0x50FEEC - 0x50F3B4

    .global sDebugEventRoom1
sDebugEventRoom1: @ 0x0850FEEC
    .incbin "cvaos_us_baserom.gba", 0x50FEEC, 0x5122FC - 0x50FEEC

    .global sDebugExitOptionsRoom1
sDebugExitOptionsRoom1: @ 0x085122FC
    .incbin "cvaos_us_baserom.gba", 0x5122FC, 0x5132D4 - 0x5122FC

    .global sDebugExitOptionsRoom2
sDebugExitOptionsRoom2: @ 0x085132D4
    .incbin "cvaos_us_baserom.gba", 0x5132D4, 0x513414 - 0x5132D4

    .global sDebugEventRoom2
sDebugEventRoom2: @ 0x08513414
    .incbin "cvaos_us_baserom.gba", 0x513414, 0x5134AC - 0x513414

    .global sDebugEventRoom11
sDebugEventRoom11: @ 0x085134AC
    .incbin "cvaos_us_baserom.gba", 0x5134AC, 0x514AB0 - 0x5134AC

    .global sDebugExitOptionsRoom3
sDebugExitOptionsRoom3: @ 0x08514AB0
    .incbin "cvaos_us_baserom.gba", 0x514AB0, 0x515DB0 - 0x514AB0

    .global sChronomageDestinationRoom
sChronomageDestinationRoom: @ 0x08515DB0
    .incbin "cvaos_us_baserom.gba", 0x515DB0, 0x515EF0 - 0x515DB0

    .global sDebugExitOptionsRoom16
sDebugExitOptionsRoom16: @ 0x08515EF0
    .incbin "cvaos_us_baserom.gba", 0x515EF0, 0x516768 - 0x515EF0

    .global sDebugExitOptionsRoom4
sDebugExitOptionsRoom4: @ 0x08516768
    .incbin "cvaos_us_baserom.gba", 0x516768, 0x517790 - 0x516768

    .global sDebugEventRoom9
sDebugEventRoom9: @ 0x08517790
    .incbin "cvaos_us_baserom.gba", 0x517790, 0x517B70 - 0x517790

    .global sObject15DestinationRoom0
sObject15DestinationRoom0: @ 0x08517B70
    .incbin "cvaos_us_baserom.gba", 0x517B70, 0x518710 - 0x517B70

    .global sDebug2229Room
sDebug2229Room: @ 0x08518710
    .incbin "cvaos_us_baserom.gba", 0x518710, 0x518B70 - 0x518710

    .global sDebugEventRoom5
sDebugEventRoom5: @ 0x08518B70
    .incbin "cvaos_us_baserom.gba", 0x518B70, 0x5196E0 - 0x518B70

    .global sDebugExitOptionsRoom6
sDebugExitOptionsRoom6: @ 0x085196E0
    .incbin "cvaos_us_baserom.gba", 0x5196E0, 0x5198A0 - 0x5196E0

    .global sDebugEventRoom4
sDebugEventRoom4: @ 0x085198A0
    .incbin "cvaos_us_baserom.gba", 0x5198A0, 0x51AC14 - 0x5198A0

    .global sDebugEventRoom6
sDebugEventRoom6: @ 0x0851AC14
    .incbin "cvaos_us_baserom.gba", 0x51AC14, 0x51ADD4 - 0x51AC14

    .global sDebugExitOptionsRoom7
sDebugExitOptionsRoom7: @ 0x0851ADD4
    .incbin "cvaos_us_baserom.gba", 0x51ADD4, 0x51BF1C - 0x51ADD4

    .global sDebugExitOptionsRoom14
sDebugExitOptionsRoom14: @ 0x0851BF1C
    .incbin "cvaos_us_baserom.gba", 0x51BF1C, 0x51E490 - 0x51BF1C

    .global sDebugEventRoom7
sDebugEventRoom7: @ 0x0851E490
    .incbin "cvaos_us_baserom.gba", 0x51E490, 0x51E7F8 - 0x51E490

    .global sDebugExitOptionsRoom8
sDebugExitOptionsRoom8: @ 0x0851E7F8
    .incbin "cvaos_us_baserom.gba", 0x51E7F8, 0x51FD70 - 0x51E7F8

    .global sDebugEventRoom0
sDebugEventRoom0: @ 0x0851FD70
    .incbin "cvaos_us_baserom.gba", 0x51FD70, 0x520240 - 0x51FD70

    .global sDebugEventRoom8
sDebugEventRoom8: @ 0x08520240
    .incbin "cvaos_us_baserom.gba", 0x520240, 0x5209B8 - 0x520240

    .global sDebugExitOptionsRoom9
sDebugExitOptionsRoom9: @ 0x085209B8
    .incbin "cvaos_us_baserom.gba", 0x5209B8, 0x52269C - 0x5209B8

    .global sDebugEventRoom10
sDebugEventRoom10: @ 0x0852269C
    .incbin "cvaos_us_baserom.gba", 0x52269C, 0x522ABC - 0x52269C

    .global sDebugExitOptionsRoom11
sDebugExitOptionsRoom11: @ 0x08522ABC
    .incbin "cvaos_us_baserom.gba", 0x522ABC, 0x522BDC - 0x522ABC

    .global sChaosDestinationRoom
sChaosDestinationRoom: @ 0x08522BDC
    .incbin "cvaos_us_baserom.gba", 0x522BDC, 0x78

    .global sEndingRoomDefinition
sEndingRoomDefinition: @ 0x08522C54
    .incbin "cvaos_us_baserom.gba", 0x522C54, 0x523064 - 0x522C54

    .global sDebugExitOptionsRoom10
sDebugExitOptionsRoom10: @ 0x08523064
    .incbin "cvaos_us_baserom.gba", 0x523064, 0x52320C - 0x523064

    .global sObject15DestinationRoom1
sObject15DestinationRoom1: @ 0x0852320C
    .incbin "cvaos_us_baserom.gba", 0x52320C, 0x52362C - 0x52320C

    .global sDebugEventRoom14
sDebugEventRoom14: @ 0x0852362C
    .incbin "cvaos_us_baserom.gba", 0x52362C, 0x5236A4 - 0x52362C

    .global sDebugEventRoom17
sDebugEventRoom17: @ 0x085236A4
    .incbin "cvaos_us_baserom.gba", 0x5236A4, 0x524020 - 0x5236A4

    .global sDebug4531Room2
sDebug4531Room2: @ 0x08524020
    .incbin "cvaos_us_baserom.gba", 0x524020, 0x5241E0 - 0x524020

    .global sDebug1967Room
sDebug1967Room: @ 0x085241E0
    .incbin "cvaos_us_baserom.gba", 0x5241E0, 0x5247B4 - 0x5241E0

    .global sBossRushMenuRoom
sBossRushMenuRoom: @ 0x085247B4
    .incbin "cvaos_us_baserom.gba", 0x5247B4, 0x524E5C - 0x5247B4

    .global sDebugExitOptionsRoom12
sDebugExitOptionsRoom12: @ 0x08524E5C
    .incbin "cvaos_us_baserom.gba", 0x524E5C, 0x5250E4 - 0x524E5C

    .global sGoldDrops
sGoldDrops: @ 0x085250E4
    .incbin "cvaos_us_baserom.gba", 0x5250E4, 0x54

    .global sTimedParticleFrames
sTimedParticleFrames: @ 0x08525138
    .incbin "cvaos_us_baserom.gba", 0x525138, 0x70

    .global sUnk_085251A8
sUnk_085251A8: @ 0x085251A8
    .incbin "cvaos_us_baserom.gba", 0x5251A8, 0x4C

    .global sPlayerFlashInfo
sPlayerFlashInfo: @ 0x085251F4
    .incbin "cvaos_us_baserom.gba", 0x5251F4, 0x2A4

    .global sParticleTemplateLists
sParticleTemplateLists: @ 0x08525498
    .incbin "cvaos_us_baserom.gba", 0x525498, 0x8

    .global sPauseMenuBgMaps
sPauseMenuBgMaps: @ 0x085254A0
    .incbin "cvaos_us_baserom.gba", 0x5254A0, 0x20

    .global sPauseMenuPageOpenFuncs
sPauseMenuPageOpenFuncs: @ 0x085254C0
    .incbin "cvaos_us_baserom.gba", 0x5254C0, 0x1C

    .global sPauseMenuPageUpdateFuncs
sPauseMenuPageUpdateFuncs: @ 0x085254DC
    .incbin "cvaos_us_baserom.gba", 0x5254DC, 0x1C

    .global sButtonConfigCursorFrames
sButtonConfigCursorFrames: @ 0x085254F8
    .incbin "cvaos_us_baserom.gba", 0x5254F8, 0x4

    .global sMenuCursorPalette
sMenuCursorPalette: @ 0x085254FC
    .incbin "cvaos_us_baserom.gba", 0x5254FC, 0x8

    .global sButtonConfigButtons
sButtonConfigButtons: @ 0x08525504
    .incbin "cvaos_us_baserom.gba", 0x525504, 0x8

    .global sButtonConfigDefaultChoices
sButtonConfigDefaultChoices: @ 0x0852550C
    .incbin "cvaos_us_baserom.gba", 0x52550C, 0x4

    .global sButtonConfigChoiceSlots
sButtonConfigChoiceSlots: @ 0x08525510
    .incbin "cvaos_us_baserom.gba", 0x525510, 0xC

    .global sBestiaryAttributeIcons
sBestiaryAttributeIcons: @ 0x0852551C
    .incbin "cvaos_us_baserom.gba", 0x52551C, 0x48

    .global sGatePaletteAnim0
sGatePaletteAnim0: @ 0x08525564
    .incbin "cvaos_us_baserom.gba", 0x525564, 0x10

    .global sGatePaletteAnim1
sGatePaletteAnim1: @ 0x08525574
    .incbin "cvaos_us_baserom.gba", 0x525574, 0x28

    .global sUnk_0852559C
sUnk_0852559C: @ 0x0852559C
    .incbin "cvaos_us_baserom.gba", 0x52559C, 0x10

    .global sBgParticleCounts
sBgParticleCounts: @ 0x085255AC
    .incbin "cvaos_us_baserom.gba", 0x5255AC, 0x18

    .global sUnk_085255C4
sUnk_085255C4: @ 0x085255C4
    .incbin "cvaos_us_baserom.gba", 0x5255C4, 0x18

    .global sUnk_085255DC
sUnk_085255DC: @ 0x085255DC
    .incbin "cvaos_us_baserom.gba", 0x5255DC, 0x7C0

    .global sBgPaletteCycles
sBgPaletteCycles: @ 0x08525D9C
    .incbin "cvaos_us_baserom.gba", 0x525D9C, 0x220

    .global sWarpRoomEntries
sWarpRoomEntries: @ 0x08525FBC
    .incbin "cvaos_us_baserom.gba", 0x525FBC, 0x48

    .global sBreakableBlockGraphics
sBreakableBlockGraphics: @ 0x08526004
    .incbin "cvaos_us_baserom.gba", 0x526004, 0x60

    .global sBreakableBlockVariants
sBreakableBlockVariants: @ 0x08526064
    .incbin "cvaos_us_baserom.gba", 0x526064, 0x198

    .global sTriggerZoneTriggerLists
sTriggerZoneTriggerLists: @ 0x085261FC
    .incbin "cvaos_us_baserom.gba", 0x5261FC, 0xC

    .global sTriggerZoneTriggerFuncs
sTriggerZoneTriggerFuncs: @ 0x08526208
    .incbin "cvaos_us_baserom.gba", 0x526208, 0xC

    .global sBreakablePropGraphics
sBreakablePropGraphics: @ 0x08526214
    .incbin "cvaos_us_baserom.gba", 0x526214, 0xA8

    .global sBreakablePropVariants
sBreakablePropVariants: @ 0x085262BC
    .incbin "cvaos_us_baserom.gba", 0x5262BC, 0xB4

    .global sUnk_08526370
sUnk_08526370: @ 0x08526370
    .incbin "cvaos_us_baserom.gba", 0x526370, 0x20

    .global sBossRushRewards
sBossRushRewards: @ 0x08526390
    .incbin "cvaos_us_baserom.gba", 0x526390, 0x18

    .global sRoomDecorationVariants
sRoomDecorationVariants: @ 0x085263A8
    .incbin "cvaos_us_baserom.gba", 0x5263A8, 0x60

    .global sRollerVariantFuncs
sRollerVariantFuncs: @ 0x08526408
    .incbin "cvaos_us_baserom.gba", 0x526408, 0xC

    .global sObject16PaletteAnim
sObject16PaletteAnim: @ 0x08526414
    .incbin "cvaos_us_baserom.gba", 0x526414, 0x34

    .global sObject16EffectAnimScriptVariant0
sObject16EffectAnimScriptVariant0: @ 0x08526448
    .incbin "cvaos_us_baserom.gba", 0x526448, 0x14

    .global sObject16EffectAnimScript
sObject16EffectAnimScript: @ 0x0852645C
    .incbin "cvaos_us_baserom.gba", 0x52645C, 0x10

    .global sObject19Steps
sObject19Steps: @ 0x0852646C
    .incbin "cvaos_us_baserom.gba", 0x52646C, 0x30

    .global sObject22LevelThresholds
sObject22LevelThresholds: @ 0x0852649C
    .incbin "cvaos_us_baserom.gba", 0x52649C, 0x20

    .global sObject22LevelPatterns
sObject22LevelPatterns: @ 0x085264BC
    .incbin "cvaos_us_baserom.gba", 0x5264BC, 0x14

    .global sVerticalPlatformVariants
sVerticalPlatformVariants: @ 0x085264D0
    .incbin "cvaos_us_baserom.gba", 0x5264D0, 0x40

    .global sHorizontalPlatformVariants
sHorizontalPlatformVariants: @ 0x08526510
    .incbin "cvaos_us_baserom.gba", 0x526510, 0x40

    .global sObject25AnimScript0
sObject25AnimScript0: @ 0x08526550
    .incbin "cvaos_us_baserom.gba", 0x526550, 0x14

    .global sObject25AnimScript1
sObject25AnimScript1: @ 0x08526564
    .incbin "cvaos_us_baserom.gba", 0x526564, 0x1C

    .global sObject25AnimScript2
sObject25AnimScript2: @ 0x08526580
    .incbin "cvaos_us_baserom.gba", 0x526580, 0x78

    .global sObject25Points
sObject25Points: @ 0x085265F8
    .incbin "cvaos_us_baserom.gba", 0x5265F8, 0x10

    .global sObject25PointIndices
sObject25PointIndices: @ 0x08526608
    .incbin "cvaos_us_baserom.gba", 0x526608, 0x10

    .global sObject2BPaletteAnim
sObject2BPaletteAnim: @ 0x08526618
    .incbin "cvaos_us_baserom.gba", 0x526618, 0x18

    .global sObject2EAnimScript
sObject2EAnimScript: @ 0x08526630
    .incbin "cvaos_us_baserom.gba", 0x526630, 0x10

    .global sShopBgMaps
sShopBgMaps: @ 0x08526640
    .incbin "cvaos_us_baserom.gba", 0x526640, 0x18

    .global sEventSceneCreateFuncs
sEventSceneCreateFuncs: @ 0x08526658
    .incbin "cvaos_us_baserom.gba", 0x526658, 0x80

    .global sEventSceneUpdateFuncs
sEventSceneUpdateFuncs: @ 0x085266D8
    .incbin "cvaos_us_baserom.gba", 0x5266D8, 0x80

    .global sEventSceneJuliusKeepsPromiseDebrisDefinitions
sEventSceneJuliusKeepsPromiseDebrisDefinitions: @ 0x08526758
    .incbin "cvaos_us_baserom.gba", 0x526758, 0x60

    .global sEventSceneJuliusKeepsPromisePaletteSteps
sEventSceneJuliusKeepsPromisePaletteSteps: @ 0x085267B8
    .incbin "cvaos_us_baserom.gba", 0x5267B8, 0x20

    .global sEventScene03UnusedDebris
sEventScene03UnusedDebris: @ 0x085267D8
    .incbin "cvaos_us_baserom.gba", 0x5267D8, 0x2A

    .global sEventScene03Debris
sEventScene03Debris: @ 0x08526802
    .incbin "cvaos_us_baserom.gba", 0x526802, 0x2A

    .global sUnk_0852682C
sUnk_0852682C: @ 0x0852682C
    .incbin "cvaos_us_baserom.gba", 0x52682C, 0x7C

    .global sEventSceneEndingParticleFrames
sEventSceneEndingParticleFrames: @ 0x085268A8
    .incbin "cvaos_us_baserom.gba", 0x5268A8, 0x10

    .global sEventSceneEndingTimedFragments
sEventSceneEndingTimedFragments: @ 0x085268B8
    .incbin "cvaos_us_baserom.gba", 0x5268B8, 0x6C

    .global sEventSceneEndingPaletteTransitions
sEventSceneEndingPaletteTransitions: @ 0x08526924
    .incbin "cvaos_us_baserom.gba", 0x526924, 0xB8

    .global sConversationTextTable
sConversationTextTable: @ 0x085269DC
    .incbin "cvaos_us_baserom.gba", 0x5269DC, 0x20

    .global sShopStock
sShopStock: @ 0x085269FC
    .incbin "cvaos_us_baserom.gba", 0x5269FC, 0x270

    .global sShopLists
sShopLists: @ 0x08526C6C
    .incbin "cvaos_us_baserom.gba", 0x526C6C, 0xC

    .global sTextWindowGfx
sTextWindowGfx: @ 0x08526C78
    .incbin "cvaos_us_baserom.gba", 0x526C78, 0x6C

    .global sTextWindowScrollOffsets
sTextWindowScrollOffsets: @ 0x08526CE4
    .incbin "cvaos_us_baserom.gba", 0x526CE4, 0x28

    .global sHitEffects
sHitEffects: @ 0x08526D0C
    .incbin "cvaos_us_baserom.gba", 0x526D0C, 0x20

    .global sManticoreWalkAnimScript
sManticoreWalkAnimScript: @ 0x08526D2C
    .incbin "cvaos_us_baserom.gba", 0x526D2C, 0x34

    .global sGiantGhostPaletteAnim
sGiantGhostPaletteAnim: @ 0x08526D60
    .incbin "cvaos_us_baserom.gba", 0x526D60, 0x2E8

    .global sGolemPartPoses
sGolemPartPoses: @ 0x08527048
    .incbin "cvaos_us_baserom.gba", 0x527048, 0x30C

    .global sGolemPartAngles
sGolemPartAngles: @ 0x08527354
    .incbin "cvaos_us_baserom.gba", 0x527354, 0x84

    .global sHeadhunterAnimScript0
sHeadhunterAnimScript0: @ 0x085273D8
    .incbin "cvaos_us_baserom.gba", 0x5273D8, 0x8

    .global sHeadhunterAnimScript1
sHeadhunterAnimScript1: @ 0x085273E0
    .incbin "cvaos_us_baserom.gba", 0x5273E0, 0x10

    .global sHeadhunterAnimScript2
sHeadhunterAnimScript2: @ 0x085273F0
    .incbin "cvaos_us_baserom.gba", 0x5273F0, 0x10

    .global sPeepingEyeMoveSteps
sPeepingEyeMoveSteps: @ 0x08527400
    .incbin "cvaos_us_baserom.gba", 0x527400, 0x50

    .global sPeepingEyeAnimSets
sPeepingEyeAnimSets: @ 0x08527450
    .incbin "cvaos_us_baserom.gba", 0x527450, 0x33C

    .global sGreatArmorPartPoses
sGreatArmorPartPoses: @ 0x0852778C
    .incbin "cvaos_us_baserom.gba", 0x52778C, 0x3F0

    .global sGreatArmorPartAngles
sGreatArmorPartAngles: @ 0x08527B7C
    .incbin "cvaos_us_baserom.gba", 0x527B7C, 0xA0

    .global sDevilPartPoses
sDevilPartPoses: @ 0x08527C1C
    .incbin "cvaos_us_baserom.gba", 0x527C1C, 0xB4

    .global sDevilPartAngles
sDevilPartAngles: @ 0x08527CD0
    .incbin "cvaos_us_baserom.gba", 0x527CD0, 0x308

    .global sCreakingSkullPartPoses
sCreakingSkullPartPoses: @ 0x08527FD8
    .incbin "cvaos_us_baserom.gba", 0x527FD8, 0xD8

    .global sCreakingSkullPartAngles
sCreakingSkullPartAngles: @ 0x085280B0
    .incbin "cvaos_us_baserom.gba", 0x5280B0, 0x74

    .global sBiphronShotAnimScript
sBiphronShotAnimScript: @ 0x08528124
    .incbin "cvaos_us_baserom.gba", 0x528124, 0x24

    .global sStolasPartPoses
sStolasPartPoses: @ 0x08528148
    .incbin "cvaos_us_baserom.gba", 0x528148, 0x60

    .global sStolasPartAngles
sStolasPartAngles: @ 0x085281A8
    .incbin "cvaos_us_baserom.gba", 0x5281A8, 0x20

    .global sLegionAnimScript0
sLegionAnimScript0: @ 0x085281C8
    .incbin "cvaos_us_baserom.gba", 0x5281C8, 0xD0

    .global sLegionAnimScript1
sLegionAnimScript1: @ 0x08528298
    .incbin "cvaos_us_baserom.gba", 0x528298, 0x24

    .global sLegionAnimScript2
sLegionAnimScript2: @ 0x085282BC
    .incbin "cvaos_us_baserom.gba", 0x5282BC, 0x14

    .global sLegionPaletteAnim0
sLegionPaletteAnim0: @ 0x085282D0
    .incbin "cvaos_us_baserom.gba", 0x5282D0, 0x24

    .global sLegionPaletteAnim1
sLegionPaletteAnim1: @ 0x085282F4
    .incbin "cvaos_us_baserom.gba", 0x5282F4, 0x34

    .global sLegionAnimScript3
sLegionAnimScript3: @ 0x08528328
    .incbin "cvaos_us_baserom.gba", 0x528328, 0x8

    .global sLegionDestructibleFragmentAnimScript
sLegionDestructibleFragmentAnimScript: @ 0x08528330
    .incbin "cvaos_us_baserom.gba", 0x528330, 0x10

    .global sUnk_08528340
sUnk_08528340: @ 0x08528340
    .incbin "cvaos_us_baserom.gba", 0x528340, 0x2C

    .global sKyomaDemonHitAnimScript
sKyomaDemonHitAnimScript: @ 0x0852836C
    .incbin "cvaos_us_baserom.gba", 0x52836C, 0x8

    .global sChaos2PaletteSequence0
sChaos2PaletteSequence0: @ 0x08528374
    .incbin "cvaos_us_baserom.gba", 0x528374, 0xD

    .global sChaos2PaletteSequence1
sChaos2PaletteSequence1: @ 0x08528381
    .incbin "cvaos_us_baserom.gba", 0x528381, 0xD

    .global sChaos2PaletteSequence2
sChaos2PaletteSequence2: @ 0x0852838E
    .incbin "cvaos_us_baserom.gba", 0x52838E, 0xD

    .global sChaos2PaletteSequence3
sChaos2PaletteSequence3: @ 0x0852839B
    .incbin "cvaos_us_baserom.gba", 0x52839B, 0x1D

    .global sChaos2SparkAnimScript
sChaos2SparkAnimScript: @ 0x085283B8
    .incbin "cvaos_us_baserom.gba", 0x5283B8, 0x44

    .global sTsuchinokoHitAnimScript
sTsuchinokoHitAnimScript: @ 0x085283FC
    .incbin "cvaos_us_baserom.gba", 0x5283FC, 0x8

    .global sSkyFishFlyAnimScript
sSkyFishFlyAnimScript: @ 0x08528404
    .incbin "cvaos_us_baserom.gba", 0x528404, 0x8

    .global sSkyFishHitAnimScript
sSkyFishHitAnimScript: @ 0x0852840C
    .incbin "cvaos_us_baserom.gba", 0x52840C, 0x40

    .global sBigGolemPartPoses
sBigGolemPartPoses: @ 0x0852844C
    .incbin "cvaos_us_baserom.gba", 0x52844C, 0x140

    .global sBigGolemPartAngles
sBigGolemPartAngles: @ 0x0852858C
    .incbin "cvaos_us_baserom.gba", 0x52858C, 0x78

    .global sUnk_08528604
sUnk_08528604: @ 0x08528604
    .incbin "cvaos_us_baserom.gba", 0x528604, 0x4

    .global sWaiterSkeletonWalkProfile
sWaiterSkeletonWalkProfile: @ 0x08528608
    .incbin "cvaos_us_baserom.gba", 0x528608, 0xC

    .global sUnk_08528614
sUnk_08528614: @ 0x08528614
    .incbin "cvaos_us_baserom.gba", 0x528614, 0xC

    .global sSkullArcherWalkProfile
sSkullArcherWalkProfile: @ 0x08528620
    .incbin "cvaos_us_baserom.gba", 0x528620, 0xA8

    .global sMinotaurAxeFrames
sMinotaurAxeFrames: @ 0x085286C8
    .incbin "cvaos_us_baserom.gba", 0x5286C8, 0x30

    .global sMinotaurWalkProfile
sMinotaurWalkProfile: @ 0x085286F8
    .incbin "cvaos_us_baserom.gba", 0x5286F8, 0x10

    .global sAxeArmorWalkProfile
sAxeArmorWalkProfile: @ 0x08528708
    .incbin "cvaos_us_baserom.gba", 0x528708, 0xC

    .global sDiscArmorWalkProfile
sDiscArmorWalkProfile: @ 0x08528714
    .incbin "cvaos_us_baserom.gba", 0x528714, 0x18

    .global sBomberRockArmorWalkProfile
sBomberRockArmorWalkProfile: @ 0x0852872C
    .incbin "cvaos_us_baserom.gba", 0x52872C, 0x528738 - 0x52872C

    .global sBomberArmorBombWalkProfile
sBomberArmorBombWalkProfile: @ 0x08528738
    .incbin "cvaos_us_baserom.gba", 0x528738, 0x52874C - 0x528738

    .global sBaloreHandSteeringParams0
sBaloreHandSteeringParams0: @ 0x0852874C
    .incbin "cvaos_us_baserom.gba", 0x52874C, 0x18

    .global sBaloreHandSteeringParams1
sBaloreHandSteeringParams1: @ 0x08528764
    .incbin "cvaos_us_baserom.gba", 0x528764, 0x18

    .global sBaloreHandSteeringParams2
sBaloreHandSteeringParams2: @ 0x0852877C
    .incbin "cvaos_us_baserom.gba", 0x52877C, 0x18

    .global sBaloreHandSwipeParams
sBaloreHandSwipeParams: @ 0x08528794
    .incbin "cvaos_us_baserom.gba", 0x528794, 0x18

    .global sBaloreHandSteeringParams3
sBaloreHandSteeringParams3: @ 0x085287AC
    .incbin "cvaos_us_baserom.gba", 0x5287AC, 0x18

    .global sBaloreEyeAnimOffsets
sBaloreEyeAnimOffsets: @ 0x085287C4
    .incbin "cvaos_us_baserom.gba", 0x5287C4, 0x9

    .global sBalorePartAnimOffsets
sBalorePartAnimOffsets: @ 0x085287CD
    .incbin "cvaos_us_baserom.gba", 0x5287CD, 0x9

    .global sUnk_085287D6
sUnk_085287D6: @ 0x085287D6
    .incbin "cvaos_us_baserom.gba", 0x5287D6, 0x6

    .global sBaloreHandBSteeringParams0
sBaloreHandBSteeringParams0: @ 0x085287DC
    .incbin "cvaos_us_baserom.gba", 0x5287DC, 0x18

    .global sBaloreHandBSteeringParams1
sBaloreHandBSteeringParams1: @ 0x085287F4
    .incbin "cvaos_us_baserom.gba", 0x5287F4, 0x18

    .global sBaloreHandBSteeringParams2
sBaloreHandBSteeringParams2: @ 0x0852880C
    .incbin "cvaos_us_baserom.gba", 0x52880C, 0x18

    .global sChronomageWalkProfile
sChronomageWalkProfile: @ 0x08528824
    .incbin "cvaos_us_baserom.gba", 0x528824, 0xC

    .global sDeadWarriorWalkProfile
sDeadWarriorWalkProfile: @ 0x08528830
    .incbin "cvaos_us_baserom.gba", 0x528830, 0xC

    .global sSkullMillioneFamilyVariants
sSkullMillioneFamilyVariants: @ 0x0852883C
    .incbin "cvaos_us_baserom.gba", 0x52883C, 0x3C

    .global sNightmareWalkProfile
sNightmareWalkProfile: @ 0x08528878
    .incbin "cvaos_us_baserom.gba", 0x528878, 0xC

    .global sStudentWitchWalkProfile
sStudentWitchWalkProfile: @ 0x08528884
    .incbin "cvaos_us_baserom.gba", 0x528884, 0xC

    .global sWitchGroundProjectileWalkProfile
sWitchGroundProjectileWalkProfile: @ 0x08528890
    .incbin "cvaos_us_baserom.gba", 0x528890, 0xC

    .global sManEaterHeadOrbitPoints
sManEaterHeadOrbitPoints: @ 0x0852889C
    .incbin "cvaos_us_baserom.gba", 0x52889C, 0xC

    .global sUnk_085288A8
sUnk_085288A8: @ 0x085288A8
    .incbin "cvaos_us_baserom.gba", 0x5288A8, 0xC

    .global sKillerMantleWalkProfile
sKillerMantleWalkProfile: @ 0x085288B4
    .incbin "cvaos_us_baserom.gba", 0x5288B4, 0xC

    .global sCockatriceFamilyWalkProfile
sCockatriceFamilyWalkProfile: @ 0x085288C0
    .incbin "cvaos_us_baserom.gba", 0x5288C0, 0xC

    .global sDeathWeaponArmPoses
sDeathWeaponArmPoses: @ 0x085288CC
    .incbin "cvaos_us_baserom.gba", 0x5288CC, 0x30

    .global sEvilButcherWalkProfile
sEvilButcherWalkProfile: @ 0x085288FC
    .incbin "cvaos_us_baserom.gba", 0x5288FC, 0xC

    .global sKickerSkeletonWalkProfile
sKickerSkeletonWalkProfile: @ 0x08528908
    .incbin "cvaos_us_baserom.gba", 0x528908, 0xC

    .global sSlimeWalkProfile
sSlimeWalkProfile: @ 0x08528914
    .incbin "cvaos_us_baserom.gba", 0x528914, 0xE

    .global sSlimeSurfaceTurnTargets
sSlimeSurfaceTurnTargets: @ 0x08528922
    .incbin "cvaos_us_baserom.gba", 0x528922, 0xA

    .global sSkeletonPartPoints
sSkeletonPartPoints: @ 0x0852892C
    .incbin "cvaos_us_baserom.gba", 0x52892C, 0x1C

    .global sSkeletonWalkProfile
sSkeletonWalkProfile: @ 0x08528948
    .incbin "cvaos_us_baserom.gba", 0x528948, 0xA

    .global sSkeletonDeathPieceOffsets
sSkeletonDeathPieceOffsets: @ 0x08528952
    .incbin "cvaos_us_baserom.gba", 0x528952, 0xAA

    .global sNemesisScriptedAttackSteps
sNemesisScriptedAttackSteps: @ 0x085289FC
    .incbin "cvaos_us_baserom.gba", 0x5289FC, 0x38

    .global sUnk_08528A34
sUnk_08528A34: @ 0x08528A34
    .incbin "cvaos_us_baserom.gba", 0x528A34, 0x90

    .global sKillerDollPartOffsets
sKillerDollPartOffsets: @ 0x08528AC4
    .incbin "cvaos_us_baserom.gba", 0x528AC4, 0x20

    .global sLightningDollPartOffsets
sLightningDollPartOffsets: @ 0x08528AE4
    .incbin "cvaos_us_baserom.gba", 0x528AE4, 0x20

    .global sBeamSkeletonWalkProfile
sBeamSkeletonWalkProfile: @ 0x08528B04
    .incbin "cvaos_us_baserom.gba", 0x528B04, 0x13

    .global sBeamSkeletonDeathPieceOffsets
sBeamSkeletonDeathPieceOffsets: @ 0x08528B17
    .incbin "cvaos_us_baserom.gba", 0x528B17, 0x2D

    .global sNeedlesDeathParts
sNeedlesDeathParts: @ 0x08528B44
    .incbin "cvaos_us_baserom.gba", 0x528B44, 0x20

    .global sBuerWalkProfile
sBuerWalkProfile: @ 0x08528B64
    .incbin "cvaos_us_baserom.gba", 0x528B64, 0xC

    .global sBaelWalkProfile
sBaelWalkProfile: @ 0x08528B70
    .incbin "cvaos_us_baserom.gba", 0x528B70, 0x84

    .global sAlastorAttackSegments
sAlastorAttackSegments: @ 0x08528BF4
    .incbin "cvaos_us_baserom.gba", 0x528BF4, 0x38

    .global sUnk_08528C2C
sUnk_08528C2C: @ 0x08528C2C
    .incbin "cvaos_us_baserom.gba", 0x528C2C, 0x90

    .global sMandragoraAttackBox
sMandragoraAttackBox: @ 0x08528CBC
    .incbin "cvaos_us_baserom.gba", 0x528CBC, 0x4

    .global sMandragoraTerminalPartAttackBox
sMandragoraTerminalPartAttackBox: @ 0x08528CC0
    .incbin "cvaos_us_baserom.gba", 0x528CC0, 0x4

    .global sMandragoraDeathParts
sMandragoraDeathParts: @ 0x08528CC4
    .incbin "cvaos_us_baserom.gba", 0x528CC4, 0x44

    .global sBelmontMoveProfile
sBelmontMoveProfile: @ 0x08528D08
    .incbin "cvaos_us_baserom.gba", 0x528D08, 0xC

    .global sBelmontSceneObjects
sBelmontSceneObjects: @ 0x08528D14
    .incbin "cvaos_us_baserom.gba", 0x528D14, 0x24

    .global sBelmontMeleeSounds
sBelmontMeleeSounds: @ 0x08528D38
    .incbin "cvaos_us_baserom.gba", 0x528D38, 0x4

    .global sFragmentSpecs
sFragmentSpecs: @ 0x08528D3C
    .incbin "cvaos_us_baserom.gba", 0x528D3C, 0x20

    .global sGrahamSceneObjects
sGrahamSceneObjects: @ 0x08528D5C
    .incbin "cvaos_us_baserom.gba", 0x528D5C, 0x60

    .global sRecoveryPickupParticleAnimScript
sRecoveryPickupParticleAnimScript: @ 0x08528DBC
    .incbin "cvaos_us_baserom.gba", 0x528DBC, 0x6391C0 - 0x528DBC

    .global sDebugTestEnemyAnimScripts
sDebugTestEnemyAnimScripts: @ 0x086391C0
    .incbin "cvaos_us_baserom.gba", 0x6391C0, 0x650A1C - 0x6391C0
