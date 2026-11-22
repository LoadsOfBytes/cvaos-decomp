/* Blue and yellow soul helpers 0x08031D6C..0x08032443. */
#include "code_08009A0.h"
#include "yellow_soul_entity.h"
#include "blue_soul_entity.h"
#include "player_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 BlueSoul_CheckEndCondition(struct EwramData_EntityData *entity);
extern void Player_RestoreHP(s32 amount);

/* code/code_080211F0.h declares Entity_PlaceRelativeTo with s32 offsets (its callers
 * pass them that way); the function itself truncates them to u16, so this
 * file does not include that header. */

/* Blue soul info, indexed by equippedBlueSoul (12-byte records). */
struct BlueSoulInfo {
    void *updateFunc; // TODO: function signature
    u8 pad_4[0xC - 0x4];
};

extern const struct BlueSoulInfo sBlueSoulInfo[];

/**
 * @brief Return the equipped blue-soul record without adding index validation.
 */
const struct BlueSoulInfo *BlueSoul_GetEquippedInfo(void)
{
    return &sBlueSoulInfo[gEwramData->playerStatus.equippedBlueSoul];
}

/**
 * @brief Load graphics into the shared blue-soul resource slot, load one palette range and attach the supplied sprite set.
 */
void BlueSoul_LoadGraphics(struct EwramData_EntityData *entity, u8 *resource, u8 *palette, void *sheet, u8 first)
{
    gEwramData->playerState.blueSoulGfx = ObjGfx_LoadResource(resource, 0x10000);
    ObjPalette_LoadRange(palette, first, 1, 3);
    Sprite_Attach(entity, sheet, gEwramData->playerState.blueSoulGfx, 3);
}

/**
 * @brief Return the first NULL active-blue-soul slot among eight, or signed -1.
 */
s8 BlueSoul_FindFreeEntitySlot(void)
{
    s8 i;

    for (i = 0; i < 8; i++)
    {
        if (gEwramData->playerState.activeBlueSoulEntities[i] == NULL)
            return i;
    }
    return -1;
}

/**
 * @brief Release affine state under the existing condition, clear the indexed active-blue-soul slot and delete the supplied entity.
 */
void BlueSoul_DeleteEntity(struct EwramData_EntityData *entity)
{
    if (entity->affineIndex != 0 || entity->affine)
        ObjAffine_Free(entity->affineIndex);
    gEwramData->playerState.activeBlueSoulEntities[BLUE_SOUL_SLOT(entity)] = NULL;
    EntityDelete(entity);
}

/**
 * @brief Mark the entity active and advance to the supplied higher state only if the end-condition check succeeds; return one only on that transition.
 */
s32 BlueSoul_TryAdvanceEndingState(struct EwramData_EntityData *entity, u8 endState)
{
    entity->roomActive = 1;
    if (entity->state < endState && BlueSoul_CheckEndCondition(entity))
    {
        entity->state = endState;
        return 1;
    }
    return 0;
}

/**
 * @brief 31EA4 | Place the entity at (offsetX, offsetY) px from anchor,
 * mirrored and facing like anchor.
 */
void Entity_PlaceRelativeTo(struct EwramData_EntityData *entity, struct EwramData_EntityData *anchor, u16 offsetX, u16 offsetY)
{
    s16 dx = offsetX;
    s16 dy = offsetY;

    entity->flipX = anchor->flipX;
    if (entity->flipX)
        dx = -dx;
    entity->posX.part16.integer = (s32)dx + (s16)anchor->posX.part16.integer;
    entity->posY.part16.integer = (s32)dy + anchor->posY.part16.integer;
}

/**
 * @brief Empty contact callback of SkillWitchGiantGhostProjectileUpdate.
 */
void BlueSoulProjectile_IgnoreContact(void)
{
}

/**
 * @brief Alternate sprite layer between 0x14 and 0xE and draw the entity; preserve the behavior for any initial layer.
 */
void BlueSoul_DrawAlternatingLayer(struct EwramData_EntityData *entity)
{
    if (entity->spriteLayer != 0x14)
        entity->spriteLayer = 0x14;
    else
        entity->spriteLayer = 0xE;
    Sprite_DrawEntity(entity);
}

/**
 * @brief Restore five HP only when the other entity hit type is eight; retain the unused first callback argument.
 */
void PersephoneSoul_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker)
{
    if (attacker->hit.type == 8)
        Player_RestoreHP(5);
}

/* Yellow soul info, indexed by equippedYellowSoul (12-byte records). */
struct YellowSoulInfo {
    u32 *updateFunc; // update function of the soul entity (TODO: signature)
    u16 allowUnforcedRestart; // nonzero: may be started without param_1 (YellowSoul_Restart)
    u16 effectParam; // -> unk_4FE
    s32 effectValue; // -> unk_500 (effect value)
};

extern const struct YellowSoulInfo sYellowSoulInfo[];
extern void Player_RecalculateAttackDefense(void);
extern void Player_ClearStatusEffects(s32 mask);
extern void Player_RestoreMP(s32 amount);

/**
 * @brief 31F3C | Create the entity of the equipped yellow soul (slot 0x11,
 * linked to param_0) unless one exists, actionLockFlags bit 10 is set, or the
 * flag gameFlagsTail[0] bit 21 is set and the soul is > 2.
 */
/* The original used these as inline functions; YellowSoul_GetInfo, YellowSoul_Stop
 * and YellowSoul_GetEquippedInfo are their out-of-line copies. */
static inline const struct YellowSoulInfo *GetYellowSoulInfo(u8 soul)
{
    return &sYellowSoulInfo[soul];
}

static inline void StopYellowSoul(void)
{
    gEwramData->playerStatus.soulStateFlags &= 0xF8003FFF;
    if (gEwramData->playerState.activeYellowSoulEntity != NULL)
    {
        EntityDelete(gEwramData->playerState.activeYellowSoulEntity);
        gEwramData->playerState.activeYellowSoulEntity = NULL;
    }
}

static inline const struct YellowSoulInfo *GetYellowSoul(void)
{
    return &sYellowSoulInfo[gEwramData->playerStatus.equippedYellowSoul];
}
/**
 * @brief Create and initialize the equipped yellow-soul entity linked to the supplied owner under the existing mode, callback and active-entity guards.
 */
void YellowSoul_CreateEquippedEntity(struct EwramData_EntityData *entity)
{
    const struct YellowSoulInfo *yellowSoul = GetYellowSoul();
    struct EwramData_EntityData *child;

    if (gEwramData->playerState.actionLockFlags & 0x400)
        return;
    if ((gEwramData->gameState.gameFlagsTail[0] & 0x200000) && gEwramData->playerStatus.equippedYellowSoul > 2)
        return;
    if (yellowSoul->updateFunc == NULL || gEwramData->playerState.activeYellowSoulEntity != NULL)
        return;
    child = EntityCreateInRange(0x11, 0x11, yellowSoul->updateFunc);
    ENTITY_OWNER(child) = entity;
    YELLOW_SOUL_PARAM(child) = yellowSoul->effectParam;
    YELLOW_SOUL_VALUE(child) = yellowSoul->effectValue;
    gEwramData->playerState.activeYellowSoulEntity = child;
}

/**
 * @brief Under the existing guards replace the yellow-soul entity, apply the soul-0xC stat adjustment and run the new callback once; preserve allocation assumptions.
 */
void YellowSoul_Restart(u8 soul, s32 force)
{
    const struct YellowSoulInfo *yellowSoul = GetYellowSoulInfo(soul);
    struct EwramData_EntityData *child;

    if (yellowSoul->updateFunc == NULL)
        return;
    if (gEwramData->playerState.actionLockFlags & 0x400)
        return;
    if ((gEwramData->gameState.gameFlagsTail[0] & 0x200000) && gEwramData->playerStatus.equippedYellowSoul > 2)
        return;
    if (force == 0 && yellowSoul->allowUnforcedRestart == 0)
        return;
    StopYellowSoul();
    child = EntityCreateInRange(0x11, 0x11, yellowSoul->updateFunc);
    YELLOW_SOUL_OWNER(child) = gEwramData->playerState.player;
    YELLOW_SOUL_PARAM(child) = yellowSoul->effectParam;
    YELLOW_SOUL_VALUE(child) = yellowSoul->effectValue;
    gEwramData->playerState.activeYellowSoulEntity = child;
    if (soul == 0xC && (gEwramData->playerStatus.statusEffects & 1))
    {
        gEwramData->playerState.currentStats[0] >>= 2;
        gEwramData->playerState.currentStats[1] >>= 2;
        gEwramData->playerState.currentStats[2] >>= 2;
    }
    ((void (*)(struct EwramData_EntityData *))child->updateFunc)(child);
}

/**
 * @brief 320D8 | Giant Worm soul: regenerate (Player_RestoreHP(5)) every 0x3D
 * frames while HP < max and the player stands still.
 */
void SkillGiantWormUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player;

    entity->roomActive = 1;
    if (gEwramData->gameState.stateFlags & 1)
        return;
    if (gEwramData->playerStatus.currentHP >= gEwramData->playerStatus.maxHP)
        return;
    player = gEwramData->playerState.player;
    if (player->velX.whole != 0 || (PLAYER_ACTION_FLAGS(player) & 0x360))
        return;
    if ((u8)++entity->timer > 0x3C)
    {
        entity->timer = 0;
        Player_RestoreHP(5);
    }
}

/**
 * @brief 32154 | Basilisk soul: DEF + DEF / 4, ATK - ATK / 2.
 */
void SkillBasiliskUse(struct EwramData_EntityData *entity)
{
    entity->roomActive = 1;
    gEwramData->playerState.currentDEF += gEwramData->playerState.currentDEF / 4;
    gEwramData->playerState.currentATK -= gEwramData->playerState.currentATK / 2;
}

/**
 * @brief Return the equipped yellow-soul record through the matching inline lookup.
 */
const struct YellowSoulInfo *YellowSoul_GetEquippedInfo(void)
{
    return GetYellowSoul();
}

/**
 * @brief Clear the existing yellow-soul effect bits, delete a non-NULL active entity and clear its pointer.
 */
void YellowSoul_Stop(void)
{
    StopYellowSoul();
}

/**
 * @brief 32204 | Souls setting effect bits: soulStateFlags |= unk_500.
 */
void Skill_Undine_Skula_ZombieOfficer_IronGolem_DeadWarrior_PeepingEye_Succubus_FleshGolem_Tsuchinoko_Erinys_Mimic_ArcDemon_Use(struct EwramData_EntityData *entity)
{
    entity->roomActive = 1;
    gEwramData->playerStatus.soulStateFlags |= YELLOW_SOUL_VALUE(entity);
}

/**
 * @brief Mark the entity active and double current attack when the stored selector is zero, otherwise double current defense.
 */
void YellowSoul_DoubleAttackOrDefense(struct EwramData_EntityData *entity)
{
    entity->roomActive = 1;
    if (YELLOW_SOUL_VALUE(entity) == 0)
        gEwramData->playerState.currentATK *= 2;
    else
        gEwramData->playerState.currentDEF *= 2;
}

/**
 * @brief 3226C | Souls raising a stat: currentStats[unk_4FE] += unk_500.
 */
void Skill_SkeletonKnight_Minotaur_Golem_Triton_WhiteDragon_Quezlcoatl_Gorgon_DeadCrusader_RedCrow_Lilith_Bael_Stolas_GhostDancer_Gremlin_Use(struct EwramData_EntityData *entity)
{
    s16 *stat = gEwramData->playerState.currentStats;

    stat += (u16)YELLOW_SOUL_PARAM(entity);
    *stat += YELLOW_SOUL_VALUE(entity);
    entity->roomActive = 1;
    Player_RecalculateAttackDefense();
}

/**
 * @brief 322A0 | Souls giving resistances: resistances |= unk_500 (and
 * Player_ClearStatusEffects(unk_500 >> 6) when statusEffects is set).
 */
void Skill_PoisonWorm_Ectoplasm_Gargoyle_Use(struct EwramData_EntityData *entity)
{
    s32 resist = YELLOW_SOUL_VALUE(entity);

    gEwramData->playerState.resistances |= resist;
    if (gEwramData->playerStatus.statusEffects != 0)
    {
        resist >>= 6;
        Player_ClearStatusEffects(resist);
    }
    entity->roomActive = 1;
}

/**
 * @brief 322E8 | Wooden Golem soul: Player_RestoreMP(1) every 32 frames.
 */
void SkillWoodenGolemUse(struct EwramData_EntityData *entity)
{
    s32 ticks;

    entity->roomActive = 1;
    ticks = entity->timer + 1;
    entity->timer = ticks;
    if ((ticks & 0x1F) == 0)
        Player_RestoreMP(1);
}

/**
 * @brief 32310 | Zombie soul: STR, CON, INT * 5.5 when statusEffects bit 0.
 */
void SkillZombieUse(struct EwramData_EntityData *entity)
{
    s16 *stats = gEwramData->playerState.currentStats;
    s32 i;

    entity->roomActive = 1;
    if (gEwramData->playerStatus.statusEffects & 1)
    {
        for (i = 2; i >= 0; stats++, i--)
            *stats += (*stats << 2) + (*stats >> 1);
        Player_RecalculateAttackDefense();
    }
}

/**
 * @brief 32360 | Lubicant soul: STR, CON (/ 8) and INT grow with the
 * missing HP fraction.
 */
void SkillLubicantUse(struct EwramData_EntityData *entity)
{
    s16 *stats = gEwramData->playerState.currentStats;
    s32 maxHp = gEwramData->playerStatus.maxHP;
    s32 missingHp = (maxHp - gEwramData->playerStatus.currentHP) << 8;

    maxHp <<= 8;
    entity->roomActive = 1;
    stats[0] += stats[0] * missingHp / maxHp;
    stats[1] += (stats[1] * missingHp / maxHp) >> 3;
    stats[2] += stats[2] * missingHp / maxHp;
    Player_RecalculateAttackDefense();
}

/**
 * @brief 323DC | Headhunter soul: STR, CON, INT + 1 + souls / 16 (max 32).
 */
void SkillHeadhunterUse(struct EwramData_EntityData *entity)
{
    s16 *stats = gEwramData->playerState.currentStats;
    s16 bonus = gEwramData->playerStatus.totalNbrSoulsCollected >> 4;

    if (bonus > 32)
        bonus = 32;
    entity->roomActive = 1;
    stats[0]++;
    stats[0] += bonus;
    stats[1]++;
    stats[1] += bonus;
    stats[2]++;
    stats[2] += bonus;
    Player_RecalculateAttackDefense();
}

/**
 * @brief Return the indexed yellow-soul record through the matching inline lookup, retaining the byte argument.
 */
const struct YellowSoulInfo *YellowSoul_GetInfo(u8 soul)
{
    return GetYellowSoulInfo(soul);
}
