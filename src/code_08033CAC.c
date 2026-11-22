#include "code_08033CAC.h"
#include "code_08009A0.h"
#include "code_08001004.h"
#include "code_08001194.h"
#include "code_08008750.h"
#include "code_0800CB00.h"
#include "code_0800F1FC.h"
#include "code/code_08014548.h"
#include "code/code_080211F0.h"
#include "code_08032444.h"
#include "code_08032E4C.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "gba.h"
#include "macros.h"
#include "agb_sram.h"
#include "agb_multi_sio_sync.h"
#include "syscalls.h"

#include "constants/main.h"

#include "data/data_0E0334.h"
#include "data/data_0E3464.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"

/*
 * Whip chain entity (Whip_CreateChain, unk_4F4 = 8 WhipChainNode records):
 * active flag (Whip_Activate/DeactivateChain) and settle counter (frames the
 * anchor node has not moved; the node simulation stops at 120, the swing
 * helpers reset it). Each hit segment created by Whip_CreateChain stores the
 * index of the node it follows. Token-identical expansion.
 */
#define WHIP_CHAIN_ACTIVE(e) e->unk_500.part16.unk_500
#define WHIP_CHAIN_SETTLE(e) e->unk_500.part16.unk_502
#define WHIP_SEGMENT_NODE(e) e->unk_50C.part8.unk_50C

/*
 * Julius sub-weapons: the cross and axe projectiles spin by adding the spin
 * speed to the rotation angle passed to ObjAffine_Set; the holy water bottle
 * and its flame walk with Entity_Walk at the stored speed; the Grand Cross
 * scales its affine sprite by the stored 16.16 scale.
 */
#define SUBWEAPON_SPIN_ANGLE(e) e->unk_508.part16.unk_508
#define SUBWEAPON_SPIN_SPEED(e) e->unk_508.part16.unk_50A
#define SUBWEAPON_WALK_SPEED(e) e->unk_50C.whole
#define GRAND_CROSS_SCALE(e) e->unk_50C.whole

static inline s32 Player_GetExperienceForLevelInline(s32 level)
{
    // (3x + 8) * (x + 1) * (x)
    // 3x^3 + 11x^2 + 8x
    return level * (level + 1) * (level * 2 + 8 + level);
}

const u8 sLevelUpStrGains[] = {
    1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1
};

const u8 sLevelUpConGains[] = {
    1, 1, 1, 2, 2, 2, 2, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1
};

const u8 sLevelUpIntGains[] = {
    1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1
};

const u8 sLevelUpMaxMpGains[] = {
    5, 5, 6, 8, 10, 11, 12, 12, 13, 13, 13, 14, 15, 16, 16, 16, 17, 18, 19, 20
};


/**
 * @brief Apply earned levels up to 99 and return the number gained; invoke the existing stat-update hook after the loop, except for the initial level-cap return.
 */
s32 Player_ApplyEarnedLevels(struct PlayerStatus *status)
{
    s32 levelsGained;

    levelsGained = 0;
    if (status->currentLevel >= 99)
    {
        return 0;
    }

    for (levelsGained = 0; status->currentLevel < 99; levelsGained++)
    {
        // Equation is the following form: let x = status->currentLevel
        // (3x + 11) * (x + 2) * (x + 1)
        // 3x^3 + 20x^2 + 39x + 22
        if (Player_GetExperienceForLevelInline(status->currentLevel + 1) > status->currentExperience)
        {
            break;
        }

        status->maxHP += 12;
        status->maxMP += sLevelUpMaxMpGains[status->currentLevel / 5];
        status->baseStats[0] += sLevelUpStrGains[status->currentLevel / 5];
        status->baseStats[1] += sLevelUpConGains[status->currentLevel / 5];
        if (status->currentLevel & 3)
        {
            status->baseStats[2] += sLevelUpIntGains[status->currentLevel / 5];
        }
        else
        {
            status->baseStats[2] += 1;
        }
        status->baseStats[3] += 1;
        status->currentLevel += 1;
    }
    Player_RefreshEquipmentStats();
    return levelsGained;
}

/**
 * @brief Initialize level one, maximum HP/MP and base stats for the current character without changing current HP/MP or experience.
 */
void Player_InitBaseLevelStats(void)
{
    struct PlayerStatus *playerStatus;

    playerStatus = &gEwramData->playerStatus;
    playerStatus->currentLevel = 1;
    if (playerStatus->currentCharacter == 0)
    {
        playerStatus->maxHP = 320;
        playerStatus->maxMP = 80;
        playerStatus->baseStats[0] = 10;
        playerStatus->baseStats[1] = 12;
        playerStatus->baseStats[2] = 11;
        playerStatus->baseStats[3] = 9;
    }
    else
    {
        playerStatus->maxHP = 800;
        playerStatus->maxMP = 500;
        playerStatus->baseStats[0] = 15;
        playerStatus->baseStats[1] = 12;
        playerStatus->baseStats[2] = 10;
        playerStatus->baseStats[3] = 20;
    }
}

/**
 * @brief Return level*(level+1)*(3*level+8) in the original operation order; preserve unchecked signed inputs.
 */
s32 Player_GetExperienceForLevel(s32 level)
{
    // TODO: Player_GetExperienceForLevel is the same as Player_GetExperienceForLevelInline

    // (3x + 8) * (x + 1) * (x)
    // 3x^3 + 11x^2 + 8x
    return level * (level + 1) * (level * 2 + 8 + level);
}

/**
 * @brief Return the next-level polynomial threshold minus current experience, or zero at level 99; do not clamp a negative result.
 */
s32 Player_GetExperienceToNextLevel(struct PlayerStatus *status)
{
    if (status->currentLevel >= 99)
        return 0;

    return Player_GetExperienceForLevelInline(status->currentLevel + 1) - status->currentExperience;
}

/**
 * @brief Reset base level stats, seed experience from the supplied level polynomial and apply earned levels under the existing level cap.
 */
void Player_SetLevel(s32 level)
{
    struct PlayerStatus *playerStatus;

    playerStatus = &gEwramData->playerStatus;
    Player_InitBaseLevelStats();
    playerStatus->currentExperience = Player_GetExperienceForLevelInline(level);
    Player_ApplyEarnedLevels(playerStatus);
}

/**
 * @brief For a nonzero character, add 25/20/18 to the first three base stats and invoke the existing stat-update hook; repeated calls accumulate.
 */
void Player_ApplyJuliusStatGrowth(void)
{
    struct PlayerStatus *playerStatus;

    playerStatus = &gEwramData->playerStatus;
    if (playerStatus->currentCharacter != 0)
    {
        playerStatus->baseStats[0] += 25;
        playerStatus->baseStats[1] += 20;
        playerStatus->baseStats[2] += 18;
        Player_RefreshEquipmentStats();
    }
}

// TODO: proper struct usage
const u8 sUnk_080E1E1C[] = {
    0x02, 0x25, 0x01, 0x01, 0xA8, 0x04, 0x16, 0x08, 0xAC, 0x24, 0x16, 0x08, 0xB0, 0x44, 0x16, 0x08, 
    0xB4, 0x64, 0x16, 0x08, 0xB8, 0x84, 0x16, 0x08, 0xBC, 0xA4, 0x16, 0x08, 0xC0, 0xC4, 0x16, 0x08, 
    0xC4, 0xE4, 0x16, 0x08, 0xC8, 0x04, 0x17, 0x08, 0xCC, 0x24, 0x17, 0x08, 0xD0, 0x44, 0x17, 0x08, 
    0xD4, 0x64, 0x17, 0x08, 0xD8, 0x84, 0x17, 0x08, 0xDC, 0xA4, 0x17, 0x08, 0xE0, 0xC4, 0x17, 0x08, 
    0xE4, 0xE4, 0x17, 0x08, 0xE8, 0x04, 0x18, 0x08, 0xEC, 0x24, 0x18, 0x08, 0xF0, 0x44, 0x18, 0x08, 
    0xF4, 0x64, 0x18, 0x08, 0xF8, 0x84, 0x18, 0x08, 0xFC, 0xA4, 0x18, 0x08, 0x00, 0xC5, 0x18, 0x08, 
    0x04, 0xE5, 0x18, 0x08, 0x08, 0x05, 0x19, 0x08, 0x0C, 0x25, 0x19, 0x08, 0x10, 0x45, 0x19, 0x08, 
    0x14, 0x65, 0x19, 0x08, 0x18, 0x85, 0x19, 0x08, 0x1C, 0xA5, 0x19, 0x08, 0x20, 0xC5, 0x19, 0x08, 
    0x24, 0xE5, 0x19, 0x08, 0x28, 0x05, 0x1A, 0x08, 0x2C, 0x25, 0x1A, 0x08, 0x30, 0x45, 0x1A, 0x08, 
    0x34, 0x65, 0x1A, 0x08, 0x38, 0x85, 0x1A, 0x08, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF, 
    0x70, 0x5E, 0x77, 0x62, 0x79, 0x67, 0x00, 0x00, 0x14, 0x00, 0x5F, 0x1A, 0x10, 0x59, 0x1D, 0x00, 
    0xF2, 0x03, 0xF2, 0x03, 0xF2, 0x03, 0xF2, 0x03, 0xF2, 0x03, 0xF2, 0x03
};

/**
 * @brief Link play (gameState.linkPlay): create the partner player sprite
 * entity with its graphics and a second marker entity, and reset the
 * link frame counter.
 */
void LinkPartner_Create(void)
{
    struct EwramData_EntityData *child;

    child = EntityCreateInRange(0, 1, (u32 *) LinkPartner_Update);
    gEwramData->playerState.blueSoulGfx = (u32) ObjGfx_LoadResource((u8 *)sUnk_080E1E1C, 0x10000);
    ObjPalette_LoadRange((u8 *)0x082097D4, 6U, 1U, 3U);
    Sprite_Attach(child, (u8 *)0x080E11C4, (s32) gEwramData->playerState.blueSoulGfx, 3);
    child->spriteLayer = 0xF;
    child->roomActive = 1;
    child->unk_50C.part8.unk_50C = 1;
    gEwramData->playerState.linkFrameCounter = 0;
    gEwramData->playerState.enemyTarget = NULL;
    gEwramData->playerState.linkPartner = child;
    child = EntityCreateInRange(0x11, 0x11, (u32 *) LinkPartnerMarker_Update);
    gEwramData->playerState.linkTargetProxy = child;
    child->roomActive = 1;
}

/**
 * @brief Show the partner animation frame and facing received over the link
 * (unk_4DC) and create its weapon attack when the partner starts one.
 */
void LinkPartner_Update(struct EwramData_EntityData *entity)
{
    s32 prevFlags;
    struct LinkReceivedPlayerRecord *receivedState;
    u32 frame;

    frame = 0;
    receivedState = &gEwramData->linkReceivedState;
    prevFlags = entity->unk_4F4.whole;
    entity->roomActive = 1;

    switch (entity->state)
    {
        case 0:
            entity->animFrame = 0xFF;
            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            entity->state = 1;
            /* fallthrough */
        case 1:
            entity->unk_4F4.whole = 0;
            if (receivedState->flags & 2)
            {
                entity->unk_4F4.whole = 0x20;
            }
            if (receivedState->flags & 4)
            {
                entity->unk_4F4.whole |= 0x400;
            }
            if ((entity->unk_4F4.whole & 0x20) && !(prevFlags & 0x20) && (gEwramData->playerState.weaponEntities[entity->unk_50C.part8.unk_50C] == NULL))
            {
                Weapon_CreateAttackEntity(entity);
            }
            frame = receivedState->animFrame;
            entity->flipX = receivedState->flags & 1;
    }

    if (entity->animFrame != frame)
    {
        u8 *index = &entity->spriteSlot;
        u8 *sheet = gEwramData->objGfx[gEwramData->spriteSlots[*index].gfxEntry].source;
        void **tiles = (void **)(sheet + 4);
        s32 bank;

        entity->animFrame = frame;
        bank = frame >> 2;
        ObjTiles_LoadFrame(gEwramData->playerState.blueSoulGfx, tiles[bank], entity->animFrame, 0, 1, 1);
    }
}

/**
 * @brief Add the hit flag (player flag 0x80000) and the frame counter to the
 * own 8-byte state record (unk_4D4), queue it for sending and copy the
 * partner record (unk_4DC).
 */
void Link_ExchangePlayerState(void)
{
    struct EwramData_EntityData *player;
    struct LinkReceivedPlayerRecord *receivedState;
    struct LinkSendPlayerRecord *sendState;

    sendState = &gEwramData->linkSendState;
    receivedState = &gEwramData->linkReceivedState;
    player = gEwramData->playerState.player;

    receivedState->flags = 0;
    if (player->unk_4F4.whole & 0x80000)
    {
        sendState->flags |= 8;
    }
    sendState->frameCounter = gEwramData->playerState.linkFrameCounter;
    gEwramData->playerState.linkFrameCounter += 1;
    sendState->flags |= 0x10;

    Link_QueueSendBytes((u8*)&gEwramData->linkSendState, 8);
    Link_CopyReceivedBytes((u8*)&gEwramData->linkReceivedState, 8);
}

/**
 * @brief 340F0 | To document
 * 
 * @param entity To document
 * @param box To document
 */
void HitBox_SetHitRect(struct EwramData_EntityData *entity, struct HitBoxDef *box)
{
    s32 tmp;

    // TODO: is tmp necessary? or perhaps inline?
    if (!entity->flipX)
    {
        gEwramData->hitRectLeft = box->x + entity->posX.part16.integer;
        tmp = box->width;
        gEwramData->hitRectRight = gEwramData->hitRectLeft - 1 + tmp;
    }
    else
    {
        gEwramData->hitRectRight = entity->posX.part16.integer - box->x;
        tmp =  box->width;
        gEwramData->hitRectLeft = gEwramData->hitRectRight + 1 - tmp;
    }

    if (!entity->flipY)
    {
        gEwramData->hitRectTop = box->y + entity->posY.part16.integer;
        tmp = box->height;
        gEwramData->hitRectBottom = gEwramData->hitRectTop - 1 + tmp;
    }
    else
    {
        gEwramData->hitRectBottom = entity->posY.part16.integer - box->y;
        tmp = box->height;
        gEwramData->hitRectTop = gEwramData->hitRectBottom + 1 - tmp;
    }
}

struct LinkPartnerHitResult {
    u16 flags;
    u16 typeA;
    u16 typeB;
    u8 resA;
    u8 resB;
    u64 rectangle;
};

/**
 * @brief When the partner reports a hit (flag 8), run the hurt callback of
 * the shared target with a contact hit rectangle.
 */
void Link_ApplyPartnerHit(void)
{
    struct EwramData_EntityData* whipChain;
    struct LinkReceivedPlayerRecord* receivedState;
    struct EntityHit* targetHit;
    void* hurtFunc;
    struct HitBoxDef* boxes;
    struct LinkPartnerHitResult hitInfo;

    receivedState = &gEwramData->linkReceivedState;
    if ((receivedState->flags & 8) && (gEwramData->playerState.selectedTarget != NULL))
    {
        whipChain = gEwramData->playerState.whipChain;
        boxes = Hit_GetEntityBoxes(whipChain);
        targetHit = &gEwramData->playerState.selectedTarget->hit;
        HitBox_SetHitRect(whipChain, boxes);

        hitInfo.rectangle = *(u64*)&gEwramData->hitRectLeft;
        hitInfo.flags = 8;
        hitInfo.typeA = hitInfo.typeB = 0;

        hitInfo.typeB = 5;
        hitInfo.rectangle = *(u64*)&gEwramData->hitRectLeft;
        hitInfo.resA = 0;

        hurtFunc = targetHit->onHurt;
        if (hurtFunc != NULL)
        {
            ((void (*)(struct EwramData_EntityData*, struct EwramData_EntityData*, void*)) hurtFunc)(gEwramData->playerState.selectedTarget, whipChain, &hitInfo);
        }
    }
}

/**
 * @brief Fill the own state record (position, frame, facing, attack flags),
 * keep an 8-entry position history and place the partner sprite and
 * marker relative to the camera.
 */
void Link_UpdatePlayerSync(void)
{
    u16 *counter;
    s32 sampleIndex;
    s32 historyIndex;
    struct EwramData_EntityData *player;
    struct LinkSendPlayerRecord *sendState;
    struct LinkReceivedPlayerRecord *receivedState;
    s16 targetX;
    u16 roomY;
    u16 roomX;
    s16 sampleDist;
    s16 partnerDist;
    struct BgLayerState *bg;

    sendState = &gEwramData->linkSendState;
    receivedState = &gEwramData->linkReceivedState;
    player = gEwramData->playerState.player;

    roomX = GetEntityRoomXPositionInteger(player);
    roomY = GetEntityRoomYPositionInteger(player);
    sendState->roomX = roomX;
    sendState->roomY = roomY;
    sendState->animFrame = player->animFrame;

    sendState->flags = player->flipX;
    if ((player->state != 3) && (player->unk_4F4.whole & 0x20))
    {
        sendState->flags |= 2;
    }
    if (player->unk_4F4.whole & 0x400)
    {
        sendState->flags |= 4;
    }
    if (player->state == 0x10)
    {
        sendState->flags |= 0x20;
    }

    counter = &gEwramData->playerState.linkFrameCounter;
    sampleIndex = (receivedState->frameCounter - 1) & 7;
    gEwramData->playerState.linkSampleX = gEwramData->playerState.linkHistoryX[sampleIndex];
    gEwramData->playerState.linkSampleY = gEwramData->playerState.linkHistoryY[sampleIndex];

    historyIndex = (*counter - 1) & 7;
    gEwramData->playerState.linkHistoryX[historyIndex] = roomX;
    gEwramData->playerState.linkHistoryY[historyIndex] = roomY;

    gEwramData->playerState.linkPartner->posX.part16.integer = receivedState->roomX - (bg = &gEwramData->bgInfo[1])->xPos.part16.integer;
    gEwramData->playerState.linkPartner->posY.part16.integer = receivedState->roomY - bg->yPos.part16.integer;

    if (gEwramData->gameState.subMode != 0xE)
    {
        gEwramData->playerState.linkTargetProxy->posX.part16.integer = 0;
        gEwramData->playerState.linkTargetProxy->posY.part16.integer = 0;
        return;
    }

    gEwramData->playerState.linkTargetProxy->posX.part16.integer = gEwramData->playerState.linkSampleX - bg->xPos.part16.integer;
    gEwramData->playerState.linkTargetProxy->posY.part16.integer = gEwramData->playerState.linkSampleY - bg->yPos.part16.integer;

    if (gEwramData->playerState.selectedTarget != NULL)
    {
        targetX = gEwramData->playerState.selectedTarget->posX.part16.integer + bg->xPos.part16.integer;

        // TODO: macro
        sampleDist = ((s32) ((s16)gEwramData->playerState.linkSampleX - targetX) >= 0) 
            ? ((s16)gEwramData->playerState.linkSampleX - targetX) 
            : (-((s16)gEwramData->playerState.linkSampleX - targetX));

        partnerDist = ((s32) ((s16)receivedState->roomX - targetX) >= 0) 
            ? ((s16)receivedState->roomX - targetX) 
            : (-((s16)receivedState->roomX - targetX));

        if (sampleDist >= partnerDist)
        {
            if (partnerDist < sampleDist)
            {
                gEwramData->playerState.linkTargetProxy->posX.part16.integer = gEwramData->playerState.linkPartner->posX.part16.integer;
                gEwramData->playerState.linkTargetProxy->posY.part16.integer = gEwramData->playerState.linkPartner->posY.part16.integer;
            }
            else
            {
                gEwramData->playerState.linkTargetProxy->posX.part16.integer = gEwramData->playerState.selectedTarget->posX.part16.integer;
                gEwramData->playerState.linkTargetProxy->posY.part16.integer = gEwramData->playerState.selectedTarget->posY.part16.integer;
            }
            
        }
    }
}

/**
 * @brief Store the supplied entity pointer as the shared selected target, including NULL; preserve the absence of validation.
 */
void Player_SetSelectedTarget(struct EwramData_EntityData *entity)
{
    gEwramData->playerState.selectedTarget = entity;
}

/**
 * @brief In link play, set player flag 0x80000 so the hit is reported to the
 * partner (used by CreakingSkull and Gladiator damage handlers).
 */
void Link_MarkEnemyDamaged(void)
{
    if (gEwramData->gameState.linkPlay)
    {
        gEwramData->playerState.player->unk_4F4.whole |= 0x80000;
    }
}

/**
 * @brief 344E0 | To document
 * 
 */
void sub_080344E0(void)
{
    return;
}

/**
 * @brief Keep the partner marker entity alive.
 */
void LinkPartnerMarker_Update(struct EwramData_EntityData *entity)
{
    entity->roomActive = 1;
}

// TODO: this is a struct Unk_080E1ED8
const u8 sJuliusSubweaponInfo[] = {
    0xAD, 0x47, 0x03, 0x08, 0x00, 0x00, 0x19, 0x00, 0x02, 0x00, 0x64, 0x00, 0x21, 0x00, 0x00, 0x00,
    0x19, 0x4B, 0x03, 0x08, 0x00, 0x00, 0x0A, 0x00, 0x01, 0x00, 0x50, 0x00, 0x20, 0x00, 0x00, 0x00,
    0x5D, 0x4E, 0x03, 0x08, 0x00, 0x00, 0x0C, 0x00, 0x03, 0x00, 0x96, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x49, 0x4F, 0x03, 0x08, 0x01, 0x00, 0x96, 0x00, 0x01, 0x00, 0x78, 0x00, 0x20, 0x00, 0x00, 0x00, 
};

struct JuliusSubweaponIconDefinition {
    u8 frame;
    u8 palette;
    u8 pad_2[0x4 - 0x2];
};
const struct JuliusSubweaponIconDefinition sJuliusSubweaponIcons[] = {
    [0] = {
        .frame = 7,
        .palette = 3
    },
    [1] = {
        .frame = 8,
        .palette = 3
    },
    [2] = {
        .frame = 6,
        .palette = 2
    },
    [3] = {
        .frame = 9,
        .palette = 2
    },
};

// TODO: what is this data?
const u8 sUnk_080E1F28[44] = {
    0xE8, 0x88, 0x30, 0x78, 0xC4, 0xB0, 0x78, 0x28, 0x00, 0x10, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 
    0x00, 0x30, 0x00, 0x00, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x00, 
    0x00, 0x02, 0x00, 0x00, 0x54, 0x00, 0x00, 0x00, 0xFE, 0xFE, 0x04, 0x04, 
};

/**
 * @brief Julius HUD icon at (120, 12) showing the selected sub-weapon
 * (equippedRedSoul, cycled 0..3 by the guardian button) with frame
 * and palette from sJuliusSubweaponIcons; hidden by gameState.stateFlags bit 0x20.
 */
void Julius_UpdateSubweaponIcon(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
            entity->posX.part16.integer = 0x78;
            entity->posY.part16.integer = 0xC;
            gEwramData->playerState.redSoulGfx = ObjGfx_LoadResource((u8 *)0x081FA2B8, 0x10000);
            Sprite_Attach(entity, (u8 *)0x08252CCC, gEwramData->playerState.redSoulGfx, 3);
            entity->subState = gEwramData->playerStatus.equippedRedSoul;
            entity->animFrame = sJuliusSubweaponIcons[entity->subState].frame;
            entity->palette = sJuliusSubweaponIcons[entity->subState].palette;
            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            entity->spriteLayer = 0;
            entity->flipX = 1;
            entity->state = 1;
            /* fallthrough */
        case 1:
            if (entity->subState != gEwramData->playerStatus.equippedRedSoul)
            {
                entity->subState = gEwramData->playerStatus.equippedRedSoul;
                entity->animFrame = sJuliusSubweaponIcons[entity->subState].frame;
                entity->palette = sJuliusSubweaponIcons[entity->subState].palette;
            }
            break;
    }

    if (gEwramData->gameState.stateFlags & 0x20)
    {
        entity->hidden = 1;
    }
    else
    {
        entity->hidden = 0;
    }
    entity->roomActive = 1;
}

/**
 * @brief On up + new attack press, unless flags 0x540 block it, cast the
 * sub-weapon selected by equippedRedSoul from Julius's RedSoul_GetInfo
 * table when MP exceeds its cost and the active limit allows. Enter
 * state 2 (animation 0x17), or state 10 for entries with unk_4 set.
 */
void Julius_HandleSubweaponInput(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;
    s8 slot;
    struct RedSoulActivationView *info;

    if (entity->unk_4F4.whole & 0x540)
    {
        return;
    }

    if (!(gEwramData->inputData.playerHeldInput & 0x40))
    {
        return;
    }

    if (!(gEwramData->inputData.playerNewInput & gEwramData->playerStatus.attackButton))
    {
        return;
    }

    info = RedSoul_GetInfo(gEwramData->playerStatus.equippedRedSoul);
    if (gEwramData->playerState.nbrActiveRedSoulEntities == 0)
    {
        if (gEwramData->playerState.lastUsedRedSoul != gEwramData->playerStatus.equippedRedSoul)
        {
            gEwramData->playerState.lastUsedRedSoul = gEwramData->playerStatus.equippedRedSoul;
        }
    }

    if (gEwramData->playerState.lastUsedRedSoul != gEwramData->playerStatus.equippedRedSoul)
    {
        return;
    }

    if (info->updateFunc == 0)
    {
        return;
    }

    if (info->mpCost >= gEwramData->playerStatus.currentMP)
    {
        return;
    }

    if (RedSoul_GetActiveLimit() <= gEwramData->playerState.nbrActiveRedSoulEntities)
    {
        return;
    }

    slot = RedSoul_FindFreeEntitySlot();
    if (slot < 0)
    {
        return;
    }

    child = EntityCreateInRange(2, 0x10, info->updateFunc);
    if (child == NULL)
        return;

    gEwramData->playerStatus.currentMP -= info->mpCost;
    gEwramData->playerState.activeRedSoulEntities[slot] = child;

    if (info->castAnimKind == 0)
    {
        Player_TryStartAnimation(entity, 0x17, 0);
        entity->state = 2;
    }
    else
    {
        Player_SetContactImmunity(2);
        entity->state = 0xA;
        entity->subState = 0;
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
        entity->velY.whole = 0;
        entity->accelY.whole = 0;
        entity->timer = 0x20;
    }

    entity->unk_4F4.whole |= 0x100;
    child->unk_4F8.unk_4F8_entity = entity;
    child->unk_4FC.part16.unk_4FE[0] = slot;
    child->unk_500.part16.unk_500 = gEwramData->playerStatus.equippedRedSoul;
    child->unk_500.part16.unk_502 = info->unk_E;
    child->roomActive = 1;
    entity->unk_4F4.whole &= ~0x200060;
}

/**
 * @brief Sub-weapon entry 0 (Cross): after a delay, throw a rotating cross
 * forward at 0x34000 with reverse acceleration so it returns; trail
 * particles, delete when it reaches the player or
 * leaves the screen margin (Entity_IsOffscreenOrInactive).
 */
void JuliusCross_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *effect;
    struct EwramData_EntityData *owner;
    u16 affineIndex;
    u8 temp_r4;

    owner = entity->unk_4F8.unk_4F8_entity;

    switch (entity->state)
    {
        case 0:
            gEwramData->playerState.nbrActiveRedSoulEntities += 1;
            entity->timer = 0xE;
            entity->state = 1;
            /* fallthrough */
        case 1:
            if (owner->unk_4F4.whole & 0x80)
            {
                RedSoul_RemoveActiveEntity(entity);
                EntityDelete(entity);
                return;
            }

            if (entity->timer != 0)
            {
                entity->timer -= 1;
                break;
            }

            gEwramData->playerState.redSoulGfx = ObjGfx_LoadResource((u8 *)0x081FA2B8, 0x10000);
            Sprite_Attach(entity, (u8 *)0x08252CCC, (s32) gEwramData->playerState.redSoulGfx, 3);
            entity->animFrame = 1;
            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            entity->spriteLayer = 0xD;
            Hit_SetTimerOverride(1, 0x19);

            affineIndex = (u16) ObjAffine_Alloc(0);
            if ((s32) (affineIndex << 0x10) >= 0)
            {
                entity->affineIndex = affineIndex;
                entity->affine = 1;
                Entity_PlaceRelativeTo(entity, owner, 0x14, -0x1A);
                if ((RandomNumberGenerator() & 0x3F) < 0x18)
                {
                    PlaySong(0x1ED);
                }
                PlaySong(0xE4);

                entity->velX.whole = 0x34000;
                entity->accelX.whole = -0x1000;
                SUBWEAPON_SPIN_SPEED(entity) = 0x678;
                if (owner->flipX)
                {
                    entity->velX.whole = -entity->velX.whole;
                    entity->accelX.whole = -entity->accelX.whole;
                    SUBWEAPON_SPIN_SPEED(entity) = -SUBWEAPON_SPIN_SPEED(entity);
                }

                Hit_Enable(entity, 5, NULL, 0);
                HitBox_SetAttackRaw(entity, 0x1414F6F6);
                entity->hit.flags |= 4;
                entity->scrollWithBg = 1;
                entity->state = 2;
            }
            else
            {
                RedSoul_RemoveActiveEntity(entity);
                EntityDelete(entity);
                return;
            }
            /* fallthrough */
        case 2:
            if (Entity_IsInRect(gEwramData->playerState.player, (s16) (entity->posX.part16.integer - 8), (s16) (entity->posY.part16.integer + 0x10), 0x10, 0x14) != 0)
            {
                if (entity->affineIndex || entity->affine)
                {
                    ObjAffine_Free(entity->affineIndex);
                }
                RedSoul_RemoveActiveEntity(entity);
                EntityDelete(entity);
                return;
            }

            entity->timer += 1;
            if (!(entity->timer & 3))
            {
                effect = Effect_CreateFromTable(entity->posX.whole, entity->posY.whole + RandomSigned(0x100000), 0, 0, 4, 6);
                if (effect != NULL)
                {
                    effect->spriteLayer = entity->spriteLayer + 1;
                }
            }

            if (entity->subState == 0)
            {
                if (!entity->flipX)
                {
                    if (entity->velX.whole < -0x10000)
                    {
                        entity->subState = 1;
                    }
                }
                else if (entity->velX.whole > 0x10000)
                {
                    entity->subState = 1;
                }
            }

            EntityAccelerateAndMove(entity, 0x80000);
            SUBWEAPON_SPIN_ANGLE(entity) += SUBWEAPON_SPIN_SPEED(entity);
            ObjAffine_Set(SUBWEAPON_SPIN_ANGLE(entity), 0x10000, 0x10000, entity->flipX ^ 1, 0, entity->affineIndex);
            break;
    }

    if (Entity_IsOffscreenOrInactive(entity, 6) != 0)
    {
        if (entity->affineIndex || entity->affine)
        {
            ObjAffine_Free(entity->affineIndex);
        }
        RedSoul_RemoveActiveEntity(entity);
        EntityDelete(entity);
    }
}

/**
 * @brief Holy Water flame: walk along the floor with animation 2 and the
 * frame attack box until the animation ends.
 */
void JuliusHolyWaterFlame_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
            gEwramData->playerState.redSoulGfx = ObjGfx_LoadResource((u8 *)0x081FA2B8, 0x10000);
            Sprite_Attach(entity, (u8 *)0x08252CCC, gEwramData->playerState.redSoulGfx, 3);
            Anim_StartIndexed(entity, 2, 0, 0);
            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            entity->spriteLayer = 0xD;
            SUBWEAPON_WALK_SPEED(entity) = 0x12000;
            Hit_Enable(entity, 5, NULL, 0);
            entity->hit.flags |= 4;
            entity->state += 1;
            /* fallthrough */
        case 1:
            Entity_Walk(entity, -SUBWEAPON_WALK_SPEED(entity), (void *)0x084F1438, 0xC);
            if (Anim_Update(entity) == 3)
            {
                RedSoul_RemoveActiveEntity(entity);
                EntityDelete(entity);
                break;
            }
            Entity_SetAttackBoxFromFrame(entity);
            /* fallthrough */
        default:
            if (Entity_IsOffscreenOrInactive(entity, 2) != 0)
            {
                RedSoul_RemoveActiveEntity(entity);
                EntityDelete(entity);
            }
            break;
    }
}

/**
 * @brief Sub-weapon entry 1 (Holy Water): after a delay, drop the flask
 * along the floor walker; on landing hide it and spawn seven flame
 * entities at intervals, then delete.
 */
void JuliusHolyWater_Update(struct EwramData_EntityData *entity)
{
    s32 walkResult;
    struct EwramData_EntityData *child;
    struct EwramData_EntityData *owner;
    s32 tmp = 0x10000;

    owner = entity->unk_4F8.unk_4F8_entity;

    switch (entity->state)
    {
        case 0:
            gEwramData->playerState.nbrActiveRedSoulEntities += 1;
            entity->timer = 0xE;
            entity->state = 1;
            /* fallthrough */
        case 1:
            if (owner->unk_4F4.whole & 0x80)
            {
                RedSoul_RemoveActiveEntity(entity);
                EntityDelete(entity);
                return;
            }
            if (entity->timer != 0)
            {
                entity->timer -= 1;
                break;
            }
            gEwramData->playerState.redSoulGfx = ObjGfx_LoadResource((u8 *)0x081FA2B8, 0x10000);
            Sprite_Attach(entity, (u8 *)0x08252CCC, gEwramData->playerState.redSoulGfx, 3);
            entity->animFrame = 2;
            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            entity->spriteLayer = 0xD;
            Hit_SetTimerOverride(1, 0x14);
            Entity_PlaceRelativeTo(entity, owner, 0x14, -0x1A);
            entity->velY.whole = -0x8000;
            entity->accelY.whole = 0x4000;
            SUBWEAPON_WALK_SPEED(entity) = tmp; // needed to match
            entity->scrollWithBg = 1;
            entity->state = 2;
            break;

        case 2:
            walkResult = Entity_Walk(entity, -SUBWEAPON_WALK_SPEED(entity), (void *)0x084F1438, 8);
            if (walkResult & 3)
            {
                SUBWEAPON_WALK_SPEED(entity) = 0;
            }
            if (walkResult & 8)
            {
                entity->state = 3;
                entity->hidden = 1;
                entity->timer = 0xA;
                PlaySong(0x12C);
            }
            break;

        case 3:
            entity->timer += 1;
            if (entity->timer > 0xA)
            {
                child = RedSoul_CreateEntity(entity, (u32 *) JuliusHolyWaterFlame_Update);
                if (child != NULL)
                {
                    child->posX.whole = entity->posX.whole;
                    child->posY.whole = entity->posY.whole;
                    child->flipX = entity->flipX;
                }
                if (entity->subState > 1)
                {
                    PlaySong(0x91);
                }
                if (entity->subState++ > 5)
                {
                    RedSoul_RemoveActiveEntity(entity);
                    EntityDelete(entity);
                    return;
                }
                entity->timer = 0;
            }
            break;
    }

    if (Entity_IsOffscreenOrInactive(entity, 3) != 0)
    {
        RedSoul_RemoveActiveEntity(entity);
        EntityDelete(entity);
    }
}

/**
 * @brief Axe copy: rotating upward arc (velY -0x58000, gravity 0x3000)
 * in the facing direction; substate 0 has the attack box, 1/2 are
 * semi-transparent trail copies (2 also uses drawSkip).
 */
void JuliusAxeProjectile_Update(struct EwramData_EntityData *entity)
{
    u16 affineIndex;

    switch (entity->state)
    {
        case 0:
            gEwramData->playerState.redSoulGfx = ObjGfx_LoadResource((u8 *)0x081FA2B8, 0x10000);
            Sprite_Attach(entity, (u8 *)0x08252CCC, gEwramData->playerState.redSoulGfx, 2);
            entity->animFrame = 0;
            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            entity->spriteLayer = 0xD;

            affineIndex = ObjAffine_Alloc(0);
            if ((s16)affineIndex < 0)
            {
                RedSoul_RemoveActiveEntity(entity);
                EntityDelete(entity);
                return;
            }

            entity->affineIndex = affineIndex;
            entity->affine = 1;
            entity->velY.whole = -0x58000;
            entity->accelY.whole = 0x3000;
            entity->velX.whole = -0x12000;
            SUBWEAPON_SPIN_SPEED(entity) = -0xABC;
            if (!entity->flipX)
            {
                entity->velX.whole = -entity->velX.whole;
                SUBWEAPON_SPIN_SPEED(entity) = -SUBWEAPON_SPIN_SPEED(entity);
            }

            switch (entity->subState)
            {
                case 0:
                    Hit_Enable(entity, 5, NULL, 0);
                    HitBox_SetAttackRaw(entity, 0x1414F6F6);
                    entity->hit.flags |= 4;
                    break;
                case 2:
                    entity->drawSkip = 1;
                    /* fallthrough */
                case 1:
                    entity->objMode = 1;
                    entity->spriteLayer = 0xE;
                    break;
            }

            entity->state = 1;
            /* fallthrough */
        case 1:
            EntityAccelerateAndMove(entity, 0x80000);
            SUBWEAPON_SPIN_ANGLE(entity) += SUBWEAPON_SPIN_SPEED(entity);
            ObjAffine_Set(SUBWEAPON_SPIN_ANGLE(entity), 0x10000, 0x10000, entity->flipX ^ 1, 0, entity->affineIndex);
            break;
    }

    if (Entity_IsOffscreenOrInactive(entity, 3) != 0)
    {
        if (entity->affineIndex || entity->affine)
        {
            ObjAffine_Free(entity->affineIndex);
        }
        RedSoul_RemoveActiveEntity(entity);
        EntityDelete(entity);
    }
}

/**
 * @brief Sub-weapon entry 2 (Axe): after a delay, spawn three arc copies
 * three updates apart (substate 0 hits; 1 and 2 are blended trail
 * copies), then delete.
 */
void JuliusAxe_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;
    struct EwramData_EntityData *owner;

    owner = entity->unk_4F8.unk_4F8_entity;

    switch (entity->state)
    {
        case 0:
            gEwramData->playerState.nbrActiveRedSoulEntities += 1;
            entity->timer = 0xA;
            entity->state = 1;
            break;

        case 1:
            if ((owner->unk_4F4.whole & 0x80))
            {
                RedSoul_RemoveActiveEntity(entity);
                EntityDelete(entity);
                return;
            }

            if (entity->timer != 0)
            {
                entity->timer -= 1;
                return;
            }

            Hit_SetTimerOverride(1, 0x3C);
            Entity_PlaceRelativeTo(entity, owner, 6, -0x14);
            PlaySong(0x1ECU);
            PlaySong(0xE3U);
            entity->scrollWithBg = 1;
            entity->state = 2;
            /* fallthrough */
        case 2:
            if (entity->timer == 0)
            {
                child = RedSoul_CreateEntity(entity, (u32 *) JuliusAxeProjectile_Update);
                if (child != NULL)
                {
                    child->posX.whole = entity->posX.whole;
                    child->posY.whole = entity->posY.whole;
                    child->flipX = entity->flipX;
                    child->subState = entity->subState;
                }
                entity->subState += 1;
                if (entity->subState > 2)
                {
                    RedSoul_RemoveActiveEntity(entity);
                    EntityDelete(entity);
                    return;
                }
                entity->timer = 3;
            }
            entity->timer -= 1;
            break;
    }
}

/**
 * @brief Sub-weapon entry 3 (Grand Cross, MP 150): after the charge delay,
 * attach a double-size affine cross to Julius, grow it, alternate
 * large attack boxes with particles for 0x60 updates, then shrink
 * and delete with its tracked sounds.
 */
void JuliusGrandCross_Update(struct EwramData_EntityData *entity)
{
    s32 y;
    s32 x;
    struct EwramData_EntityData *owner;
    u16 affineIndex;
    s32 tmp = 0x10000;

    owner = entity->unk_4F8.unk_4F8_entity;

    switch (entity->state)
    {
        case 0:
            gEwramData->playerState.nbrActiveRedSoulEntities += 1;
            Sound_PlayTracked(0xFA);
            entity->timer = 0x20;
            entity->state = 1;
            /* fallthrough */
        case 1:
            if (owner->unk_4F4.whole & 0x80)
            {
                RedSoul_RemoveActiveEntity(entity);
                EntityDelete(entity);
                return;
            }

            if (entity->timer != 0)
            {
                entity->timer -= 1;
                break;
            }

            gEwramData->playerState.redSoulGfx = ObjGfx_LoadResource((u8 *)0x081FA2B8, 0x10000);
            Sprite_Attach(entity, (u8 *)0x08252CCC, gEwramData->playerState.redSoulGfx, 3);
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            entity->spriteLayer = 0xF;
            Hit_SetTimerOverride(1, 0xF);

            affineIndex = ObjAffine_Alloc(0);
            if ((s16)affineIndex < 0)
            {
                RedSoul_RemoveActiveEntity(entity);
                EntityDelete(entity);
                return;
            }
            entity->affineIndex = affineIndex;
            entity->doubleSize = entity->affine = 1;

            Hit_Enable(entity, 5, NULL, 0);
            entity->hit.flags |= 4;
            entity->hit.flags |= 2;
            GRAND_CROSS_SCALE(entity) = tmp;
            Sound_StopTracked(0xFA);
            Sound_PlayTracked(0xFB);
            PlaySong(0x1E6U);
            entity->timer = 0x10;
            entity->scrollWithBg = 1;
            entity->state = 2;
            /* fallthrough */
        case 2:
            Entity_PlaceRelativeTo(entity, owner, 0, 0);
            if (entity->timer != 0)
            {
                entity->timer -= 1;
            }
            else
            {
                Anim_StartIndexed(entity, 1U, 0U, 1U);
                entity->timer = 0x60;
                entity->state = 3;
            }
            break;

        case 3:
            Entity_PlaceRelativeTo(entity, owner, 0, 0);
            if (entity->timer & 1)
            {
                entity->flipX ^= 1; 
            }

            if (GRAND_CROSS_SCALE(entity) < 0x20000)
            {
                GRAND_CROSS_SCALE(entity) += 0x2000;
            }
            else
            {
                if (entity->timer & 1)
                {
                    x = entity->posX.whole + RandomSigned(0x600000);
                    y = entity->posY.whole - 0x500000 + RandomSigned(0x100000);
                    Effect_CreateFromTable(x, y, 0, RandomSigned(0x800) - 0x2000, 4, 6U);
                }

                entity->hit.flags &= ~2;
                if (entity->timer & 1)
                {
                    HitBox_SetAttackRaw(entity, 0x783088E8);
                }
                else
                {
                    HitBox_SetAttackRaw(entity, 0x2878B0C4);
                }
            }

            if (entity->timer != 0)
            {
                entity->timer -= 1;
                break;
            }
            entity->hit.flags |= 2;
            entity->state = 4;
            /* fallthrough */
        case 4:
            if (GRAND_CROSS_SCALE(entity) > 0)
            {
                GRAND_CROSS_SCALE(entity) -= 0x4000;
                break;
            }

            Sound_StopTracked(0xFB);
            if (entity->affineIndex || entity->affine)
            {
                ObjAffine_Free(entity->affineIndex);
            }
            RedSoul_RemoveActiveEntity(entity);
            EntityDelete(entity);
            return;
    }

    entity->roomActive = 1;
    if (entity->state > 1)
    {
        Anim_Update(entity);
        ObjAffine_Set(0x10000, GRAND_CROSS_SCALE(entity), GRAND_CROSS_SCALE(entity), entity->flipX ^ 1, 0, entity->affineIndex);
    }
    return;
}

/**
 * @brief Return entry n of the Julius sub-weapon table sJuliusSubweaponInfo. No
 * source caller; RedSoul_GetInfo selects the table by character.
 */
struct Unk_080E1ED8* Julius_GetSubweaponInfo(u8 subweapon)
{
    return &((struct Unk_080E1ED8*)sJuliusSubweaponInfo)[subweapon];
}

/**
 * @brief Unreferenced: load the Julius overlay graphics (0x081FA2B8) and attach
 * its sprite with palette 2, or 3 when the flag is set.
 */
void JuliusOverlay_AttachSprite(struct EwramData_EntityData *entity, s32 altPalette)
{
    s32 palette;

    palette = 2;
    gEwramData->playerState.redSoulGfx = ObjGfx_LoadResource((u8 *)0x081FA2B8, 0x10000);
    if (altPalette != 0)
    {
        palette = 3;
    }
    Sprite_Attach(entity, (u8 *)0x08252CCC, gEwramData->playerState.redSoulGfx, palette);
}

/**
 * @brief Hit entity for one chain node: enable its attack box once, then
 * every eighth update (staggered by node index) while the chain is
 * active move to its node and enable hits; otherwise disable them.
 */
void Whip_UpdateChainHitSegment(struct EwramData_EntityData *entity)
{
    struct WhipChainNode *nodes;
    struct EwramData_EntityData *chain;

    chain = entity->unk_4F8.unk_4F8_entity;

    switch (entity->state)
    {
        case 0:
            Hit_Enable(entity, 6, NULL, 0);
            HitBox_SetAttackRaw(entity, 0x0404FEFE);
            entity->hit.flags |= 4;
            entity->hit.flags |= 2;
            entity->state = 1;
            /* fallthrough */
        case 1:
            entity->timer += 1;
            if ((WHIP_CHAIN_ACTIVE(chain) != 0) && !(entity->timer & 7))
            {
                nodes = chain->unk_4F4.whipNodes;
                // TODO: what is going on here
                entity->posX.whole = nodes[WHIP_SEGMENT_NODE(entity)].posX;
                entity->posY.whole = nodes[WHIP_SEGMENT_NODE(entity)].posY;
                entity->hit.flags &= ~2;
            }
            else
            {
                entity->hit.flags |= 2;
            }
            break;
    }

    if (!entity->roomActive)
    {
        entity->roomActive = 1;
    }
}

/**
 * @brief Damp the node velocity (X by 0xD0/256, Y by 0xE0/256), add it and
 * a downward pull, then place the node 4 px from its predecessor
 * along that direction and store the resulting velocity. The third
 * parameter is unused.
 */
void Whip_ConstrainChainNode(struct WhipChainNode *node, struct WhipChainNode *prev, s16 index)
{
    s32 pos[2];
    s32 delta[2];
    s32 length;
    s32 dy2;
    s32 scale;
    s32 tmp;

    node->velX = (node->velX >> 8);
    node->velX *= 0xD0;
    node->velY = node->velY >> 8;
    tmp = 0xE0;
    node->velY *= tmp;
    Vec2_Add((s32*)&pos, (s32*)node, (s32*)&node->velX);

    pos[0] += 0x200;
    pos[1] += 0x6000;
    Vec2_Subtract((s32*)&delta, (s32*)&pos, (s32*)prev);

    length = (delta[0] >> 8) * (delta[0] >> 8);
    dy2 = (delta[1] >> 8) * (delta[1] >> 8);
    length += dy2;
    length = Sqrt(length) << 8;
    scale = FixedDivideFast(0x40000, length);

    delta[0] = (delta[0] >> 8) * (scale >> 8);
    delta[1] = (delta[1] >> 8) * (scale >> 8);
    Vec2_FromPosition((s32*)&pos, node);
    Vec2_Add((s32*)&node->posX, (s32*)prev, (s32*)&delta);
    Vec2_Subtract((s32*)&node->velX, (s32*)&node->posX, (s32*)&pos);
}

/**
 * @brief While the chain is active, follow the controller position with
 * node 0 and constrain nodes 1..7 to their predecessors. Once the stillness
 * counter (advanced while the controller does not move) reaches 120,
 * the nodes hang straight down 4 px apart.
 */
void Whip_UpdateChain(struct EwramData_EntityData *entity)
{
    s32 i;
    struct WhipChainNode *nodes;

    nodes = entity->unk_4F4.whipNodes;
    if (WHIP_CHAIN_ACTIVE(entity) != 0)
    {
        if ((nodes[0].posX == entity->posX.whole) && (nodes[0].posY == entity->posY.whole))
        {
            WHIP_CHAIN_SETTLE(entity) += 1;
        }
        else
        {
            nodes[0].posX = entity->posX.whole;
            nodes[0].posY = entity->posY.whole;
        }

        if (WHIP_CHAIN_SETTLE(entity) < 120)
        {
            for (i = 1; i < 8; i++)
            {
                Whip_ConstrainChainNode(nodes + i, nodes - 1 + i, i);
            }
        }
        else
        {
            for (i = 0; i < 7; i++)
            {
                nodes[i + 1].velX = 0;
                nodes[i + 1].velY = 0;
                nodes[i + 1].posX = nodes[i].posX;
                nodes[i + 1].posY = -0x40000; // TODO: inline also works
                nodes[i + 1].posY = nodes[i].posY - nodes[i + 1].posY;
            }
        }
    }

    if (!entity->roomActive)
    {
        entity->roomActive = 1;
    }
}

/**
 * @brief Draw the active chain: tip frame 4 at node 7, handle frame 5 at
 * node 0, then frame 3 at nodes 0..6 and at each midpoint between
 * consecutive nodes.
 */
void Whip_DrawChain(struct EwramData_EntityData *entity)
{
    s32 i;
    struct WhipChainNode *nodes;

    nodes = entity->unk_4F4.whipNodes;
    if (WHIP_CHAIN_ACTIVE(entity) == 0)
        return;

    entity->posX.whole = (nodes[7].posX * 2) >> 1;
    entity->posY.whole = (nodes[7].posY * 2) >> 1;
    entity->animFrame = 4;
    Sprite_Draw(entity);

    entity->posX.whole = nodes->posX;
    entity->posY.whole = nodes->posY;
    entity->animFrame = 5;
    Sprite_BuildSingle(entity);

    for (i = 0; i < 7; i++)
    {
        entity->posX.whole = nodes[i].posX;
        entity->posY.whole = nodes[i].posY;
        Sprite_DrawSingle(entity);

        entity->posX.whole = (nodes[i].posX + nodes[i + 1].posX) >> 1;
        entity->posY.whole = (nodes[i].posY + nodes[i + 1].posY) >> 1;
        Sprite_DrawSingle(entity);
    }
}

/**
 * @brief Create the 8-node whip chain controller (update Whip_UpdateChain,
 * draw Whip_DrawChain) over gEwramData->whipChainNodes, one hit entity
 * per node, nodes stacked 4 px apart from (x, y). Julius_InitEntity
 * stores it in playerState.whipChain; it starts active.
 */
struct EwramData_EntityData* Whip_CreateChain(s32 x, s32 y, u8 *resource, struct EwramData_EntityData *owner)
{
    struct EwramData_EntityData *segment;
    struct EwramData_EntityData *chain;
    s32 i;
    s32 gfx;

    chain = EntityCreateInRange(2, 0x10, (u32 *) Whip_UpdateChain);
    gEwramData->playerState.skillBaseAttack = 8;
    gEwramData->playerState.skillAttackAttributes = 0;
    Hit_SetTimerOverride(2, 0x14);

    chain->unk_4F4.whipNodes = gEwramData->whipChainNodes;
    chain->unk_504.unk_504_entity = owner;
    chain->posX.whole = x;
    chain->posY.whole = y;
    chain->drawFunc = (u32*)&Whip_DrawChain;

    for (i = 0; i < 8; i++)
    {
        segment = BlueSoul_CreateEntity(chain, Whip_UpdateChainHitSegment);
        if (segment != NULL)
        {
            segment->unk_50C.part8.unk_50C = i;
            segment->timer = i;
        }

        gEwramData->whipChainNodes[i].posX = x;
        gEwramData->whipChainNodes[i].posY = y + i * 0x40000;
        gEwramData->whipChainNodes[i].velX = 0;
        gEwramData->whipChainNodes[i].velY = 0;
    }

    gfx = ObjGfx_Find(resource);
    Sprite_Attach(chain, (void *)0x08252CCC, gfx, 0U);

    chain->animFrame = 3;
    chain->spriteLayer = 0xE;
    chain->unk_500.part16.unk_500 = 1;
    chain->unk_500.part16.unk_502 = 0;
    chain->roomActive = chain->scrollWithBg = 1;
    return chain;
}

/**
 * @brief Create the second Julius sprite entity (slot range 2..0x10, layer 0xD)
 * used by Julius_InitEntity.
 */
struct EwramData_EntityData* Julius_CreateOverlaySprite(void)
{
    struct EwramData_EntityData *child;
    s32 gfx;

    child = EntityCreateInRange(2, 0x10, (u32*)&JuliusOverlay_Update);
    gfx = ObjGfx_Find((u8 *)0x081FA2B8);
    Sprite_Attach(child, (void *)0x08252CCC, gfx, 0);
    child->spriteLayer = 0xD;
    child->roomActive = 1;
    return child;
}

/**
 * @brief Unreferenced: delete the child entity in unk_4F8 and the entity.
 */
void Entity_DeleteWithChild(struct EwramData_EntityData *entity)
{
    EntityDelete(entity->unk_4F8.unk_4F8_entity);
    EntityDelete(entity);
}

/**
 * @brief Set the chain active flag (unk_500), enabling update, drawing
 * and node hits. Called each update of the held-whip substates.
 */
void Whip_ActivateChain(struct EwramData_EntityData *entity)
{
    WHIP_CHAIN_ACTIVE(entity) = 1;
}

/**
 * @brief Clear the chain active flag (unk_500).
 */
void Whip_DeactivateChain(struct EwramData_EntityData *entity)
{
    WHIP_CHAIN_ACTIVE(entity) = 0;
}

/**
 * @brief Place the nodes in a horizontal line 4 px apart from the controller
 * position, rightward for direction 0 and leftward otherwise, clear
 * their velocities and the stillness counter.
 */
void Whip_LayChainStraight(struct EwramData_EntityData *entity, s16 flip)
{
    s32 step;
    s32 x;
    s32 i;
    s32 y;

    x = entity->posX.whole;
    y = entity->posY.whole;
    
    if (flip == 0)
    {
        step = 0x40000;
    }
    else
    {
        step = -0x40000;
    }

    for (i = 0; i < 8; i++)
    {
        gEwramData->whipChainNodes[i].posX = x + i * step;
        gEwramData->whipChainNodes[i].posY = y;
        gEwramData->whipChainNodes[i].velX = 0;
        gEwramData->whipChainNodes[i].velY = 0;
    }

    WHIP_CHAIN_SETTLE(entity) = 0;
}

/**
 * @brief Add rightward node velocity from 0x180000 at node 0, decreasing by
 * 0x18000 per node, and clear the stillness counter (d-pad right).
 */
void Whip_SwingChainRight(struct EwramData_EntityData *entity)
{
    struct WhipChainNode *nodes;
    s32 i;
    s32 push;
    s32 falloff;

    nodes = entity->unk_4F4.whipNodes;
    push = 0x180000;
    falloff = 0x18000;

    for (i = 0; i < 8; i++)
    {
        nodes[i].velX += push;
        push -= falloff;
    }

    WHIP_CHAIN_SETTLE(entity) = 0;
}

/**
 * @brief Mirror of Whip_SwingChainRight with leftward velocity (d-pad left).
 */
void Whip_SwingChainLeft(struct EwramData_EntityData *entity)
{
    struct WhipChainNode *nodes;
    s32 i;
    s32 push;
    s32 falloff;

    nodes = entity->unk_4F4.whipNodes;
    push = -0x180000;
    falloff = -0x18000;

    for (i = 0; i < 8; i++)
    {
        nodes[i].velX += push;
        push -= falloff;
    }

    WHIP_CHAIN_SETTLE(entity) = 0;
}

/**
 * @brief Add downward node velocity from 0x200000 at node 0, decreasing by
 * 0x20000 per node, and clear the stillness counter (d-pad down).
 */
void Whip_SwingChainDown(struct EwramData_EntityData *entity)
{
    struct WhipChainNode *nodes;
    s32 i;
    s32 push;
    s32 falloff;

    nodes = entity->unk_4F4.whipNodes;
    push = 0x200000;
    falloff = 0x20000;

    for (i = 0; i < 8; i++)
    {
        nodes[i].velY += push;
        push -= falloff;
    }

    WHIP_CHAIN_SETTLE(entity) = 0;
}

/**
 * @brief Mirror of Whip_SwingChainDown with upward velocity (d-pad up).
 */
void Whip_SwingChainUp(struct EwramData_EntityData *entity)
{
    struct WhipChainNode *nodes;
    s32 i;
    s32 push;
    s32 falloff;

    nodes = entity->unk_4F4.whipNodes;
    push = -0x200000;
    falloff = -0x20000;

    for (i = 0; i < 8; i++)
    {
        nodes[i].velY += push;
        push -= falloff;
    }

    WHIP_CHAIN_SETTLE(entity) = 0;
}

/**
 * @brief Keep the overlay sprite alive with palette 0.
 */
void JuliusOverlay_Update(struct EwramData_EntityData *entity)
{
    entity->palette = 0;
    entity->roomActive = 1;
}

static inline s32 DebugPlatform_ToPixel(s32 val)
{
    s16 var;
    if (val < 0)
    {
        var = -(-val >> 0x10);
    }
    else
    {
        var = val >> 0x10;
    }
    return var;
}

static inline s32 DebugPlatform_ToScreen(s16 val1, u16 val2)
{
    return val1 - val2;
}

/**
 * @brief Floor correction callback of the debug platform: return the push-up
 * offset when the point is on its top surface, else 0.
 */
s16 DebugPlatform_FloorCorrection(struct EwramData_EntityData *entity, s32 x, s32 y)
{
    s16 correction;
    struct BgLayerState *bgInfo;
    s16 screenX;
    s16 screenY;
    s16 nextY;
    s16 nextX;

    bgInfo = &gEwramData->bgInfo[1];

    screenX = DebugPlatform_ToScreen(x, bgInfo->xPos.part16.integer);
    screenY = DebugPlatform_ToScreen(y, bgInfo->yPos.part16.integer);

    nextY = DebugPlatform_ToPixel(entity->posY.whole + entity->velY.whole);
    nextX = DebugPlatform_ToPixel(entity->posX.whole + entity->velX.whole);

    if ((nextX - screenX) >= 0)
    {
        if ((nextX - screenX) > 0x13)
        {
            return 0;
        }
    }
    else if (-(nextX - screenX) > 0x13)
    {
        return 0;
    }

    correction = (nextY - screenY) - 0x20;
    if (correction <= -0x10 || correction > 0x0)
        return 0;

    entity->unk_4FC.part8.unk_4FC = 1;
    gUnk_03002CB0.collisionProbeFlags = 2;
    return correction;
}

/**
 * @brief Debug moving platform (callback table sUnk_080E1F74): register as
 * solid object and circle; free the slot when deactivated.
 */
void DebugPlatform_Update(struct EwramData_EntityData *entity)
{
    s32 gfx;
    s32 paletteIndex;

    switch (entity->state)
    {
        case 0:
            gfx = ObjGfx_LoadResource((u8 *)0x081CBE14, 0);
            paletteIndex = ObjPalette_FindOrLoad((void *)0x08209C64);
            Sprite_Attach(entity, (void *)0x0820F18C, gfx, paletteIndex);

            entity->animFrame = 0;
            entity->scrollWithBg = 1;
            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            Player_RegisterSolidObject(entity, (void *)0x080E1F74);
            entity->roomActive = 1;
            entity->state += 1;
            /* fallthrough */
        case 1:
            entity->posX.whole += entity->velX.whole;
            entity->velX.whole = Sin(entity->timer << 0xA) * 3;
            entity->posY.whole += entity->velY.whole;
            entity->velY.whole = Sin((entity->timer << 0xA) + 0x4000) * 3;
            entity->timer += 1;

            if (!entity->roomActive)
            {
                entity->deleteRequested = 1;
                Player_ClearSolidObject();
            }
            return;
    }
}
