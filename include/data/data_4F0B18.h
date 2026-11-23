#ifndef DATA_4F0B18_H
#define DATA_4F0B18_H

/* Data of asm/data/data_4F0B18.s used by C files. */

#include "types.h"
#include "structs/equipment_stats.h"

struct SoulKey {
    u8 soulType;
    u8 oneBasedSoulId;
    u8 pad_2[0x4 - 0x2];
};

struct BladeSoulVariant {
    void *graphics;
    void *palette;
    void *spriteSheet;
    u8 animFrame;
    u8 unk_D;
    u8 paletteIndex;
    u8 unk_F;
};

extern u8 *sUnk_084F10A0[];
extern u8 sPlayerStatusIconPalettes[];
extern void *sWeaponGfxTable[];
extern void *sWeaponSpriteSheetTable[];
extern u8 *sWeaponAttackAnims[];
extern u16 sWeaponSwingSounds[];
extern u16 sJuliusWhipSounds[];
extern struct WeaponInfo sUnarmedWeaponInfo;
extern struct BladeSoulVariant sBladeSoulVariants[];
extern u8 sSkillStudentWitchWalkProfile[];
extern u16 sSkillSirenNotes[];
extern u8 sSkillUneWalkProfile[];
extern u8 sUnk_084F13B4[];
extern u8 sSkillGladiatorWalkProfile[];
extern u8 sSkillCreakingSkullFrames[];
extern u8 sSkillCreakingSkullBiteFrames[];
extern struct SoulKey sKeySouls[];
extern u8 sJuliusHolyWaterFlameWalkProfile[];

#endif /* DATA_4F0B18_H */
