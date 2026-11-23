#include "code/code_08014548.h"
#include "player_entity.h"
#include "code_08009A0.h"
#include "code_08001004.h"
#include "code_08001194.h"
#include "code_08008750.h"
#include "code_0800CB00.h"
#include "code_0800F1FC.h"
#include "code/code_080211F0.h"
#include "code_08032444.h"
#include "code_08032E4C.h"
#include "code_08033CAC.h"
#include "code_08035930.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "gba.h"
#include "macros.h"
#include "agb_sram.h"
#include "syscalls.h"

#include "constants/main.h"

#include "data/data_0E0334.h"
#include "data/data_0E3464.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "code/code_08060B98.h"
#include "structs/main.h"

// ROM data passed as pointers; previously hard-coded addresses in the draft.
extern u8 sSomaSpriteSheet[];
extern u8 sSomaGfx[];
extern u8 sUnk_080E12C8[];
extern u8 sJuliusGfx[];
extern u8 sSomaPalette[];
extern u8 sJuliusEntityPalette[];
extern u8 sJuliusPalette[];
extern u8 sJuliusSpriteSheet[];

/**
 * @brief When currentLevel is zero, initialize equipment, inventory,
 * base stats, button settings, HP/MP and the starting area/room
 * coordinates. Leave an already initialized level unchanged.
 */
void Player_InitNewGameIfNeeded(void)
{
    if (gEwramData->playerStatus.currentLevel == 0)
    {
        Player_ClearEquipmentAndSoulLists();

        gEwramData->playerStatus.equippedRedSoul = 0;
        gEwramData->playerStatus.equippedBlueSoul = 0;
        gEwramData->playerStatus.equippedYellowSoul = 0;

        Player_EquipItem(2, 0);
        Player_EquipItem(0, 0);
        Player_EquipItem(1, 0xFF);
        gEwramData->playerStatus.weaponInventory[0] = 1;
        gEwramData->playerStatus.armorInventory[0] = 1;
        Player_InitBaseLevelStats();
        ButtonConfig_ResetDefaults();

        gEwramData->playerStatus.currentHP = gEwramData->playerStatus.maxHP;
        gEwramData->playerStatus.currentMP = gEwramData->playerStatus.maxMP;

        gEwramData->gameState.currentArea = 0;
        gEwramData->gameState.currentRoom = 0;

        gEwramData->gameState.savedCameraX = 0x110;
        gEwramData->gameState.savedCameraY = 0x230;
        gEwramData->gameState.savedPlayerX = 0xD0;
        gEwramData->gameState.savedPlayerY = 0x6F;
    }
}

// TODO: investigate Soma_InitEntity and the data 
struct PlayerBodyBox {
    u8 x;
    u8 y;
    u8 width;
    u8 height;
};

extern struct PlayerBodyBox sSomaStandingBodyBox[];

struct PlayerAnimPaletteView {
    u8 anim00Palette;
    u8 anim01Palette;
    u8 pad_2[0x13 - 0x2];
    u8 anim13Palette;
};
extern struct PlayerAnimPaletteView sSomaAnimPalettes;

/**
 * @brief Initialize the character-0 player entity graphics, palette,
 * animation, body box, draw callback and contact setup. Entry 0
 * of sPlayerInitFuncs; preserve the existing animation-mode sequence.
 */
void Soma_InitEntity(struct EwramData_EntityData *entity)
{
    s32 gfx;
    s32 palette;
    struct PlayerBodyBox unk_0;

    gfx = ObjGfx_LoadResource(sSomaGfx, 0x10000);
    ObjPalette_LoadRange(sSomaPalette, 0, 1, 0);
    Sprite_Attach(entity, sSomaSpriteSheet, gfx, 0);
    entity->animScriptIndex = 0xFF;
    unk_0 = *sSomaStandingBodyBox;
    if (!(gEwramData->playerState.actionLockFlags & 0x800) && (entity->animScriptIndex != 0))
    {
        Anim_StartIndexed(entity, 0, 3, 1);
        HitBox_SetBody(entity, &unk_0);
        entity->ext.player.requestedAnim = 0xFF;
        entity->ext.player.actionFlags &= ~0x200000;
        palette = sSomaAnimPalettes.anim00Palette;
        if (entity->ext.player.animPalette != palette)
        {
            entity->ext.player.animPalette = palette;
            ObjPalette_LoadRange(sSomaPalette, palette, 1, 0);
        }
    }
    Anim_Update(entity);
    entity->drawFunc = (u32 *) Player_DrawWithOffset;
    Hit_Enable(entity, 1, Player_MarkAttackContact, 0);
    HitBox_SetBody(entity, &sSomaStandingBodyBox);
    entity->hit.flags |= 2;
    gEwramData->playerStatus.soulStateFlags = 0;
}

/**
 * @brief Initialize the character-1 player entity, level/equipment,
 * graphics, palettes and linked weapon setup. Entry 1 of
 * sPlayerInitFuncs; preserve resource order and fixed entity slot use.
 */
void Julius_InitEntity(struct EwramData_EntityData *entity)
{
    s32 gfx;
    s32 i;

    ButtonConfig_ResetDefaults();
    if (gEwramData->playerStatus.currentLevel > 1)
    {
        Player_SetLevel(1);
    }

    gfx = ObjGfx_LoadResource(sUnk_080E12C8, 0x10000);
    ObjPalette_LoadRange(sJuliusEntityPalette, 0, 1, 0);
    Sprite_Attach(entity, sJuliusSpriteSheet, gfx, 0);

    entity->animScriptIndex = 0xFF;
    Player_TryStartAnimation(entity, 0, 1);
    Anim_Update(entity);

    entity->drawFunc = (u32 *) Player_DrawMirrored;
    Hit_Enable(entity, 1, Player_MarkAttackContact, 0);
    Entity_UpdateBoxes(entity);

    gEwramData->playerStatus.soulStateFlags = 0x8000;
    Player_SetAbility(5, 1);

    for (i = 0; i < 8; i++)
    {
        gEwramData->playerState.activeBlueSoulEntities[i] = 0;
    }

    Hit_SetTimerOverride(0, 0x1E);
    Player_EquipItem(2, 0x32);
    Player_EquipItem(0, 0xFF);
    Player_EquipItem(1, 0xFF);
    Player_EquipSoulsAndRestore(0, 0, 0);

    ObjGfx_LoadResource(sJuliusGfx, 0x10000);
    ObjPalette_LoadRange(sJuliusPalette, 0, 1, 2);
    ObjPalette_LoadRange(sJuliusPalette, 1, 1, 3);
    EntityDelete(&gEwramData->entityData[0x11]);

    EntityCreateInRange(0x11, 0x11, (u32 *) Julius_UpdateSubweaponIcon);
    PLAYER_JULIUS_OVERLAY(entity) = Julius_CreateOverlaySprite();
    gEwramData->playerState.whipChain = Whip_CreateChain(0x01300000, 0, sJuliusGfx, PLAYER_JULIUS_OVERLAY(entity));
    Whip_DeactivateChain(gEwramData->playerState.whipChain);
}

// Soma_InitEntity, Julius_InitEntity
extern void (*sPlayerInitFuncs[])(struct EwramData_EntityData *);
// Soma_Update, Julius_Update
extern void (*sPlayerUpdateFuncs[])(struct EwramData_EntityData *);

/**
 * @brief 1487C | Create the player entity for the current character
 *
 * Resets the player's soul/weapon state, creates entity slot 0 with the
 * character's update function (sPlayerUpdateFuncs) and runs its init
 * function (sPlayerInitFuncs); gEwramData->playerState.player points to it.
 *
 * @param unused Unused
 */
void Player_Create(s32 unused)
{
    struct EwramData_EntityData *player;

    gEwramData->playerState.nbrActiveRedSoulEntities = 0;
    gEwramData->playerState.weaponGfx = -1;
    gEwramData->playerState.redSoulGfx = -1;
    gEwramData->playerState.blueSoulGfx = -1;
    gEwramData->playerState.weaponEntities[0] = 0;
    gEwramData->playerState.whipChain = 0;
    gEwramData->playerState.cachedWeaponGraphicsIndices[0] = -1;
    gEwramData->playerState.cachedWeaponGraphicsIndex1 = -1;

    RedSoul_CancelAll();
    BlueSoul_StopAll();

    player = EntityCreateInRange(0, 0, (u32*) sPlayerUpdateFuncs[gEwramData->playerStatus.currentCharacter]);
    sPlayerInitFuncs[gEwramData->playerStatus.currentCharacter](player);
    gEwramData->playerState.player = player;
    gEwramData->playerState.enemyTarget = player;

    Player_ClearStatusEffects(7);
    player->scrollWithBg = 1;
    player->spriteLayer = 0xE;
    player->ext.player.actionFlags = 0x300000;
    /* address of the default wall probe offset list (s8 offsets) */
    player->ext.player.wallProbes = 0x080E12DC;

    Player_CopyBaseStats();
    gEwramData->playerState.cameraAnchorX = 0x78;
    gEwramData->playerState.cameraAnchorY = 0x6F;
    gEwramData->playerState.collisionQueueCount = 0;
    player->ext.player.requestedAnim = 0xFF;
    player->ext.player.drawOffsetX = 0;
    player->ext.player.drawOffsetY = 0;
    player->ext.player.floorKind = 0xFF;
    player->ext.player.weaponSlot = 0;
    gEwramData->playerState.wallProbeCount = 4;
    gEwramData->playerState.wallProbeY0 = 0xF8;
    gEwramData->playerState.wallProbeY1 = 0xF4;
    gEwramData->playerState.wallProbeY2 = 0xEC;
    gEwramData->playerState.wallProbeY3 = 0xE4;
    gEwramData->playerState.actionLockFlags = 0;
    HudHPNumber_Create();
    gEwramData->gameState.timeStopTimer = 0;
}


static inline s32 Entity_TestFlag100000(struct EwramData_EntityData *entity)
{
    return entity->ext.player.actionFlags & 0x100000;
}

static inline s32 Entity_TestFlag1000(struct EwramData_EntityData *entity)
{
    return entity->ext.player.actionFlags & 0x1000;
}

static inline s32 CollisionProbe_TestBit1()
{
    return gUnk_03002CB0.collisionProbeFlags & 0x2;
}

static inline s32 CollisionProbe_TestSlopeBits()
{
    return gUnk_03002CB0.collisionProbeFlags & 0xC0;
}
