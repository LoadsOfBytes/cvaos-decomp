#ifndef DATA_0E0B68_H
#define DATA_0E0B68_H

/* Data of asm/data/data_0E0B68.s used by C files. */

#include "types.h"

struct StatusEffectApplication {
    u16 initialCounter;
    u16 soundId;
};

/* Graphics resources of a red soul skill (sSkillGraphicsSets). */
struct SoulGraphicsResources {
    void *graphics;
    void *palette;
    void *spriteSheet;
};

// TODO: investigate Soma_InitEntity and the data
struct PlayerBodyBox {
    u8 x;
    u8 y;
    u8 width;
    u8 height;
};

struct BlueSoulForm {
    void (*func)(struct EwramData_EntityData *);
    u16 baseAttack;
    u16 attackAttributes;
    u16 song;
    u16 pad_A;
};

/* Yellow soul info, indexed by equippedYellowSoul (12-byte records). */
struct YellowSoulInfo {
    u32 *updateFunc; // update function of the soul entity (TODO: signature)
    u16 allowUnforcedRestart; // nonzero: may be started without param_1 (YellowSoul_Restart)
    u16 effectParam; // -> unk_4FE
    s32 effectValue; // -> unk_500 (effect value)
};

extern u8 sSomaSpriteSheet[];
extern u8 sSomaGfx[];
extern u8 sSomaAnimPalettes[];
extern u8 sUnk_080E12C8[];
extern u8 sSomaWallProbesStand[];
extern u8 sSomaWallProbesBat[];
extern u8 sSomaWallProbesDash[];
extern u8 sSomaOffsets[];
extern struct PlayerBodyBox sSomaStandingBodyBox[];
extern struct PlayerBodyBox sSomaAirborneBodyBox[];
extern struct PlayerBodyBox sSomaCrouchBodyBoxes[];
extern struct PlayerBodyBox sSomaBatFormBodyBoxes[];
extern u8 sSomaDiveAttackBoxFast[];
extern u8 sSomaDiveAttackBoxSlow[];
extern u16 sAttackPowerMultipliers[]; // array or struct?
extern struct StatusEffectApplication sStatusEffectApplications[];
extern const u8 sUnk_080E13B8[];
extern struct SoulGraphicsResources sSkillGraphicsSets[];
extern const u8 sRedSoulCastAnims[][2];
extern s32 sSkillProjectileLaunchVelocities[];
extern struct BlueSoulForm sBlueSoulForms[];
extern u8 sSkillCreakingSkullAngles[];
extern u8 sUnk_080E1AF8[];
extern s16 sUnk_080E1AFC[];
extern const struct YellowSoulInfo sYellowSoulInfo[];

#endif /* DATA_0E0B68_H */
