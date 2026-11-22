	.include "asm/gba_constants.inc"
    .include "asm/macros.inc"

	.syntax unified

    .section .rodata

    .incbin "cvaos_us_baserom.gba", 0xE28E0, 0x90

    .global sSpriteSheetDefault1
sSpriteSheetDefault1: @ 0x080E2970
    .incbin "cvaos_us_baserom.gba", 0xE2970, 0x10

    .global sSpriteSheetDefault2
sSpriteSheetDefault2: @ 0x080E2980
    .incbin "cvaos_us_baserom.gba", 0xE2980, 0x10

    .global sSpriteSheetDefault4
sSpriteSheetDefault4: @ 0x080E2990
    .incbin "cvaos_us_baserom.gba", 0xE2990, 0x10

    .global sUnk_080E29A0
sUnk_080E29A0: @ 0x080E29A0
    .incbin "cvaos_us_baserom.gba", 0xE29A0, 0x164

    .global sObjTileGrid4bpp
sObjTileGrid4bpp: @ 0x080E2B04
    .incbin "cvaos_us_baserom.gba", 0xE2B04, 0x20

    .global sObjTileGrid8bpp
sObjTileGrid8bpp: @ 0x080E2B24
    .incbin "cvaos_us_baserom.gba", 0xE2B24, 0x10

    .global sAnimStepFuncs
sAnimStepFuncs: @ 0x080E2B34
    .incbin "cvaos_us_baserom.gba", 0xE2B34, 0x18

    .global sText12GlyphMetrics
sText12GlyphMetrics: @ 0x080E2B4C
    .incbin "cvaos_us_baserom.gba", 0xE2B4C, 0x374

    .global sText8GlyphMetrics
sText8GlyphMetrics: @ 0x080E2EC0
    .incbin "cvaos_us_baserom.gba", 0xE2EC0, 0x530

    .global sHitTimers
sHitTimers: @ 0x080E33F0
    .incbin "cvaos_us_baserom.gba", 0xE33F0, 0xC

    .global sUnk_080E33FC
sUnk_080E33FC: @ 0x080E33FC
    .incbin "cvaos_us_baserom.gba", 0xE33FC, 0x68
