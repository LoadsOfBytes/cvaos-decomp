#include "code/code_080211F0.h"
#include "soul_skill_entity.h"
#include "red_soul_entity.h"
#include "player_entity.h"
#include "code_08009A0.h"
#include "code_08001004.h"
#include "code_08001194.h"
#include "code_0800B700.h"
#include "code_08013960.h"
#include "code_08008750.h"
#include "code_0800CB00.h"
#include "code_0800F1FC.h"
#include "code/code_08014548.h"
#include "code_08032444.h"
#include "code_08032E4C.h"
#include "code_08033CAC.h"
#include "code_08035930.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "gba.h"
#include "macros.h"
#include "agb_sram.h"
#include "syscalls.h"

/*
 * Player status icon (Player_SpawnStatusIcon, PlayerStatusIcon_Update): index
 * of the status effect it shows (its low byte picks the frame).
 * Token-identical expansion.
 */
#define STATUS_ICON_INDEX(e) e->unk_4FC.part16.unk_4FC
#define STATUS_ICON_INDEX_LOW(e) e->unk_4FC.part8.unk_4FC

#include "constants/main.h"

#include "data/data_0E0334.h"
#include "data/data_0E3464.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"

// ROM data passed as pointers; previously hard-coded addresses in the draft.
extern u8 sCommonObjectGfx[];
extern u8 sWeaponPalettes[];
extern u8 sCommonObjectSpriteSheet[];

/**
 * @brief Empty per-frame hook of debug mode 2229.
 */
void Debug2229_FrameHook(void)
{
    return;
}

/**
 * @brief Hit callback (player body and Julius whip strike): set player
 * contact flag 0x80000.
 */
void Player_MarkAttackContact(void)
{
    struct EwramData_EntityData *player;

    player = gEwramData->playerState.player;
    PLAYER_ACTION_FLAGS(player) |= 0x80000;
}

/**
 * @brief Set player X/Y velocity and horizontal acceleration 0x1000 opposing the horizontal sign.
 */
void Player_SetVelocityWithBraking(s32 velocityX, s32 velocityY)
{
    gEwramData->playerState.player->velX.whole = velocityX;
    gEwramData->playerState.player->velY.whole = velocityY;
 
    gEwramData->playerState.player->accelX.whole = (gEwramData->playerState.player->velX.whole < 0) ? 0x1000 : -0x1000;
}

/**
 * @brief When the player hit reaction hitReaction is 3, set its velocity
 * knockbackVelocityX / knockbackVelocityY to (vx, vy); other reactions are left unchanged.
 */
void Player_SetKnockbackVelocity(s32 vx, s32 vy)
{
    if (gEwramData->playerState.hitReaction == 3)
    {
        gEwramData->playerState.knockbackVelocityX = vx;
        gEwramData->playerState.knockbackVelocityY = vy;
    }
}

/**
 * @brief Append an entity and byte value to the seven-entry player queue when capacity remains; otherwise do nothing.
 */
void Player_QueueEntity(struct EwramData_EntityData *entity, u8 collisionFlags)
{
    if (gEwramData->playerState.collisionQueueCount < 7)
    {
        gEwramData->playerState.collisionQueueEntities[gEwramData->playerState.collisionQueueCount] = entity;
        gEwramData->playerState.collisionQueueFlags[gEwramData->playerState.collisionQueueCount] = collisionFlags;
        gEwramData->playerState.collisionQueueCount += 1;
    }
}

/**
 * @brief Return the player action flags (PLAYER_ACTION_FLAGS) when the control focus
 * enemyTarget is the player itself, else 0. Enemies test them for attack
 * flags.
 */
s32 Player_GetFocusedActionFlags(void)
{
    if (gEwramData->playerState.enemyTarget == gEwramData->playerState.player)
    {
        return PLAYER_ACTION_FLAGS(gEwramData->playerState.enemyTarget);
    }
    return 0;
}


/**
 * @brief Return Player_HasAbility(n - 1).
 */
u8 Player_HasAbilityOneBased(s32 abilityIndexOneBased)
{
    return Player_HasAbility(abilityIndexOneBased - 1);
}

/**
 * @brief Return 0 only when the player is in state 0 with |velX| at most
 * 0x8000 and flags 0x30000120 clear; otherwise nonzero.
 */
s32 Player_IsBusy(void)
{
    struct EwramData_EntityData *player;
    s32 speed;

    player = gEwramData->playerState.player;
    if (player->state == 0)
    {
        speed = player->velX.whole;
        if (speed < 0)
        {
            speed = -speed;
        }
        if (speed <= 0x8000)
        {
            return PLAYER_ACTION_FLAGS(player) & 0x30000120;
        }
    }
    return 1;
}

/**
 * @brief Run the equipment preparation helper, equip red/blue/yellow soul IDs and restore HP/MP while clearing status effects.
 */
void Player_EquipSoulsAndRestore(u8 redSoulId, u8 blueSoulId, u8 yellowSoulId)
{
    ButtonConfig_ResetDefaults();
    gEwramData->playerStatus.equippedRedSoul = redSoulId;
    gEwramData->playerStatus.equippedBlueSoul = blueSoulId;
    gEwramData->playerStatus.equippedYellowSoul = yellowSoulId;
    Player_RestoreHPAndMP();
}

/**
 * @brief Set the player fixed-point X fractional halfword.
 */
void Player_SetXFraction(s32 fraction)
{
    gEwramData->playerState.player->posX.part16.fraction = fraction;
}

/**
 * @brief Set the player fixed-point Y fractional halfword.
 */
void Player_SetYFraction(s32 fraction)
{
    gEwramData->playerState.player->posY.part16.fraction = fraction;
}

/**
 * @brief Set current HP and MP to their maxima and clear all three status-effect bits and timers.
 */
void Player_RestoreHPAndMP(void)
{
    gEwramData->playerStatus.currentHP = gEwramData->playerStatus.maxHP;
    gEwramData->playerStatus.currentMP = gEwramData->playerStatus.maxMP;
    Player_ClearStatusEffects(7);
}

/**
 * @brief Return 1 during a boss fight (stateFlags bit 2) when the selected target
 * has 1..199 HP.
 */
s32 Player_IsBossTargetLowHp(void)
{
    if (!(gEwramData->gameState.stateFlags & 2))
    {
        return 0;
    }
    if (gEwramData->playerState.selectedTarget == NULL)
    {
        return 0;
    }
    if (gEwramData->playerState.selectedTarget->unk_518.part16.hp > 0 && gEwramData->playerState.selectedTarget->unk_518.part16.hp < 200)
    {
        return 1;
    }
    return 0;
}

/**
 * @brief If flag 0x800 is clear and the animation index differs, start the requested animation and reset the associated attack markers; return 1 only when started.
 */
s32 Player_TryStartAnimation(struct EwramData_EntityData *entity, u8 animationIndex, u8 flags)
{
    if (gEwramData->playerState.actionLockFlags & 0x800)
    {
        return 0;
    }
    if (entity->animScriptIndex != animationIndex)
    {
        Anim_StartIndexed(entity, animationIndex, 1, flags);
        PLAYER_REQUESTED_ANIM(entity) = 0xFF;
        PLAYER_ACTION_FLAGS(entity) &= ~0x200000;
        return 1;
    }
    return 0;
}

/**
 * @brief Temporarily add signed byte offsets from unk_508 to position, draw the entity and restore its full fixed-point coordinates.
 */
void Player_DrawWithOffset(struct EwramData_EntityData *entity)
{
    s32 oldX;
    s32 oldY;

    oldX = entity->posX.whole;
    oldY = entity->posY.whole;

    entity->posX.part16.integer += (s8) PLAYER_DRAW_OFFSET_X(entity);
    entity->posY.part16.integer += (s8) PLAYER_DRAW_OFFSET_Y(entity);
    Sprite_DrawEntity(entity);

    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}

/**
 * @brief Unreferenced: ButtonConfig_ResetDefaults.
 */
void ButtonConfig_ResetDefaultsWrapper(void)
{
    ButtonConfig_ResetDefaults();
}

/**
 * @brief Toggle horizontal flip for one entity draw, then restore it.
 */
void Player_DrawMirrored(struct EwramData_EntityData *entity)
{
    entity->flipX ^= 1;
    Sprite_DrawEntity(entity);
    entity->flipX ^= 1;
}

/**
 * @brief Set the player contact-immunity value (PLAYER_INVULNERABLE_TIMER);
 * Player_ApplyContactDamage ignores contact damage while it is nonzero.
 */
void Player_SetContactImmunity(s32 immunityValue)
{
    PLAYER_INVULNERABLE_TIMER(gEwramData->playerState.player) = immunityValue;
}

extern u16 sAttackPowerMultipliers[]; // array or struct?

/**
 * @brief Calculate outgoing base attack power from the supplied category, player stats and mode/equipment data, applying the mode reduction when enabled.
 */
u16 Player_CalculateAttackPower(u16 attackCategory)
{
    u32 power;
    struct RedSoulAttackView *info;

    if (attackCategory == 3)
    {
        power = gEwramData->playerState.currentATK * sAttackPowerMultipliers[3];
        power /= 16;
    }
    else if (attackCategory == 5)
    {
        if (gEwramData->gameState.linkPlay == 0)
        {
            info = RedSoul_GetLastUsedInfo();
            power = ((gEwramData->playerState.currentStats[0] * info->baseAttack) / 10) + (gEwramData->playerState.currentStats[2] >> 2) + (info->baseAttack * 4) + (info->baseAttack * 2);
        }
        else
        {
            power = gEwramData->unk_133EE * sAttackPowerMultipliers[3];
        }
        power /= 16;
    }
    else if (attackCategory == 6)
    {
        power = gEwramData->playerState.skillBaseAttack + ((gEwramData->playerState.skillBaseAttack >> 3) * (gEwramData->playerState.currentStats[2] >> 1));
    }
    else
    {
        power = gEwramData->playerState.currentStats[0] * sAttackPowerMultipliers[attackCategory];
        power /= 16;
    }

    if (gEwramData->gameState.difficultyMode)
    {
        power = (power * 8) / 10;
    }
    return power;
}

struct StatusEffectApplication {
    u16 initialCounter;
    u16 soundId;
};
extern struct StatusEffectApplication sStatusEffectApplications[];

static inline s16 Player_GetX()
{
    return gEwramData->playerState.player->posX.part16.integer;
}

/**
 * @brief Resolve incoming damage and avoidance against player defense, update HP and hit metadata, and apply nonresisted status effects.
 */
void Player_ApplyContactDamage(struct PlayerContactDamage *contactDamage)
{
    s16 y;
    u16 x;
    s16 blocked;
    u16 defenseBonus;
    s32 reduced;
    s32 raw;
    s32 damage;
    s32 i;
    struct StatusEffectApplication *var_r5;
    s32 temp_r5_2;
    u16 hitAttribute;
    s32 inflicted;

    blocked = 0;
    if (PLAYER_INVULNERABLE_TIMER(gEwramData->playerState.player) != 0)
    {
        return;
    }
    hitAttribute = Attribute_GetHighestIndex(contactDamage->attributes);
    if (hitAttribute != 0)
    {
        // temp_r5_2 = (s16) gEwramData->playerState.player->posX.part16.integer;
        // temp_r5 = temp_r5_2 + RandomSigned(8);
        x = Player_GetX() + RandomSigned(8);
        y = Player_GetOverheadY() + RandomSigned(4);
        HitEffect_Spawn(x, y, hitAttribute);
    }
    if (!gEwramData->gameState.difficultyMode)
    {
        raw = contactDamage->attack * 4;
    }
    else
    {
        defenseBonus = (gEwramData->playerState.equipmentDefenseBonus >> 0x1) + (gEwramData->playerState.equipmentDefenseBonus >> 0x2) + (gEwramData->playerStatus.maxHP >> 4);
        raw = (contactDamage->attack * 5) + (defenseBonus >> 1);
    }
    reduced = raw - (gEwramData->playerState.currentDEF * 2);
    damage = reduced >> 1;
    if ((gEwramData->playerState.resistances != 0) && (contactDamage->attributes & gEwramData->playerState.resistances))
    {
        damage = reduced >> 2;
    }
    if (gEwramData->playerStatus.statusEffects & 4)
    {
        damage *= 2;
    }
    if (damage <= 0)
    {
        damage = 1;
    }

    if ((gEwramData->playerStatus.soulStateFlags & 0x2000))
    {
        PlaySong(0x13B);
        Player_ShowDamage(0, 0);
        return;
    }
    if ((gEwramData->playerStatus.equippedArmor == 0x17) && ((RandomNumberGenerator() & 0xF) < 4))
    {
        blocked = 1;
    }
    if (blocked != 0)
    {
        PlaySong(0x13B);
        Player_ShowDamage(0, 0);
        return;
    }

    gEwramData->playerStatus.currentHP -= damage;
    if (gEwramData->playerStatus.currentHP <= 0)
    {
        gEwramData->playerStatus.currentHP = blocked;
        gEwramData->gameState.stateFlags |= 0x10;
        Player_ClearStatusEffects(7);
        Game_SetMenusEnabled(0);
        gEwramData->paused = 0;
    }
    gEwramData->playerState.pendingHitDamage = damage;
    gEwramData->playerState.hitSourceX = contactDamage->sourceX;
    gEwramData->playerState.hitSourceY = contactDamage->sourceY;
    gEwramData->playerState.hitReaction = contactDamage->reaction;
    if (gEwramData->playerStatus.soulStateFlags & 0x40000)
    {
        Player_AddGold(damage * 5);
    }
    if (gEwramData->playerStatus.soulStateFlags & 0x80000)
    {
        Player_RestoreMP(damage * 2);
    }
    if ((gEwramData->playerState.hitReaction == 0) && (damage > (gEwramData->playerStatus.maxHP >> 2)))
    {
        gEwramData->playerState.hitReaction = 1;
    }
    Player_ShowDamage(damage, 0);
    PlaySong(0xB3);
    if ((gEwramData->playerStatus.currentCharacter == 0) && (gEwramData->playerStatus.petrifyCounter == 0))
    {
        inflicted = (contactDamage->attributes & (7 << 6)) >> 6;
        if (((gEwramData->playerState.resistances >> 6) & inflicted) != inflicted)
        {
            gEwramData->playerStatus.statusEffects |= inflicted;
            for (i = 2; i >= 0; i--)
            {
                if ((inflicted >> i) & 1)
                {
                    PlaySong(0x198);
                    Player_SpawnStatusIcon(i);
                    gEwramData->playerStatus.statusEffectTimers[i] = sStatusEffectApplications[i].initialCounter;
                    PlaySong(sStatusEffectApplications[i].soundId);
                }
            }
            contactDamage->sourceX++,contactDamage->sourceX--; // Fake
        }
    }
    return;
}

/**
 * @brief Refresh the counter of an existing entity in the four-entry player list or use the first free slot; return 0 only when full and absent.
 */
s32 Player_TrackEntity(struct EwramData_EntityData *entity)
{
    s32 i;
    s32 freeSlot;
    u8 *counters;

    freeSlot = -1;
    counters = gEwramData->playerState.trackedEntityRefreshCounters;

    for (i = 0; i < 4; i++)
    {
        if (gEwramData->playerState.trackedEntities[i] == entity)
        {
            counters[i * 2 + 1] += 1;
            return 1;
        }

        if ((freeSlot < 0) && (gEwramData->playerState.trackedEntities[i] == 0))
        {
            freeSlot = i;
        }
    }

    if (freeSlot >= 0)
    {
        gEwramData->playerState.trackedEntities[freeSlot] = entity;
        counters[freeSlot * 2 + 1] = 1;
        counters[freeSlot * 2 + 0] = 0;
        return 1;
    }
    return 0;
}

/**
 * @brief Process status-escape input, status countdowns and stat penalties, then apply equipment-dependent MP regeneration or drain.
 */
void Player_UpdateStatusAndRegen(struct EwramData_EntityData *entity)
{
    s32 playerNewInput;
    s32 amount;
    s32 i;
    struct EwramData_EntityData *dust;
    struct EwramData_EntityData *shard;

    playerNewInput = gEwramData->inputData.playerNewInput & (KEY_DOWN | KEY_UP | KEY_LEFT | KEY_RIGHT | KEY_B | KEY_A);
    if (gEwramData->playerStatus.petrifyCounter != 0)
    {
        amount = ((playerNewInput & KEY_A) >> 0) + ((playerNewInput & KEY_B) >> 1) + ((playerNewInput & KEY_RIGHT) >> 4) + ((playerNewInput & KEY_LEFT) >> 5) + ((playerNewInput & KEY_UP) >> 6) + ((playerNewInput & KEY_DOWN) >> 7);
        if (amount & 1)
        {
            dust = Effect_SpawnShared(entity->posX.whole, entity->posY.whole, 4, 6);
            if (dust != NULL)
            {
                dust->posX.part16.integer += RandomSigned(0x10);
                dust->posY.part16.integer += RandomSigned(8) - 4;
                dust->drawSkip = 1;
                dust->velY.whole = RandomSigned(0x1000) - 0x8000;
                dust->accelY.whole = 0x400;
            }
            if (gEwramData->playerStatus.petrifyCounter & 1)
            {
                PlaySong(0xB0);
            }
        }
        gEwramData->playerStatus.petrifyCounter -= amount;
        if (gEwramData->playerStatus.petrifyCounter <= 0)
        {
            gEwramData->playerStatus.petrifyCounter = 0;
            gEwramData->playerStatus.statusEffects &= ~0x4;
            PlaySong(0xAF);

            for (i = 0; i < 4; i++)
            {
                shard = Effect_CreateSharedFrame(entity->posX.whole, entity->posY.whole, (1 & i) + 0x40, 6);
                if (shard == NULL)
                    break;

                shard->posX.part16.integer += RandomSigned(0x10);
                shard->posY.part16.integer += RandomSigned(0x10) - 0x10;
                shard->flipX = RandomNumberGenerator() & 1;
                shard->velX.whole = RandomSigned(0x8000) + 0x8000;
                if (shard->flipX)
                {
                    shard->velX.whole = -shard->velX.whole;
                }
                shard->velY.whole = RandomSigned(0x4000) - 0x20000;
                shard->accelY.whole = 0x2000;
            }
        }
    }

    for (i = 1; i >= 0; i--)
    {
        if (gEwramData->playerStatus.statusEffectTimers[i] != 0)
        {
            gEwramData->playerStatus.statusEffectTimers[i] -= 1;
            if (gEwramData->playerStatus.statusEffectTimers[i] <= 0)
            {
                gEwramData->playerStatus.statusEffects &= ~(1 << i);
            }
        }
    }

    if (gEwramData->playerStatus.statusEffects & 1)
    {
        gEwramData->playerState.currentStats[0] >>= 2;
        gEwramData->playerState.currentStats[1] >>= 2;
        gEwramData->playerState.currentStats[2] >>= 2;
    }

    if (gEwramData->playerStatus.statusEffects & 2)
    {
        if (!(gEwramData->playerStatus.statusEffectTimers[1] & 3))
        {
            amount = gEwramData->playerStatus.maxMP >> 7;
            if (amount == 0)
            {
                amount = 1;
            }
            Player_ConsumeMP(amount);
        }
    }
    else if (gEwramData->playerStatus.equippedAccessory != 0x29)
    {
        if (!(PLAYER_FRAME_COUNTER(entity) & 0x3F))
        {
            Player_RestoreMP(1);
        }
    }
    else if (!(PLAYER_FRAME_COUNTER(entity) & 0xF))
    {
        Player_RestoreMP(1);
    }

    if (gEwramData->playerStatus.equippedAccessory == 0x2C)
    {
        gEwramData->playerStatus.currentMP = gEwramData->playerStatus.maxMP;
    }
}

/**
 * @brief Compute current attack and defense from the corresponding stats and stored bonuses, clamping negative results to zero.
 */
void Player_RecalculateAttackDefense(void)
{
    gEwramData->playerState.currentATK = Player_CalculateAttack(gEwramData->playerState.currentStats[0]);
    gEwramData->playerState.currentDEF = Player_CalculateDefense(gEwramData->playerState.currentStats[1]);

    if (gEwramData->playerState.currentATK < 0)
    {
        gEwramData->playerState.currentATK = 0;
    }

    if (gEwramData->playerState.currentDEF < 0)
    {
        gEwramData->playerState.currentDEF = 0;
    }
}

extern u8 sPlayerStatusIconPalettes[];

/**
 * @brief Attach the indexed status icon, converge its horizontal oscillation on the player and delete it when the corresponding status timer clears.
 */
void PlayerStatusIcon_Update(struct EwramData_EntityData *entity)
{
    s32 shake;
    s32 playerX;
    s32 gfx;

    switch (entity->state)
    {
        case 0:
            gfx = ObjGfx_Find(sCommonObjectGfx);
            Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, sPlayerStatusIconPalettes[(u16) STATUS_ICON_INDEX(entity)]);
            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            entity->animFrame = STATUS_ICON_INDEX_LOW(entity) + 0x59;
            entity->spriteLayer = 0xD;
            entity->scrollWithBg = 1;
            entity->state = 1;
            entity->velX.whole = 0x01000000;
            /* fallthrough */
        case 1:
            if (entity->velX.whole > 0)
            {
                entity->velX.whole -= 0x80000;
            }
            shake = entity->velX.whole;
            if (entity->timer & 1)
            {
                shake = -shake;
            }
            entity->timer = entity->timer + 1;
            playerX = (s16) gEwramData->playerState.player->posX.part16.integer;
            entity->posX.part16.integer = (shake < 0) ? (playerX - (-shake >> 0x10)) : (playerX + (shake >> 0x10));
            entity->posY.part16.integer = Player_GetOverheadY() - 4;
            entity->roomActive = 1;
            if (gEwramData->playerStatus.statusEffectTimers[(u16) STATUS_ICON_INDEX(entity)] != 0)
            {
                if (gEwramData->playerStatus.statusEffectTimers[(u16) STATUS_ICON_INDEX(entity)] < 30)
                {
                    entity->drawSkip = 1;
                }
                else
                {
                    entity->drawSkip = 0;
                }
            }
            else
            {
                EntityDelete(entity);
            }
            break;
    }
}

/**
 * @brief 21DB0 | To document
 * 
 * @return s32 To document
 */
s32 sub_08021DB0(void)
{
    return gEwramData->playerState.unk_131BC;
}

/**
 * @brief Remove four-list entities whose refresh counter has not changed since the last sample; otherwise record the new counter.
 */
void Player_PruneTrackedEntities(void)
{
    s32 i;
    struct EwramData_EntityData **tracked;
    u8 *counters;
    u8 *var_r4;

    
    counters = gEwramData->playerState.trackedEntityRefreshCounters;
    tracked = gEwramData->playerState.trackedEntities;

    for (i = 0; i < 4; i++)
    {
        if (tracked[i] != 0)
        {
            if (counters[i * 2 + 0] == counters[i * 2 + 1])
            {
                tracked[i] = NULL;
                counters[i * 2 + 0] = 0;
                counters[i * 2 + 1] = 0;
            }
            else
            {
                counters[i * 2 + 0] = counters[i * 2 + 1];
            }
        }
    } 
}

/**
 * @brief Unreferenced: clear bit n of playerStatus.inputOverrides.
 */
void Player_ClearStatusBit(s32 bitIndex)
{
    gEwramData->playerStatus.inputOverrides &= ~(1 << bitIndex);
}

/**
 * @brief 21E2C | To document
 * 
 */
void sub_08021E2C(void)
{
    return;
}

/**
 * @brief 21E30 | To document
 * 
 */
void sub_08021E30(void)
{
    return;
}

/**
 * @brief 21E34 | To document
 * 
 */
void sub_08021E34(void)
{
    return;
}

/**
 * @brief Return playerStatus.soulStateFlags masked by the argument (soul state
 * bits such as 0x200 bat form and 0x1000 transformation dash).
 */
s32 Player_TestSoulStateFlags(s32 mask)
{
    return gEwramData->playerStatus.soulStateFlags & mask;
}

/**
 * @brief For each selected bit among the three status effects, clear its timer and active flag.
 */
void Player_ClearStatusEffects(s32 mask)
{
    s32 i;
    s32 bit;

    for (i = 0; i < 3; i++)
    {
        bit = (1 << i);
        if (mask & bit)
        {
            gEwramData->playerStatus.statusEffectTimers[i] = 0;
            gEwramData->playerStatus.statusEffects &= ~bit;
        }
    } 
}

/**
 * @brief For each selected bit among the three status effects, store the supplied timer and set its active flag.
 */
void Player_SetStatusEffects(s32 mask, u16 timerValue)
{
    s32 i;
    s32 bit;

    for (i = 0; i < 3; i++)
    {
        bit = (1 << i);
        if (mask & bit)
        {
            gEwramData->playerStatus.statusEffectTimers[i] = timerValue;
            gEwramData->playerStatus.statusEffects |= bit;
        }
    } 
}

/**
 * @brief Copy four saved base stats into the player prepared-stat array.
 */
void Player_CopyBaseStats(void)
{
    s32 i;
    u16 *src;
    u16 *dst;

    dst = gEwramData->playerState.preparedStats;
    src = gEwramData->playerStatus.baseStats;

    for (i = 0; i < 4; i++)
    {
        *dst++ = *src++;
    } 
}

/**
 * @brief Add the supplied unsigned halfword to one prepared stat, retaining the original halfword arithmetic.
 */
void Player_AddPreparedStat(u16 statIndex, u16 amount)
{
    u16 *stat;

    stat = gEwramData->playerState.preparedStats + statIndex;
    *stat += amount;
    // gEwramData->playerState.preparedStats[statIndex] += amount;
}

/**
 * @brief Return the supplied signed stat plus the stored attack bonus, as a signed halfword.
 */
s16 Player_CalculateAttack(s16 statValue)
{
    return gEwramData->playerState.equipmentAttackBonus + statValue;
}

/**
 * @brief Return half the supplied signed stat plus the stored defense bonus, as a signed halfword.
 */
s16 Player_CalculateDefense(s16 statValue)
{
    return (statValue >> 1) + gEwramData->playerState.equipmentDefenseBonus;
}

/**
 * @brief Copy four prepared stats into current stats and copy the prepared resistance mask into the current mask.
 */
void Player_ApplyPreparedStats(void)
{
    u16 *dst;
    u16 *src;
    s32 i;

    dst = gEwramData->playerState.currentStats;
    src = gEwramData->playerState.preparedStats;

    for (i = 0; i < 4; i++)
    {
        *dst++ = *src++;
    } 

    gEwramData->playerState.resistances = gEwramData->playerState.preparedResistances;
}

/**
 * @brief Add HP with upper clamping to maxHP and submit a healing number; retain the original behavior for negative input.
 */
void Player_RestoreHP(s32 amount)
{
    gEwramData->playerStatus.currentHP += amount;
    if (gEwramData->playerStatus.currentHP > gEwramData->playerStatus.maxHP)
    {
        gEwramData->playerStatus.currentHP = gEwramData->playerStatus.maxHP;
    }

    Player_ShowDamage(amount, 1);
}

/**
 * @brief Treat zero as one, add MP and clamp only to maxMP; retain the original behavior for negative input.
 */
void Player_RestoreMP(s32 amount)
{
    if (amount == 0)
    {
        amount = 1;
    }

    gEwramData->playerStatus.currentMP += amount;
    if (gEwramData->playerStatus.currentMP > gEwramData->playerStatus.maxMP)
    {
        gEwramData->playerStatus.currentMP = gEwramData->playerStatus.maxMP;
    }
}

/**
 * @brief Subtract the supplied value from MP and clamp only at zero; retain the original behavior for negative input.
 */
void Player_ConsumeMP(s32 amount)
{
    gEwramData->playerStatus.currentMP -= amount;
    if (gEwramData->playerStatus.currentMP < 0)
    {
        gEwramData->playerStatus.currentMP = 0;
    }
}

/**
 * @brief If playerState.solidObject is empty, store the entity there with
 * its collision-callback table in unk_4F4 and return 1; otherwise
 * return 0. Player world collision consults this object.
 */
s32 Player_RegisterSolidObject(struct EwramData_EntityData *entity, void *callbackTable)
{
    if (gEwramData->playerState.solidObject == NULL)
    {
        gEwramData->playerState.solidObject = entity;
        SOLID_OBJECT_CALLBACK_TABLE(entity) = callbackTable;
        return 1;
    }
    return 0;
}

/**
 * @brief Clear playerState.solidObject.
 */
void Player_ClearSolidObject(void)
{
    gEwramData->playerState.solidObject = NULL;
}

/**
 * @brief Return the flags word of the callback table of the registered solid
 * object, or 0 without one.
 */
s32 Player_GetSolidObjectFlags(void)
{
    if (gEwramData->playerState.solidObject != NULL)
    {
        return *SOLID_OBJECT_CALLBACKS(gEwramData->playerState.solidObject);
    }
    return 0;
}

/**
 * @brief Empty solid-object correction callback used by the debug platform
 * table sUnk_080E1F74.
 */
s32 SolidObject_NoCorrection(void)
{
    return 0;
}

/**
 * @brief Solid-object callback slot 5 that reports no carry velocity (returns
 * 0); used by the push-block group table.
 */
s32 SolidObject_NoCarry(void)
{
    return 0;
}

/**
 * @brief If the indexed status timer is zero, attempt to create its icon in slots 0x16..0x19 and store the status index.
 */
void Player_SpawnStatusIcon(s32 statusIndex)
{
    struct EwramData_EntityData *child;

    if (gEwramData->playerStatus.statusEffectTimers[statusIndex] == 0)
    {
        child = EntityCreateInRange(0x16, 0x19, (u32 *) PlayerStatusIcon_Update);
        if (child != NULL)
        {
            STATUS_ICON_INDEX(child) = statusIndex;
        }
    }
}

/**
 * @brief Set the contact-immunity value and its blink timer (PLAYER_BLINK_TIMER) to n frames
 * (used by the Kyoma Demon soul).
 */
void Player_StartContactImmunity(u8 frames)
{
    Player_SetContactImmunity(frames);
    PLAYER_BLINK_TIMER(gEwramData->playerState.player) = frames;
}

/**
 * @brief Set the contact-immunity value to 1, clear its timer and the draw skip
 * (RedSoul_CancelAll).
 */
void Player_ResetContactImmunity(void)
{
    Player_SetContactImmunity(1);
    PLAYER_BLINK_TIMER(gEwramData->playerState.player) = 0;
    gEwramData->playerState.player->drawSkip = 0;
}

/**
 * @brief Set playerState.actionLockFlags bits 0x700 and cancel all red, blue
 * and yellow soul effects.
 */
void Player_LockAndCancelAllSouls(void)
{
    gEwramData->playerState.actionLockFlags |= 0x700;
    RedSoul_CancelAll();
    BlueSoul_StopAll();
    YellowSoul_Stop();
}

/**
 * @brief Unreferenced: clear the given flags of playerStatus.inputOverrides.
 */
void Player_ClearStatusFlags(s32 mask)
{
    gEwramData->playerStatus.inputOverrides &= ~mask;
}

/**
 * @brief 221C8 | To document
 * 
 */
void sub_080221C8(void)
{
    return;
}

extern void *sWeaponGfxTable[];
extern void *sWeaponSpriteSheetTable[];
extern u16 sWeaponSwingSounds[];
extern u16 sJuliusWhipSounds[];
/*
 * Weapon attack entities (Weapon*_Update, created by Weapon_CreateAttackEntity;
 * Julius' whip strike by Julius_CreateWhipAttackEntity): the attacking entity
 * (player or link partner) and the weapon that was equipped when the attack
 * started (the attack stops hitting when it changes). Heavy weapons (axes)
 * also keep the swing angle of their affine sprite, its speed per frame and
 * the frame on which the attack ends; the gun shot keeps a stop flag (set when
 * it hits a wall or an enemy). Token-identical expansion.
 */
#define WEAPON_OWNER(e) e->unk_4F8.unk_4F8_entity
#define WEAPON_ID(e) e->unk_4FC.part8.unk_4FC
#define WEAPON_SWING_ANGLE(e) e->unk_500.whole
#define WEAPON_SWING_SPEED(e) e->unk_504.whole
#define WEAPON_END_TIME(e) e->unk_4FC.part8.unk_4FD
#define WEAPON_GUN_STOPPED(e) e->unk_500.whole

extern const u8 sUnk_080E13B8[];

/**
 * @brief Attack entity for weapon types 0, 2 and 3 (swords, thrusting
 * weapons, fists): load the weapon sprite and sound, follow the owner
 * and take attack boxes from the animation frames until it ends.
 */
void WeaponMelee_Update(struct EwramData_EntityData *entity)
{
    s32 gfx;
    s32 type;
    s32 index;
    struct EwramData_EntityData *owner;
    struct EwramData_EntityData *spawner;
    struct WeaponInfo *attackerInfo;
    u32 paletteSlot;
    u8 frame;
    union EwramData_unk50C *ownerSlot;
    s32 spriteSlot;

    owner = WEAPON_OWNER(entity);
    switch (entity->state)
    {
        case 0:
            spawner = WEAPON_OWNER(entity);
            attackerInfo = Weapon_GetAttackerInfo(PLAYER_WEAPON_SLOT(spawner));
            WEAPON_ID(entity) = gEwramData->playerStatus.equippedWeapon;
            if (PLAYER_WEAPON_SLOT(spawner) == 0)
            {
                if (gEwramData->playerState.cachedWeaponGraphicsIndices[0] != attackerInfo->graphicsIndex)
                {
                    if (gEwramData->playerState.weaponGfx != -1)
                    {
                        ObjGfx_Unload(gEwramData->playerState.weaponGfx);
                        gEwramData->playerState.weaponGfx = -1;
                    }
                }
                paletteSlot = 1;
                entity->spriteLayer = 0xE;
            }
            else
            {
                if (gEwramData->playerState.cachedWeaponGraphicsIndex1 != attackerInfo->graphicsIndex)
                {
                    if (gEwramData->playerState.redSoulGfx != -1)
                    {
                        ObjGfx_Unload(gEwramData->playerState.redSoulGfx);
                        gEwramData->playerState.redSoulGfx = -1;
                    }
                }
                paletteSlot = 2;
                entity->spriteLayer = 0xF;
            }
            gfx = ObjGfx_LoadResource((u8 *) sWeaponGfxTable[attackerInfo->graphicsIndex], 0x10000);
            ObjPalette_LoadRange(sWeaponPalettes, (u16) attackerInfo->paletteIndex, 1, paletteSlot);
            Sprite_Attach(entity, (u8 *) sWeaponSpriteSheetTable[attackerInfo->spriteSheetIndex], gfx, (s32) paletteSlot);
            Anim_StartIndexed(entity, (u16) attackerInfo->animScriptIndex, 0, 0);
            gEwramData->playerState.cachedWeaponGraphicsIndices[PLAYER_WEAPON_SLOT(spawner)] = attackerInfo->graphicsIndex;
            if (PLAYER_WEAPON_SLOT(spawner) == 0)
            {
                gEwramData->playerState.weaponGfx = gfx;
                type = 3;
                index = 0;
            }
            else
            {
                gEwramData->playerState.redSoulGfx = gfx;
                type = 5;
                index = 1;
            }
            Hit_Enable(entity, type, WeaponAttack_OnHit, 0);
            entity->hit.flags |= 2;
            entity->hit.flags |= 4;
            Hit_SetTimerOverride(index, attackerInfo->hitTimer);
            ownerSlot = &WEAPON_OWNER(entity)->unk_50C;
            PlaySong(Weapon_GetAttackerInfo(ownerSlot->part8.unk_50C)->attackSound);
            if ((ownerSlot->part8.unk_50C == 0) && (1 & RandomNumberGenerator()))
            {
                PlaySong(sWeaponSwingSounds[RandomNumberGenerator() & 3]);
            }
            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            entity->timer = 0;
            entity->state += 1;
            if (PLAYER_ACTION_FLAGS(owner) & 0x400)
            {
                entity->velY.part16.integer = 0xD;
            }
            /* fallthrough */
        case 1:
            entity->roomActive = 1;
            if ((PLAYER_ACTION_FLAGS(owner) & 0x20) == 0)
            {
                gEwramData->playerState.weaponEntities[PLAYER_WEAPON_SLOT(owner)] = 0;
                EntityDelete(entity);
                return;
            }
            entity->posX.whole = owner->posX.whole;
            entity->posY.part16.integer = (u16) owner->posY.part16.integer + (u16) entity->velY.part16.integer;
            /* fallthrough */
        default:
            entity->drawSkip = owner->drawSkip;
            frame = entity->animFrame;
            if (Anim_Update(entity) != 3)
            {
                if (!(gEwramData->gameState.linkPlay) && ((u8) WEAPON_ID(entity) != gEwramData->playerStatus.equippedWeapon))
                {
                    entity->hit.flags |= 2;
                }
                else if (frame != entity->animFrame)
                {
                    spriteSlot = entity->spriteSlot;
                    if (gEwramData->spriteSlots[entity->spriteSlot].sheet->frames[entity->animFrame].boxCount)
                    {
                        HitBox_SetAttack(entity, gEwramData->spriteSlots[entity->spriteSlot].sheet->frames[entity->animFrame].boxes);
                        entity->hit.flags &= ~2;
                    }
                    else
                    {
                        entity->hit.flags |= 2;
                    }
                }
            }
            else
            {
                gEwramData->playerState.weaponEntities[PLAYER_WEAPON_SLOT(owner)] = 0;
                EntityDelete(entity);
            }
            break;
    }
}

/**
 * @brief Attack entity for weapon type 1 (great swords, katanas, axes and
 * hammers): rotate the affine weapon sprite through its swing arc
 * (angle step unk_504 for subState updates) while stepping up to 3 px
 * forward, taking frame attack boxes; free the affine slot on exit.
 */
void WeaponHeavy_Update(struct EwramData_EntityData *entity)
{
    struct WeaponInfo *info;
    s32 gfx;
    s32 type;
    s32 index;
    struct EwramData_EntityData *owner;
    struct EwramData_EntityData *spawner;
    struct WeaponInfo *attackerInfo;
    u16 affineIndex;
    u16 step;
    u32 offsetX;
    u32 paletteSlot;
    u8 frame;
    union EwramData_unk50C *ownerSlot;
    s32 spriteSlot;

    owner = WEAPON_OWNER(entity);
    info = Weapon_GetAttackerInfo(PLAYER_WEAPON_SLOT(owner));

    switch (entity->state)
    {
        case 0:
            affineIndex = ObjAffine_Alloc(0);
            if ((s32) (affineIndex << 0x10) < 0)
            {
                gEwramData->playerState.weaponEntities[PLAYER_WEAPON_SLOT(owner)] = 0;
                EntityDelete(entity);
                return;
            }
            entity->affineIndex = affineIndex;
            entity->affine = 1;
            spawner = WEAPON_OWNER(entity);
            attackerInfo = Weapon_GetAttackerInfo(PLAYER_WEAPON_SLOT(spawner));
            WEAPON_ID(entity) = gEwramData->playerStatus.equippedWeapon;
            if (PLAYER_WEAPON_SLOT(spawner) == 0)
            {
                if (gEwramData->playerState.cachedWeaponGraphicsIndices[0] != attackerInfo->graphicsIndex)
                {
                    if (gEwramData->playerState.weaponGfx != -1)
                    {
                        ObjGfx_Unload(gEwramData->playerState.weaponGfx);
                        gEwramData->playerState.weaponGfx = -1;
                    }
                }
                paletteSlot = 1;
                entity->spriteLayer = 0xE;
            }
            else
            {
                if (gEwramData->playerState.cachedWeaponGraphicsIndex1 != attackerInfo->graphicsIndex)
                {
                    if (gEwramData->playerState.redSoulGfx != -1)
                    {
                        ObjGfx_Unload(gEwramData->playerState.redSoulGfx);
                        gEwramData->playerState.redSoulGfx = -1;
                    }
                }
                paletteSlot = 2;
                entity->spriteLayer = 0xF;
            }
            gfx = ObjGfx_LoadResource((u8 *) sWeaponGfxTable[attackerInfo->graphicsIndex], 0x10000);
            ObjPalette_LoadRange(sWeaponPalettes, attackerInfo->paletteIndex, 1, paletteSlot);
            Sprite_Attach(entity, (u8 *) sWeaponSpriteSheetTable[attackerInfo->spriteSheetIndex], gfx, paletteSlot);
            Anim_StartIndexed(entity, attackerInfo->animScriptIndex, 0, 0);
            gEwramData->playerState.cachedWeaponGraphicsIndices[PLAYER_WEAPON_SLOT(spawner)] = attackerInfo->graphicsIndex;
            if (PLAYER_WEAPON_SLOT(spawner) == 0)
            {
                gEwramData->playerState.weaponGfx = gfx;
                type = 3;
                index = 0;
            }
            else
            {
                gEwramData->playerState.redSoulGfx = gfx;
                type = 5;
                index = 1;
            }
            Hit_Enable(entity, type, WeaponAttack_OnHit, 0);
            entity->hit.flags |= 2;
            entity->hit.flags |= 4;
            Hit_SetTimerOverride(index, attackerInfo->hitTimer);
            ownerSlot = &WEAPON_OWNER(entity)->unk_50C;
            PlaySong(Weapon_GetAttackerInfo(ownerSlot->part8.unk_50C)->attackSound);
            if ((ownerSlot->part8.unk_50C == 0) && (RandomNumberGenerator() & 1))
            {
                PlaySong(sWeaponSwingSounds[RandomNumberGenerator() & 3]);
            }
            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            WEAPON_SWING_ANGLE(entity) = -0x3000;
            if (info->attackVariant == 0)
            {
                entity->subState = 0x10;
                entity->stage = 5;
                WEAPON_SWING_SPEED(entity) = 0x980;
            }
            else
            {
                entity->subState = 0xC;
                entity->stage = 4;
                WEAPON_SWING_SPEED(entity) = 0xCC0;
            }
            WEAPON_END_TIME(entity) = sUnk_080E13B8[info->attackVariant];
            if (entity->flipX)
            {
                WEAPON_SWING_ANGLE(entity) = -WEAPON_SWING_ANGLE(entity);
                WEAPON_SWING_SPEED(entity) = -WEAPON_SWING_SPEED(entity);
            }
            entity->velY.part16.integer = -0x19;
            if (PLAYER_ACTION_FLAGS(owner) & 0x400)
            {
                entity->velY.part16.integer += 0xC;
            }
            entity->timer = 0;
            entity->state += 1;
            /* fallthrough */
        case 1:
            entity->roomActive = 1;
            if ((PLAYER_ACTION_FLAGS(owner) & 0x20) == 0)
            {
                gEwramData->playerState.weaponEntities[PLAYER_WEAPON_SLOT(owner)] = 0;
                ObjAffine_Free(entity->affineIndex);
                EntityDelete(entity);
                return;
            }
            entity->posY.part16.integer = (u16) owner->posY.part16.integer + (u16) entity->velY.part16.integer;
            entity->timer += 1;
            step = Div(entity->timer, entity->stage) + 1;
            offsetX = step;
            if ((s32) (s16) step > 3)
            {
                offsetX = 3;
            }
            if (entity->flipX)
            {
                offsetX = (u16) ((u32) (0 - (offsetX << 0x10)) >> 0x10);
            }
            entity->posX.part16.integer = owner->posX.part16.integer + offsetX;
            if (entity->timer < entity->subState)
            {
                WEAPON_SWING_ANGLE(entity) += WEAPON_SWING_SPEED(entity);
            }
            /* fallthrough */
        default:
            entity->drawSkip = owner->drawSkip;
            frame = entity->animFrame;
            Anim_Update(entity);
            if (!(gEwramData->gameState.linkPlay) && ((u8) WEAPON_ID(entity) != gEwramData->playerStatus.equippedWeapon))
            {
                entity->hit.flags |= 2;
                break;
            }
            if (entity->timer != WEAPON_END_TIME(entity))
            {
                if (frame != entity->animFrame)
                {
                    spriteSlot = entity->spriteSlot;
                    if (gEwramData->spriteSlots[entity->spriteSlot].sheet->frames[entity->animFrame].boxCount)
                    {
                        HitBox_SetAttack(entity, gEwramData->spriteSlots[entity->spriteSlot].sheet->frames[entity->animFrame].boxes);
                        entity->hit.flags &= ~2;
                    }
                    else
                    {
                        entity->hit.flags |= 2;
                    }
                }
                break;
            }
            gEwramData->playerState.weaponEntities[PLAYER_WEAPON_SLOT(owner)] = 0;
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
    }
    ObjAffine_Set(WEAPON_SWING_ANGLE(entity), 0x10000, 0x10000, entity->flipX, 0U, entity->affineIndex);
    return;
}

/**
 * @brief Ejected casing (frame 0x35): delete inside solid tiles, fall with
 * gravity, bounce once with sound 0x14F, then drift until it leaves
 * the screen.
 */
void Effect_UpdateGunCasing(struct EwramData_EntityData *entity)
{
    s32 tile;
    s32 solid;
    struct BgLayerState *bg;
    struct BgLayerState *bgLayer;
    s32 spawnX;
    s32 x;
    s32 gfx;

    switch (entity->state)
    {
        case 0:
            spawnX = (s16)entity->posX.part16.integer;
            bg = &gEwramData->bgInfo[1];
            tile = Collision_GetTileOnScreen(spawnX + (s16)bg->xPos.part16.integer, entity->posY.part16.integer + (s16)bg->yPos.part16.integer);
            if (tile != 0)
            {
                if (tile & 8)
                {
                    if (tile & 1)
                    {
                        solid = 1;
                    }
                    else
                    {
                        solid = 1;
                    }
                }
                else
                {
                    solid = 0;
                }

                if (solid != 0)
                {
                    entity->deleteRequested = 1;
                    return;
                }
            }

            gfx = ObjGfx_Find(sCommonObjectGfx);
            Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, 8);
            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            entity->animFrame = 0x35;
            entity->state = 1;
            entity->accelY.whole = 0x3000;
            /*fallthrough */
        case 1:
            EntityAccelerateAndMoveX(entity, 0x40000);
            EntityAccelerateAndMoveY(entity, 0x40000);

            x = (s16)entity->posX.part16.integer;
            bgLayer = &gEwramData->bgInfo[1];
            if (Collision_GetGroundOffset(x + (s16)bgLayer->xPos.part16.integer, entity->posY.part16.integer + (s16)bgLayer->yPos.part16.integer + 2) != 0)
            {
                PlaySong(0x14F);
                entity->velY.whole = -(entity->velY.whole / 2);
                entity->velX.whole += RandomSigned(0x4000);
                entity->state = 2;
            }
            break;

        case 2:
            EntityAccelerateAndMoveX(entity, 0x40000);
            EntityAccelerateAndMoveY(entity, 0x40000);
            break;
    }

    if (Entity_IsInRect(entity, -0x10, -0x10, 0x110, 0xC0) == 0)
    {
        entity->deleteRequested = 1;
    }
}

static inline void WeaponGun_SetAttackBox(struct EwramData_EntityData *entity, s32 box)
{
    HitBox_SetAttackRaw(entity, box);
}

/**
 * @brief Attack entity for weapon type 4 (Handgun, Silver Gun): eject a
 * casing, extend the hit position from the owner by 0x80000 per
 * update, stop at a wall with a spark and sound 0x14E, and end with
 * the animation or when the owner attack flag clears.
 */
void WeaponGun_Update(struct EwramData_EntityData *entity)
{
    s16 push;
    s32 gfx;
    s32 type;
    s32 index;
    struct EwramData_EntityData *owner;
    struct EwramData_EntityData *child;
    struct EwramData_EntityData *spawner;
    struct WeaponInfo *info;
    struct WeaponInfo *attackerInfo;
    s16 roomY;
    s16 roomX;
    s32 paletteSlot;

    owner = WEAPON_OWNER(entity);
    info = Weapon_GetAttackerInfo(PLAYER_WEAPON_SLOT(owner));

    switch (entity->state)
    {
        case 0:
            if (PLAYER_WEAPON_SLOT(owner) == 0)
            {
                PlaySong(info->attackSound);
            }
            spawner = WEAPON_OWNER(entity);
            attackerInfo = Weapon_GetAttackerInfo(PLAYER_WEAPON_SLOT(spawner));
            WEAPON_ID(entity) = gEwramData->playerStatus.equippedWeapon;

            if (PLAYER_WEAPON_SLOT(spawner) == 0)
            {
                if (gEwramData->playerState.cachedWeaponGraphicsIndices[0] != attackerInfo->graphicsIndex)
                {
                    if (gEwramData->playerState.weaponGfx != -1)
                    {
                        ObjGfx_Unload(gEwramData->playerState.weaponGfx);
                        gEwramData->playerState.weaponGfx = -1;
                    }
                }
                paletteSlot = 1;
                entity->spriteLayer = 0xE;
            }
            else
            {
                if (gEwramData->playerState.cachedWeaponGraphicsIndex1 != attackerInfo->graphicsIndex)
                {
                    if (gEwramData->playerState.redSoulGfx != -1)
                    {
                        ObjGfx_Unload(gEwramData->playerState.redSoulGfx);
                        gEwramData->playerState.redSoulGfx = -1;
                    }
                }
                paletteSlot = 2;
                entity->spriteLayer = 0xF;
            }
            
            gfx = ObjGfx_LoadResource((u8 *) sWeaponGfxTable[attackerInfo->graphicsIndex], 0x10000);
            ObjPalette_LoadRange(sWeaponPalettes, (u16) attackerInfo->paletteIndex, 1U, paletteSlot);
            Sprite_Attach(entity, (u8 *) sWeaponSpriteSheetTable[attackerInfo->spriteSheetIndex], gfx, (s32) paletteSlot);
            Anim_StartIndexed(entity, (u16) attackerInfo->animScriptIndex, 0U, 0U);
            gEwramData->playerState.cachedWeaponGraphicsIndices[PLAYER_WEAPON_SLOT(spawner)] = attackerInfo->graphicsIndex;

            if (PLAYER_WEAPON_SLOT(spawner) == 0)
            {
                gEwramData->playerState.weaponGfx = (u32) gfx;
                type = 3;
                index = 0;
            }
            else
            {
                gEwramData->playerState.redSoulGfx = (u32) gfx;
                type = 5;
                index = 1;
            }
            Hit_Enable(entity, type, WeaponGun_OnHit, 0);
            WeaponGun_SetAttackBox(entity, 0x0810FCF8);
            entity->hit.flags |= 2;
            entity->hit.flags |= 4;
            Hit_SetTimerOverride(index, (s32) attackerInfo->hitTimer);

            entity->drawFunc = (u32 *) WeaponGun_Draw;
            entity->accelX.whole = 0x80000;
            if (entity->flipX)
            {
                WEAPON_GUN_STOPPED(entity) = -WEAPON_GUN_STOPPED(entity);
                entity->unk_504.whole = -entity->unk_504.whole;
                entity->accelX.whole = -entity->accelX.whole;
            }
            entity->velX.whole += entity->accelX.whole;
            if ((PLAYER_ACTION_FLAGS(owner) & 0x400) != 0)
            {
                entity->velY.part16.integer = 0xC;
            }
            else
            {
                entity->velY.part16.integer = 0x0;
            }

            child = EntityCreateInRange(0x49, 0xCF, (u32 *) Effect_UpdateGunCasing);
            if (child != NULL)
            {
                Entity_PlaceRelativeTo(child, owner, 8U, entity->velY.part16.integer - 0x1A);
                child->velX.whole = (RandomNumberGenerator() & 0x7FFF) - 0x10000;
                child->velY.whole = -0x14000 - (RandomNumberGenerator() & 0x7FFF);
                if (child->flipX)
                {
                    child->velX.whole = -child->velX.whole;
                }
                child->scrollWithBg = 1;
            }

            entity->timer = 0;
            entity->state += 1;
            /* fallthrough */
        case 1:
            entity->roomActive = 1;
            if ((PLAYER_ACTION_FLAGS(owner) & 0x20) == 0)
            {
                gEwramData->playerState.weaponEntities[PLAYER_WEAPON_SLOT(owner)] = 0;
                EntityDelete(entity);
                break;
            }

            entity->timer += 1;
            if ((WEAPON_GUN_STOPPED(entity) == 0) && (entity->timer == 3))
            {
                entity->hit.flags &= ~2;
            }
            if (entity->timer > 3)
            {
                entity->velX.whole += entity->accelX.whole;
            }

            entity->posX.whole = owner->posX.whole + entity->velX.whole;
            entity->posY.whole = owner->posY.whole + entity->velY.whole - 0x1C0000;
            if (WEAPON_GUN_STOPPED(entity) == 0)
            {
                roomX = GetEntityRoomXPositionInteger(entity);
                roomY = GetEntityRoomYPositionInteger(entity);
                if (entity->accelX.whole > 0)
                {
                    push = Collision_GetPushLeftFiltered(roomX, roomY, 0);
                }
                else
                {
                    push = Collision_GetPushRightFiltered(roomX, roomY, 0);
                }
                if (push != 0)
                {
                    Effect_SpawnShared(entity->posX.whole + (push << 0x10), entity->posY.whole, 7, 8);
                    WEAPON_GUN_STOPPED(entity) = 1;
                    PlaySong(0x14E);
                }
            }
            else
            {
                entity->hit.flags |= 2;
            }
            /* fallthrough */
        default:
            entity->drawSkip = owner->drawSkip;
            if (Anim_Update(entity) == 3)
            {
                gEwramData->playerState.weaponEntities[PLAYER_WEAPON_SLOT(owner)] = 0;
                EntityDelete(entity);
            }
            break;
    }
}

/**
 * @brief Attack entity for weapon type 5, used only by table entry 58
 * (Valmanway): follow the owner without a player attack animation and
 * take frame attack boxes while the weapon stays equipped.
 */
void WeaponValmanway_Update(struct EwramData_EntityData *entity)
{
    s32 gfx;
    s32 type;
    s32 index;
    struct EwramData_EntityData *owner;
    struct WeaponInfo *attackerInfo;
    s32 paletteSlot;
    u8 frame;
    union EwramData_unk50C *ownerSlot;
    s32 spriteSlot;
    struct EwramData_EntityData *spawner;

    owner = WEAPON_OWNER(entity);

    switch (entity->state)
    {
        case 0:
            spawner = WEAPON_OWNER(entity);
            attackerInfo = Weapon_GetAttackerInfo(PLAYER_WEAPON_SLOT(spawner));
            WEAPON_ID(entity) = gEwramData->playerStatus.equippedWeapon;

            if (PLAYER_WEAPON_SLOT(spawner) == 0)
            {
                if (gEwramData->playerState.cachedWeaponGraphicsIndices[0] != attackerInfo->graphicsIndex)
                {
                    if (gEwramData->playerState.weaponGfx != -1)
                    {
                        ObjGfx_Unload(gEwramData->playerState.weaponGfx);
                        gEwramData->playerState.weaponGfx = -1;
                    }
                }
                paletteSlot = 1;
                entity->spriteLayer = 0xE;
            }
            else
            {
                if (gEwramData->playerState.cachedWeaponGraphicsIndex1 != attackerInfo->graphicsIndex)
                {
                    if (gEwramData->playerState.redSoulGfx != -1)
                    {
                        ObjGfx_Unload(gEwramData->playerState.redSoulGfx);
                        gEwramData->playerState.redSoulGfx = -1;
                    }
                }
                paletteSlot = 2;
                entity->spriteLayer = 0xF;
            }

            gfx = ObjGfx_LoadResource((u8 *) sWeaponGfxTable[attackerInfo->graphicsIndex], 0x10000);
            ObjPalette_LoadRange(sWeaponPalettes, attackerInfo->paletteIndex, 1, paletteSlot);
            Sprite_Attach(entity, (u8 *) sWeaponSpriteSheetTable[attackerInfo->spriteSheetIndex], gfx, paletteSlot);
            Anim_StartIndexed(entity, attackerInfo->animScriptIndex, 0, 0);
            gEwramData->playerState.cachedWeaponGraphicsIndices[PLAYER_WEAPON_SLOT(spawner)] = attackerInfo->graphicsIndex;

            if (PLAYER_WEAPON_SLOT(spawner) == 0)
            {
                gEwramData->playerState.weaponGfx = gfx;
                type = 3;
                index = 0;
            }
            else
            {
                gEwramData->playerState.redSoulGfx = gfx;
                type = 5;
                index = 1;
            }
            Hit_Enable(entity, type, WeaponAttack_OnHit, 0);
            entity->hit.flags |= 2;
            entity->hit.flags |= 4;
            Hit_SetTimerOverride(index, attackerInfo->hitTimer);

            ownerSlot = &WEAPON_OWNER(entity)->unk_50C;
            PlaySong(Weapon_GetAttackerInfo(ownerSlot->part8.unk_50C)->attackSound);
            if ((ownerSlot->part8.unk_50C == 0) && (1 & RandomNumberGenerator()))
            {
                PlaySong(sWeaponSwingSounds[3 & RandomNumberGenerator()]);
            }

            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            if (PLAYER_ACTION_FLAGS(owner) & 0x400)
            {
                entity->velY.part16.integer = 0xD;
            }

            entity->timer = 0;
            entity->state += 1;
            /* fallthrough */
        case 1:
            entity->roomActive = 1;
            entity->posX.whole = owner->posX.whole;
            entity->posY.part16.integer = (u16) owner->posY.part16.integer + (u16) entity->velY.part16.integer;
            break;
    }

    entity->drawSkip = owner->drawSkip;
    frame = entity->animFrame;

    if (Anim_Update(entity) != 3)
    {
        if (!(gEwramData->gameState.linkPlay) && ((u8) WEAPON_ID(entity) != gEwramData->playerStatus.equippedWeapon))
        {
            entity->hit.flags |= 2;
        }
        else if (frame != entity->animFrame)
        {
            spriteSlot = entity->spriteSlot;
            if (gEwramData->spriteSlots[entity->spriteSlot].sheet->frames[entity->animFrame].boxCount)
            {
                HitBox_SetAttack(entity, gEwramData->spriteSlots[entity->spriteSlot].sheet->frames[entity->animFrame].boxes);
                entity->hit.flags &= ~2;
            }
            else
            {
                entity->hit.flags |= 2;
            }
        }
    }
    else
    {
        gEwramData->playerState.weaponEntities[PLAYER_WEAPON_SLOT(owner)] = 0;
        EntityDelete(entity);
    }
}

extern void (*sWeaponAttackUpdateFuncs[])(struct EwramData_EntityData *);

/**
 * @brief Create the attack entity for the attacker slot (unk_50C) from
 * sWeaponAttackUpdateFuncs indexed by the weapon type (info unk_8), store it in
 * playerState.weaponEntities[slot] with owner and facing, and return it.
 */
struct EwramData_EntityData* Weapon_CreateAttackEntity(struct EwramData_EntityData *entity)
{
    struct WeaponInfo *attackerInfo;

    attackerInfo = Weapon_GetAttackerInfo(PLAYER_WEAPON_SLOT(entity));
    if (attackerInfo == NULL)
    {
        return 0;
    }

    gEwramData->playerState.weaponEntities[PLAYER_WEAPON_SLOT(entity)] = EntityCreateInRange(2, 0x10, (u32 *) sWeaponAttackUpdateFuncs[attackerInfo->attackAnimKind]);
    if (gEwramData->playerState.weaponEntities[PLAYER_WEAPON_SLOT(entity)] != NULL)
    {
        WEAPON_OWNER(gEwramData->playerState.weaponEntities[PLAYER_WEAPON_SLOT(entity)]) = entity;
        gEwramData->playerState.weaponEntities[PLAYER_WEAPON_SLOT(entity)]->flipX = entity->flipX;
        gEwramData->playerState.weaponEntities[PLAYER_WEAPON_SLOT(entity)]->scrollWithBg = 1;
    }

    return gEwramData->playerState.weaponEntities[PLAYER_WEAPON_SLOT(entity)];
}

/**
 * @brief Julius whip strike hit entity: follow Julius with mirrored facing,
 * use the second attack box of his current frame and play a random
 * voice after 10 updates; delete when his attack flag clears.
 */
void JuliusWhipAttack_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner;
    s32 spriteSlot;

    owner = WEAPON_OWNER(entity);

    switch (entity->state)
    {
        case 0:
            Hit_Enable(entity, 3, Player_MarkAttackContact, 0);
            entity->hit.flags |= 2;
            entity->hit.flags |= 4;
            Hit_SetTimerOverride(0, 0x14);
            entity->timer = 0xA;
            entity->state += 1;
            /* fallthrough */
        case 1:
            if (entity->timer != 0)
            {
                entity->timer -= 1;
                if (entity->timer == 0)
                {
                    // Original reads the 2-entry table at 0x084F126C, not sWeaponSwingSounds (draft bug).
                    PlaySong(sJuliusWhipSounds[RandomNumberGenerator() & 1]);
                }
            }

            entity->roomActive = 1;
            if ((PLAYER_ACTION_FLAGS(owner) & 0x20) == 0)
            {
                gEwramData->playerState.weaponEntities[PLAYER_WEAPON_SLOT(owner)] = 0;
                EntityDelete(entity);
                break;
            }

            entity->posX.whole = owner->posX.whole;
            entity->posY.whole = owner->posY.whole;
            entity->flipX = owner->flipX ^ 1;

            spriteSlot = owner->spriteSlot;
            if (gEwramData->spriteSlots[owner->spriteSlot].sheet->frames[owner->animFrame].boxCount == 2)
            {
                // Next 4-byte record (+4 bytes in the original); the draft predates unk_8 becoming a typed pointer.
                HitBox_SetAttack(entity, gEwramData->spriteSlots[owner->spriteSlot].sheet->frames[owner->animFrame].boxes + 1);
                entity->hit.flags &= ~2;
            }
            else
            {
                entity->hit.flags |= 2;
            }
            break;
    }
}

/**
 * @brief Create the whip strike hit entity in playerState.weaponEntities[slot]
 * with owner and facing, and return it.
 */
struct EwramData_EntityData* Julius_CreateWhipAttackEntity(struct EwramData_EntityData *entity)
{
    gEwramData->playerState.weaponEntities[PLAYER_WEAPON_SLOT(entity)] = EntityCreateInRange(2, 0x10, (u32 *) JuliusWhipAttack_Update);

    if (gEwramData->playerState.weaponEntities[PLAYER_WEAPON_SLOT(entity)] != NULL)
    {
        WEAPON_OWNER(gEwramData->playerState.weaponEntities[PLAYER_WEAPON_SLOT(entity)]) = entity;
        gEwramData->playerState.weaponEntities[PLAYER_WEAPON_SLOT(entity)]->flipX = entity->flipX;
        gEwramData->playerState.weaponEntities[PLAYER_WEAPON_SLOT(entity)]->scrollWithBg = 1;
        gEwramData->playerState.weaponEntities[PLAYER_WEAPON_SLOT(entity)]->roomActive = 1;
    }

    return gEwramData->playerState.weaponEntities[PLAYER_WEAPON_SLOT(entity)];
}

extern struct WeaponInfo sUnarmedWeaponInfo;
extern struct WeaponInfo sWeaponInfo[];

static inline struct WeaponInfo *Weapon_GetInfoInline(s32 equippedWeapon)
{
    if (equippedWeapon != 0xFF)
    {
        return &sWeaponInfo[equippedWeapon];
    }
    return &sUnarmedWeaponInfo;
}

/**
 * @brief Return the weapon table entry for id, or the bare-hand entry
 * sUnarmedWeaponInfo for 0xFF.
 */
struct WeaponInfo* Weapon_GetInfo(u8 weaponId)
{
    if (weaponId != 0xFF)
    {
        return &sWeaponInfo[weaponId];
    }
    return &sUnarmedWeaponInfo;
}

/**
 * @brief 23334 | To document
 * 
 * @return struct WeaponInfo* To document
 */

/**
 * @brief Return Weapon_GetInfo of the equipped weapon.
 */
struct WeaponInfo* Weapon_GetEquippedInfo(void)
{
    return Weapon_GetInfoInline(gEwramData->playerStatus.equippedWeapon);
}

/**
 * @brief Return the weapon info for attacker slot 0 (equipped weapon) or
 * another slot (unk_133F2).
 */
struct WeaponInfo* Weapon_GetAttackerInfo(s32 slot)
{
    if (slot == 0)
    {
        return Weapon_GetInfoInline(gEwramData->playerStatus.equippedWeapon);
        
    }
    return Weapon_GetInfoInline(gEwramData->unk_133F2);
}

extern u8 *sWeaponAttackAnims[];

/**
 * @brief Return the player animation for the equipped weapon from table
 * sWeaponAttackAnims[pose]: poses 0..2 (standing, crouching, airborne)
 * also depend on info attackVariant; other poses only on the weapon type.
 */
u8 Weapon_GetPlayerAttackAnim(s32 attackType)
{
    struct WeaponInfo *attackerInfo;
    u8 *anims;
    u8 temp_r1;

    attackerInfo = Weapon_GetInfoInline(gEwramData->playerStatus.equippedWeapon);

    anims = sWeaponAttackAnims[attackType];
    if (attackerInfo == NULL)
    {
        return *anims;
    }

    if (attackType <= 2)
    {
        return anims[(attackerInfo->attackAnimKind * 3) + attackerInfo->attackVariant];
    }
    else
    {
        return anims[attackerInfo->attackAnimKind];
    }
}

/**
 * @brief Return equipped weapon flag 0x2000; when set, landing keeps the
 * attack flags 0x20/0x40 instead of clearing them.
 */
s32 Weapon_KeepsAttackOnLanding(void)
{
    struct WeaponInfo *attackerInfo;

    attackerInfo = Weapon_GetInfoInline(gEwramData->playerStatus.equippedWeapon);
    if (attackerInfo != NULL)
    {
        return attackerInfo->attackAttributes & 0x2000;
        
    }
    return 0;
}

/**
 * @brief Hit callback of melee attack entities: set player contact flag
 * 0x80000 (used by the dive-attack rebound), restricted to the
 * selected target when gameState.linkPlay is set.
 */
void WeaponAttack_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker)
{
    if (!(gEwramData->gameState.linkPlay) || (attacker == gEwramData->playerState.selectedTarget))
    {
        PLAYER_ACTION_FLAGS(gEwramData->playerState.player) |= 0x80000;
    }
}

/**
 * @brief Gun hit callback: disable further hits, mark the shot as stopped
 * and set the player contact flag as WeaponAttack_OnHit.
 */
void WeaponGun_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker)
{
    entity->hit.flags |= 2;
    WEAPON_GUN_STOPPED(entity) = 1;

    if (!(gEwramData->gameState.linkPlay) || (attacker == gEwramData->playerState.selectedTarget))
    {
        PLAYER_ACTION_FLAGS(gEwramData->playerState.player) |= 0x80000;
    }
}

/**
 * @brief Draw the gun sprite at the owner position offset by velY,
 * restoring the extended hit position afterwards.
 */
void WeaponGun_Draw(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner;
    s32 oldX;
    s32 oldY;

    owner = WEAPON_OWNER(entity);
    oldX = entity->posX.whole;
    oldY = entity->posY.whole;

    entity->posX.whole = owner->posX.whole;
    entity->posY.whole = owner->posY.whole + entity->velY.whole;
    Sprite_DrawEntity(entity);

    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}

static inline s8 RedSoul_FindFreeSlot()
{
    s8 i;

    for (i = 0; i < 8; i++)
    {
        if (gEwramData->playerState.activeRedSoulEntities[i] == NULL)
        {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Create a red soul entity with the given update in slots 2..0x10 and
 * register it in the first free active red soul slot (owner, slot index,
 * count); NULL when no entity or list slot is free.
 */
struct EwramData_EntityData* RedSoul_CreateEntity(struct EwramData_EntityData *owner, u32 *update)
{
    struct EwramData_EntityData *entity;
    s8 slot;

    entity = EntityCreateInRange(2, 0x10, update);
    if (entity == NULL)
    {
        return NULL;
    }

    slot = RedSoul_FindFreeSlot();
    if (slot < 0)
    {
        EntityDelete(entity);
        return NULL;
    }

    gEwramData->playerState.activeRedSoulEntities[slot] = entity;
    RED_SOUL_OWNER(entity) = owner;
    RED_SOUL_SLOT(entity) = slot;
    entity->roomActive = 1;
    entity->scrollWithBg = 1;
    gEwramData->playerState.nbrActiveRedSoulEntities += 1;
    return entity;
}

/**
 * @brief Delete every active red soul entity (freeing affine slots) and clear the
 * list; end a running time stop (timeStopTimer) and restore its sounds; reset the
 * tracked sounds, the red soul count and enemyTarget, and for Soma end an
 * active soul-use substate.
 */
void RedSoul_CancelAll(void)
{
    s8 i;
    struct EwramData_EntityData *soul;

    for (i = 0; i < 8; i++)
    {
        soul = gEwramData->playerState.activeRedSoulEntities[i];
        if (soul != NULL)
        {
            if (soul->updateFunc != NULL)
            {
                if (soul->affineIndex || soul->affine)
                {
                    ObjAffine_Free(soul->affineIndex);
                }
                EntityDelete(soul);
            }
            gEwramData->playerState.activeRedSoulEntities[i] = 0;
        }
    }

    if (gEwramData->paletteInvert != 0)
    {
        gEwramData->paletteInvert = 1;
    }

    if (gEwramData->gameState.timeStopTimer != 0)
    {
        gEwramData->gameState.timeStopTimer = 0;
        if (!(gEwramData->gameState.stateFlags & 0x40000))
        {
            PlaySong(0x1038);
        }
        Sound_ResumeAndReplayTracked();

        if (gEwramData->musicOverrideSong != 0)
        {
            PlaySong(gEwramData->musicOverrideSong);
        }

        if (gEwramData->specialRoomSong != 0)
        {
            PlaySong(gEwramData->specialRoomSong);
        }
        Sound_StopTracked(0x16E);
    }

    Sound_ClearTracked();
    gEwramData->playerState.enemyTarget = gEwramData->playerState.player;
    gEwramData->playerState.nbrActiveRedSoulEntities = 0;
    Player_ResetContactImmunity();

    if (gEwramData->playerStatus.currentCharacter == 0)
    {
        if ((gEwramData->playerState.player->state == 2) && (gEwramData->playerState.player->subState != 0))
        {
            gEwramData->playerState.player->subState = gEwramData->playerStatus.currentCharacter;
            PLAYER_ACTION_FLAGS(gEwramData->playerState.player) |= 0x200000;
            PLAYER_ACTION_FLAGS(gEwramData->playerState.player) &= ~0x100;
        }
    }
}

static inline s32 RedSoul_NoneActive()
{
    s32 noneActive;

    if ((gEwramData->playerState.nbrActiveRedSoulEntities == 0) && (gEwramData->playerState.activeRedSoulEntities[0] == NULL))
    {
        noneActive = 1;
    }
    else
    {
        noneActive = 0;
    }
    return noneActive;
}

/**
 * @brief Shift the 8-entry history, record the source soul position and
 * frame, and show the entry selected by subState. Delete when no red
 * soul is active or the source slot is empty.
 */
void RedSoulTrail_Update(struct EwramData_EntityData *entity)
{
    u16 *posHistory;
    struct EwramData_EntityData *source;
    u8 *frameHistory;
    s32 noneActive;
    s32 i;
    struct BgLayerState *bg;
    s32 histX;
    s32 curState;

    // TODO: what is happening with these fields?
    posHistory = &entity->unk_4F8.part16.unk_4F8;
    source = gEwramData->playerState.activeRedSoulEntities[entity->stage];
    frameHistory = &entity->velX.part8.fraction0;

    noneActive = RedSoul_NoneActive();
    if ((noneActive != 0) || (gEwramData->playerState.activeRedSoulEntities[entity->stage] == NULL))
    {
        EntityDelete(entity);
        return;
    }

    curState = entity->state;
    if (curState == 0)
    {
        entity->drawFunc = (u32*) Sprite_DrawEntity;
        entity->state += 1;
    }

    for (i = 7; i > 0; i--)
    {
        frameHistory[i * 1 + 0] = frameHistory[i * 1 - 1];

        posHistory[i * 2 + 0] = posHistory[i * 2 - 2];
        posHistory[i * 2 + 1] = posHistory[i * 2 - 1];
    }

    posHistory[0] = GetEntityRoomXPositionInteger(source);
    posHistory[1] = GetEntityRoomYPositionInteger(source);
    frameHistory[0] = source->animFrame;
    entity->animFrame = frameHistory[entity->subState];

    histX = posHistory[entity->subState * 2 + 0];
    bg = &gEwramData->bgInfo[1];
    entity->posX.part16.integer = posHistory[entity->subState * 2 + 0] - bg->xPos.part16.integer;
    entity->posY.part16.integer = posHistory[entity->subState * 2 + 1] - bg->yPos.part16.integer;
}

/**
 * @brief Create a sprite copy of a red-soul entity that replays its room
 * position and frame `delay` updates late from an 8-entry history,
 * primed with the current state. Return NULL on failure.
 */
struct EwramData_EntityData* RedSoul_CreateTrailCopy(struct EwramData_EntityData *entity, u8 delay)
{
    s32 tileSlot;
    u32 i;
    struct EwramData_EntityData *trail;
    struct SpriteSheet *sheet;
    struct SpriteSheet *attachSheet;
    u8 *frames;
    u8 *spriteSlot;

    tileSlot = Sprite_GetTileSlot(entity);
    spriteSlot = &entity->spriteSlot;
    sheet = (gEwramData->spriteSlots[*spriteSlot].sheet);
    attachSheet = sheet;

    trail = EntityCreateInRange(0x49, 0xCF, (u32 *) RedSoulTrail_Update);
    if (trail == NULL)
    {
        return NULL;
    }

    if (sheet == NULL)
    {
        EntityDelete(trail);
        return NULL;
    }

    Sprite_Attach(trail, (u8*)attachSheet, tileSlot, entity->palette);
    trail->animFrame = entity->animFrame;
    trail->spriteLayer = entity->spriteLayer + 1;
    frames = (u8*)&trail->velX;

    trail->flipX = entity->flipX;
    trail->subState = delay;
    trail->stage = (u8) (u16) entity->unk_4FC.part16.unk_4FE[0];

    for (i = 7; i > 0; i--)
    {
        frames[i] = entity->animFrame;
        ((u16*)&trail->unk_4F8.part16.unk_4F8)[i * 2] = GetEntityRoomXPositionInteger(entity);
        ((u16*)&trail->unk_4F8.part16.unk_4FA)[i * 2] = GetEntityRoomYPositionInteger(entity);
    }

    trail->scrollWithBg = 1;
    return trail;
}

/**
 * @brief Move the entity down in 8 px steps (up to five probes) onto the
 * first floor. Return 1 when it landed and is not embedded, else 0.
 */
s32 Entity_DropToFloor(struct EwramData_EntityData *entity)
{
    s32 i;
    s32 found;
    s16 push;
    s16 roomY;
    s16 roomX;
    s16 y;

    found = 0;
    roomX = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);

    for (i = 0; i < 5; i++)
    {
        if ((push = Collision_GetPushUp(roomX, y)) != 0)
        {
            entity->posY.part16.integer += push;
            found = 1;
            break;
        }
        y += 8;
        entity->posY.part16.integer += 8;
    }

    roomY = GetEntityRoomYPositionInteger(entity);
    if ((found == 0) || (Collision_GetPushUp(roomX, roomY) != 0))
    {
        return 0;
    }
    return 1;
}

/**
 * @brief Keep the owner soul entity at its hit target plus the stored offset
 * (accelX/accelY). Delete when no red soul is active, the owner is
 * gone or reused; when the target HP (unk_518) is not positive, set
 * unk_508 to signal the owner instead.
 */
void RedSoulAnchor_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *follower;
    struct EwramData_EntityData *target;
    s32 noneActive;

    follower = ENTITY_OWNER(entity);
    target = RED_SOUL_ANCHOR_TARGET(entity);
    
    if ((gEwramData->playerState.nbrActiveRedSoulEntities == 0) && (gEwramData->playerState.activeRedSoulEntities[0] == NULL))
    {
        noneActive = 1;
    }
    else
    {
        noneActive = 0;
    }

    if ((noneActive != 0) || (follower->updateFunc == NULL) || (follower->unk_4F4.part8.unk_4F4 != entity->unk_4F4.part8.unk_4F4))
    {
        EntityDelete(entity);
        return;
    }

    if ((s16) target->unk_518.part16.hp <= 0)
    {
        RED_SOUL_ANCHOR_TARGET_DEAD(entity) = 1;
        return;
    }
    follower->posX.part16.integer = target->posX.part16.integer + entity->accelX.part16.integer;
    follower->posY.part16.integer = target->posY.part16.integer + entity->accelY.part16.integer;
}

struct BladeSoulVariant {
    void *graphics;
    void *palette;
    void *spriteSheet;
    u8 animFrame;
    u8 unk_D;
    u8 paletteIndex;
    u8 unk_F;
};
extern struct BladeSoulVariant sBladeSoulVariants[];

static inline void SkillLoadGraphicsInline(struct EwramData_EntityData *entity, u8 first, void *resource, void *palette, void *sheet)
{
    gEwramData->playerState.redSoulGfx = ObjGfx_LoadResource(resource, 0x10000);
    ObjPalette_LoadRange(palette, first, 1, 2);
    Sprite_Attach(entity, sheet, gEwramData->playerState.redSoulGfx, 2);
}

/**
 * @brief 23A70 | To document
 * 
 * @param entity To document
 */
void Skill_EvilButcher_Ripper_Use(struct EwramData_EntityData *entity)
{
    s32 push;
    struct EwramData_EntityData *anchor;
    struct EwramData_EntityData *owner;
    struct BladeSoulVariant *info;
    u16 affineIndex;
    s16 roomY;
    s16 roomX;
    struct EwramData_EntityData *target;

    info = &sBladeSoulVariants[(u16)RED_SOUL_PARAM(entity)];

    switch (entity->state)
    {
        case 0:
            SkillLoadGraphicsInline(entity, info->paletteIndex, info->graphics, info->palette, info->spriteSheet);
            entity->animFrame = info->animFrame;
            if ((u16) RED_SOUL_PARAM(entity) == 0)
            {
                Hit_SetTimerOverride(1, 0xA);
            }
            else
            {
                Hit_SetTimerOverride(1, 0x14);
            }
            affineIndex = ObjAffine_Alloc(0);
            if ((s32) (affineIndex << 0x10) < 0)
            {
                gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
                EntityDelete(entity);
                return;
            }
            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            entity->spriteLayer = 0xD;
            entity->affineIndex = affineIndex;
            entity->affine = 1;
            owner = RED_SOUL_OWNER(entity);
            entity->posX.part16.integer = owner->posX.part16.integer;
            entity->posY.part16.integer = (u16) owner->posY.part16.integer + 0xFFE6;
            entity->scrollWithBg = 1;
            entity->state += 1;
            entity->flipX = owner->flipX == 1;
            ObjAffine_Set(0, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
            if (entity->flipX)
            {
                entity->velX.whole = -0x60000;
            }
            else
            {
                entity->velX.whole = 0x60000;
            }
            Hit_Enable(entity, 5, RedSoul_AnchorToHitTargetTimed, 0);
            HitBox_SetAttackRaw(entity, 0x0202FFFF);
            entity->hit.flags |= 4;
            PlaySong(0xE6U);
            gEwramData->playerState.nbrActiveRedSoulEntities += 1;
            /* fallthrough */
        case 1:
            anchor = RED_SOUL_ANCHOR(entity);
            if (anchor != NULL)
            {
                if (RED_SOUL_ANCHOR_TARGET_DEAD(anchor) == 0)
                {
                    target = RED_SOUL_ANCHOR_TARGET(anchor);
                    if (RED_SOUL_ANCHOR_TIMER(entity)++ > 20)
                    {
                        entity->state = 4;
                    }
                    else if ((target->hit.flags & 4) || ((u16) RED_SOUL_PARAM(entity) == 0))
                    {
                        entity->state = 4;
                    }
                    else
                    {
                        entity->timer -= 1;
                        if (entity->timer == 0)
                        {
                            entity->hit.flags |= 2;
                            entity->state = 3;
                            entity->timer = 15;
                        }
                        goto block_33;
                    }
                }
                else
                {
                    entity->state = 4;
                    goto block_33;
                }
            }
            else
            {
                entity->posX.whole += entity->velX.whole;
                roomX = GetEntityRoomXPositionInteger(entity);
                roomY = GetEntityRoomYPositionInteger(entity);
                if (entity->flipX)
                {
                    push = Collision_GetPushRight(roomX - 6, roomY);
                }
                else
                {
                    push = Collision_GetPushLeft(roomX + 6, roomY);
                }
                if (push != 0)
                {
                    entity->posX.whole += push << 0x10;
                    entity->timer = 50;
                    entity->hit.flags |= 2;
                    if (info->unk_F == 0)
                    {
                        PlaySong(0x138);
                        entity->state = 2;
                    }
                    else
                    {
                        entity->state = 3;
                    }
                    if (entity->flipX)
                    {
                        entity->velX.whole = 0x18000;
                    }
                    else
                    {
                        entity->velX.whole = -0x18000;
                    }
                    entity->velY.whole = -0x20000;
                    entity->accelY.whole = 0x2000;
                }
block_33:
                if ((EntityIsOutsideScreen(entity, 4) != 0) || !(entity->roomActive))
                {
                    entity->roomActive = 1;
                    entity->hidden = 1;
                    entity->state = 4;
                }
            }
            break;

        case 2:
            entity->posX.whole += entity->velX.whole;
            entity->posY.whole += entity->velY.whole;
            entity->velY.whole += entity->accelY.whole;
            ObjAffine_Set((SKILL_RIPPER_SPIN(entity) << 0x10) / 20, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
            if (entity->flipX)
            {
                SKILL_RIPPER_SPIN(entity) += 1;
            }
            else
            {
                SKILL_RIPPER_SPIN(entity) -= 1;
            }
            /* fallthrough */
        case 3:
            if (entity->timer > 10)
            {
                entity->drawSkip = 1;
            }
            else
            {
                entity->drawSkip = 2;
            }
            if (entity->timer != 0)
            {
                entity->timer -= 1;
                break;
            }
            /* fallthrough */
        case 4:
            ObjAffine_Free(entity->affineIndex);
            gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities -= 1;
            EntityDelete(entity);
            return;
    }

    entity->roomActive = 1;
}

struct SoulGraphicsResources {
    void *graphics;
    void *palette;
    void *spriteSheet;
};
extern struct SoulGraphicsResources sSkillGraphicsSets[];

/**
 * @brief Unreferenced red-soul projectile: thrown horizontally from the
 * caster's head, spinning; it decelerates, turns back and accelerates up
 * to 8 px/frame, and is freed off screen.
 */
void SkillBoomerangProjectileUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner;
    u16 affineIndex;

    switch (entity->state)
    {
        case 0:
            SkillLoadGraphicsInline(entity, 0, sSkillGraphicsSets->graphics, sSkillGraphicsSets->palette, sSkillGraphicsSets->spriteSheet);
            Anim_StartIndexed(entity, (u16) RED_SOUL_PARAM(entity), 0, 1);
            if ((u16) RED_SOUL_PARAM(entity) == 0)
            {
                Hit_SetTimerOverride(1, 0xF);
            }
            else
            {
                Hit_SetTimerOverride(1, 0x1E);
            }
            affineIndex = ObjAffine_Alloc(0);
            if ((affineIndex << 0x10) < 0)
            {
                gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
                EntityDelete(entity);
                return;
            }
            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            entity->affineIndex = affineIndex;
            entity->affine = 1;
            owner = RED_SOUL_OWNER(entity);
            entity->posX.part16.integer = owner->posX.part16.integer;
            entity->posY.part16.integer = (u16) owner->posY.part16.integer + 0xFFE6;
            entity->scrollWithBg = 1;
            SKILL_BOOMERANG_SPIN(entity) = 0;
            entity->spriteLayer = 0xD;
            entity->state += 1;
            if (owner->flipX)
            {
                entity->velX.whole = -0x34000;
                entity->accelX.whole = 0x1000;
                SKILL_BOOMERANG_SPIN_DIR(entity) = -1;
            }
            else
            {
                entity->velX.whole = 0x34000;
                entity->accelX.whole = -0x1000;
                SKILL_BOOMERANG_SPIN_DIR(entity) = 1;
            }
            Hit_Enable(entity, 5, NULL, 0);
            HitBox_SetAttackRaw(entity, 0x1414F6F6);
            entity->hit.flags |= 4;
            gEwramData->playerState.nbrActiveRedSoulEntities += 1;
            /* fallthrough */
        case 1:
            entity->posX.whole += entity->velX.whole;
            if ((entity->accelX.whole < 0) && (entity->velX.whole > -0x80000))
            {
                entity->velX.whole += entity->accelX.whole;
            }
            else if ((entity->accelX.whole > 0) && (entity->velX.whole < 0x80000))
            {
                entity->velX.whole += entity->accelX.whole;
            }
            ObjAffine_Set(SKILL_BOOMERANG_SPIN(entity) << 7, 0x10000, 0x10000, 0, 0, entity->affineIndex);
            SKILL_BOOMERANG_SPIN(entity) += SKILL_BOOMERANG_SPIN_DIR(entity);
            if (EntityIsOutsideScreen(entity, 4) != 0)
            {
                entity->state = 2;
            }
            entity->roomActive = 1;
            break;

        case 2:
            ObjAffine_Free(entity->affineIndex);
            gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities -= 1;
            EntityDelete(entity);
            return;
    }
    Anim_Update(entity);
}

/**
 * @brief Affine trail sprite created by SkillAxeArmorUse that copies the owner
 * frame and follows its position history.
 */
void SkillAxeArmorTrailUpdate(struct EwramData_EntityData *entity)
{
    s32 i;
    struct EwramData_EntityData *owner;
    u16 *history;
    u16 affineIndex;
    struct BgLayerState *bg;
    s32 x;

    owner = RED_SOUL_OWNER(entity);
    history = &entity->unk_504.part16.unk_504;

    switch (entity->state)
    {
        case 0:
            affineIndex = ObjAffine_Alloc(0);
            if ((affineIndex << 0x10) < 0)
            {
                gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
                gEwramData->playerState.nbrActiveRedSoulEntities -= 1;
                EntityDelete(entity);
                return;
            }

            SkillLoadGraphicsInline(entity, 0, sSkillGraphicsSets->graphics, sSkillGraphicsSets->palette, sSkillGraphicsSets->spriteSheet);
            entity->animFrame = owner->unk_500.part16.unk_502;
            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            entity->affineIndex = affineIndex;
            entity->affine = 1;
            entity->objMode = 1;
            entity->posX.part16.integer = GetEntityRoomXPositionInteger(owner);

            for (i = 0; i < 10; i++)
            {
                history[i] = entity->posX.part16.integer;
            }
            entity->state += 1;
            /* fallthrough */
        case 1:
            if (owner->state == 2)
            {
                ObjAffine_Free(entity->affineIndex);
                gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
                gEwramData->playerState.nbrActiveRedSoulEntities -= 1;
                EntityDelete(entity);
                return;
            }

            for (i = 9; i > 0; i--)
            {
                history[i + 0] = history[i - 1];
            }
            history[0] = GetEntityRoomXPositionInteger(owner);

            x = history[entity->subState];
            bg = &gEwramData->bgInfo[1];
            entity->posX.part16.integer = history[entity->subState] - bg->xPos.part16.integer;
            entity->posY.part16.integer = owner->posY.part16.integer;

            entity->velX.whole += entity->accelX.whole;
            break;
    }

    entity->roomActive = 1;
    ObjAffine_Set(entity->velX.whole, 0x10000, 0x10000, 0U, 0U, entity->affineIndex);
}

/**
 * @brief 24274 | To document
 * 
 * @param entity To document
 */
void SkillAxeArmorUse(struct EwramData_EntityData *entity)
{
    s32 i;
    struct EwramData_EntityData *child;
    struct EwramData_EntityData *owner;
    u16 affineIndex;

    switch (entity->state)
    {
        case 0:
            SkillLoadGraphicsInline(entity, 0, sSkillGraphicsSets->graphics, sSkillGraphicsSets->palette, sSkillGraphicsSets->spriteSheet);
            entity->animFrame = RED_SOUL_PARAM(entity);
            if ((u16) RED_SOUL_PARAM(entity) == 0)
            {
                Hit_SetTimerOverride(1, 0xF);
            }
            else
            {
                Hit_SetTimerOverride(1, 0x1E);
            }

            affineIndex = ObjAffine_Alloc(0);
            if ((affineIndex << 0x10) < 0)
            {
                gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
                EntityDelete(entity);
                return;
            }

            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            entity->affineIndex = affineIndex;
            entity->affine = 1;
            owner = RED_SOUL_OWNER(entity);
            entity->posX.part16.integer = owner->posX.part16.integer;
            entity->posY.part16.integer = (u16) owner->posY.part16.integer + 0xFFE6;
            entity->scrollWithBg = 1;
            SKILL_BOOMERANG_SPIN(entity) = 0;
            entity->spriteLayer = 0xD;
            entity->state += 1;
            if (owner->flipX)
            {
                entity->velX.whole = -0x34000;
                entity->accelX.whole = 0x1000;
                SKILL_BOOMERANG_SPIN_DIR(entity) = -1;
            }
            else
            {
                entity->velX.whole = 0x34000;
                entity->accelX.whole = -0x1000;
                SKILL_BOOMERANG_SPIN_DIR(entity) = 1;
            }

            Hit_Enable(entity, 5, NULL, 0);
            HitBox_SetAttackRaw(entity, 0x1414F6F6);
            entity->hit.flags |= 4;
            PlaySong(0xE3);
            gEwramData->playerState.nbrActiveRedSoulEntities += 1;

            for (i = 1; i < 3; i++)
            {
                child = RedSoul_CreateEntity(entity, (u32 *) SkillAxeArmorTrailUpdate);
                if (child != NULL)
                {
                    child->posX.whole = entity->posX.whole;
                    child->posY.whole = entity->posY.whole;
                    child->accelX.whole = 0x800;
                    child->subState = i * 3;
                    if (owner->flipX)
                    {
                        child->accelX.whole = -child->accelX.whole;
                    }
                    child->velX.whole = -child->accelX.whole * child->subState;
                    child->spriteLayer = entity->spriteLayer + i;
                }
            }
            /* fallthrough */
        case 1:
            entity->posX.whole += entity->velX.whole;
            if ((entity->accelX.whole < 0) && (entity->velX.whole > -0x80000))
            {
                entity->velX.whole = entity->velX.whole + entity->accelX.whole;
            }
            else if ((entity->accelX.whole > 0) && (entity->velX.whole < 0x80000))
            {
                entity->velX.whole = entity->velX.whole + entity->accelX.whole;
            }

            ObjAffine_Set(SKILL_BOOMERANG_SPIN(entity) << 0xB, 0x10000, 0x10000, 0, 0, entity->affineIndex);
            SKILL_BOOMERANG_SPIN(entity) += SKILL_BOOMERANG_SPIN_DIR(entity);
            if (EntityIsOutsideScreen(entity, 4) != 0)
            {
                entity->state = 2;
            }
            entity->roomActive = 1;
            break;

        case 2:
            if (entity->affineIndex || entity->affine)
            {
                ObjAffine_Free(entity->affineIndex);
            }
            gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities -= 1;
            EntityDelete(entity);
            break;
    }
}

/* Convert the target from 16.16 with truncation toward zero, then subtract
 * the integer origin. Retain the explicit sign branches for agbcc matching. */
static inline s16 ProjectileIntegerDelta(s32 targetFixed, s32 originInteger)
{
    s32 delta;

    if (targetFixed < 0)
    {
        delta = -(-targetFixed >> 0x10) - originInteger;
    }
    else
    {
        delta = (targetFixed >> 0x10) - originInteger;
    }
    return delta;
}

static inline s32 CheckUnk_53C_6(struct EwramData_EntityData *entity)
{
    return entity->flipX == 0;
}

/**
 * @brief Unreferenced: initialize, steer, move, and release a homing red-soul
 * projectile.
 */
void HomingSoulProjectile_Update(struct EwramData_EntityData *entity)
{
    u32 nearestDistanceSquared;
    s32 initialTargetX;
    s32 angleDelta;
    s32 moveY;
    s32 candidateDy;
    s32 candidateDx;
    s32 initialTargetY;
    s32 moveX;
    s32 steeringDx;
    s32 steeringComponent;
    s32 initialDx;
    s32 initialComponent;
    s32 targetY;
    s32 targetX;
    s32 targetIndex;
    struct EwramData_EntityData *owner;
    struct EwramData_EntityData *target;
    struct EwramData_EntityData *player;
    struct EwramData_EntityData *selectedTarget;
    struct EwramData_EntityData *nearestTarget;
    struct SoulGraphicsResources *graphicsEntry;
    struct SoulGraphicsResources *graphicsTable;
    u16 affineSlot;
    s32 angle;
    u32 distanceSquared;
    s32 cosine;
    s32 speed;
    s32 sine;
    s32 collisionFlags;

    switch (entity->state)
    {
        case 0:
            /* Keep the table base distinct from its selected entry. */
            graphicsTable = sSkillGraphicsSets;
            graphicsEntry = &graphicsTable[4];
            SkillLoadGraphicsInline(entity, 0, graphicsEntry->graphics, graphicsEntry->palette, graphicsEntry->spriteSheet);
            Anim_StartIndexed(entity, RED_SOUL_PARAM(entity), 0U, 1U);
            affineSlot = ObjAffine_Alloc(0);
            if ((affineSlot << 0x10) < 0)
            {
                gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = 0;
                EntityDelete(entity);
                return;
            }
            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            entity->affineIndex = affineSlot;
            entity->affine = 1;
            owner = RED_SOUL_OWNER(entity);
            entity->posX.part16.integer = owner->posX.part16.integer;
            entity->posY.part16.integer = owner->posY.part16.integer - 0x10;
            entity->scrollWithBg = 1;
            entity->unk_50C.part16.unk_50E = 0;
            entity->spriteLayer = 0xD;
            Hit_SetTimerOverride(1, 0x1E);
            Hit_Enable(entity, 5, HomingSoulProjectile_OnPlayerContact, 0);
            HitBox_SetAttackRaw(entity, 0x1414F6F6);
            collisionFlags = entity->hit.flags;
            entity->hit.flags = collisionFlags | 4;
            gEwramData->playerState.nbrActiveRedSoulEntities += 1;
            entity->state += 1;
            nearestDistanceSquared = -1;
            nearestTarget = NULL;
            selectedTarget = gEwramData->playerState.selectedTarget;
            if (selectedTarget == NULL)
            {
                for (targetIndex = 0; targetIndex < 4; targetIndex++)
                {
                    if (gEwramData->playerState.trackedEntities[targetIndex] != NULL)
                    {
                        candidateDx = (s16) gEwramData->playerState.trackedEntities[targetIndex]->posX.part16.integer - (s16) entity->posX.part16.integer;
                        candidateDy = gEwramData->playerState.trackedEntities[targetIndex]->posY.part16.integer - entity->posY.part16.integer;
                        distanceSquared = (candidateDx * candidateDx) + (candidateDy * candidateDy);
                        if (nearestDistanceSquared > distanceSquared)
                        {
                            nearestDistanceSquared = distanceSquared;
                            nearestTarget = gEwramData->playerState.trackedEntities[targetIndex];
                        }
                    }
                }
                selectedTarget = nearestTarget;
            }
            HOMING_SOUL_TARGET(entity) = selectedTarget;
            target = HOMING_SOUL_TARGET(entity);
            if (target != NULL)
            {
                initialTargetX = target->posX.whole;
                initialTargetY = target->posY.whole;

                /* The component temporary holds origin X first, then delta Y.
                 * Sharing it preserves the inline-result copy in agbcc. */
                initialDx = ProjectileIntegerDelta(initialTargetX, (s16) (initialComponent = entity->posX.part16.integer));
                initialComponent = ProjectileIntegerDelta(initialTargetY, (s16) entity->posY.part16.integer);
                HOMING_SOUL_HEADING(entity) = ArcTan2(initialDx, initialComponent) + 0x8000;
            }
            else if (CheckUnk_53C_6(gEwramData->playerState.player) == 1)
            {
                HOMING_SOUL_HEADING(entity) = 0x8000;
            }
            /* fallthrough */
        case 1:
            target = HOMING_SOUL_TARGET(entity);
            if ((target != NULL) && ((s16) target->unk_518.part16.hp > 0))
            {
                targetX = target->posX.whole;
                targetY = target->posY.whole;
            }
            else
            {
                player = gEwramData->playerState.player;
                if (CheckUnk_53C_6(player) == 1)
                {
                    targetX = player->posX.whole + 0xC00000;
                }
                else
                {
                    targetX = player->posX.whole - 0xC00000;
                }
                targetY = gEwramData->playerState.player->posY.whole;
            }
            angle = HOMING_SOUL_HEADING(entity);

            /* As above, reuse the component temporary for the Y result. */
            steeringDx = ProjectileIntegerDelta(targetX, (s16) (steeringComponent = entity->posX.part16.integer));
            steeringComponent = ProjectileIntegerDelta(targetY, (s16) entity->posY.part16.integer);
            angleDelta = ArcTan2(steeringDx, steeringComponent) - angle;
            if (angleDelta > 0)
            {
                if (angleDelta < 0x8000)
                {
                    angle += 0x400;
                }
                else
                {
                    angle -= 0x400;
                }
            }
            else if (angleDelta < -0x8000)
            {
                angle += 0x400;
            }
            else
            {
                angle -= 0x400;
            }
            HOMING_SOUL_HEADING(entity) = angle & 0xFFFF;
            HOMING_SOUL_SPEED(entity) += 0x2000;
            if (HOMING_SOUL_SPEED(entity) > 0x60000)
            {
                HOMING_SOUL_SPEED(entity) = 0x60000;
            }
            if (entity->timer++ > 0x78)
            {
                entity->state = 2;
            }

        case 2:
            break;

        case 3:
            ObjAffine_Free(entity->affineIndex);
            gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities -= 1;
            EntityDelete(entity);
            return;
    }

    cosine = Sin(HOMING_SOUL_HEADING(entity) + 0x4000);
    speed = HOMING_SOUL_SPEED(entity);
    moveX = (cosine >> 8) * (speed >> 8);
    sine = Sin(HOMING_SOUL_HEADING(entity));
    speed = HOMING_SOUL_SPEED(entity);
    moveY = (sine >> 8) * (speed >> 8);
    entity->posX.whole += moveX;
    entity->posY.whole += moveY;

    ObjAffine_Set(HOMING_SOUL_HEADING(entity) + 0x4000, 0x10000, 0x10000, 0U, 0U, entity->affineIndex);
    if (Entity_IsInRect(entity, -0x20, -0x20, 0x130, 0xE0) == 0)
    {
        entity->state = 3;
    }
    if (!(entity->roomActive))
    {
        entity->state = 3;
    }
    Anim_Update(entity);
    return;
}

/**
 * @brief 24950 | To document
 * 
 * @param entity To document
 */
void SkillChronomageUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner;
    struct SoulGraphicsResources *info;
    struct SoulGraphicsResources *infos;
    u16 affineIndex;

    owner = RED_SOUL_OWNER(entity);
    entity->roomActive = 1;

    switch (entity->state)
    {
        case 0:
            affineIndex = ObjAffine_Alloc(0);
            if ((affineIndex << 0x10) < 0)
            {
                gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
                EntityDelete(entity);
                return;
            }

            infos = sSkillGraphicsSets;
            info = &infos[1];
            SkillLoadGraphicsInline(entity, 0, info->graphics, info->palette, info->spriteSheet);
            Anim_StartIndexed(entity, 0U, 0U, 0U);
            entity->drawFunc = (u32 *) Soul_DrawWithPaletteInvert;
            entity->spriteLayer = 0xD;
            entity->affineIndex = affineIndex;
            entity->affine = 1;
            entity->scrollWithBg = 1;

            gEwramData->gameState.timeStopTimer = RED_SOUL_PARAM(entity);
            gEwramData->playerState.nbrActiveRedSoulEntities += 1;
            PlaySong(0x1030);
            PlaySong(0x19D);
            PlaySong(0xEC);
            entity->state += 1;
            /* fallthorugh */
        case 1:
            entity->posX.whole = owner->posX.whole;
            entity->posY.part16.integer = owner->posY.part16.integer - 0x30;
            SKILL_CHRONOMAGE_SCALE(entity) += 0x1000;
            SKILL_CHRONOMAGE_RAMP(entity) += 0x2000;
            if (SKILL_CHRONOMAGE_SCALE(entity) > 0x10000)
            {
                SKILL_CHRONOMAGE_SCALE(entity) = 0x10000;
            }
            if (SKILL_CHRONOMAGE_RAMP(entity) > 0x10000)
            {
                SKILL_CHRONOMAGE_RAMP(entity) = 0x10000;
            }
            SKILL_CHRONOMAGE_INVERT(entity) = SKILL_CHRONOMAGE_SCALE(entity);

            if (gEwramData->gameState.timeStopTimer != 0)
            {
                if (!(gEwramData->gameState.timeStopTimer & 0xF))
                {
                    Sound_PlayTrackedIfAbsent(0x16E);
                }
                gEwramData->gameState.timeStopTimer -= 1;
                break;
            }

            if (!(gEwramData->gameState.stateFlags & 0x40000))
            {
                PlaySong(0x1038);
            }
            Sound_ResumeAndReplayTracked();

            if (gEwramData->musicOverrideSong != 0)
            {
                PlaySong(gEwramData->musicOverrideSong);
            }

            if (gEwramData->specialRoomSong != 0)
            {
                PlaySong(gEwramData->specialRoomSong);
            }
            Sound_StopTracked(0x16E);

            entity->state += 1;
            entity->timer = 0x20;
            /* fallthrough */
        case 2:
            SKILL_CHRONOMAGE_SCALE(entity) -= 0x1000;
            SKILL_CHRONOMAGE_RAMP(entity) -= 0x800;
            if (SKILL_CHRONOMAGE_SCALE(entity) < 0)
            {
                SKILL_CHRONOMAGE_SCALE(entity) = 0;
            }
            if (SKILL_CHRONOMAGE_RAMP(entity) < 0)
            {
                SKILL_CHRONOMAGE_RAMP(entity) = 0;
            }

            if (entity->timer == 0)
            {
                gEwramData->paletteInvert = 1;
                ObjAffine_Free(entity->affineIndex);
                gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
                gEwramData->playerState.nbrActiveRedSoulEntities -= 1;
                EntityDelete(entity);
                return;
            }

            entity->timer -= 1;
            SKILL_CHRONOMAGE_INVERT(entity) = entity->timer << 0xB;
            break;
    }

    gEwramData->paletteInvert = SKILL_CHRONOMAGE_INVERT(entity);
    ObjAffine_Set(0, SKILL_CHRONOMAGE_SCALE(entity), SKILL_CHRONOMAGE_SCALE(entity), entity->flipX, 0, entity->affineIndex);
    Anim_Update(entity);
}

/**
 * @brief Unreferenced: compute the centre of the first box of the current
 * animation frame (mirrored by the flip flags) as 16.16 screen
 * coordinates into x/y and return 1; return 0 without boxes.
 */
s32 Sprite_GetFirstBoxCenter(struct EwramData_EntityData *entity, u32 *outX, u32 *outY)
{
    struct SpriteBox *boxes;
    s32 spriteSlot;
    s16 x;
    s16 y;

    spriteSlot = entity->spriteSlot;
    boxes = gEwramData->spriteSlots[entity->spriteSlot].sheet->frames[entity->animFrame].boxes;
    if (boxes != NULL)
    {
        x = boxes->x + (boxes->width >> 1);
        if (entity->flipX)
        {
            x = -x;
        }
        x += (s16) entity->posX.part16.integer;

        y = boxes->y + (boxes->height >> 1);
        if (entity->flipY)
        {
            y = -y;
        }
        y += (s16) entity->posY.part16.integer;

        *outX = x << 0x10;
        *outY = y << 0x10;
        return 1;
    }
    return 0;
}

/**
 * @brief 24CBC | To document
 * 
 * @param entity To document
 */
void SkillValkyrieUse(struct EwramData_EntityData *entity)
{
    s32 baseY;
    s32 y;
    s32 baseX;
    s32 x;
    s32 offsetX;
    struct EwramData_EntityData *anchor;
    struct SoulGraphicsResources *info;
    struct SoulGraphicsResources *infos;
    struct SpriteBox *boxes;
    s32 spriteSlot;
    s32 offsetY;

    anchor = RED_SOUL_OWNER(entity);

    switch (entity->state)
    {
        case 0:
            infos = sSkillGraphicsSets;
            info = &infos[2];
            SkillLoadGraphicsInline(entity, 0, info->graphics, info->palette, info->spriteSheet);
            Anim_StartIndexed(entity, 0, 0, 0);
            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            entity->spriteLayer = 0xD;
            entity->scrollWithBg = 1;

            Entity_PlaceRelativeTo(entity, anchor, 0x14, -0x10);
            Hit_SetTimerOverride(1, 0x3C);
            gEwramData->playerState.nbrActiveRedSoulEntities += 1;
            Hit_Enable(entity, 5, NULL, 0);
            entity->hit.flags |= 4;
            entity->hit.flags &= ~2;
            PlaySong(0x1AF);
            entity->state += 1;
            /* fallthrough */
        case 1:
            entity->timer += 1;
            if (entity->timer > 0x13)
            {
                if ((entity->timer & 3) == 0)
                {
                    offsetX = (entity->timer * 0xC000) + 0x200000;
                    offsetY = 0x380000 - (entity->timer * 0x1F000);
                    if (entity->flipX)
                    {
                        offsetX = -offsetX;
                    }
                    baseX = entity->posX.whole + offsetX;
                    baseY = entity->posY.whole + offsetY;
                    x = baseX + (((RandomNumberGenerator() & 0x1F) - 0x10) << 0x10);
                    y = baseY + (((RandomNumberGenerator() & 0x1F) - 0x10) << 0x10);
                    Effect_CreateFromTable(x, y, 0, ((RandomNumberGenerator() & 0x1F) * 0x10) - 0x2000, 0, 6);
                }

                if (Entity_IsInRect(entity, -0x40, -8, 0x170, 0xB0) != 0)
                {
                    entity->hidden = 0;
                }
                else
                {
                    entity->hidden = 1;
                }
            }
            break;

        case 2:
            if (entity->timer != 0)
            {
                entity->timer -= 1;
            }
            else
            {
                gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = 0;
                gEwramData->playerState.nbrActiveRedSoulEntities -= 1;
                entity->deleteRequested = 1;
            }
            break;
    }
    entity->roomActive = 1;
    if ((Anim_Update(entity) == 3) && (entity->state != 2))
    {
        entity->state = 2;
        entity->timer = 0xA;
        entity->drawSkip = 1;
    }

    spriteSlot = entity->spriteSlot;
    boxes = gEwramData->spriteSlots[entity->spriteSlot].sheet->frames[entity->animFrame].boxes;
    if (boxes != NULL)
    {
        entity->hit.flags &= ~2;
        HitBox_SetAttack(entity, boxes);
    }
    else
    {
        entity->hit.flags |= 2;
    }
}

/**
 * @brief 24F38 | To document
 * 
 * @param entity To document
 */
void SkillKillerDollUse(struct EwramData_EntityData *entity)
{
    s16 pushUp;
    s16 pushDown;
    s32 newX;
    s32 friction;
    struct EwramData_EntityData *anchor;
    struct SoulGraphicsResources *info;
    struct SoulGraphicsResources *infos;
    s16 roomY;
    s16 roomX;
    s16 push;

    anchor = RED_SOUL_OWNER(entity);
    friction = 0x100;

    switch (entity->state)
    {
        case 0:
            infos = sSkillGraphicsSets;
            info = &infos[6];
            SkillLoadGraphicsInline(entity, 0, info->graphics, info->palette, info->spriteSheet);
            Anim_StartIndexed(entity, RandomNumberGenerator() & 1, 0U, 1U);
            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            entity->spriteLayer = 0xD;
            entity->scrollWithBg = 1;

            PlaySong(0xE5);
            gEwramData->playerState.nbrActiveRedSoulEntities += 1;
            gEwramData->playerState.enemyTarget = entity;
            Entity_PlaceRelativeTo(entity, anchor, 8, -0x10);
            entity->velX.whole = 0x18000;
            if (entity->flipX)
            {
                entity->velX.whole = -entity->velX.whole;
            }

            entity->velY.whole = -0x28000;
            entity->timer = 0xB4;
            entity->state += 1;
            /* fallthrough */
        case 1:
            entity->posX.whole += entity->velX.whole;
            entity->posY.whole += entity->velY.whole;
            roomX = GetEntityRoomXPositionInteger(entity);
            roomY = GetEntityRoomYPositionInteger(entity);
            entity->velY.whole = entity->velY.whole + 0x2000;
            if (entity->velY.whole > 0x80000)
            {
                entity->velY.whole = 0x80000;
            }

            if (entity->velY.whole >= 0)
            {
                pushUp = Collision_GetPushUp(roomX, roomY + 1);
                if (pushUp != 0)
                {
                    entity->posY.part16.integer += pushUp + 2;
                    entity->velY.whole = 0;
                    friction = 0x4000;
                }
            }
            else
            {
                pushDown = Collision_GetPushDown(roomX, roomY - 0x20);
                if (pushDown != 0)
                {
                    entity->posY.part16.integer += pushDown;
                    entity->velY.whole = 0;
                }
            }

            if (entity->velX.whole > 0)
            {
                push = Collision_GetPushLeft(roomX + 8, roomY - 0x10);
                entity->velX.whole -= friction;
                if (entity->velX.whole <= 0)
                {
                    entity->velX.whole = 0;
                }

                if (push != 0)
                {
                    entity->posX.part16.integer = newX = push + entity->posX.part16.integer;
                    entity->velX.whole = -(entity->velX.whole >> 1);
                }
            }
            else if (entity->velX.whole < 0)
            {
                push = Collision_GetPushRight(roomX - 8, roomY - 0x10);
                entity->velX.whole += friction;
                if (entity->velX.whole >= 0)
                {
                    entity->velX.whole = 0;
                }

                if (push != 0)
                {
                    entity->posX.part16.integer = newX = push + entity->posX.part16.integer;
                    entity->velX.whole = -(entity->velX.whole >> 1);
                }
            }

            if (entity->timer == 0)
            {
                entity->drawSkip = 1;
                entity->timer = 0x1E;
                entity->state += 1;
            }
            else
            {
                entity->timer -= 1;
                break;
            }
            /* fallthrough */
        case 2:
            if (entity->timer == 0)
            {
                gEwramData->playerState.enemyTarget = gEwramData->playerState.player;
                gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
                gEwramData->playerState.nbrActiveRedSoulEntities -= 1;
                EntityDelete(entity);
                return;
            }
            entity->timer -= 1;
            break;
    }

    if ((EntityIsOutsideScreen(entity, 2) != 0) || !(entity->roomActive))
    {
        entity->roomActive = 1;
        gEwramData->playerState.enemyTarget = gEwramData->playerState.player;
        gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities -= 1;
        EntityDelete(entity);
    }
    else
    {
        Anim_Update(entity);
    }
}

/**
 * @brief 251F0 | To document
 * 
 * @param entity To document
 */
void SkillNemesisUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *anchor;
    struct SoulGraphicsResources *info;
    struct SoulGraphicsResources *infos;

    anchor = RED_SOUL_OWNER(entity);

    switch (entity->state)
    {
        case 0:
            infos = sSkillGraphicsSets;
            info = &infos[21];
            SkillLoadGraphicsInline(entity, 0, info->graphics, info->palette, info->spriteSheet);
            Anim_StartIndexed(entity, 2U, 0U, 1U);
            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            entity->spriteLayer = 0xF;
            entity->scrollWithBg = 1;

            gEwramData->playerState.nbrActiveRedSoulEntities += 1;
            PlaySong(0x97);
            gEwramData->playerState.enemyTarget = 0;
            SKILL_NEMESIS_TIMER(entity) = 0x12C;
            entity->state += 1;
            /* fallthrough */
        case 1:
            if (SKILL_NEMESIS_TIMER(entity) != 0)
            {
                SKILL_NEMESIS_TIMER(entity) -= 1;
                break;
            }

            entity->drawSkip = 1;
            entity->timer = 0x1E;
            entity->state += 1;
            /* fallthrough */
        case 2:
            if (entity->timer == 0)
            {
                gEwramData->playerState.enemyTarget = gEwramData->playerState.player;
                gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = 0;
                gEwramData->playerState.nbrActiveRedSoulEntities -= 1;
                EntityDelete(entity);
                return;
            }

            entity->timer -= 1;
            break;
    }

    if (entity->subState < 0x1E)
    {
        entity->subState += 1;
    }

    Entity_PlaceRelativeTo(entity, anchor, 0, -8 - entity->subState);
    Anim_Update(entity);
    entity->roomActive = 1;
}
