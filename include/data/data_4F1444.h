#ifndef DATA_4F1444_H
#define DATA_4F1444_H

/* Data of asm/data/data_4F1444.s used by C files. */

#include "types.h"
#include "objects_00_07.h"
#include "structs/ewram.h"

extern u8 sUnk_085059BC[];
extern u8 sUnk_08505C94[];
extern const u16 sItemNameTextIds[]; /* name text ids */
extern const u16 sItemDescriptionTextIds[]; /* description text ids */
extern u8 sText8Font[][10]; // TODO: array or struct?
extern u8 sText12Font[][14]; // TODO: array or struct?
extern u32 *sTransitionRooms[]; // transition rooms list
extern u32 **sAreaRoomTables[]; // List of room pointers
extern u8 sChronomageDestinationRoom[];
extern u8 sObject15DestinationRoom0[];
extern u8 sChaosDestinationRoom[];
extern u8 sObject15DestinationRoom1[];
extern u8 sTimedParticleFrames[];
extern u8 *const sPlayerFlashInfo[];
extern struct BgMapMetadata *const sPauseMenuBgMaps[];
/* Cursor frames for the four entries unk_4FC..4FF (ButtonConfigMenu_DrawCursor). */
extern const u8 sButtonConfigCursorFrames[];
extern u8 *sMenuCursorPalette;
extern const u16 sButtonConfigButtons[];
extern const u8 sButtonConfigDefaultChoices[4];
extern const u8 sButtonConfigChoiceSlots[4];
extern u8 sGatePaletteAnim0[];
extern u8 sGatePaletteAnim1[];
extern const s32 sUnk_0852559C[];
extern const s32 sBgParticleCounts[];
extern const u16 sUnk_085255C4[];
extern const u8 sUnk_085255DC[];
extern struct WarpRoomEntry sWarpRoomEntries[];
extern u8 sUnk_08526370[][4];
extern u8 sObject16PaletteAnim[];
extern struct AnimScript sObject16EffectAnimScriptVariant0[];
extern struct AnimScript sObject16EffectAnimScript[];
extern const s16 sObject22LevelThresholds[];
extern u8 *sObject22LevelPatterns[];
extern u8 sObject25AnimScript0[];
extern u8 sObject25AnimScript1[];
extern u8 sObject25AnimScript2[];
extern const u32 sObject25PointIndices[];
extern u8 sObject2BPaletteAnim[];
extern u8 sObject2EAnimScript[];
extern struct BgMapMetadata *sShopBgMaps[];
extern const s8 sEventScene03UnusedDebris[];
extern const s8 sEventScene03Debris[];
extern const u8 sUnk_0852682C[];
extern u8 sTextWindowScrollOffsets[];
extern struct AnimScript sManticoreWalkAnimScript;
extern u8 sGiantGhostPaletteAnim[];
extern const u8 *const sGolemPartPoses[];
extern const s32 *const sGolemPartAngles[];
extern struct AnimScript sHeadhunterAnimScript0;
extern struct AnimScript sHeadhunterAnimScript1;
extern struct AnimScript sHeadhunterAnimScript2;
extern u8 sPeepingEyeMoveSteps[];
extern u8 *sPeepingEyeAnimSets[];
extern const u8 *const sGreatArmorPartPoses[];
extern const s32 *const sGreatArmorPartAngles[];
extern const s8 *const sDevilPartPoses[];
extern const s32 *const sDevilPartAngles[];
extern const u8 *const sCreakingSkullPartPoses[];
extern const s32 *const sCreakingSkullPartAngles[];
extern u8 sBiphronShotAnimScript[];
extern const u8 *const sStolasPartPoses[];
extern const s32 *const sStolasPartAngles[];
extern u8 sLegionAnimScript0[];
extern u8 sLegionAnimScript1[];
extern u8 sLegionAnimScript2[];
extern u8 sLegionPaletteAnim0[];
extern u8 sLegionPaletteAnim1[];
extern struct AnimScript sLegionAnimScript3[];
extern struct AnimScript sLegionDestructibleFragmentAnimScript[];
extern u8 sUnk_08528340[];
extern struct AnimScript sKyomaDemonHitAnimScript;
extern u8 sChaos2PaletteSequence0[];
extern u8 sChaos2PaletteSequence1[];
extern u8 sChaos2PaletteSequence2[];
extern u8 sChaos2PaletteSequence3[];
extern struct AnimScript sChaos2SparkAnimScript;
extern struct AnimScript sTsuchinokoHitAnimScript;
extern struct AnimScript sSkyFishFlyAnimScript;
extern struct AnimScript sSkyFishHitAnimScript;
extern const s16 *const sBigGolemPartPoses[];
extern const s32 *const sBigGolemPartAngles[];
extern u8 sUnk_08528604[];
extern u8 sWaiterSkeletonWalkProfile[];
extern s32 sUnk_08528614[];
extern u8 sSkullArcherWalkProfile[];
extern u8 sMinotaurWalkProfile[];
/* Axe Armor walk speeds per animation frame. */
extern u8 sAxeArmorWalkProfile[];
extern u8 sDiscArmorWalkProfile[];
extern u8 sBomberRockArmorWalkProfile[];
extern const u8 sBaloreEyeAnimOffsets[];
extern const u8 sBalorePartAnimOffsets[];
extern const u8 sUnk_085287D6[4];
extern u8 sChronomageWalkProfile[];
extern u8 sDeadWarriorWalkProfile[];
extern u8 sNightmareWalkProfile[];
extern u8 sStudentWitchWalkProfile[];
extern u8 sWitchGroundProjectileWalkProfile[];
extern const u8 sUnk_085288A8[][4];
extern u8 sKillerMantleWalkProfile[];
extern u8 sCockatriceFamilyWalkProfile[];
extern u8 sEvilButcherWalkProfile[];
extern u8 sKickerSkeletonWalkProfile[];
extern u8 sSlimeWalkProfile[];
extern const u16 sSlimeSurfaceTurnTargets[];
extern s16 sSkeletonWalkProfile[];
extern const s8 sSkeletonDeathPieceOffsets[][6];
extern s32 sUnk_08528A34[][9];
/* 6 bytes per child index; byte 1 is a y offset (s8). */
extern s8 sKillerDollPartOffsets[][6];
/* 6 bytes per child index; byte 1 is a y offset (s8). */
extern s8 sLightningDollPartOffsets[][6];
extern s16 sBeamSkeletonWalkProfile[];
extern s8 sBeamSkeletonDeathPieceOffsets[][6];
extern const s8 sNeedlesDeathParts[5][6];
extern s16 sBuerWalkProfile[];
extern s16 sBaelWalkProfile[];
extern s16 sAlastorAttackSegments[][4];
extern s32 sUnk_08528C2C[][9];
extern s32 sMandragoraAttackBox[];
extern s32 sMandragoraTerminalPartAttackBox[];
extern const s8 sMandragoraDeathParts[5][6];
extern s16 sBelmontMoveProfile[];
extern u16 sBelmontMeleeSounds[];
extern struct AnimScript sRecoveryPickupParticleAnimScript[];

#endif /* DATA_4F1444_H */
