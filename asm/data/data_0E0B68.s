	.include "asm/gba_constants.inc"
    .include "asm/macros.inc"

	.syntax unified

    .section .rodata

    .incbin "cvaos_us_baserom.gba", 0xE117C, 0x48

    .global sSomaSpriteSheet
sSomaSpriteSheet: @ 0x080E11C4
    .incbin "cvaos_us_baserom.gba", 0xE11C4, 0x10

    .global sSomaGfx
sSomaGfx: @ 0x080E11D4
    .incbin "cvaos_us_baserom.gba", 0xE11D4, 0x98

    .global sSomaAnimPalettes
sSomaAnimPalettes: @ 0x080E126C
    .incbin "cvaos_us_baserom.gba", 0xE126C, 0x5C

    .global sUnk_080E12C8
sUnk_080E12C8: @ 0x080E12C8
    .incbin "cvaos_us_baserom.gba", 0x0E12C8, 0x0E12DC - 0x0E12C8

    .global sSomaWallProbesStand
sSomaWallProbesStand: @ 0x080E12DC
    .incbin "cvaos_us_baserom.gba", 0x0E12DC, 0x0E12EA - 0x0E12DC

    .global sSomaWallProbesBat
sSomaWallProbesBat: @ 0x080E12EA
    .incbin "cvaos_us_baserom.gba", 0x0E12EA, 0x0E12EF - 0x0E12EA

    .global sSomaWallProbesDash
sSomaWallProbesDash: @ 0x080E12EF
    .incbin "cvaos_us_baserom.gba", 0x0E12EF, 0x0E12F4 - 0x0E12EF

    .global sSomaOffsets
sSomaOffsets: @ 0x080E12F4
    .incbin "cvaos_us_baserom.gba", 0x0E12F4, 0x0E12F8 - 0x0E12F4

    .global sSomaStandingBodyBox
sSomaStandingBodyBox: @ 0x080E12F8
    .incbin "cvaos_us_baserom.gba", 0xE12F8, 0x4

    .global sSomaAirborneBodyBox
sSomaAirborneBodyBox: @ 0x080E12FC
    .incbin "cvaos_us_baserom.gba", 0xE12FC, 0x4

    .global sSomaCrouchBodyBoxes
sSomaCrouchBodyBoxes: @ 0x080E1300
    .incbin "cvaos_us_baserom.gba", 0xE1300, 0x8

    .global sSomaBatFormBodyBoxes
sSomaBatFormBodyBoxes: @ 0x080E1308
    .incbin "cvaos_us_baserom.gba", 0xE1308, 0xC

    .global sSomaDiveAttackBoxFast
sSomaDiveAttackBoxFast: @ 0x080E1314
    .incbin "cvaos_us_baserom.gba", 0xE1314, 0x4

    .global sSomaDiveAttackBoxSlow
sSomaDiveAttackBoxSlow: @ 0x080E1318
    .incbin "cvaos_us_baserom.gba", 0xE1318, 0x8

    .global sAttackPowerMultipliers
sAttackPowerMultipliers: @ 0x080E1320
    .incbin "cvaos_us_baserom.gba", 0xE1320, 0x18

    .global sStatusEffectApplications
sStatusEffectApplications: @ 0x080E1338
    .incbin "cvaos_us_baserom.gba", 0xE1338, 0x80

    .global sUnk_080E13B8
sUnk_080E13B8: @ 0x080E13B8
    .incbin "cvaos_us_baserom.gba", 0xE13B8, 0x4

    .global sSkillGraphicsSets
sSkillGraphicsSets: @ 0x080E13BC
    .incbin "cvaos_us_baserom.gba", 0xE13BC, 0x1DC

    .global sSomaRedSoulInfo
sSomaRedSoulInfo: @ 0x080E1598
    .incbin "cvaos_us_baserom.gba", 0xE1598, 0x380

    .global sRedSoulCastAnims
sRedSoulCastAnims: @ 0x080E1918
    .incbin "cvaos_us_baserom.gba", 0xE1918, 0x14

    .global sBlueSoulInfo
sBlueSoulInfo: @ 0x080E192C
    .incbin "cvaos_us_baserom.gba", 0xE192C, 0x138

    .global sSkillProjectileLaunchVelocities
sSkillProjectileLaunchVelocities: @ 0x080E1A64
    .incbin "cvaos_us_baserom.gba", 0xE1A64, 0x10

    .global sBlueSoulForms
sBlueSoulForms: @ 0x080E1A74
    .incbin "cvaos_us_baserom.gba", 0x0E1A74, 0x0E1AA8 - 0x0E1A74

    .global sSkillCreakingSkullAngles
sSkillCreakingSkullAngles: @ 0x080E1AA8
    .incbin "cvaos_us_baserom.gba", 0x0E1AA8, 0x0E1AF8 - 0x0E1AA8

    .global sUnk_080E1AF8
sUnk_080E1AF8: @ 0x080E1AF8
    .incbin "cvaos_us_baserom.gba", 0xE1AF8, 0x4

    .global sUnk_080E1AFC
sUnk_080E1AFC: @ 0x080E1AFC
    .incbin "cvaos_us_baserom.gba", 0xE1AFC, 0xC

    .global sYellowSoulInfo
sYellowSoulInfo: @ 0x080E1B08
    .incbin "cvaos_us_baserom.gba", 0xE1B08, 0x1C8
