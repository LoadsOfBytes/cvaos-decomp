	.include "asm/gba_constants.inc"
    .include "asm/macros.inc"

	.syntax unified

    .section .rodata

    .global sEventScene30PieceEmitterFrames
sEventScene30PieceEmitterFrames: @ 0x08118B48
    .incbin "cvaos_us_baserom.gba", 0x118B48, 0x4

    .global sEventScene30OrbitPoints
sEventScene30OrbitPoints: @ 0x08118B4C
    .incbin "cvaos_us_baserom.gba", 0x118B4C, 0xA0

    .global sEventScene30PieceEmitterAltFrames
sEventScene30PieceEmitterAltFrames: @ 0x08118BEC
    .incbin "cvaos_us_baserom.gba", 0x118BEC, 0x8

    .global sEventScene30CenterEffectOffsets
sEventScene30CenterEffectOffsets: @ 0x08118BF4
    .incbin "cvaos_us_baserom.gba", 0x118BF4, 0x91

    .global sEventScene27TalkTable
sEventScene27TalkTable: @ 0x08118C85
    .incbin "cvaos_us_baserom.gba", 0x118C85, 0x5B

    .global sEntityVisibilityPattern
sEntityVisibilityPattern: @ 0x08118CE0
    .incbin "cvaos_us_baserom.gba", 0x118CE0, 0x28

    .global sOffscreenMargins
sOffscreenMargins: @ 0x08118D08
    .incbin "cvaos_us_baserom.gba", 0x118D08, 0x1C

    .global sAnimViewerInitialFrame
sAnimViewerInitialFrame: @ 0x08118D24
    .incbin "cvaos_us_baserom.gba", 0x118D24, 0x9C

    .global sAssertFileLtInline
sAssertFileLtInline: @ 0x08118DC0
    .incbin "cvaos_us_baserom.gba", 0x118DC0, 0x14

    .global sAssertExprOpdId
sAssertExprOpdId: @ 0x08118DD4
    .incbin "cvaos_us_baserom.gba", 0x118DD4, 0x88

    .global sArachneLegsFrames
sArachneLegsFrames: @ 0x08118E5C
    .incbin "cvaos_us_baserom.gba", 0x118E5C, 0x24

    .global sGolemFrames
sGolemFrames: @ 0x08118E80
    .incbin "cvaos_us_baserom.gba", 0x118E80, 0xB

    .global sGolemFrames65
sGolemFrames65: @ 0x08118E8B
    .incbin "cvaos_us_baserom.gba", 0x118E8B, 0xB

    .global sGolemFrames22
sGolemFrames22: @ 0x08118E96
    .incbin "cvaos_us_baserom.gba", 0x118E96, 0xB

    .global sGolemFrames22Rare
sGolemFrames22Rare: @ 0x08118EA1
    .incbin "cvaos_us_baserom.gba", 0x118EA1, 0xB

    .global sGolemFrames50
sGolemFrames50: @ 0x08118EAC
    .incbin "cvaos_us_baserom.gba", 0x118EAC, 0xB

    .global sGolemAnchors
sGolemAnchors: @ 0x08118EB7
    .incbin "cvaos_us_baserom.gba", 0x118EB7, 0xB

    .global sGolemOrder
sGolemOrder: @ 0x08118EC2
    .incbin "cvaos_us_baserom.gba", 0x118EC2, 0xB

    .global sGolemOrder50
sGolemOrder50: @ 0x08118ECD
    .incbin "cvaos_us_baserom.gba", 0x118ECD, 0xB

    .global sGolemHopTimes
sGolemHopTimes: @ 0x08118ED8
    .incbin "cvaos_us_baserom.gba", 0x118ED8, 0x4

    .global sGolemLandTimes
sGolemLandTimes: @ 0x08118EDC
    .incbin "cvaos_us_baserom.gba", 0x118EDC, 0x4

    .global sGolemChain
sGolemChain: @ 0x08118EE0
    .incbin "cvaos_us_baserom.gba", 0x118EE0, 0xB

    .global sUnk_08118EEB
sUnk_08118EEB: @ 0x08118EEB
    .incbin "cvaos_us_baserom.gba", 0x118EEB, 0x4

    .global sUnk_08118EEF
sUnk_08118EEF: @ 0x08118EEF
    .incbin "cvaos_us_baserom.gba", 0x118EEF, 0x4

    .global sUnk_08118EF3
sUnk_08118EF3: @ 0x08118EF3
    .incbin "cvaos_us_baserom.gba", 0x118EF3, 0x4

    .global sUnk_08118EF7
sUnk_08118EF7: @ 0x08118EF7
    .incbin "cvaos_us_baserom.gba", 0x118EF7, 0x4

    .global sGolemDelay
sGolemDelay: @ 0x08118EFB
    .incbin "cvaos_us_baserom.gba", 0x118EFB, 0x5

    .global sHeadhunterGfx
sHeadhunterGfx: @ 0x08118F00
    .incbin "cvaos_us_baserom.gba", 0x118F00, 0x14

    .global sHeadhunterFrameAnim
sHeadhunterFrameAnim: @ 0x08118F14
    .incbin "cvaos_us_baserom.gba", 0x118F14, 0x8

    .global sHeadhunterAttackTipFrameAnim
sHeadhunterAttackTipFrameAnim: @ 0x08118F1C
    .incbin "cvaos_us_baserom.gba", 0x118F1C, 0x8

    .global sHeadhunterAttackSegmentFrameAnim
sHeadhunterAttackSegmentFrameAnim: @ 0x08118F24
    .incbin "cvaos_us_baserom.gba", 0x118F24, 0x8

    .global sTritonSpearOffsets
sTritonSpearOffsets: @ 0x08118F2C
    .incbin "cvaos_us_baserom.gba", 0x118F2C, 0x20

    .global sPeepingEyeFrameAnim
sPeepingEyeFrameAnim: @ 0x08118F4C
    .incbin "cvaos_us_baserom.gba", 0x118F4C, 0x8

    .global sGorgonFrameAnim
sGorgonFrameAnim: @ 0x08118F54
    .incbin "cvaos_us_baserom.gba", 0x118F54, 0x8

    .global sGorgonWalk
sGorgonWalk: @ 0x08118F5C
    .incbin "cvaos_us_baserom.gba", 0x118F5C, 0x10

    .global sGorgonTurn
sGorgonTurn: @ 0x08118F6C
    .incbin "cvaos_us_baserom.gba", 0x118F6C, 0xC

    .global sUnk_08118F78
sUnk_08118F78: @ 0x08118F78
    .incbin "cvaos_us_baserom.gba", 0x118F78, 0xE

    .global sGreatArmorFrames67
sGreatArmorFrames67: @ 0x08118F86
    .incbin "cvaos_us_baserom.gba", 0x118F86, 0xE

    .global sGreatArmorFrames69
sGreatArmorFrames69: @ 0x08118F94
    .incbin "cvaos_us_baserom.gba", 0x118F94, 0xE

    .global sGreatArmorAnchors
sGreatArmorAnchors: @ 0x08118FA2
    .incbin "cvaos_us_baserom.gba", 0x118FA2, 0xE

    .global sGreatArmorChain
sGreatArmorChain: @ 0x08118FB0
    .incbin "cvaos_us_baserom.gba", 0x118FB0, 0xE

    .global sUnk_08118FBE
sUnk_08118FBE: @ 0x08118FBE
    .incbin "cvaos_us_baserom.gba", 0x118FBE, 0x3

    .global sUnk_08118FC1
sUnk_08118FC1: @ 0x08118FC1
    .incbin "cvaos_us_baserom.gba", 0x118FC1, 0x3

    .global sUnk_08118FC4
sUnk_08118FC4: @ 0x08118FC4
    .incbin "cvaos_us_baserom.gba", 0x118FC4, 0x3

    .global sUnk_08118FC7
sUnk_08118FC7: @ 0x08118FC7
    .incbin "cvaos_us_baserom.gba", 0x118FC7, 0x3

    .global sUnk_08118FCA
sUnk_08118FCA: @ 0x08118FCA
    .incbin "cvaos_us_baserom.gba", 0x118FCA, 0x3

    .global sUnk_08118FCD
sUnk_08118FCD: @ 0x08118FCD
    .incbin "cvaos_us_baserom.gba", 0x118FCD, 0x3

    .global sUnk_08118FD0
sUnk_08118FD0: @ 0x08118FD0
    .incbin "cvaos_us_baserom.gba", 0x118FD0, 0x3

    .global sUnk_08118FD3
sUnk_08118FD3: @ 0x08118FD3
    .incbin "cvaos_us_baserom.gba", 0x118FD3, 0x3

    .global sUnk_08118FD6
sUnk_08118FD6: @ 0x08118FD6
    .incbin "cvaos_us_baserom.gba", 0x118FD6, 0x3

    .global sUnk_08118FD9
sUnk_08118FD9: @ 0x08118FD9
    .incbin "cvaos_us_baserom.gba", 0x118FD9, 0x3

    .global sUnk_08118FDC
sUnk_08118FDC: @ 0x08118FDC
    .incbin "cvaos_us_baserom.gba", 0x118FDC, 0x3

    .global sGreatArmorPartTimers
sGreatArmorPartTimers: @ 0x08118FDF
    .incbin "cvaos_us_baserom.gba", 0x118FDF, 0xF

    .global sGreatArmorPartOffsets
sGreatArmorPartOffsets: @ 0x08118FEE
    .incbin "cvaos_us_baserom.gba", 0x118FEE, 0x1E

    .global sDevilFrames30
sDevilFrames30: @ 0x0811900C
    .incbin "cvaos_us_baserom.gba", 0x11900C, 0x8

    .global sDevilFrames48
sDevilFrames48: @ 0x08119014
    .incbin "cvaos_us_baserom.gba", 0x119014, 0x8

    .global sDevilAnchors
sDevilAnchors: @ 0x0811901C
    .incbin "cvaos_us_baserom.gba", 0x11901C, 0x8

    .global sDevilOrder
sDevilOrder: @ 0x08119024
    .incbin "cvaos_us_baserom.gba", 0x119024, 0x8

    .global sUnk_0811902C
sUnk_0811902C: @ 0x0811902C
    .incbin "cvaos_us_baserom.gba", 0x11902C, 0x8

    .global sDevilPartTimers
sDevilPartTimers: @ 0x08119034
    .incbin "cvaos_us_baserom.gba", 0x119034, 0x8

    .global sCreakingSkullFrames0
sCreakingSkullFrames0: @ 0x0811903C
    .incbin "cvaos_us_baserom.gba", 0x11903C, 0xD

    .global sCreakingSkullAnchors
sCreakingSkullAnchors: @ 0x08119049
    .incbin "cvaos_us_baserom.gba", 0x119049, 0xD

    .global sCreakingSkullOrder
sCreakingSkullOrder: @ 0x08119056
    .incbin "cvaos_us_baserom.gba", 0x119056, 0xD

    .global sUnk_08119063
sUnk_08119063: @ 0x08119063
    .incbin "cvaos_us_baserom.gba", 0x119063, 0xD

    .global sCreakingSkullTimes0
sCreakingSkullTimes0: @ 0x08119070
    .incbin "cvaos_us_baserom.gba", 0x119070, 0x2

    .global sCreakingSkullTimes1
sCreakingSkullTimes1: @ 0x08119072
    .incbin "cvaos_us_baserom.gba", 0x119072, 0x2

    .global sCreakingSkullTimes2
sCreakingSkullTimes2: @ 0x08119074
    .incbin "cvaos_us_baserom.gba", 0x119074, 0x2

    .global sUnk_08119076
sUnk_08119076: @ 0x08119076
    .incbin "cvaos_us_baserom.gba", 0x119076, 0x2

    .global sCreakingSkullSwingTimes
sCreakingSkullSwingTimes: @ 0x08119078
    .incbin "cvaos_us_baserom.gba", 0x119078, 0x3

    .global sCreakingSkullArmTimes
sCreakingSkullArmTimes: @ 0x0811907B
    .incbin "cvaos_us_baserom.gba", 0x11907B, 0x3

    .global sCreakingSkullSpitTimes
sCreakingSkullSpitTimes: @ 0x0811907E
    .incbin "cvaos_us_baserom.gba", 0x11907E, 0x3

    .global sCreakingSkullPartTimers
sCreakingSkullPartTimers: @ 0x08119081
    .incbin "cvaos_us_baserom.gba", 0x119081, 0xF

    .global sMimicAnimation
sMimicAnimation: @ 0x08119090
    .incbin "cvaos_us_baserom.gba", 0x119090, 0x8

    .global sStolasPreviewAnimation
sStolasPreviewAnimation: @ 0x08119098
    .incbin "cvaos_us_baserom.gba", 0x119098, 0x8

    .global sStolasFrames0
sStolasFrames0: @ 0x081190A0
    .incbin "cvaos_us_baserom.gba", 0x1190A0, 0x6

    .global sStolasAnchors
sStolasAnchors: @ 0x081190A6
    .incbin "cvaos_us_baserom.gba", 0x1190A6, 0x6

    .global sUnk_081190AC
sUnk_081190AC: @ 0x081190AC
    .incbin "cvaos_us_baserom.gba", 0x1190AC, 0x6

    .global sUnk_081190B2
sUnk_081190B2: @ 0x081190B2
    .incbin "cvaos_us_baserom.gba", 0x1190B2, 0xE

    .global sLegionGfx
sLegionGfx: @ 0x081190C0
    .incbin "cvaos_us_baserom.gba", 0x1190C0, 0xC

    .global sLegionShellRadii
sLegionShellRadii: @ 0x081190CC
    .incbin "cvaos_us_baserom.gba", 0x1190CC, 0x18

    .global sLegionShotFrames
sLegionShotFrames: @ 0x081190E4
    .incbin "cvaos_us_baserom.gba", 0x1190E4, 0x60

    .global sGladiatorOffsetsA
sGladiatorOffsetsA: @ 0x08119144
    .incbin "cvaos_us_baserom.gba", 0x119144, 0x18

    .global sGladiatorOffsetsB
sGladiatorOffsetsB: @ 0x0811915C
    .incbin "cvaos_us_baserom.gba", 0x11915C, 0x18

    .global sUnk_08119174
sUnk_08119174: @ 0x08119174
    .incbin "cvaos_us_baserom.gba", 0x119174, 0x8

    .global sGladiatorBodyDx
sGladiatorBodyDx: @ 0x0811917C
    .incbin "cvaos_us_baserom.gba", 0x11917C, 0x18

    .global sGladiatorArmDx
sGladiatorArmDx: @ 0x08119194
    .incbin "cvaos_us_baserom.gba", 0x119194, 0x18

    .global sUnk_081191AC
sUnk_081191AC: @ 0x081191AC
    .incbin "cvaos_us_baserom.gba", 0x1191AC, 0x20

    .global sChaosGfx
sChaosGfx: @ 0x081191CC
    .incbin "cvaos_us_baserom.gba", 0x1191CC, 0x14

    .global sChaosMarkFrames
sChaosMarkFrames: @ 0x081191E0
    .incbin "cvaos_us_baserom.gba", 0x1191E0, 0xC

    .global sChaosMarkOffsets
sChaosMarkOffsets: @ 0x081191EC
    .incbin "cvaos_us_baserom.gba", 0x1191EC, 0x38

    .global sChaos2Gfx
sChaos2Gfx: @ 0x08119224
    .incbin "cvaos_us_baserom.gba", 0x119224, 0xC

    .global sChaos2ShotAngleFrames
sChaos2ShotAngleFrames: @ 0x08119230
    .incbin "cvaos_us_baserom.gba", 0x119230, 0x34

    .global sUnk_08119264
sUnk_08119264: @ 0x08119264
    .incbin "cvaos_us_baserom.gba", 0x119264, 0x8

    .global sUnk_0811926C
sUnk_0811926C: @ 0x0811926C
    .incbin "cvaos_us_baserom.gba", 0x11926C, 0x14

    .global sUnk_08119280
sUnk_08119280: @ 0x08119280
    .incbin "cvaos_us_baserom.gba", 0x119280, 0x20

    .global sUnk_081192A0
sUnk_081192A0: @ 0x081192A0
    .incbin "cvaos_us_baserom.gba", 0x1192A0, 0x20

    .global sUnk_081192C0
sUnk_081192C0: @ 0x081192C0
    .incbin "cvaos_us_baserom.gba", 0x1192C0, 0x30

    .global sBigGolemFrames
sBigGolemFrames: @ 0x081192F0
    .incbin "cvaos_us_baserom.gba", 0x1192F0, 0xE

    .global sBigGolemAnchors
sBigGolemAnchors: @ 0x081192FE
    .incbin "cvaos_us_baserom.gba", 0x1192FE, 0xE

    .global sBigGolemOrder
sBigGolemOrder: @ 0x0811930C
    .incbin "cvaos_us_baserom.gba", 0x11930C, 0xE

    .global sBigGolemChain
sBigGolemChain: @ 0x0811931A
    .incbin "cvaos_us_baserom.gba", 0x11931A, 0xE

    .global sBigGolemOrderA
sBigGolemOrderA: @ 0x08119328
    .incbin "cvaos_us_baserom.gba", 0x119328, 0xE

    .global sBigGolemOrderB
sBigGolemOrderB: @ 0x08119336
    .incbin "cvaos_us_baserom.gba", 0x119336, 0xE

    .global sBigGolemOrderC
sBigGolemOrderC: @ 0x08119344
    .incbin "cvaos_us_baserom.gba", 0x119344, 0xE

    .global sBigGolemPartTimers
sBigGolemPartTimers: @ 0x08119352
    .incbin "cvaos_us_baserom.gba", 0x119352, 0xE

    .global sBigGolemPartOffsets
sBigGolemPartOffsets: @ 0x08119360
    .incbin "cvaos_us_baserom.gba", 0x119360, 0x1C

    .global sWaiterSkeletonTrayOffsets
sWaiterSkeletonTrayOffsets: @ 0x0811937C
    .incbin "cvaos_us_baserom.gba", 0x11937C, 0x60

    .global sUnk_081193DC
sUnk_081193DC: @ 0x081193DC
    .incbin "cvaos_us_baserom.gba", 0x1193DC, 0x20

    .global sBaloreGfx
sBaloreGfx: @ 0x081193FC
    .incbin "cvaos_us_baserom.gba", 0x1193FC, 0x10

    .global sUnk_0811940C
sUnk_0811940C: @ 0x0811940C
    .incbin "cvaos_us_baserom.gba", 0x11940C, 0x1C

    .global sBaloreIntroPieceOrbitSpeeds
sBaloreIntroPieceOrbitSpeeds: @ 0x08119428
    .incbin "cvaos_us_baserom.gba", 0x119428, 0x68

    .global sUnk_08119490
sUnk_08119490: @ 0x08119490
    .incbin "cvaos_us_baserom.gba", 0x119490, 0x8

    .global sDeathGfx
sDeathGfx: @ 0x08119498
    .incbin "cvaos_us_baserom.gba", 0x119498, 0x28

    .global sDeathFirstPhaseAnims
sDeathFirstPhaseAnims: @ 0x081194C0
    .incbin "cvaos_us_baserom.gba", 0x1194C0, 0x10

    .global sNemesisTurn
sNemesisTurn: @ 0x081194D0
    .incbin "cvaos_us_baserom.gba", 0x1194D0, 0xC

    .global sBuerPositions
sBuerPositions: @ 0x081194DC
    .incbin "cvaos_us_baserom.gba", 0x1194DC, 0x24

    .global sBaelPositions
sBaelPositions: @ 0x08119500
    .incbin "cvaos_us_baserom.gba", 0x119500, 0x20

    .global sBelmontGfx
sBelmontGfx: @ 0x08119520
    .incbin "cvaos_us_baserom.gba", 0x119520, 0x14

    .global sBelmontWeaponSolidCallbacks
sBelmontWeaponSolidCallbacks: @ 0x08119534
    .incbin "cvaos_us_baserom.gba", 0x119534, 0x18

    .global sGrahamFirstFormGfx
sGrahamFirstFormGfx: @ 0x0811954C
    .incbin "cvaos_us_baserom.gba", 0x11954C, 0xC

    .global sGrahamSpawnTable
sGrahamSpawnTable: @ 0x08119558
    .incbin "cvaos_us_baserom.gba", 0x119558, 0x28

    .global sUnk_08119580
sUnk_08119580: @ 0x08119580
    .incbin "cvaos_us_baserom.gba", 0x119580, 0x40

    .global sUnk_081195C0
sUnk_081195C0: @ 0x081195C0
    .incbin "cvaos_us_baserom.gba", 0x1195C0, 0x20

    .global sUnk_081195E0
sUnk_081195E0: @ 0x081195E0
    .incbin "cvaos_us_baserom.gba", 0x1195E0, 0x20

    .global sGrahamAttackerPositions
sGrahamAttackerPositions: @ 0x08119600
    .incbin "cvaos_us_baserom.gba", 0x119600, 0x80

    .global sGrahamPartSteps
sGrahamPartSteps: @ 0x08119680
    .incbin "cvaos_us_baserom.gba", 0x119680, 0x10

    .global sBossDefeatBurstOffsets
sBossDefeatBurstOffsets: @ 0x08119690
    .incbin "cvaos_us_baserom.gba", 0x119690, 0xF0

    .global sGrahamBreakupFragmentPoints
sGrahamBreakupFragmentPoints: @ 0x08119780
    .incbin "cvaos_us_baserom.gba", 0x119780, 0x110

    .global sEventScene30TransformationGfx
sEventScene30TransformationGfx: @ 0x08119890
    .incbin "cvaos_us_baserom.gba", 0x119890, 0x8

    .global sUnk_08119898
sUnk_08119898: @ 0x08119898
    .incbin "cvaos_us_baserom.gba", 0x119898, 0x57C

    .global sWarpRoomPalette
sWarpRoomPalette: @ 0x08119E14
    .incbin "cvaos_us_baserom.gba", 0x119E14, 0x1C8

    .global sEventScene30TransformationPalette
sEventScene30TransformationPalette: @ 0x08119FDC
    .incbin "cvaos_us_baserom.gba", 0x119FDC, 0x2A88

    .global sObject1BBgPalette
sObject1BBgPalette: @ 0x0811CA64
    .incbin "cvaos_us_baserom.gba", 0x11CA64, 0x41378

    .global sEventScene30TransformationBg2Map
sEventScene30TransformationBg2Map: @ 0x0815DDDC
    .incbin "cvaos_us_baserom.gba", 0x15DDDC, 0x90

    .global sEventScene30BgMap
sEventScene30BgMap: @ 0x0815DE6C
    .incbin "cvaos_us_baserom.gba", 0x15DE6C, 0x218

    .global sShopBgGfx
sShopBgGfx: @ 0x0815E084
    .incbin "cvaos_us_baserom.gba", 0x15E084, 0x8

    .global sShopBgGfxLanguage3
sShopBgGfxLanguage3: @ 0x0815E08C
    .incbin "cvaos_us_baserom.gba", 0x15E08C, 0x8

    .global sShopBgGfxLanguage3Extra
sShopBgGfxLanguage3Extra: @ 0x0815E094
    .incbin "cvaos_us_baserom.gba", 0x15E094, 0x8

    .global sShopBgGfxLanguage2
sShopBgGfxLanguage2: @ 0x0815E09C
    .incbin "cvaos_us_baserom.gba", 0x15E09C, 0x350

    .global sShopBgPalette
sShopBgPalette: @ 0x0815E3EC
    .incbin "cvaos_us_baserom.gba", 0x15E3EC, 0x5E4

    .global sBestiaryDetailBgMap
sBestiaryDetailBgMap: @ 0x0815E9D0
    .incbin "cvaos_us_baserom.gba", 0x15E9D0, 0x1AB0

    .global sTitleScreenGfx
sTitleScreenGfx: @ 0x08160480
    .incbin "cvaos_us_baserom.gba", 0x160480, 0x8

    .global sTitleFlashGfx
sTitleFlashGfx: @ 0x08160488
    .incbin "cvaos_us_baserom.gba", 0x160488, 0x8

    .incbin "cvaos_us_baserom.gba", 0x160490, 0x10

    .global sObject10Gfx
sObject10Gfx: @ 0x081604A0
    .incbin "cvaos_us_baserom.gba", 0x1604A0, 0x61154

    .global sCommonObjectGfx
sCommonObjectGfx: @ 0x081C15F4
    .incbin "cvaos_us_baserom.gba", 0x1C15F4, 0x4

    .global sCommonObjectGfxTiles
sCommonObjectGfxTiles: @ 0x081C15F8
    .incbin "cvaos_us_baserom.gba", 0x1C15F8, 0x2004

    .global sDefeatEffectTiles
sDefeatEffectTiles: @ 0x081C35FC
    .incbin "cvaos_us_baserom.gba", 0x1C35FC, 0x804

    .global sPickupIconTiles
sPickupIconTiles: @ 0x081C3E00
    .incbin "cvaos_us_baserom.gba", 0x1C3E00, 0x800C

    .global sGateGfx
sGateGfx: @ 0x081CBE0C
    .incbin "cvaos_us_baserom.gba", 0x1CBE0C, 0x50

    .global sObject07ParallaxSpriteGfx
sObject07ParallaxSpriteGfx: @ 0x081CBE5C
    .incbin "cvaos_us_baserom.gba", 0x1CBE5C, 0x18

    .global sObject0DGfx
sObject0DGfx: @ 0x081CBE74
    .incbin "cvaos_us_baserom.gba", 0x1CBE74, 0x8

    .global sObject13FamilyGfx
sObject13FamilyGfx: @ 0x081CBE7C
    .incbin "cvaos_us_baserom.gba", 0x1CBE7C, 0x8

    .global sObject1BGfx
sObject1BGfx: @ 0x081CBE84
    .incbin "cvaos_us_baserom.gba", 0x1CBE84, 0x8

    .global sObject22Gfx
sObject22Gfx: @ 0x081CBE8C
    .incbin "cvaos_us_baserom.gba", 0x1CBE8C, 0x8

    .global sObject21Object23Gfx
sObject21Object23Gfx: @ 0x081CBE94
    .incbin "cvaos_us_baserom.gba", 0x1CBE94, 0x8

    .global sObject17Gfx
sObject17Gfx: @ 0x081CBE9C
    .incbin "cvaos_us_baserom.gba", 0x1CBE9C, 0x8

    .global sObject14Gfx
sObject14Gfx: @ 0x081CBEA4
    .incbin "cvaos_us_baserom.gba", 0x1CBEA4, 0x18

    .global sObject2DGfx
sObject2DGfx: @ 0x081CBEBC
    .incbin "cvaos_us_baserom.gba", 0x1CBEBC, 0x10

    .global sObject2EGfx
sObject2EGfx: @ 0x081CBECC
    .incbin "cvaos_us_baserom.gba", 0x1CBECC, 0x8

    .global sObject2CGfx
sObject2CGfx: @ 0x081CBED4
    .incbin "cvaos_us_baserom.gba", 0x1CBED4, 0x8

    .global sObject2BGfx
sObject2BGfx: @ 0x081CBEDC
    .incbin "cvaos_us_baserom.gba", 0x1CBEDC, 0x8

    .global sObject15Gfx
sObject15Gfx: @ 0x081CBEE4
    .incbin "cvaos_us_baserom.gba", 0x1CBEE4, 0x8

    .global sObject2FFamilyGfx
sObject2FFamilyGfx: @ 0x081CBEEC
    .incbin "cvaos_us_baserom.gba", 0x1CBEEC, 0x10

    .global sObject33Gfx
sObject33Gfx: @ 0x081CBEFC
    .incbin "cvaos_us_baserom.gba", 0x1CBEFC, 0x10

    .global sObject32Gfx
sObject32Gfx: @ 0x081CBF0C
    .incbin "cvaos_us_baserom.gba", 0x1CBF0C, 0x8

    .global sObject34Gfx
sObject34Gfx: @ 0x081CBF14
    .incbin "cvaos_us_baserom.gba", 0x1CBF14, 0x8

    .global sObject35Gfx
sObject35Gfx: @ 0x081CBF1C
    .incbin "cvaos_us_baserom.gba", 0x1CBF1C, 0x10

    .global sValkyrieGfx
sValkyrieGfx: @ 0x081CBF2C
    .incbin "cvaos_us_baserom.gba", 0x1CBF2C, 0x8

    .global sWerewolfGfx
sWerewolfGfx: @ 0x081CBF34
    .incbin "cvaos_us_baserom.gba", 0x1CBF34, 0x8

    .global sKillerFishGfx
sKillerFishGfx: @ 0x081CBF3C
    .incbin "cvaos_us_baserom.gba", 0x1CBF3C, 0x8

    .global sMermanGfx
sMermanGfx: @ 0x081CBF44
    .incbin "cvaos_us_baserom.gba", 0x1CBF44, 0x8

    .global sMudmanGfx
sMudmanGfx: @ 0x081CBF4C
    .incbin "cvaos_us_baserom.gba", 0x1CBF4C, 0x8

    .global sManticoreGfx
sManticoreGfx: @ 0x081CBF54
    .incbin "cvaos_us_baserom.gba", 0x1CBF54, 0x8

    .global sUneGfx
sUneGfx: @ 0x081CBF5C
    .incbin "cvaos_us_baserom.gba", 0x1CBF5C, 0x8

    .global sWingedSkeletonGfx
sWingedSkeletonGfx: @ 0x081CBF64
    .incbin "cvaos_us_baserom.gba", 0x1CBF64, 0x8

    .global sHarpyGfx
sHarpyGfx: @ 0x081CBF6C
    .incbin "cvaos_us_baserom.gba", 0x1CBF6C, 0x8

    .global sQuezlcoatlWhiteDragonGfx
sQuezlcoatlWhiteDragonGfx: @ 0x081CBF74
    .incbin "cvaos_us_baserom.gba", 0x1CBF74, 0x8

    .global sWerejaguarGfx
sWerejaguarGfx: @ 0x081CBF7C
    .incbin "cvaos_us_baserom.gba", 0x1CBF7C, 0x8

    .global sArachneGfx
sArachneGfx: @ 0x081CBF84
    .incbin "cvaos_us_baserom.gba", 0x1CBF84, 0x8

    .global sZombieGfx
sZombieGfx: @ 0x081CBF8C
    .incbin "cvaos_us_baserom.gba", 0x1CBF8C, 0x8

    .global sPersephoneGfx
sPersephoneGfx: @ 0x081CBF94
    .incbin "cvaos_us_baserom.gba", 0x1CBF94, 0x8

    .global sAluraUneGfx
sAluraUneGfx: @ 0x081CBF9C
    .incbin "cvaos_us_baserom.gba", 0x1CBF9C, 0x8

    .global sGiantGhostGfx
sGiantGhostGfx: @ 0x081CBFA4
    .incbin "cvaos_us_baserom.gba", 0x1CBFA4, 0x8

    .global sGolemGfx
sGolemGfx: @ 0x081CBFAC
    .incbin "cvaos_us_baserom.gba", 0x1CBFAC, 0x8

    .global sGolemGfxEnemy50
sGolemGfxEnemy50: @ 0x081CBFB4
    .incbin "cvaos_us_baserom.gba", 0x1CBFB4, 0x28

    .global sGargoyleGfx
sGargoyleGfx: @ 0x081CBFDC
    .incbin "cvaos_us_baserom.gba", 0x1CBFDC, 0x8

    .global sTritonGfx
sTritonGfx: @ 0x081CBFE4
    .incbin "cvaos_us_baserom.gba", 0x1CBFE4, 0x8

    .global sPeepingEyeGfx
sPeepingEyeGfx: @ 0x081CBFEC
    .incbin "cvaos_us_baserom.gba", 0x1CBFEC, 0x8

    .global sGorgonGfx
sGorgonGfx: @ 0x081CBFF4
    .incbin "cvaos_us_baserom.gba", 0x1CBFF4, 0x8

    .global sGhostGfx
sGhostGfx: @ 0x081CBFFC
    .incbin "cvaos_us_baserom.gba", 0x1CBFFC, 0x8

    .global sGreatArmorGfx
sGreatArmorGfx: @ 0x081CC004
    .incbin "cvaos_us_baserom.gba", 0x1CC004, 0x8

    .global sDeadCrusaderSkeletonKnightGfx
sDeadCrusaderSkeletonKnightGfx: @ 0x081CC00C
    .global sDeadCrusaderSkeletonKnightGfx
sDeadCrusaderSkeletonKnightGfx: @ 0x081CC00C
    .incbin "cvaos_us_baserom.gba", 0x1CC00C, 0x8

    .global sDevilGfx
sDevilGfx: @ 0x081CC014
    .incbin "cvaos_us_baserom.gba", 0x1CC014, 0x8

    .global sZombieOfficerGfx
sZombieOfficerGfx: @ 0x081CC01C
    .incbin "cvaos_us_baserom.gba", 0x1CC01C, 0x8

    .global sZombieSoldierGfx
sZombieSoldierGfx: @ 0x081CC024
    .incbin "cvaos_us_baserom.gba", 0x1CC024, 0x8

    .global sCurlyGfx
sCurlyGfx: @ 0x081CC02C
    .incbin "cvaos_us_baserom.gba", 0x1CC02C, 0x8

    .global sMedusaHeadGfx
sMedusaHeadGfx: @ 0x081CC034
    .incbin "cvaos_us_baserom.gba", 0x1CC034, 0x8

    .global sLilithSuccubusGfx
sLilithSuccubusGfx: @ 0x081CC03C
    .global sLilithSuccubusGfx
sLilithSuccubusGfx: @ 0x081CC03C
    .incbin "cvaos_us_baserom.gba", 0x1CC03C, 0x8

    .global sCreakingSkullGfx
sCreakingSkullGfx: @ 0x081CC044
    .incbin "cvaos_us_baserom.gba", 0x1CC044, 0x8

    .global sMimicGfx
sMimicGfx: @ 0x081CC04C
    .incbin "cvaos_us_baserom.gba", 0x1CC04C, 0x8

    .global sEctoplasmGfx
sEctoplasmGfx: @ 0x081CC054
    .incbin "cvaos_us_baserom.gba", 0x1CC054, 0x8

    .global sBiphronGfx
sBiphronGfx: @ 0x081CC05C
    .incbin "cvaos_us_baserom.gba", 0x1CC05C, 0x8

    .global sStolasGfx
sStolasGfx: @ 0x081CC064
    .incbin "cvaos_us_baserom.gba", 0x1CC064, 0x18

    .global sObject25Gfx
sObject25Gfx: @ 0x081CC07C
    .incbin "cvaos_us_baserom.gba", 0x1CC07C, 0x28

    .global sKyomaDemonGfx
sKyomaDemonGfx: @ 0x081CC0A4
    .incbin "cvaos_us_baserom.gba", 0x1CC0A4, 0x8

    .global sGladiatorGfx
sGladiatorGfx: @ 0x081CC0AC
    .incbin "cvaos_us_baserom.gba", 0x1CC0AC, 0x8

    .global sTsuchinokoGfx
sTsuchinokoGfx: @ 0x081CC0B4
    .incbin "cvaos_us_baserom.gba", 0x1CC0B4, 0x8

    .global sSkyFishGfx
sSkyFishGfx: @ 0x081CC0BC
    .incbin "cvaos_us_baserom.gba", 0x1CC0BC, 0x10088

    .global sSkillCagnazzoGfx
sSkillCagnazzoGfx: @ 0x081DC144
    .incbin "cvaos_us_baserom.gba", 0x1DC144, 0x1008

    .global sSkillDevilManticoreSparkGfx
sSkillDevilManticoreSparkGfx: @ 0x081DD14C
    .incbin "cvaos_us_baserom.gba", 0x1DD14C, 0x2814

    .global sSkillFinalGuardGfx
sSkillFinalGuardGfx: @ 0x081DF960
    .incbin "cvaos_us_baserom.gba", 0x1DF960, 0x804

    .global sSkillBuerOrbitGfx
sSkillBuerOrbitGfx: @ 0x081E0164
    .incbin "cvaos_us_baserom.gba", 0x1E0164, 0x1008

    .global sUnk_081E116C
sUnk_081E116C: @ 0x081E116C
    .incbin "cvaos_us_baserom.gba", 0x1E116C, 0x1008

    .global sSkillDeathGfx
sSkillDeathGfx: @ 0x081E2174
    .incbin "cvaos_us_baserom.gba", 0x1E2174, 0x2010

    .global sWormGfx
sWormGfx: @ 0x081E4184
    .incbin "cvaos_us_baserom.gba", 0x1E4184, 0x8

    .global sDryadGfx
sDryadGfx: @ 0x081E418C
    .incbin "cvaos_us_baserom.gba", 0x1E418C, 0x8

    .global sWaiterSkeletonGfx
sWaiterSkeletonGfx: @ 0x081E4194
    .incbin "cvaos_us_baserom.gba", 0x1E4194, 0x8

    .global sSkullArcherGfx
sSkullArcherGfx: @ 0x081E419C
    .incbin "cvaos_us_baserom.gba", 0x1E419C, 0x8

    .global sMinotaurGfx
sMinotaurGfx: @ 0x081E41A4
    .incbin "cvaos_us_baserom.gba", 0x1E41A4, 0x8

    .global sAxeArmorGfx
sAxeArmorGfx: @ 0x081E41AC
    .incbin "cvaos_us_baserom.gba", 0x1E41AC, 0x8

    .global sDiscArmorGfx
sDiscArmorGfx: @ 0x081E41B4
    .incbin "cvaos_us_baserom.gba", 0x1E41B4, 0x8

    .global sBomberArmorRockArmorGfx
sBomberArmorRockArmorGfx: @ 0x081E41BC
    .incbin "cvaos_us_baserom.gba", 0x1E41BC, 0x8

    .global sWitchGfx
sWitchGfx: @ 0x081E41C4
    .incbin "cvaos_us_baserom.gba", 0x1E41C4, 0x8

    .global sCockatriceFamilyGfx
sCockatriceFamilyGfx: @ 0x081E41CC
    .incbin "cvaos_us_baserom.gba", 0x1E41CC, 0x8020

    .global sChronomageGfx
sChronomageGfx: @ 0x081EC1EC
    .incbin "cvaos_us_baserom.gba", 0x1EC1EC, 0x8

    .global sDeadWarriorGfx
sDeadWarriorGfx: @ 0x081EC1F4
    .incbin "cvaos_us_baserom.gba", 0x1EC1F4, 0x8

    .global sNightmareGfx
sNightmareGfx: @ 0x081EC1FC
    .incbin "cvaos_us_baserom.gba", 0x1EC1FC, 0x8018

    .global sSkullMillioneFamilyGfx
sSkullMillioneFamilyGfx: @ 0x081F4214
    .incbin "cvaos_us_baserom.gba", 0x1F4214, 0x8

    .global sKillerMantleGfx
sKillerMantleGfx: @ 0x081F421C
    .incbin "cvaos_us_baserom.gba", 0x1F421C, 0x8

    .global sManEaterGfx
sManEaterGfx: @ 0x081F4224
    .incbin "cvaos_us_baserom.gba", 0x1F4224, 0x8

    .global sBatGfx
sBatGfx: @ 0x081F422C
    .incbin "cvaos_us_baserom.gba", 0x1F422C, 0x8

    .global sAltairGfx
sAltairGfx: @ 0x081F4234
    .incbin "cvaos_us_baserom.gba", 0x1F4234, 0x8

    .global sFleamanRipperGfx
sFleamanRipperGfx: @ 0x081F423C
    .incbin "cvaos_us_baserom.gba", 0x1F423C, 0x8

    .global sGhostDancerGfx
sGhostDancerGfx: @ 0x081F4244
    .incbin "cvaos_us_baserom.gba", 0x1F4244, 0x8

    .global sSkeletonGfx
sSkeletonGfx: @ 0x081F424C
    .incbin "cvaos_us_baserom.gba", 0x1F424C, 0x8

    .global sImpTinyDevilGfx
sImpTinyDevilGfx: @ 0x081F4254
    .incbin "cvaos_us_baserom.gba", 0x1F4254, 0x8

    .global sUkobackGfx
sUkobackGfx: @ 0x081F425C
    .incbin "cvaos_us_baserom.gba", 0x1F425C, 0x8

    .global sNemesisGfx
sNemesisGfx: @ 0x081F4264
    .incbin "cvaos_us_baserom.gba", 0x1F4264, 0x8

    .global sKillerDollLightningDollGfx
sKillerDollLightningDollGfx: @ 0x081F426C
    .incbin "cvaos_us_baserom.gba", 0x1F426C, 0x8

    .global sBlueCrowRedCrowGfx
sBlueCrowRedCrowGfx: @ 0x081F4274
    .incbin "cvaos_us_baserom.gba", 0x1F4274, 0x8

    .global sBeamSkeletonGfx
sBeamSkeletonGfx: @ 0x081F427C
    .incbin "cvaos_us_baserom.gba", 0x1F427C, 0x8

    .global sNeedlesGfx
sNeedlesGfx: @ 0x081F4284
    .incbin "cvaos_us_baserom.gba", 0x1F4284, 0x8

    .global sBaelBuerGfx
sBaelBuerGfx: @ 0x081F428C
    .incbin "cvaos_us_baserom.gba", 0x1F428C, 0x8

    .global sAlastorGfx
sAlastorGfx: @ 0x081F4294
    .incbin "cvaos_us_baserom.gba", 0x1F4294, 0x8

    .global sMandragoraGfx
sMandragoraGfx: @ 0x081F429C
    .incbin "cvaos_us_baserom.gba", 0x1F429C, 0x4018

    .global sBelmontWeaponGfx
sBelmontWeaponGfx: @ 0x081F82B4
    .incbin "cvaos_us_baserom.gba", 0x1F82B4, 0x2004

    .global sJuliusGfx
sJuliusGfx: @ 0x081FA2B8
    .incbin "cvaos_us_baserom.gba", 0x1FA2B8, 0x1004

    .global sGrahamGfx1
sGrahamGfx1: @ 0x081FB2BC
    .incbin "cvaos_us_baserom.gba", 0x1FB2BC, 0x2004

    .global sGrahamGfx2
sGrahamGfx2: @ 0x081FD2C0
    .incbin "cvaos_us_baserom.gba", 0x1FD2C0, 0x2004

    .global sGrahamGfx3
sGrahamGfx3: @ 0x081FF2C4
    .incbin "cvaos_us_baserom.gba", 0x1FF2C4, 0x600C

    .global sEvilButcherGfx
sEvilButcherGfx: @ 0x082052D0
    .incbin "cvaos_us_baserom.gba", 0x2052D0, 0x8

    .global sMenuCursorGfx
sMenuCursorGfx: @ 0x082052D8
    .incbin "cvaos_us_baserom.gba", 0x2052D8, 0x10

    .global sSaveWarpRoomGfx
sSaveWarpRoomGfx: @ 0x082052E8
    .incbin "cvaos_us_baserom.gba", 0x2052E8, 0x8

    .global sWarpRoomGfx
sWarpRoomGfx: @ 0x082052F0
    .incbin "cvaos_us_baserom.gba", 0x2052F0, 0x44C0

    .global sObject10Palette
sObject10Palette: @ 0x082097B0
    .incbin "cvaos_us_baserom.gba", 0x2097B0, 0x24

    .global sSomaPalette
sSomaPalette: @ 0x082097D4
    .incbin "cvaos_us_baserom.gba", 0x2097D4, 0xE4

    .global sWeaponPalettes
sWeaponPalettes: @ 0x082098B8
    .incbin "cvaos_us_baserom.gba", 0x2098B8, 0x144

    .global sSavePointPalette
sSavePointPalette: @ 0x082099FC
    .incbin "cvaos_us_baserom.gba", 0x2099FC, 0xE4

    .global sGatePalette
sGatePalette: @ 0x08209AE0
    .incbin "cvaos_us_baserom.gba", 0x209AE0, 0x408

    .global sObject07ParallaxSpritePalette
sObject07ParallaxSpritePalette: @ 0x08209EE8
    .incbin "cvaos_us_baserom.gba", 0x209EE8, 0x6C

    .global sObject0DPalette
sObject0DPalette: @ 0x08209F54
    .incbin "cvaos_us_baserom.gba", 0x209F54, 0x84

    .global sUnk_08209FD8
sUnk_08209FD8: @ 0x08209FD8
    .incbin "cvaos_us_baserom.gba", 0x209FD8, 0x104

    .global sObject14Palette
sObject14Palette: @ 0x0820A0DC
    .incbin "cvaos_us_baserom.gba", 0x20A0DC, 0x24

    .global sObject2DObject2EPalette
sObject2DObject2EPalette: @ 0x0820A100
    .incbin "cvaos_us_baserom.gba", 0x20A100, 0xC4

    .global sUnk_0820A1C4
sUnk_0820A1C4: @ 0x0820A1C4
    .incbin "cvaos_us_baserom.gba", 0x20A1C4, 0x168

    .global sObject33Palette
sObject33Palette: @ 0x0820A32C
    .incbin "cvaos_us_baserom.gba", 0x20A32C, 0x24

    .global sObject32Object34Palette
sObject32Object34Palette: @ 0x0820A350
    .incbin "cvaos_us_baserom.gba", 0x20A350, 0x64

    .global sObject35Palette
sObject35Palette: @ 0x0820A3B4
    .incbin "cvaos_us_baserom.gba", 0x20A3B4, 0x48

    .global sValkyriePalette
sValkyriePalette: @ 0x0820A3FC
    .incbin "cvaos_us_baserom.gba", 0x20A3FC, 0x44

    .global sWerewolfPalette
sWerewolfPalette: @ 0x0820A440
    .incbin "cvaos_us_baserom.gba", 0x20A440, 0x24

    .global sKillerFishPalette
sKillerFishPalette: @ 0x0820A464
    .incbin "cvaos_us_baserom.gba", 0x20A464, 0x24

    .global sMermanPalette
sMermanPalette: @ 0x0820A488
    .incbin "cvaos_us_baserom.gba", 0x20A488, 0x24

    .global sMudmanPalette
sMudmanPalette: @ 0x0820A4AC
    .incbin "cvaos_us_baserom.gba", 0x20A4AC, 0x24

    .global sManticorePalette
sManticorePalette: @ 0x0820A4D0
    .incbin "cvaos_us_baserom.gba", 0x20A4D0, 0x24

    .global sUnePalette
sUnePalette: @ 0x0820A4F4
    .incbin "cvaos_us_baserom.gba", 0x20A4F4, 0x24

    .global sWingedSkeletonPalette
sWingedSkeletonPalette: @ 0x0820A518
    .incbin "cvaos_us_baserom.gba", 0x20A518, 0x24

    .global sHarpyPalette
sHarpyPalette: @ 0x0820A53C
    .incbin "cvaos_us_baserom.gba", 0x20A53C, 0x44

    .global sQuezlcoatlWhiteDragonPalette
sQuezlcoatlWhiteDragonPalette: @ 0x0820A580
    .incbin "cvaos_us_baserom.gba", 0x20A580, 0x44

    .global sWerejaguarPalette
sWerejaguarPalette: @ 0x0820A5C4
    .incbin "cvaos_us_baserom.gba", 0x20A5C4, 0x44

    .global sArachnePalette
sArachnePalette: @ 0x0820A608
    .incbin "cvaos_us_baserom.gba", 0x20A608, 0x24

    .global sZombiePalette
sZombiePalette: @ 0x0820A62C
    .incbin "cvaos_us_baserom.gba", 0x20A62C, 0x24

    .global sPersephonePalette
sPersephonePalette: @ 0x0820A650
    .incbin "cvaos_us_baserom.gba", 0x20A650, 0x24

    .global sAluraUnePalette
sAluraUnePalette: @ 0x0820A674
    .incbin "cvaos_us_baserom.gba", 0x20A674, 0x24

    .global sGiantGhostPalette
sGiantGhostPalette: @ 0x0820A698
    .incbin "cvaos_us_baserom.gba", 0x20A698, 0x64

    .global sGolemPalette
sGolemPalette: @ 0x0820A6FC
    .incbin "cvaos_us_baserom.gba", 0x20A6FC, 0x84

    .global sHeadhunterPalette
sHeadhunterPalette: @ 0x0820A780
    .incbin "cvaos_us_baserom.gba", 0x20A780, 0x84

    .global sGargoylePalette
sGargoylePalette: @ 0x0820A804
    .incbin "cvaos_us_baserom.gba", 0x20A804, 0x44

    .global sTritonPalette
sTritonPalette: @ 0x0820A848
    .incbin "cvaos_us_baserom.gba", 0x20A848, 0x24

    .global sPeepingEyePalette
sPeepingEyePalette: @ 0x0820A86C
    .incbin "cvaos_us_baserom.gba", 0x20A86C, 0x24

    .global sGorgonPalette
sGorgonPalette: @ 0x0820A890
    .incbin "cvaos_us_baserom.gba", 0x20A890, 0x44

    .global sGhostPalette
sGhostPalette: @ 0x0820A8D4
    .incbin "cvaos_us_baserom.gba", 0x20A8D4, 0x24

    .global sGreatArmorPalette
sGreatArmorPalette: @ 0x0820A8F8
    .incbin "cvaos_us_baserom.gba", 0x20A8F8, 0x64

    .global sDeadCrusaderSkeletonKnightPalette
sDeadCrusaderSkeletonKnightPalette: @ 0x0820A95C
    .global sDeadCrusaderSkeletonKnightPalette
sDeadCrusaderSkeletonKnightPalette: @ 0x0820A95C
    .incbin "cvaos_us_baserom.gba", 0x20A95C, 0x44

    .global sDevilPalette
sDevilPalette: @ 0x0820A9A0
    .incbin "cvaos_us_baserom.gba", 0x20A9A0, 0x84

    .global sZombieOfficerPalette
sZombieOfficerPalette: @ 0x0820AA24
    .incbin "cvaos_us_baserom.gba", 0x20AA24, 0x24

    .global sZombieSoldierPalette
sZombieSoldierPalette: @ 0x0820AA48
    .incbin "cvaos_us_baserom.gba", 0x20AA48, 0x24

    .global sCurlyPalette
sCurlyPalette: @ 0x0820AA6C
    .incbin "cvaos_us_baserom.gba", 0x20AA6C, 0x44

    .global sMedusaHeadPalette
sMedusaHeadPalette: @ 0x0820AAB0
    .incbin "cvaos_us_baserom.gba", 0x20AAB0, 0x44

    .global sLilithSuccubusPalette
sLilithSuccubusPalette: @ 0x0820AAF4
    .global sLilithSuccubusPalette
sLilithSuccubusPalette: @ 0x0820AAF4
    .incbin "cvaos_us_baserom.gba", 0x20AAF4, 0x44

    .global sCreakingSkullPalette
sCreakingSkullPalette: @ 0x0820AB38
    .incbin "cvaos_us_baserom.gba", 0x20AB38, 0x44

    .global sMimicPalette
sMimicPalette: @ 0x0820AB7C
    .incbin "cvaos_us_baserom.gba", 0x20AB7C, 0x64

    .global sEctoplasmPalette
sEctoplasmPalette: @ 0x0820ABE0
    .incbin "cvaos_us_baserom.gba", 0x20ABE0, 0x24

    .global sBiphronPalette
sBiphronPalette: @ 0x0820AC04
    .incbin "cvaos_us_baserom.gba", 0x20AC04, 0x24

    .global sStolasPalette
sStolasPalette: @ 0x0820AC28
    .incbin "cvaos_us_baserom.gba", 0x20AC28, 0x24

    .global sLegionObject25Palette
sLegionObject25Palette: @ 0x0820AC4C
    .incbin "cvaos_us_baserom.gba", 0x20AC4C, 0x124

    .global sChaosPalette
sChaosPalette: @ 0x0820AD70
    .incbin "cvaos_us_baserom.gba", 0x20AD70, 0x204

    .global sKyomaDemonPalette
sKyomaDemonPalette: @ 0x0820AF74
    .incbin "cvaos_us_baserom.gba", 0x20AF74, 0x24

    .global sGladiatorPalette
sGladiatorPalette: @ 0x0820AF98
    .incbin "cvaos_us_baserom.gba", 0x20AF98, 0x44

    .global sTsuchinokoPalette
sTsuchinokoPalette: @ 0x0820AFDC
    .incbin "cvaos_us_baserom.gba", 0x20AFDC, 0x24

    .global sSkyFishPalette
sSkyFishPalette: @ 0x0820B000
    .incbin "cvaos_us_baserom.gba", 0x20B000, 0x188

    .global sSkillLightningDollPalette
sSkillLightningDollPalette: @ 0x0820B188
    .incbin "cvaos_us_baserom.gba", 0x20B188, 0x35C

    .global sSkillCagnazzoPalette
sSkillCagnazzoPalette: @ 0x0820B4E4
    .incbin "cvaos_us_baserom.gba", 0x20B4E4, 0x48

    .global sSkillDevilManticoreSparkPalette
sSkillDevilManticoreSparkPalette: @ 0x0820B52C
    .incbin "cvaos_us_baserom.gba", 0x20B52C, 0xB4

    .global sSkillFinalGuardPalette
sSkillFinalGuardPalette: @ 0x0820B5E0
    .incbin "cvaos_us_baserom.gba", 0x20B5E0, 0x24

    .global sSkillBuerOrbitPalette
sSkillBuerOrbitPalette: @ 0x0820B604
    .incbin "cvaos_us_baserom.gba", 0x20B604, 0x48

    .global sUnk_0820B64C
sUnk_0820B64C: @ 0x0820B64C
    .incbin "cvaos_us_baserom.gba", 0x20B64C, 0x48

    .global sSkillDeathPalette
sSkillDeathPalette: @ 0x0820B694
    .incbin "cvaos_us_baserom.gba", 0x20B694, 0x90

    .global sWormPalette
sWormPalette: @ 0x0820B724
    .incbin "cvaos_us_baserom.gba", 0x20B724, 0x44

    .global sDryadPalette
sDryadPalette: @ 0x0820B768
    .incbin "cvaos_us_baserom.gba", 0x20B768, 0x24

    .global sSkullArcherPalette
sSkullArcherPalette: @ 0x0820B78C
    .incbin "cvaos_us_baserom.gba", 0x20B78C, 0x24

    .global sMinotaurPalette
sMinotaurPalette: @ 0x0820B7B0
    .incbin "cvaos_us_baserom.gba", 0x20B7B0, 0x44

    .global sAxeArmorPalette
sAxeArmorPalette: @ 0x0820B7F4
    .incbin "cvaos_us_baserom.gba", 0x20B7F4, 0x24

    .global sDiscArmorPalette
sDiscArmorPalette: @ 0x0820B818
    .incbin "cvaos_us_baserom.gba", 0x20B818, 0x24

    .global sBomberArmorRockArmorPalette
sBomberArmorRockArmorPalette: @ 0x0820B83C
    .incbin "cvaos_us_baserom.gba", 0x20B83C, 0x44

    .global sWitchPalette
sWitchPalette: @ 0x0820B880
    .incbin "cvaos_us_baserom.gba", 0x20B880, 0x24

    .global sCockatriceFamilyPalette
sCockatriceFamilyPalette: @ 0x0820B8A4
    .incbin "cvaos_us_baserom.gba", 0x20B8A4, 0x88

    .global sBalorePalette
sBalorePalette: @ 0x0820B92C
    .incbin "cvaos_us_baserom.gba", 0x20B92C, 0x284

    .global sChronomagePalette
sChronomagePalette: @ 0x0820BBB0
    .incbin "cvaos_us_baserom.gba", 0x20BBB0, 0x24

    .global sDeadWarriorPalette
sDeadWarriorPalette: @ 0x0820BBD4
    .incbin "cvaos_us_baserom.gba", 0x20BBD4, 0x24

    .global sNightmarePalette
sNightmarePalette: @ 0x0820BBF8
    .incbin "cvaos_us_baserom.gba", 0x20BBF8, 0x24

    .global sDeathPalette
sDeathPalette: @ 0x0820BC1C
    .incbin "cvaos_us_baserom.gba", 0x20BC1C, 0x84

    .global sSkullMillioneFamilyPalette
sSkullMillioneFamilyPalette: @ 0x0820BCA0
    .incbin "cvaos_us_baserom.gba", 0x20BCA0, 0x64

    .global sKillerMantlePalette
sKillerMantlePalette: @ 0x0820BD04
    .incbin "cvaos_us_baserom.gba", 0x20BD04, 0x24

    .global sManEaterPalette
sManEaterPalette: @ 0x0820BD28
    .incbin "cvaos_us_baserom.gba", 0x20BD28, 0x24

    .global sBatPalette
sBatPalette: @ 0x0820BD4C
    .incbin "cvaos_us_baserom.gba", 0x20BD4C, 0x24

    .global sAltairPalette
sAltairPalette: @ 0x0820BD70
    .incbin "cvaos_us_baserom.gba", 0x20BD70, 0x24

    .global sFleamanRipperPalette
sFleamanRipperPalette: @ 0x0820BD94
    .incbin "cvaos_us_baserom.gba", 0x20BD94, 0x44

    .global sGhostDancerPalette
sGhostDancerPalette: @ 0x0820BDD8
    .incbin "cvaos_us_baserom.gba", 0x20BDD8, 0x64

    .global sSkeletonWaiterSkeletonPalette
sSkeletonWaiterSkeletonPalette: @ 0x0820BE3C
    .incbin "cvaos_us_baserom.gba", 0x20BE3C, 0x24

    .global sImpTinyDevilPalette
sImpTinyDevilPalette: @ 0x0820BE60
    .incbin "cvaos_us_baserom.gba", 0x20BE60, 0x44

    .global sUkobackPalette
sUkobackPalette: @ 0x0820BEA4
    .incbin "cvaos_us_baserom.gba", 0x20BEA4, 0x24

    .global sNemesisPalette
sNemesisPalette: @ 0x0820BEC8
    .incbin "cvaos_us_baserom.gba", 0x20BEC8, 0x24

    .global sKillerDollLightningDollPalette
sKillerDollLightningDollPalette: @ 0x0820BEEC
    .incbin "cvaos_us_baserom.gba", 0x20BEEC, 0x24

    .global sBlueCrowRedCrowPalette
sBlueCrowRedCrowPalette: @ 0x0820BF10
    .incbin "cvaos_us_baserom.gba", 0x20BF10, 0x44

    .global sBeamSkeletonPalette
sBeamSkeletonPalette: @ 0x0820BF54
    .incbin "cvaos_us_baserom.gba", 0x20BF54, 0x24

    .global sNeedlesPalette
sNeedlesPalette: @ 0x0820BF78
    .incbin "cvaos_us_baserom.gba", 0x20BF78, 0x24

    .global sBaelBuerPalette
sBaelBuerPalette: @ 0x0820BF9C
    .incbin "cvaos_us_baserom.gba", 0x20BF9C, 0x44

    .global sAlastorPalette
sAlastorPalette: @ 0x0820BFE0
    .incbin "cvaos_us_baserom.gba", 0x20BFE0, 0x24

    .global sMandragoraPalette
sMandragoraPalette: @ 0x0820C004
    .incbin "cvaos_us_baserom.gba", 0x20C004, 0x24

    .global sJuliusEntityPalette
sJuliusEntityPalette: @ 0x0820C028
    .incbin "cvaos_us_baserom.gba", 0x20C028, 0x24

    .global sJuliusPalette
sJuliusPalette: @ 0x0820C04C
    .incbin "cvaos_us_baserom.gba", 0x20C04C, 0x44

    .global sGrahamPalette1
sGrahamPalette1: @ 0x0820C090
    .incbin "cvaos_us_baserom.gba", 0x20C090, 0xA4

    .global sGrahamShotPalette
sGrahamShotPalette: @ 0x0820C134
    .incbin "cvaos_us_baserom.gba", 0x20C134, 0x24

    .global sGrahamPalette2
sGrahamPalette2: @ 0x0820C158
    .incbin "cvaos_us_baserom.gba", 0x20C158, 0x124

    .global sEvilButcherPalette
sEvilButcherPalette: @ 0x0820C27C
    .incbin "cvaos_us_baserom.gba", 0x20C27C, 0x44

    .global sCursorSpritePalette
sCursorSpritePalette: @ 0x0820C2C0
    .incbin "cvaos_us_baserom.gba", 0x20C2C0, 0xC4

    .global sSaveWarpRoomPalette
sSaveWarpRoomPalette: @ 0x0820C384
    .incbin "cvaos_us_baserom.gba", 0x20C384, 0xA4

    .global sSavePromptBgPalette
sSavePromptBgPalette: @ 0x0820C428
    .incbin "cvaos_us_baserom.gba", 0x20C428, 0x480

    .global sTitleScreenSpriteSheet
sTitleScreenSpriteSheet: @ 0x0820C8A8
    .incbin "cvaos_us_baserom.gba", 0x20C8A8, 0x1 @ TODO: size

    .incbin "cvaos_us_baserom.gba", 0x20C8A9, 0x20CB1C - 0x20C8A9

    .global sTitleFlashSpriteSheet
sTitleFlashSpriteSheet: @ 0x0820CB1C
    .incbin "cvaos_us_baserom.gba", 0x20CB1C, 0x1 @ TODO: size

    .incbin "cvaos_us_baserom.gba", 0x20CB1D, 0x3DB

    .global sObject10SpriteSheet
sObject10SpriteSheet: @ 0x0820CEF8
    .incbin "cvaos_us_baserom.gba", 0x20CEF8, 0x1E68

    .global sCommonObjectSpriteSheet
sCommonObjectSpriteSheet: @ 0x0820ED60
    .incbin "cvaos_us_baserom.gba", 0x20ED60, 0x2B0

    .global sDefeatParticleSpriteSheet
sDefeatParticleSpriteSheet: @ 0x0820F010
    .incbin "cvaos_us_baserom.gba", 0x20F010, 0x150

    .global sGateSpriteSheet
sGateSpriteSheet: @ 0x0820F160
    .incbin "cvaos_us_baserom.gba", 0x20F160, 0xCE4

    .global sObject07ParallaxSpriteSpriteSheet
sObject07ParallaxSpriteSpriteSheet: @ 0x0820FE44
    .incbin "cvaos_us_baserom.gba", 0x20FE44, 0x338

    .global sObject0DSpriteSheet
sObject0DSpriteSheet: @ 0x0821017C
    .incbin "cvaos_us_baserom.gba", 0x21017C, 0xE8

    .global sObject13FamilySpriteSheet
sObject13FamilySpriteSheet: @ 0x08210264
    .incbin "cvaos_us_baserom.gba", 0x210264, 0xB4

    .global sObject1BSpriteSheet
sObject1BSpriteSheet: @ 0x08210318
    .incbin "cvaos_us_baserom.gba", 0x210318, 0xDC

    .global sObject22SpriteSheet
sObject22SpriteSheet: @ 0x082103F4
    .incbin "cvaos_us_baserom.gba", 0x2103F4, 0x3F8

    .global sObject21Object23SpriteSheet
sObject21Object23SpriteSheet: @ 0x082107EC
    .incbin "cvaos_us_baserom.gba", 0x2107EC, 0x2A4

    .global sObject17SpriteSheet
sObject17SpriteSheet: @ 0x08210A90
    .incbin "cvaos_us_baserom.gba", 0x210A90, 0xD0

    .global sObject14SpriteSheet
sObject14SpriteSheet: @ 0x08210B60
    .incbin "cvaos_us_baserom.gba", 0x210B60, 0x694

    .global sObject2DSpriteSheet
sObject2DSpriteSheet: @ 0x082111F4
    .incbin "cvaos_us_baserom.gba", 0x2111F4, 0x188

    .global sObject2ESpriteSheet
sObject2ESpriteSheet: @ 0x0821137C
    .incbin "cvaos_us_baserom.gba", 0x21137C, 0xE8

    .global sObject2CSpriteSheet
sObject2CSpriteSheet: @ 0x08211464
    .incbin "cvaos_us_baserom.gba", 0x211464, 0xE8

    .global sObject2BSpriteSheet
sObject2BSpriteSheet: @ 0x0821154C
    .incbin "cvaos_us_baserom.gba", 0x21154C, 0xAC

    .global sObject15SpriteSheet
sObject15SpriteSheet: @ 0x082115F8
    .incbin "cvaos_us_baserom.gba", 0x2115F8, 0x24C

    .global sObject2FFamilySpriteSheet
sObject2FFamilySpriteSheet: @ 0x08211844
    .incbin "cvaos_us_baserom.gba", 0x211844, 0x46C

    .global sObject33SpriteSheet
sObject33SpriteSheet: @ 0x08211CB0
    .incbin "cvaos_us_baserom.gba", 0x211CB0, 0x114

    .global sObject34SpriteSheet
sObject34SpriteSheet: @ 0x08211DC4
    .incbin "cvaos_us_baserom.gba", 0x211DC4, 0x144

    .global sObject32SpriteSheet
sObject32SpriteSheet: @ 0x08211F08
    .incbin "cvaos_us_baserom.gba", 0x211F08, 0xA8

    .global sObject35SpriteSheet
sObject35SpriteSheet: @ 0x08211FB0
    .incbin "cvaos_us_baserom.gba", 0x211FB0, 0x8AC

    .global sValkyrieSpriteSheet
sValkyrieSpriteSheet: @ 0x0821285C
    .incbin "cvaos_us_baserom.gba", 0x21285C, 0x408

    .global sWerewolfSpriteSheet
sWerewolfSpriteSheet: @ 0x08212C64
    .incbin "cvaos_us_baserom.gba", 0x212C64, 0x350

    .global sKillerFishSpriteSheet
sKillerFishSpriteSheet: @ 0x08212FB4
    .incbin "cvaos_us_baserom.gba", 0x212FB4, 0x440

    .global sMermanSpriteSheet
sMermanSpriteSheet: @ 0x082133F4
    .incbin "cvaos_us_baserom.gba", 0x2133F4, 0x4B8

    .global sMudmanSpriteSheet
sMudmanSpriteSheet: @ 0x082138AC
    .incbin "cvaos_us_baserom.gba", 0x2138AC, 0xD10

    .global sManticoreSpriteSheet
sManticoreSpriteSheet: @ 0x082145BC
    .incbin "cvaos_us_baserom.gba", 0x2145BC, 0x4E8

    .global sManticoreOverlaySpriteSheet
sManticoreOverlaySpriteSheet: @ 0x08214AA4
    .incbin "cvaos_us_baserom.gba", 0x214AA4, 0x194

    .global sUneSpriteSheet
sUneSpriteSheet: @ 0x08214C38
    .incbin "cvaos_us_baserom.gba", 0x214C38, 0x1084

    .global sWingedSkeletonSpriteSheet
sWingedSkeletonSpriteSheet: @ 0x08215CBC
    .incbin "cvaos_us_baserom.gba", 0x215CBC, 0x3A4

    .global sHarpySpriteSheet
sHarpySpriteSheet: @ 0x08216060
    .incbin "cvaos_us_baserom.gba", 0x216060, 0x2BC

    .global sQuezlcoatlWhiteDragonSpriteSheet
sQuezlcoatlWhiteDragonSpriteSheet: @ 0x0821631C
    .incbin "cvaos_us_baserom.gba", 0x21631C, 0x724

    .global sWerejaguarSpriteSheet
sWerejaguarSpriteSheet: @ 0x08216A40
    .incbin "cvaos_us_baserom.gba", 0x216A40, 0xA24

    .global sArachneSpriteSheet
sArachneSpriteSheet: @ 0x08217464
    .incbin "cvaos_us_baserom.gba", 0x217464, 0x454

    .global sZombieSpriteSheet
sZombieSpriteSheet: @ 0x082178B8
    .incbin "cvaos_us_baserom.gba", 0x2178B8, 0x490

    .global sPersephoneSpriteSheet
sPersephoneSpriteSheet: @ 0x08217D48
    .incbin "cvaos_us_baserom.gba", 0x217D48, 0xFF4

    .global sAluraUneSpriteSheet
sAluraUneSpriteSheet: @ 0x08218D3C
    .incbin "cvaos_us_baserom.gba", 0x218D3C, 0x404

    .global sGiantGhostSpriteSheet
sGiantGhostSpriteSheet: @ 0x08219140
    .incbin "cvaos_us_baserom.gba", 0x219140, 0x6C4

    .global sGolemSpriteSheet
sGolemSpriteSheet: @ 0x08219804
    .incbin "cvaos_us_baserom.gba", 0x219804, 0x298C

    .global sHeadhunterSpriteSheet
sHeadhunterSpriteSheet: @ 0x0821C190
    .incbin "cvaos_us_baserom.gba", 0x21C190, 0x4F8

    .global sGargoyleSpriteSheet
sGargoyleSpriteSheet: @ 0x0821C688
    .incbin "cvaos_us_baserom.gba", 0x21C688, 0xE60

    .global sTritonSpriteSheet
sTritonSpriteSheet: @ 0x0821D4E8
    .incbin "cvaos_us_baserom.gba", 0x21D4E8, 0x2CC

    .global sPeepingEyeSpriteSheet
sPeepingEyeSpriteSheet: @ 0x0821D7B4
    .incbin "cvaos_us_baserom.gba", 0x21D7B4, 0x1534

    .global sGorgonSpriteSheet
sGorgonSpriteSheet: @ 0x0821ECE8
    .incbin "cvaos_us_baserom.gba", 0x21ECE8, 0x124

    .global sGhostSpriteSheet
sGhostSpriteSheet: @ 0x0821EE0C
    .incbin "cvaos_us_baserom.gba", 0x21EE0C, 0x434

    .global sGreatArmorSpriteSheet
sGreatArmorSpriteSheet: @ 0x0821F240
    .incbin "cvaos_us_baserom.gba", 0x21F240, 0x81C

    .global sDeadCrusaderSkeletonKnightSpriteSheet
sDeadCrusaderSkeletonKnightSpriteSheet: @ 0x0821FA5C
    .global sDeadCrusaderSkeletonKnightSpriteSheet
sDeadCrusaderSkeletonKnightSpriteSheet: @ 0x0821FA5C
    .incbin "cvaos_us_baserom.gba", 0x21FA5C, 0xFDC

    .global sDevilSpriteSheet
sDevilSpriteSheet: @ 0x08220A38
    .incbin "cvaos_us_baserom.gba", 0x220A38, 0x54C

    .global sZombieOfficerSpriteSheet
sZombieOfficerSpriteSheet: @ 0x08220F84
    .incbin "cvaos_us_baserom.gba", 0x220F84, 0x640

    .global sZombieSoldierSpriteSheet
sZombieSoldierSpriteSheet: @ 0x082215C4
    .incbin "cvaos_us_baserom.gba", 0x2215C4, 0x288

    .global sCurlySpriteSheet
sCurlySpriteSheet: @ 0x0822184C
    .incbin "cvaos_us_baserom.gba", 0x22184C, 0x12C

    .global sMedusaHeadSpriteSheet
sMedusaHeadSpriteSheet: @ 0x08221978
    .incbin "cvaos_us_baserom.gba", 0x221978, 0x1584

    .global sLilithSuccubusSpriteSheet
sLilithSuccubusSpriteSheet: @ 0x08222EFC
    .global sLilithSuccubusSpriteSheet
sLilithSuccubusSpriteSheet: @ 0x08222EFC
    .incbin "cvaos_us_baserom.gba", 0x222EFC, 0x3BC

    .global sCreakingSkullSpriteSheet
sCreakingSkullSpriteSheet: @ 0x082232B8
    .incbin "cvaos_us_baserom.gba", 0x2232B8, 0x3E0

    .global sMimicSpriteSheet
sMimicSpriteSheet: @ 0x08223698
    .incbin "cvaos_us_baserom.gba", 0x223698, 0x1F4

    .global sEctoplasmSpriteSheet
sEctoplasmSpriteSheet: @ 0x0822388C
    .incbin "cvaos_us_baserom.gba", 0x22388C, 0x76C

    .global sBiphronSpriteSheet
sBiphronSpriteSheet: @ 0x08223FF8
    .incbin "cvaos_us_baserom.gba", 0x223FF8, 0x122C

    .global sStolasSpriteSheet
sStolasSpriteSheet: @ 0x08225224
    .incbin "cvaos_us_baserom.gba", 0x225224, 0x6D8

    .global sLegionSpriteSheet
sLegionSpriteSheet: @ 0x082258FC
    .incbin "cvaos_us_baserom.gba", 0x2258FC, 0x10C

    .global sObject25SpriteSheet
sObject25SpriteSheet: @ 0x08225A08
    .incbin "cvaos_us_baserom.gba", 0x225A08, 0x4F0

    .global sChaosSpriteSheet
sChaosSpriteSheet: @ 0x08225EF8
    .incbin "cvaos_us_baserom.gba", 0x225EF8, 0xC60

    .global sChaos2SpriteSheet
sChaos2SpriteSheet: @ 0x08226B58
    .incbin "cvaos_us_baserom.gba", 0x226B58, 0x354

    .global sKyomaDemonSpriteSheet
sKyomaDemonSpriteSheet: @ 0x08226EAC
    .incbin "cvaos_us_baserom.gba", 0x226EAC, 0x2258

    .global sGladiatorSpriteSheet
sGladiatorSpriteSheet: @ 0x08229104
    .incbin "cvaos_us_baserom.gba", 0x229104, 0x428

    .global sTsuchinokoSpriteSheet
sTsuchinokoSpriteSheet: @ 0x0822952C
    .incbin "cvaos_us_baserom.gba", 0x22952C, 0x240

    .global sSkyFishSpriteSheet
sSkyFishSpriteSheet: @ 0x0822976C
    .incbin "cvaos_us_baserom.gba", 0x22976C, 0xCA38

    .global sSkillCagnazzoSpriteSheet
sSkillCagnazzoSpriteSheet: @ 0x082361A4
    .incbin "cvaos_us_baserom.gba", 0x2361A4, 0x114

    .global sSkillBonePillarProjectileSpriteSheet
sSkillBonePillarProjectileSpriteSheet: @ 0x082362B8
    .incbin "cvaos_us_baserom.gba", 0x2362B8, 0x164

    .global sSkillDevilManticoreSparkSpriteSheet
sSkillDevilManticoreSparkSpriteSheet: @ 0x0823641C
    .incbin "cvaos_us_baserom.gba", 0x23641C, 0xDD4

    .global sSkillFinalGuardSpriteSheet
sSkillFinalGuardSpriteSheet: @ 0x082371F0
    .incbin "cvaos_us_baserom.gba", 0x2371F0, 0x114

    .global sSkillBuerOrbitSpriteSheet
sSkillBuerOrbitSpriteSheet: @ 0x08237304
    .incbin "cvaos_us_baserom.gba", 0x237304, 0x308

    .global sUnk_0823760C
sUnk_0823760C: @ 0x0823760C
    .incbin "cvaos_us_baserom.gba", 0x23760C, 0x164

    .global sSkillDeathSpriteSheet
sSkillDeathSpriteSheet: @ 0x08237770
    .incbin "cvaos_us_baserom.gba", 0x237770, 0x114

    .global sSkillCatoblepasProjectileSpriteSheet
sSkillCatoblepasProjectileSpriteSheet: @ 0x08237884
    .incbin "cvaos_us_baserom.gba", 0x237884, 0x3C0

    .global sWormSpriteSheet
sWormSpriteSheet: @ 0x08237C44
    .incbin "cvaos_us_baserom.gba", 0x237C44, 0xC68

    .global sDryadSpriteSheet
sDryadSpriteSheet: @ 0x082388AC
    .incbin "cvaos_us_baserom.gba", 0x2388AC, 0xB34

    .global sWaiterSkeletonSpriteSheet
sWaiterSkeletonSpriteSheet: @ 0x082393E0
    .incbin "cvaos_us_baserom.gba", 0x2393E0, 0x114C

    .global sSkullArcherSpriteSheet
sSkullArcherSpriteSheet: @ 0x0823A52C
    .incbin "cvaos_us_baserom.gba", 0x23A52C, 0x8E4

    .global sMinotaurSpriteSheet
sMinotaurSpriteSheet: @ 0x0823AE10
    .incbin "cvaos_us_baserom.gba", 0x23AE10, 0x1480

    .global sAxeArmorSpriteSheet
sAxeArmorSpriteSheet: @ 0x0823C290
    .incbin "cvaos_us_baserom.gba", 0x23C290, 0x14FC

    .global sDiscArmorSpriteSheet
sDiscArmorSpriteSheet: @ 0x0823D78C
    .incbin "cvaos_us_baserom.gba", 0x23D78C, 0x1474

    .global sBomberArmorSpriteSheet
sBomberArmorSpriteSheet: @ 0x0823EC00
    .incbin "cvaos_us_baserom.gba", 0x23EC00, 0x13D8

    .global sRockArmorSpriteSheet
sRockArmorSpriteSheet: @ 0x0823FFD8
    .incbin "cvaos_us_baserom.gba", 0x23FFD8, 0x17B8

    .global sWitchSpriteSheet
sWitchSpriteSheet: @ 0x08241790
    .incbin "cvaos_us_baserom.gba", 0x241790, 0x1204

    .global sCockatriceFamilySpriteSheet
sCockatriceFamilySpriteSheet: @ 0x08242994
    .incbin "cvaos_us_baserom.gba", 0x242994, 0x188C

    .global sBaloreSpriteSheet
sBaloreSpriteSheet: @ 0x08244220
    .incbin "cvaos_us_baserom.gba", 0x244220, 0xB2C

    .global sBaloreSpriteSheet2
sBaloreSpriteSheet2: @ 0x08244D4C
    .incbin "cvaos_us_baserom.gba", 0x244D4C, 0xF5C

    .global sChronomageSpriteSheet
sChronomageSpriteSheet: @ 0x08245CA8
    .incbin "cvaos_us_baserom.gba", 0x245CA8, 0x1258

    .global sDeadWarriorSpriteSheet
sDeadWarriorSpriteSheet: @ 0x08246F00
    .incbin "cvaos_us_baserom.gba", 0x246F00, 0x588

    .global sNightmareSpriteSheet
sNightmareSpriteSheet: @ 0x08247488
    .incbin "cvaos_us_baserom.gba", 0x247488, 0x27C0

    .global sDeathSpriteSheet
sDeathSpriteSheet: @ 0x08249C48
    .incbin "cvaos_us_baserom.gba", 0x249C48, 0x11C4

    .global sKillerMantleSpriteSheet
sKillerMantleSpriteSheet: @ 0x0824AE0C
    .incbin "cvaos_us_baserom.gba", 0x24AE0C, 0x36C

    .global sManEaterSpriteSheet
sManEaterSpriteSheet: @ 0x0824B178
    .incbin "cvaos_us_baserom.gba", 0x24B178, 0x14C

    .global sBatSpriteSheet
sBatSpriteSheet: @ 0x0824B2C4
    .incbin "cvaos_us_baserom.gba", 0x24B2C4, 0x39C

    .global sAltairSpriteSheet
sAltairSpriteSheet: @ 0x0824B660
    .incbin "cvaos_us_baserom.gba", 0x24B660, 0x2CC

    .global sFleamanRipperSpriteSheet
sFleamanRipperSpriteSheet: @ 0x0824B92C
    .incbin "cvaos_us_baserom.gba", 0x24B92C, 0x358

    .global sGhostDancerSpriteSheet
sGhostDancerSpriteSheet: @ 0x0824BC84
    .incbin "cvaos_us_baserom.gba", 0x24BC84, 0xC8C

    .global sSkeletonSpriteSheet
sSkeletonSpriteSheet: @ 0x0824C910
    .incbin "cvaos_us_baserom.gba", 0x24C910, 0x5FC

    .global sImpTinyDevilSpriteSheet
sImpTinyDevilSpriteSheet: @ 0x0824CF0C
    .incbin "cvaos_us_baserom.gba", 0x24CF0C, 0x28C

    .global sUkobackSpriteSheet
sUkobackSpriteSheet: @ 0x0824D198
    .incbin "cvaos_us_baserom.gba", 0x24D198, 0x140

    .global sNemesisSpriteSheet
sNemesisSpriteSheet: @ 0x0824D2D8
    .incbin "cvaos_us_baserom.gba", 0x24D2D8, 0x834

    .global sKillerDollSpriteSheet
sKillerDollSpriteSheet: @ 0x0824DB0C
    .incbin "cvaos_us_baserom.gba", 0x24DB0C, 0xCE8

    .global sLightningDollSpriteSheet
sLightningDollSpriteSheet: @ 0x0824E7F4
    .incbin "cvaos_us_baserom.gba", 0x24E7F4, 0x410

    .global sBlueCrowRedCrowSpriteSheet
sBlueCrowRedCrowSpriteSheet: @ 0x0824EC04
    .incbin "cvaos_us_baserom.gba", 0x24EC04, 0xDA8

    .global sBeamSkeletonSpriteSheet
sBeamSkeletonSpriteSheet: @ 0x0824F9AC
    .incbin "cvaos_us_baserom.gba", 0x24F9AC, 0x164

    .global sNeedlesSpriteSheet
sNeedlesSpriteSheet: @ 0x0824FB10
    .incbin "cvaos_us_baserom.gba", 0x24FB10, 0x2A0

    .global sBaelBuerSpriteSheet
sBaelBuerSpriteSheet: @ 0x0824FDB0
    .incbin "cvaos_us_baserom.gba", 0x24FDB0, 0x130

    .global sAlastorSpriteSheet
sAlastorSpriteSheet: @ 0x0824FEE0
    .incbin "cvaos_us_baserom.gba", 0x24FEE0, 0xFB4

    .global sMandragoraSpriteSheet
sMandragoraSpriteSheet: @ 0x08250E94
    .incbin "cvaos_us_baserom.gba", 0x250E94, 0x18B4

    .global sJuliusSpriteSheet
sJuliusSpriteSheet: @ 0x08252748
    .incbin "cvaos_us_baserom.gba", 0x252748, 0x2D0

    .global sBelmontSpriteSheet
sBelmontSpriteSheet: @ 0x08252A18
    .incbin "cvaos_us_baserom.gba", 0x252A18, 0x1348

    .global sGrahamSpriteSheet1
sGrahamSpriteSheet1: @ 0x08253D60
    .incbin "cvaos_us_baserom.gba", 0x253D60, 0x1024

    .global sGrahamSpriteSheet2
sGrahamSpriteSheet2: @ 0x08254D84
    .incbin "cvaos_us_baserom.gba", 0x254D84, 0x648

    .global sGrahamSideSpriteSheet
sGrahamSideSpriteSheet: @ 0x082553CC
    .incbin "cvaos_us_baserom.gba", 0x2553CC, 0x1010

    .global sGrahamFirstFormSpriteSheet
sGrahamFirstFormSpriteSheet: @ 0x082563DC
    .incbin "cvaos_us_baserom.gba", 0x2563DC, 0x344

    .global sEvilButcherSpriteSheet
sEvilButcherSpriteSheet: @ 0x08256720
    .incbin "cvaos_us_baserom.gba", 0x256720, 0x7D8

    .global sMenuSpriteSheet
sMenuSpriteSheet: @ 0x08256EF8
    .incbin "cvaos_us_baserom.gba", 0x256EF8, 0xA6C

    .global sSaveWarpRoomSpriteSheet
sSaveWarpRoomSpriteSheet: @ 0x08257964
    .incbin "cvaos_us_baserom.gba", 0x257964, 0x274

    .global sWarpRoomSpriteSheet
sWarpRoomSpriteSheet: @ 0x08257BD8
    .incbin "cvaos_us_baserom.gba", 0x257BD8, 0x1558

    .global sObject37Gfx
sObject37Gfx: @ 0x08259130
    .incbin "cvaos_us_baserom.gba", 0x259130, 0x8

    .global sObject37Palette
sObject37Palette: @ 0x08259138
    .incbin "cvaos_us_baserom.gba", 0x259138, 0x84

    .global sObject37SpriteSheet
sObject37SpriteSheet: @ 0x082591BC
    .incbin "cvaos_us_baserom.gba", 0x2591BC, 0x50

    .global sObject24Gfx
sObject24Gfx: @ 0x0825920C
    .incbin "cvaos_us_baserom.gba", 0x25920C, 0x8

    .global sKickerSkeletonGfx
sKickerSkeletonGfx: @ 0x08259214
    .incbin "cvaos_us_baserom.gba", 0x259214, 0x310C

    .global sKickerSkeletonPalette
sKickerSkeletonPalette: @ 0x0825C320
    .incbin "cvaos_us_baserom.gba", 0x25C320, 0xBB0

    .global sObject24SpriteSheet
sObject24SpriteSheet: @ 0x0825CED0
    .incbin "cvaos_us_baserom.gba", 0x25CED0, 0xB88

    .global sKickerSkeletonSpriteSheet
sKickerSkeletonSpriteSheet: @ 0x0825DA58
    .incbin "cvaos_us_baserom.gba", 0x25DA58, 0x62C

    .global sObject16Gfx
sObject16Gfx: @ 0x0825E084
    .incbin "cvaos_us_baserom.gba", 0x25E084, 0x8

    .global sSkillFlyingArmorGfx
sSkillFlyingArmorGfx: @ 0x0825E08C
    .incbin "cvaos_us_baserom.gba", 0x25E08C, 0x804

    .global sObject16Palette
sObject16Palette: @ 0x0825E890
    .incbin "cvaos_us_baserom.gba", 0x25E890, 0x84

    .global sSkillFlyingArmorPalette
sSkillFlyingArmorPalette: @ 0x0825E914
    .incbin "cvaos_us_baserom.gba", 0x25E914, 0x15C

    .global sObject16SpriteSheet
sObject16SpriteSheet: @ 0x0825EA70
    .incbin "cvaos_us_baserom.gba", 0x25EA70, 0x54

    .global sSkillFlyingArmorSpriteSheet
sSkillFlyingArmorSpriteSheet: @ 0x0825EAC4
    .incbin "cvaos_us_baserom.gba", 0x25EAC4, 0x10

    .global sTitleMenuGfx
sTitleMenuGfx: @ 0x0825EAD4
    .incbin "cvaos_us_baserom.gba", 0x25EAD4, 0x8

    .incbin "cvaos_us_baserom.gba", 0x25EADC, 0x18

    .global sObject18Gfx
sObject18Gfx: @ 0x0825EAF4
    .incbin "cvaos_us_baserom.gba", 0x25EAF4, 0x8

    .global sObject19Gfx
sObject19Gfx: @ 0x0825EAFC
    .incbin "cvaos_us_baserom.gba", 0x25EAFC, 0x8

    .global sBigGolemGfx
sBigGolemGfx: @ 0x0825EB04
    .incbin "cvaos_us_baserom.gba", 0x25EB04, 0x8

    .global sBigGolemGfx2
sBigGolemGfx2: @ 0x0825EB0C
    .incbin "cvaos_us_baserom.gba", 0x25EB0C, 0x1814

    .global sSlimeGfx
sSlimeGfx: @ 0x08260320
    .incbin "cvaos_us_baserom.gba", 0x260320, 0x70

    .global sEventScene30Gfx
sEventScene30Gfx: @ 0x08260390
    .incbin "cvaos_us_baserom.gba", 0x260390, 0x6054

    .global sObject18Palette
sObject18Palette: @ 0x082663E4
    .incbin "cvaos_us_baserom.gba", 0x2663E4, 0x24

    .global sObject19Palette
sObject19Palette: @ 0x08266408
    .incbin "cvaos_us_baserom.gba", 0x266408, 0x24

    .global sBigGolemPalette
sBigGolemPalette: @ 0x0826642C
    .incbin "cvaos_us_baserom.gba", 0x26642C, 0xB0

    .global sSlimePalette
sSlimePalette: @ 0x082664DC
    .incbin "cvaos_us_baserom.gba", 0x2664DC, 0x68

    .global sEventScene30Palette
sEventScene30Palette: @ 0x08266544
    .incbin "cvaos_us_baserom.gba", 0x266544, 0x3F8

    .global sTitleMenuSpriteSheet
sTitleMenuSpriteSheet: @ 0x0826693C
    .incbin "cvaos_us_baserom.gba", 0x26693C, 0x1 @ TODO: size

    .incbin "cvaos_us_baserom.gba", 0x26693D, 0x2C3

    .global sObject18SpriteSheet
sObject18SpriteSheet: @ 0x08266C00
    .incbin "cvaos_us_baserom.gba", 0x266C00, 0x50

    .global sObject19SpriteSheet
sObject19SpriteSheet: @ 0x08266C50
    .incbin "cvaos_us_baserom.gba", 0x266C50, 0x1B8

    .global sBigGolemSpriteSheet
sBigGolemSpriteSheet: @ 0x08266E08
    .incbin "cvaos_us_baserom.gba", 0x266E08, 0x70

    .global sBigGolemSpriteSheet2
sBigGolemSpriteSheet2: @ 0x08266E78
    .incbin "cvaos_us_baserom.gba", 0x266E78, 0xB14

    .global sSlimeSpriteSheet
sSlimeSpriteSheet: @ 0x0826798C
    .incbin "cvaos_us_baserom.gba", 0x26798C, 0x558

    .global sEventScene30SpriteSheet
sEventScene30SpriteSheet: @ 0x08267EE4
    .incbin "cvaos_us_baserom.gba", 0x267EE4, 0x358

    .global sTitleMenuGfxLanguage2
sTitleMenuGfxLanguage2: @ 0x0826823C
    .incbin "cvaos_us_baserom.gba", 0x26823C, 0x8

    .global sTitleMenuGfxLanguage3
sTitleMenuGfxLanguage3: @ 0x08268244
    .incbin "cvaos_us_baserom.gba", 0x268244, 0x8

    .incbin "cvaos_us_baserom.gba", 0x26824C, 0x268344 - 0x26824C

    .global sTitleMenuSpriteSheetLanguage2
sTitleMenuSpriteSheetLanguage2: @ 0x08268344
    .incbin "cvaos_us_baserom.gba", 0x268344, 0x1 @ TODO: size

    .incbin "cvaos_us_baserom.gba", 0x268345, 0x268440 - 0x268345

    .global sTitleMenuSpriteSheetLanguage3
sTitleMenuSpriteSheetLanguage3: @ 0x08268440
    .incbin "cvaos_us_baserom.gba", 0x268440, 0x1 @ TODO: size

    .incbin "cvaos_us_baserom.gba", 0x268441, 0xF

    .global sObject10GfxLanguage2
sObject10GfxLanguage2: @ 0x08268450
    .incbin "cvaos_us_baserom.gba", 0x268450, 0x8

    .global sObject10GfxLanguage3
sObject10GfxLanguage3: @ 0x08268458
    .incbin "cvaos_us_baserom.gba", 0x268458, 0x1F0

    .global sObject10SpriteSheetLanguage2
sObject10SpriteSheetLanguage2: @ 0x08268648
    .incbin "cvaos_us_baserom.gba", 0x268648, 0x1E0

    .global sObject10SpriteSheetLanguage3
sObject10SpriteSheetLanguage3: @ 0x08268828
    .incbin "cvaos_us_baserom.gba", 0x268828, 0x4018

    .global sMenuCursorGfxLanguage3
sMenuCursorGfxLanguage3: @ 0x0826C840
    .incbin "cvaos_us_baserom.gba", 0x26C840, 0x8

    .global sLocalizedWindowGfxLanguage3
sLocalizedWindowGfxLanguage3: @ 0x0826C848
    .incbin "cvaos_us_baserom.gba", 0x26C848, 0x305C

    .global sCommonObjectGfxLanguage2
sCommonObjectGfxLanguage2: @ 0x0826F8A4
    .incbin "cvaos_us_baserom.gba", 0x26F8A4, 0x2004

    .global sCommonObjectGfxLanguage3
sCommonObjectGfxLanguage3: @ 0x082718A8
    .incbin "cvaos_us_baserom.gba", 0x2718A8, 0x60DC

    .global sLocalizedWindowGfxLanguage2
sLocalizedWindowGfxLanguage2: @ 0x08277984
    .incbin "cvaos_us_baserom.gba", 0x277984, 0x8

    .global sMenuCursorGfxLanguage2
sMenuCursorGfxLanguage2: @ 0x0827798C
    .incbin "cvaos_us_baserom.gba", 0x27798C, 0x3864

    .global sMenuPageTitleSpriteSheet
sMenuPageTitleSpriteSheet: @ 0x0827B1F0
    .incbin "cvaos_us_baserom.gba", 0x27B1F0, 0x10

    .global sLocalizedWindowGfx
sLocalizedWindowGfx: @ 0x0827B200
    .incbin "cvaos_us_baserom.gba", 0x27B200, 0x10
