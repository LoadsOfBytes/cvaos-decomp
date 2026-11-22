	.include "asm/gba_constants.inc"
    .include "asm/macros.inc"

	.syntax unified

    .section .data

    .global sUnk_084F10A0
sUnk_084F10A0: @ 0x084F10A0
    .incbin "cvaos_us_baserom.gba", 0x4F10A0, 0xC

    .global sPlayerInitFuncs
sPlayerInitFuncs: @ 0x084F10AC
    .incbin "cvaos_us_baserom.gba", 0x4F10AC, 0x8

    .global sPlayerUpdateFuncs
sPlayerUpdateFuncs: @ 0x084F10B4
    .incbin "cvaos_us_baserom.gba", 0x4F10B4, 0x8

    .global sPlayerStatusIconPalettes
sPlayerStatusIconPalettes: @ 0x084F10BC
    .incbin "cvaos_us_baserom.gba", 0x4F10BC, 0x4

    .global sWeaponGfxTable
sWeaponGfxTable: @ 0x084F10C0
    .incbin "cvaos_us_baserom.gba", 0x4F10C0, 0xBC

    .global sWeaponSpriteSheetTable
sWeaponSpriteSheetTable: @ 0x084F117C
    .incbin "cvaos_us_baserom.gba", 0x4F117C, 0xBC

    .global sWeaponAttackAnims
sWeaponAttackAnims: @ 0x084F1238
    .incbin "cvaos_us_baserom.gba", 0x4F1238, 0x14

    .global sWeaponAttackUpdateFuncs
sWeaponAttackUpdateFuncs: @ 0x084F124C
    .incbin "cvaos_us_baserom.gba", 0x4F124C, 0x18

    .global sWeaponSwingSounds
sWeaponSwingSounds: @ 0x084F1264
    .incbin "cvaos_us_baserom.gba", 0x4F1264, 0x8

    .global sJuliusWhipSounds
sJuliusWhipSounds: @ 0x084F126C
    .incbin "cvaos_us_baserom.gba", 0x4F126C, 0x4

    .global sUnarmedWeaponInfo
sUnarmedWeaponInfo: @ 0x084F1270
    .incbin "cvaos_us_baserom.gba", 0x4F1270, 0x1C

    .global sBladeSoulVariants
sBladeSoulVariants: @ 0x084F128C
    .incbin "cvaos_us_baserom.gba", 0x4F128C, 0x30

    .global sSkillStudentWitchWalkProfile
sSkillStudentWitchWalkProfile: @ 0x084F12BC
    .incbin "cvaos_us_baserom.gba", 0x4F12BC, 0xC

    .global sUnk_084F12C8
sUnk_084F12C8: @ 0x084F12C8
    .incbin "cvaos_us_baserom.gba", 0x4F12C8, 0xDE

    .global sSkillSirenNotes
sSkillSirenNotes: @ 0x084F13A6
    .incbin "cvaos_us_baserom.gba", 0x4F13A6, 0x4

    .global sSkillUneWalkProfile
sSkillUneWalkProfile: @ 0x084F13AA
    .incbin "cvaos_us_baserom.gba", 0x4F13AA, 0xA

    .global sUnk_084F13B4
sUnk_084F13B4: @ 0x084F13B4
    .incbin "cvaos_us_baserom.gba", 0x4F13B4, 0x14

    .global sSkillGladiatorWalkProfile
sSkillGladiatorWalkProfile: @ 0x084F13C8
    .incbin "cvaos_us_baserom.gba", 0x4F13C8, 0x44

    .global sKeySouls
sKeySouls: @ 0x084F140C
    .incbin "cvaos_us_baserom.gba", 0x4F140C, 0x2C

    .incbin "cvaos_us_baserom.gba", 0x4F1438, 0x4F1444 - 0x4F1438
