#ifndef DATA_4F1444_H
#define DATA_4F1444_H

/* Data of asm/data/data_4F1444.s used by C files. */

#include "types.h"
#include "doors.h"
#include "structs/ewram.h"

/* Flight parameters, e.g. sBaloreHandSteeringParams1 (BaloreHand_FadeIn). */
struct BaloreFlightSteeringParameters {
    /* 0x00 */ u16 offsetX;     // point relative to unk_508/unk_50A, mirrored when not flipX
    /* 0x02 */ u16 offsetY;
    /* 0x04 */ u16 targetAngle; // for unk_500, negated when not flipX
    /* 0x06 */ u16 turnStep;
    /* 0x08 */ s32 targetRadius;
    /* 0x0C */ s32 radiusStep;
    /* 0x10 */ s32 targetSpeed;
    /* 0x14 */ s32 stepSpeed;
};

struct BelmontSceneObject { s16 x,y; u8 event,pad5,kind,pad7; u16 param1,param2; };

struct FragmentSpec { s8 x,y; u8 frame; s8 vx,vy; u8 timer; } __attribute__((packed));

struct ArmPose {
    u16 angle;
    u16 dx;
    u16 dy;
    u8 layer;
    u8 flag;
};

struct GrahamSceneObject { s16 x,y; u8 unused[4]; u16 param1,param2; };

struct WeaponFrame {
    s16 frame;
    s16 pad;
    s32 angle;
    s8 dx;
    s8 dy;
    u8 pad2[2];
};

/* One movement step of sNemesisScriptedAttackSteps; dx == 0x7FFF ends the table. */
struct Unk_C70F0_Step {
    s16 dx;       // pixels over the whole step, mirrored with flag bit 1
    s16 dy;
    s16 rotation; // total affine angle change
    u16 frames;
};

struct PartPoint {
    u16 point;
    s8 dx;
    s8 dy;
};

struct Unk_085251A8 { u8 frame; u8 life; u16 pad; struct AnimScript *anim; };

struct ParticleTemplate {
    u8 animation, palette, layer, lifetime;
    s16 x, y;
    s32 vx, vy, ax, ay;
};

struct HitEffectEntry {
    u8 animation;
    u8 palette;
    u16 song;
};

struct GoldDrop { u16 amount; u16 pad_2; s32 message; u8 palette; u8 frame; u16 animated; };

struct TimedFragment {u32 frame;s16 x,y;u8 delay;u8 pad[3];};

struct PaletteTransition { void *data; u16 count,duration; };

/* Debris piece of EventSceneJuliusKeepsPromise_SpawnBlockDebris (8-byte records, 12 pieces). */
struct EventSceneJuliusKeepsPromiseDebrisDefinition {
    u8 frame;
    s8 dx;
    s8 dy;
    s8 vx;
    s8 vy;
    u8 initialTimer;
    u8 pad_6[2];
};

/* Palette animation step of EventSceneJuliusKeepsPromise_StepPaletteAnim (4 steps). */
struct EventSceneJuliusKeepsPromisePaletteStep {
    u8 *palette;
    u16 sourcePalette;
    u16 duration;
};

struct ParticleFrame {
    u8 *data;
    u16 size;
    u16 pad;
};

struct ParticleFrames {
    u16 count;
    u16 destination;
    struct ParticleFrame *frames;
};

struct ConversationTextList {
    u16 unk_0;
    u16 closingText;
    u16 count;
    u16 entries[1];
};

struct ConversationTextTable {
    u32 unk_0;
    struct ConversationTextList *lists[1];
};

struct BestiaryAttributeIcon {
    s32 tileColumn; // low four bits of the tile coordinate
    s32 tileRow; // row in the 16-column tile layout
};

struct ShopStockEntry {
    u8 type;
    u8 id;
    u8 pad_2[2];
};

struct ShopList {
    u8 count;
    u8 items[1];
};

struct WindowGfx {
    void *tiles;
    u8 *palette;
    void *layer;
};

/* Palette animation frame: palette data, palette slot, duration. */
struct BgPaletteCycleFrame {
    u8 *paletteResource;
    u16 sourcePalette;
    u16 duration;
};

/* Palette animation (variant spawnParam1 of object 0x07 type 1). */
struct BgPaletteCycle {
    u16 frameCount; // number of frames
    u16 destinationPalette;
    struct BgPaletteCycleFrame *frames;
};

struct BreakableBlockVariant {
    u16 graphicsIndex; // index into sBreakableBlockGraphics
    u8 paletteIndex; // palette
    u8 animFrame; // animation frame (animFrame)
    s8 collisionOffsetX;
    s8 collisionOffsetY;
    u8 collisionWidth;
    u8 collisionHeight;
    u8 spriteLayer; // layer (spriteLayer)
    u8 collisionMode; // mode (unk_4F8)
    u8 pad_A[0xC - 0xA];
};

struct BreakableBlockGraphics {
    void *graphics; // graphics
    void *palette; // palette
    void *spriteSheet; // animation set
};

struct TriggerZoneTrigger {
    u16 type;
    u16 unk_2;
    s32 value;
    s16 x;
    s16 y;
    u16 width;
    u16 height;
};

struct BreakablePropGraphics {
    void *graphics;
    void *palette;
    void *sprites;
};

struct BreakablePropVariant {
    u8 graphics;
    u8 palette;
    u8 animation;
    u8 firstPieceFrame; // BreakableProp_SpawnPieces
    u8 pieceCount;
    u8 overlayFrame; // drawn every other two frames when nonzero (BreakableProp_Draw)
    u16 skipGroundSnap;
    u32 box;
};

struct BossRushReward
{
    u32 maxValue;
    s32 item;
};

/* Object 0x12 variants (spawnParam0). */
struct RoomDecorationVariantDefinition {
    void *graphics; // graphics
    void *palette; // palette
    void *spriteSheet; // animation set
    u8 paletteIndex; // palette index
    u8 animationIndex; // animation
    u8 pad_E;
    u8 spriteLayer; // layer (spriteLayer)
    u16 flags; // flags: 1 OBJ mode 1, 2 room coordinates, 4 no animation, 8 only near the screen
    u8 pad_12[2];
    EntityFunc updateFunc; // update, NULL for the plain animation
};

struct ObjectVariant { u8 *graphics; void *palette; void *sprites; u8 animation; u8 paletteIndex; u16 delay; }; /* delay: frames at each end */

/* A spawn point of object 0x25 (16.16 world coordinates). */
struct LegionWalkingCorpsePoint {
    s32 x;
    s32 y;
};

extern u8 sUnk_085059BC[];
extern u8 sUnk_08505C94[];
extern const u16 sItemNameTextIds[]; /* name text ids */
extern const u16 sItemDescriptionTextIds[]; /* description text ids */
extern u8 sText8Font[][10]; // TODO: array or struct?
extern u8 sText12Font[][14]; // TODO: array or struct?
extern u32 *sTransitionRooms[]; // transition rooms list
extern u32 **sAreaRoomTables[]; // List of room pointers
extern u8 sDebugHitTestRoom[];
extern u8 sDebugEventMenuRoom[];
extern u8 sDebugEventRoom12[];
extern u8 sDebugEventRoom3[];
extern u8 sDebugEventRoom1[];
extern u8 sDebugExitOptionsRoom1[];
extern u8 sDebugExitOptionsRoom2[];
extern u8 sDebugEventRoom2[];
extern u8 sDebugEventRoom11[];
extern u8 sDebugExitOptionsRoom3[];
extern u8 sChronomageDestinationRoom[];
extern u8 sDebugExitOptionsRoom16[];
extern u8 sDebugExitOptionsRoom4[];
extern u8 sDebugEventRoom9[];
extern u8 sChaoticRealmPortalDestinationRoom0[];
extern u8 sDebugRoomRoom[];
extern u8 sDebugEventRoom5[];
extern u8 sDebugExitOptionsRoom6[];
extern u8 sDebugEventRoom4[];
extern u8 sDebugEventRoom6[];
extern u8 sDebugExitOptionsRoom7[];
extern u8 sDebugExitOptionsRoom14[];
extern u8 sDebugEventRoom7[];
extern u8 sDebugExitOptionsRoom8[];
extern u8 sDebugEventRoom0[];
extern u8 sDebugEventRoom8[];
extern u8 sDebugExitOptionsRoom9[];
extern u8 sDebugEventRoom10[];
extern u8 sDebugExitOptionsRoom11[];
extern u8 sChaosDestinationRoom[];
extern u8 sDebugExitOptionsRoom10[];
extern u8 sChaoticRealmPortalDestinationRoom1[];
extern u8 sDebugEventRoom14[];
extern u8 sDebugEventRoom17[];
extern u8 sDebugEventMenuRoom2[];
extern u8 sDebugTestMenuRoom[];
extern u8 sBossRushMenuRoom[];
extern u8 sDebugExitOptionsRoom12[];
extern struct GoldDrop sGoldDrops[];
extern u8 sTimedParticleFrames[];
extern struct Unk_085251A8 sUnk_085251A8[];
extern u8 *const sPlayerFlashInfo[];
extern struct ParticleTemplate *sParticleTemplateLists[];
extern struct BgMapMetadata *const sPauseMenuBgMaps[];
/* Cursor frames for the four entries unk_4FC..4FF (ButtonConfigMenu_DrawCursor). */
extern const u8 sButtonConfigCursorFrames[];
extern u8 *sMenuCursorPalette;
extern const u16 sButtonConfigButtons[];
extern const u8 sButtonConfigDefaultChoices[4];
extern const u8 sButtonConfigChoiceSlots[4];
extern const struct BestiaryAttributeIcon sBestiaryAttributeIcons[];
extern u8 sGatePaletteAnim0[];
extern u8 sGatePaletteAnim1[];
extern const s32 sUnk_0852559C[];
extern const s32 sBgParticleCounts[];
extern const u16 sUnk_085255C4[];
extern const u8 sUnk_085255DC[];
extern struct BgPaletteCycle sBgPaletteCycles[];
extern struct WarpRoomEntry sWarpRoomEntries[];
extern struct BreakableBlockGraphics sBreakableBlockGraphics[];
extern struct BreakableBlockVariant sBreakableBlockVariants[];
extern struct TriggerZoneTrigger *sTriggerZoneTriggerLists[];
extern struct BreakablePropGraphics sBreakablePropGraphics[];
extern struct BreakablePropVariant sBreakablePropVariants[];
extern u8 sUnk_08526370[][4];
extern struct BossRushReward sBossRushRewards[];
extern const struct RoomDecorationVariantDefinition sRoomDecorationVariants[];
extern u8 sChaoticRealmVortexPaletteAnim[];
extern struct AnimScript sChaoticRealmVortexEffectAnimScriptVariant0[];
extern struct AnimScript sChaoticRealmVortexEffectAnimScript[];
extern const s16 sInputChallengePlatformLevelThresholds[];
extern u8 *sInputChallengePlatformLevelPatterns[];
extern struct ObjectVariant sVerticalPlatformVariants[];
extern struct ObjectVariant sHorizontalPlatformVariants[];
extern u8 sLegionWalkingCorpseAnimScript0[];
extern u8 sLegionWalkingCorpseAnimScript1[];
extern u8 sLegionWalkingCorpseAnimScript2[];
extern const struct LegionWalkingCorpsePoint *const sLegionWalkingCorpsePoints[];
extern const u32 sLegionWalkingCorpsePointIndices[];
extern u8 sGardenAnimatedPropPaletteAnim[];
extern u8 sProximityBurstAnimScript[];
extern struct BgMapMetadata *sShopBgMaps[];
extern const struct EventSceneJuliusKeepsPromiseDebrisDefinition sEventSceneJuliusKeepsPromiseDebrisDefinitions[];
extern const struct EventSceneJuliusKeepsPromisePaletteStep sEventSceneJuliusKeepsPromisePaletteSteps[];
extern const s8 sEventScenePrologueBattleUnusedDebris[];
extern const s8 sEventScenePrologueBattleDebris[];
extern const u8 sUnk_0852682C[];
extern struct ParticleFrames sEventSceneEndingParticleFrames;
extern struct TimedFragment sEventSceneEndingTimedFragments[9];
extern struct PaletteTransition sEventSceneEndingPaletteTransitions[];
extern struct ConversationTextTable sConversationTextTable;
extern const struct ShopStockEntry sShopStock[];
extern struct ShopList *const sShopLists[];
extern struct WindowGfx sTextWindowGfx[];
extern u8 sTextWindowScrollOffsets[];
extern const struct HitEffectEntry sHitEffects[];
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
extern struct WeaponFrame *sMinotaurAxeFrames[];
extern u8 sMinotaurWalkProfile[];
/* Axe Armor walk speeds per animation frame. */
extern u8 sAxeArmorWalkProfile[];
extern u8 sDiscArmorWalkProfile[];
extern u8 sBomberRockArmorWalkProfile[];
extern u8 sBomberArmorBombWalkProfile[];
extern const struct BaloreFlightSteeringParameters sBaloreHandSteeringParams0[];
extern const struct BaloreFlightSteeringParameters sBaloreHandSteeringParams1[];
extern const struct BaloreFlightSteeringParameters sBaloreHandSteeringParams2[];
extern const struct BaloreFlightSteeringParameters sBaloreHandSwipeParams[];
extern const struct BaloreFlightSteeringParameters sBaloreHandSteeringParams3[];
extern const u8 sBaloreEyeAnimOffsets[];
extern const u8 sBalorePartAnimOffsets[];
extern const u8 sUnk_085287D6[4];
extern const struct BaloreFlightSteeringParameters sBaloreHandBSteeringParams0[];
extern const struct BaloreFlightSteeringParameters sBaloreHandBSteeringParams1[];
extern const struct BaloreFlightSteeringParameters sBaloreHandBSteeringParams2[];
extern u8 sChronomageWalkProfile[];
extern u8 sDeadWarriorWalkProfile[];
extern u8 sNightmareWalkProfile[];
extern u8 sStudentWitchWalkProfile[];
extern u8 sWitchGroundProjectileWalkProfile[];
extern const u8 sUnk_085288A8[][4];
extern u8 sKillerMantleWalkProfile[];
extern u8 sCockatriceFamilyWalkProfile[];
extern struct ArmPose sDeathWeaponArmPoses[];
extern u8 sEvilButcherWalkProfile[];
extern u8 sKickerSkeletonWalkProfile[];
extern u8 sSlimeWalkProfile[];
extern const u16 sSlimeSurfaceTurnTargets[];
extern const struct PartPoint sSkeletonPartPoints[];
extern s16 sSkeletonWalkProfile[];
extern const s8 sSkeletonDeathPieceOffsets[][6];
extern struct Unk_C70F0_Step sNemesisScriptedAttackSteps[];
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
extern struct BelmontSceneObject sBelmontSceneObjects[];
extern u16 sBelmontMeleeSounds[];
extern struct FragmentSpec sFragmentSpecs[];
extern struct GrahamSceneObject sGrahamSceneObjects[];
extern struct AnimScript sRecoveryPickupParticleAnimScript[];
extern u8 sDebugTestEnemyAnimScripts[];

#endif /* DATA_4F1444_H */
