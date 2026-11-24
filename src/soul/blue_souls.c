/*
 * Blue (guardian) soul skills: soul slots and end conditions, input, Flying Armor up to Alastor, afterimages, Great Armor, Big Golem and the shared blue soul helpers.
 * Built without -f2003-patch (SkillAlastorUse needs it off; the rest compiles the same either way).
 */
#include "blue_souls.h"
#include "core.h"
#include "structs/ewram.h"
#include "graphics.h"
#include "structs/entity_render_flags.h"
#include "music.h"
#include "gameplay.h"
#include "macros.h"
#include "sound.h"
#include "soul_skill_entity.h"
#include "syscalls.h"
#include "damage_numbers.h"
#include "effects.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "palette.h"
#include "red_souls_b.h"
#include "data/data_0E0B68.h"
#include "data/data_4F0B18.h"
#include "data/data_118B48.h"
#include "player_core.h"

extern void Hit_GetContactPoint(struct EwramData_EntityData *a, struct EwramData_EntityData *b, s16 *outX, s16 *outY);

void BlueSoulProjectile_IgnoreContact(void);
void SkillWitchGiantGhostProjectileUpdate(struct EwramData_EntityData *entity);
void BlueSoul_DrawAlternatingLayer(struct EwramData_EntityData *);
void PersephoneSoul_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker);

/* Blue soul info, indexed by equippedBlueSoul (as in this file). */
struct BlueSoulInfo {
    EntityFunc updateFunc;
    u8 mpPerSecond;
    u8 activationFlags;
    u16 effectParam;
    u32 effectValue; // e.g. the transformation form
};

extern const struct BlueSoulInfo sBlueSoulInfo[];

static inline s8 FindFreeSoulSlot(void)
{
    s8 i;
    for (i = 0; i <= 7; i++)
        if (gEwramData->playerState.activeBlueSoulEntities[i] == NULL) return i;
    return -1;
}
/**
 * @brief Create a blue soul entity with the given update in slots 2..16 and
 * register it in the first free active blue soul slot; NULL when no slot or
 * entity is free.
 */
struct EwramData_EntityData *BlueSoul_CreateEntity(struct EwramData_EntityData *owner, void *update)
{
    s8 slot = FindFreeSoulSlot();
    struct EwramData_EntityData *entity;
    if (slot < 0)
    {
fail:
        return NULL;
    }
    entity = EntityCreateInRange(2, 16, update);
    if (entity == NULL) goto fail;
    entity->ext.blueSoul.owner = owner;
    entity->ext.blueSoul.slot = slot;
    gEwramData->playerState.activeBlueSoulEntities[slot] = entity;
    entity->roomActive = 1;
    return entity;
}

static inline void ReleaseAffineSlot(struct EwramData_EntityData *entity)
{
    if (ENTITY_RENDER_FLAGS16(entity) & 0x101F) ObjAffine_Free(entity->affineIndex);
}
/**
 * @brief Release root affine state, clear root slot, delete children one through seven, unload graphics and clear flags; leave root entity deletion to the caller.
 */
void BlueSoul_ReleaseActiveResources(struct EwramData_EntityData *entity)
{
    s32 i;
    ReleaseAffineSlot(entity);
    gEwramData->playerState.activeBlueSoulEntities[0] = NULL;
    for (i = 1; i <= 7; i++)
    {
        struct EwramData_EntityData *child = gEwramData->playerState.activeBlueSoulEntities[i];
        if (child != NULL)
        {
            ReleaseAffineSlot(child);
            EntityDelete(child);
            gEwramData->playerState.activeBlueSoulEntities[i] = NULL;
        }
    }
    if (gEwramData->playerState.blueSoulGfx != -1)
    {
        ObjGfx_Unload(gEwramData->playerState.blueSoulGfx);
        gEwramData->playerState.blueSoulGfx = -1;
    }
    gEwramData->playerStatus.soulStateFlags &= ~0x3F00;
    gEwramData->playerState.player->ext.player.actionFlags &= ~0x200;
}

/* BlueSoul_ShouldEnd; as
 * BlueSoul_CheckEndCondition (this file) plus BlueSoul_ReleaseActiveResources on ending. */

/**
 * @brief 2DE5C | Whether an active blue soul ends this frame: every unk_504
 * frames (counter unk_506) it costs 1 MP and ends at 0 MP; a hold-type soul
 * (info activationFlags bit 1 clear) ends when the guardian button is released, a
 * toggle-type one when it is pressed again after the first state; also ends
 * when stateFlags bit 0 is set without bit 8. Sets roomActive;
 * on ending calls BlueSoul_ReleaseActiveResources (same tests as BlueSoul_CheckEndCondition).
 *
 * @return 1 when the soul should end, else 0
 */
s32 BlueSoul_ShouldEnd(struct EwramData_EntityData *entity)
{
    struct EwramData *ew = gEwramData;
    const struct BlueSoulInfo *info = &sBlueSoulInfo[ew->playerStatus.equippedBlueSoul];
    s32 rem;

    entity->roomActive = 1;
    entity->ext.blueSoul.mpCounter++;
    rem = (u16)entity->ext.blueSoul.mpCounter % entity->ext.blueSoul.mpInterval;
    if (rem == 0 && --ew->playerStatus.currentMP <= 0)
    {
        ew->playerStatus.currentMP = rem;
        BlueSoul_ReleaseActiveResources(entity);
        return 1;
    }
    if (!(info->activationFlags & 2))
    {
        if (!(gEwramData->inputData.playerHeldInput & gEwramData->playerStatus.guardianButton))
        {
            BlueSoul_ReleaseActiveResources(entity);
            return 1;
        }
    }
    else if (entity->state != 0 && (gEwramData->inputData.playerNewInput & gEwramData->playerStatus.guardianButton))
        goto end;
    {
        u32 flags = gEwramData->gameState.stateFlags;
        if (flags & 0x100)
            goto keep;
        flags &= 1;
        if (!flags)
            return 0;
    }
end:
    BlueSoul_ReleaseActiveResources(entity);
    return 1;
keep:
    return 0;
}

/* BlueSoul_CheckEndCondition. */
/* Blue soul info, indexed by equippedBlueSoul (as in this file). */

/**
 * @brief Advance the MP interval counter and check depletion, guardian input and interaction flags; return the end decision without releasing active resources.
 */
s32 BlueSoul_CheckEndCondition(struct EwramData_EntityData *entity)
{
    struct EwramData *ew = gEwramData;
    const struct BlueSoulInfo *info = &sBlueSoulInfo[ew->playerStatus.equippedBlueSoul];
    s32 rem;

    entity->roomActive = 1;
    entity->ext.blueSoul.mpCounter++;
    rem = (u16)entity->ext.blueSoul.mpCounter % entity->ext.blueSoul.mpInterval;
    if (rem == 0 && --ew->playerStatus.currentMP <= 0)
    {
        ew->playerStatus.currentMP = rem;
        return 1;
    }
    if (!(info->activationFlags & 2))
    {
        if (!(gEwramData->inputData.playerHeldInput & gEwramData->playerStatus.guardianButton))
            return 1;
    }
    else if (entity->state != 0 && (gEwramData->inputData.playerNewInput & gEwramData->playerStatus.guardianButton))
        return 1;
    {
        u32 flags = gEwramData->gameState.stateFlags;
        if (!(flags & 0x100))
        {
            flags &= 1;
            if (flags)
                return 1;
        }
    }
    return 0;
}

/* BlueSoul_StopAll. */
/**
 * @brief 2DFF0 | For Soma (currentCharacter 0): delete all active blue
 * soul entities (releasing their affine slots as BlueSoul_DeleteEntity), then
 * Sound_StopTracked(0x9C), clear soulStateFlags bits 8-13 and the player's unk_4F4
 * bit 9, and free the blue soul graphics blueSoulGfx.
 */
void BlueSoul_StopAll(void)
{
    s8 i;

    if (gEwramData->playerStatus.currentCharacter != 0)
        return;
    for (i = 0; i < 8; i++)
    {
        struct EwramData_EntityData *soul = gEwramData->playerState.activeBlueSoulEntities[i];
        if (soul != NULL)
        {
            if (soul->affineIndex != 0 || soul->affine)
                ObjAffine_Free(soul->affineIndex);
            EntityDelete(soul);
            gEwramData->playerState.activeBlueSoulEntities[i] = NULL;
        }
    }
    Sound_StopTracked(0x9C);
    gEwramData->playerStatus.soulStateFlags &= ~0x3F00;
    gEwramData->playerState.player->ext.player.actionFlags &= ~0x200;
    if (gEwramData->playerState.blueSoulGfx != -1)
    {
        ObjGfx_Unload(gEwramData->playerState.blueSoulGfx);
        gEwramData->playerState.blueSoulGfx = -1;
    }
}

/* Activate an equipped blue soul when input, state and MP checks permit it. */
/* Blue soul info, indexed by equippedBlueSoul (as in this file). */

/* Keep the post-allocation reload separate from the earlier table index. */
static inline u8 GetBlueSoulId(void)
{
    return gEwramData->playerStatus.equippedBlueSoul;
}
/**
 * @brief Activate the equipped blue soul when its button is held (or newly
 * pressed for trigger-type souls), no blue-soul entity is active, the
 * scene permits it and MP exceeds maxMP / 128. Create and register the
 * soul entity and set player flag 0x200.
 */
void Soma_HandleBlueSoulInput(struct EwramData_EntityData *entity)
{
    struct EwramData *ew = gEwramData;
    struct EwramData_EntityData *active;
    const struct BlueSoulInfo *info;
    struct EwramData_EntityData *soul;
    u16 button;

    if (ew->playerState.actionLockFlags & 0x200)
        return;
    if ((ew->gameState.gameFlagsTail[0] & 0x200000) && ew->playerStatus.equippedBlueSoul > 3)
        return;
    active = ew->playerState.activeBlueSoulEntities[0];
    if (active != NULL)
        return;
    {
        /* Preserve address calculation, held-input read, then button read.
         * The register constraint emits no inline instructions. */
        u16 *buttonPtr = &ew->playerStatus.guardianButton;
        u16 held = *(vu16 *)&ew->inputData.playerHeldInput; /* volatile read (replaces a pinned register) */
        if (!(held & (button = *buttonPtr)))
            return;
    }
    info = &sBlueSoulInfo[ew->playerStatus.equippedBlueSoul];
    if ((info->activationFlags & 2) && !(button &= ew->inputData.playerNewInput))
        return;
    if (info->updateFunc == NULL)
        return;
    if (ew->playerStatus.currentMP < (ew->playerStatus.maxMP >> 7) + 1)
        return;
    soul = EntityCreateInRange(2, 0x10, info->updateFunc);
    if (soul == NULL)
        return;
    gEwramData->playerState.activeBlueSoulEntities[0] = soul;
    soul->ext.blueSoul.owner = entity;
    soul->ext.blueSoul.id = GetBlueSoulId();
    soul->ext.blueSoul.param = info->effectParam;
    soul->ext.blueSoul.slot = (u32)active;
    soul->ext.blueSoul.value = info->effectValue;
    soul->ext.blueSoul.mpInterval = 60 / info->mpPerSecond;
    entity->ext.player.actionFlags |= 0x200;
}

/* Update the attack box when animation advances to another sprite frame. */
static inline struct SpriteSheet *Graphics(u8 *index) { return gEwramData->spriteSlots[*index].sheet; }
/**
 * @brief Advance the animation; when the sprite frame changes, use its first
 * box as attack box and enable hits, or disable hits for a frame
 * without boxes. Used by four blue-soul update functions.
 */
void Anim_UpdateWithFrameAttackBox(struct EwramData_EntityData *entity)
{
    struct SpriteSheet *graphics = Graphics(&entity->spriteSlot);
    u8 frame = entity->animFrame;
    Anim_Update(entity);
    if (frame != entity->animFrame)
    {
        struct SpriteBox *box = graphics->frames[entity->animFrame].boxes;
        if (box != NULL)
        {
            entity->hit.flags &= ~2;
            HitBox_SetAttack(entity, box);
        }
        else entity->hit.flags |= 2;
    }
}

#define GRAPHICS_CONTEXT ((struct PlayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, playerState)))

static inline void FollowParent(struct EwramData_EntityData *entity, struct EwramData_EntityData *parent, u16 x_, u16 y_)
{
    s16 x = x_, y = y_;
    entity->flipX = parent->flipX;
    if (entity->flipX) x = -x;
    entity->posX.part16.integer = (s32)x + (s16)parent->posX.part16.integer;
    entity->posY.part16.integer = (s32)y + parent->posY.part16.integer;
}

/* Flying Armor guardian: follows the owner and emits drifting particles. */
void SkillFlyingArmorUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;
    struct EwramData_EntityData *particle;
    u32 timer;
    s32 x, y, r;
    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
    {
        u8 *graphics = sSkillFlyingArmorGfx;
        u8 *palette = sSkillFlyingArmorPalette;
        u8 *sprite = sSkillFlyingArmorSpriteSheet;
        GRAPHICS_CONTEXT->blueSoulGfx = ObjGfx_LoadResource(graphics, 0x10000);
        ObjPalette_LoadRange(palette, 0, 1, 3);
        Sprite_Attach(entity, sprite, GRAPHICS_CONTEXT->blueSoulGfx, 3);
    }
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->spriteLayer = 13;
        entity->drawFunc = Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        entity->state++;
    case 1:
        FollowParent(entity, parent, 0, 0);
        Anim_Update(entity);
        break;
    }
    timer = entity->timer + 1;
    entity->timer = timer;
    if ((timer & 3) == 0)
    {
        x = parent->posX.whole + RandomSigned(0x80000);
        y = RandomSigned(0x10000) - 0x1A0000;
        particle = Effect_CreateFromTable(x, parent->posY.whole + y, 0, 0, 8, 9);
        if (particle != NULL)
        {
            particle->velX.whole = RandomSigned(0x8000);
            particle->velY.whole = RandomSigned(0x4000) - 0x10000;
            r = RandomSigned(0x1000);
            particle->accelX.whole = -particle->velX.whole / 4 + r;
            r = RandomSigned(0x1000);
            particle->accelY.whole = -particle->velY.whole / 64 + r;
        }
    }
    gEwramData->playerStatus.soulStateFlags |= 0x100;
}

void SkillBlackPantherUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;

    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        entity->scrollWithBg = 1;
        Hit_Enable(entity, 6, 0, 0);
        HitBox_SetAttackRaw(entity, 0x3030D8F4);
        entity->hit.flags |= 4;
        Hit_SetTimerOverride(2, 0x3C);
        {
            u16 a = 8;
            u16 b = 1;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = b;
        }
        entity->state++;
    case 1:
        if (parent->animScriptIndex == 3)
        {
            if (entity->subState == 0)
            {
                entity->subState = 1;
                PlaySong(0x9A);
            }
            entity->hit.flags &= ~2;
        }
        else
        {
            entity->subState = 0;
            entity->hit.flags |= 2;
        }
        FollowParent(entity, parent, 0, 0);
    }
    gEwramData->playerStatus.soulStateFlags |= entity->ext.blueSoul.value;
}

static inline void FollowParent2(struct EwramData_EntityData *entity, struct EwramData_EntityData *parent, u16 x_, s32 y_)
{
    s16 x = x_;
    s32 y = y_;
    entity->flipX = parent->flipX;
    if (entity->flipX) x = -x;
    entity->posX.part16.integer = (s32)x + (s16)parent->posX.part16.integer;
    entity->posY.part16.integer = (s32)y + parent->posY.part16.integer;
}

void SkillCagnazzoUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.blueSoul.owner;

    u8 state;

    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    switch (state = entity->state)
    {
    case 0:
    {
        u8 *gfx = sSkillCagnazzoGfx;
        u8 *pal = sSkillCagnazzoPalette;
        void *oam = sSkillCagnazzoSpriteSheet;
        GRAPHICS_CONTEXT->blueSoulGfx = ObjGfx_LoadResource(gfx, 0x10000);
        ObjPalette_LoadRange(pal, 0, 1, 3);
        Sprite_Attach(entity, oam, GRAPHICS_CONTEXT->blueSoulGfx, 3);
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->spriteLayer = 0xF;
        entity->drawFunc = Sprite_DrawEntity;
        {
            u16 n = 0xC;
            GRAPHICS_CONTEXT->skillBaseAttack = n;
        }
        GRAPHICS_CONTEXT->skillAttackAttributes = state;
        entity->scrollWithBg = 1;
        entity->state++;
        Hit_Enable(entity, 6, 0, 0);
        HitBox_SetAttackRaw(entity, 0x1414050A);
        entity->hit.flags |= 4;
        Hit_SetTimerOverride(2, 0x14);
        PlaySong(0x1AA);
    }
    case 1:
        FollowParent2(entity, owner, 0x10, -0x20);
    }
    Anim_Update(entity);
}

/* SkillBonePillarProjectileUpdate (blue soul projectile). */

/* Same as BlueSoul_DeleteEntity: end a blue soul entity. */
static inline void BlueSoulProjectile_Delete(struct EwramData_EntityData *entity)
{
    if (entity->affineIndex != 0 || entity->affine)
        ObjAffine_Free(entity->affineIndex);
    gEwramData->playerState.activeBlueSoulEntities[entity->ext.blueSoul.slot] = NULL;
    EntityDelete(entity);
}

/**
 * @brief Child of SkillBonePillarUse: blue-soul projectile with the soul
 * graphics, box 0x1414F6F6 (slot 6) and a 26-update timer; flies with
 * the layer following the timer and ends when the timer runs out.
 */
void SkillBonePillarProjectileUpdate(struct EwramData_EntityData *entity)
{
    entity->roomActive = 1;
    switch (entity->state)
    {
        case 0:
            Sprite_Attach(entity, sSkillBonePillarProjectileSpriteSheet, gEwramData->playerState.blueSoulGfx, 3);
            Anim_StartIndexed(entity, 0, 0, 0);
            entity->spriteLayer = 0xD;
            entity->drawFunc = Sprite_DrawEntity;
            Hit_Enable(entity, 6, NULL, 0);
            HitBox_SetAttackRaw(entity, 0x1414F6F6);
            entity->hit.flags |= 4;
            entity->timer = 0x1A;
            entity->scrollWithBg = 1;
            entity->state++;
        case 1:
            entity->spriteLayer = (entity->timer >> 2) + 7;
            EntityAccelerateAndMove(entity, 0x80000);
            if (--entity->timer == 0)
            {
                BlueSoulProjectile_Delete(entity);
                return;
            }
            break;
    }
    Anim_Update(entity);
}

static inline void Follow(struct EwramData_EntityData *entity, struct EwramData_EntityData *anchor, u16 offsetX, s32 offsetY)
{
    s16 dx = offsetX;

    entity->flipX = anchor->flipX;
    if (entity->flipX)
        dx = -dx;
    entity->posX.part16.integer = (s32)dx + (s16)anchor->posX.part16.integer;
    entity->posY.part16.integer = anchor->posY.part16.integer + offsetY;
}

static inline void LoadGfx(struct EwramData_EntityData *entity, u8 paletteSlot, void *gfxSrc, void *palette, void *sheet)
{
    gEwramData->playerState.blueSoulGfx = ObjGfx_LoadResource(gfxSrc, 0x10000);
    ObjPalette_LoadRange(palette, paletteSlot, 1, 3);
    Sprite_Attach(entity, sheet, gEwramData->playerState.blueSoulGfx, 3);
}

/**
 * @brief 2E730 | Bone Pillar blue soul (ends via BlueSoul_ShouldEnd or when the
 * caster leaves the 0x100000 state of unk_4F4 & 0x1001A2): graphics
 * sSkillBonePillarGfx / sSkillBonePillarPalette / sSkillBonePillarProjectileSpriteSheet (slot 3), hidden, Hit_SetTimerOverride(2,
 * 15), skillBaseAttack/skillAttackAttributes = 0x14 / 2, the player (playerState.player) to state 10 with
 * animation 0x35. Sits 16 px in front of and 28 px above the caster and
 * every 5 frames throws a SkillBonePillarProjectileUpdate bone (song 0x91 on throw 0 and 50 %
 * on throw 2 of 5) with speed 0x18000 + caster velX, deceleration 0x400 and
 * vertical speed sSkillProjectileLaunchVelocities[unk_50A] + 0x14000 (unk_50A cycles 0..2).
 */
void SkillBonePillarUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;

    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    if ((parent->ext.player.actionFlags & 0x1001A2) != 0x100000)
    {
        BlueSoul_ReleaseActiveResources(entity);
        EntityDelete(entity);
        return;
    }
    gEwramData->playerState.actionLockFlags |= 0x820;
    switch (entity->state)
    {
    case 0:
    {
        u8 *graphics = (u8 *)sSkillBonePillarGfx;
        u8 *palette = (u8 *)sSkillBonePillarPalette;
        u8 *sprite = (u8 *)sSkillBonePillarProjectileSpriteSheet;
        LoadGfx(entity, 0, graphics, palette, sprite);
    }
        entity->hidden = 1;
        Hit_SetTimerOverride(2, 0xF);
        {
            u16 a = 0x14;
            u16 b = 2;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = b;
        }
        gEwramData->playerState.player->state = 0xA;
        Anim_StartIndexed(gEwramData->playerState.player, 0x35, 3, 1);
        parent->ext.player.actionFlags &= 0xEFFFFBFF;
        entity->state++;
    case 1:
        Follow(entity, parent, 0x10, -0x1C);
        if (entity->timer == 0)
        {
            struct EwramData_EntityData *child = BlueSoul_CreateEntity(entity, SkillBonePillarProjectileUpdate);
            if (child != NULL)
            {
                if (entity->subState == 0)
                    PlaySong(0x91);
                else if (entity->subState == 2 && (RandomNumberGenerator() & 1))
                    PlaySong(0x91);
                if (++entity->subState > 4)
                    entity->subState = 0;
                ENTITY_OWNER(child) = entity;
                child->posX.whole = entity->posX.whole;
                child->posY.whole = entity->posY.whole;
                ENTITY_INDEX(child) = ENTITY_INDEX(entity);
                if (parent->flipX)
                {
                    child->velX.whole = -0x18000;
                    child->accelX.whole = -0x400;
                }
                else
                {
                    child->velX.whole = 0x18000;
                    child->accelX.whole = 0x400;
                }
                child->velX.whole += parent->velX.whole;
                child->flipX = parent->flipX;
                child->velY.whole = sSkillProjectileLaunchVelocities[entity->ext.skillBonePillar.shot] + 0x14000;
                child->accelY.whole = -0x1800;
            }
            if (++entity->ext.skillBonePillar.shot > 2)
                entity->ext.skillBonePillar.shot = 0;
            entity->timer = 5;
        }
        entity->timer--;
    }
}

/* Same as BlueSoul_DeleteEntity: end a blue soul entity. */
static inline void EndBlueSoul(struct EwramData_EntityData *entity)
{
    if (entity->affineIndex != 0 || entity->affine)
        ObjAffine_Free(entity->affineIndex);
    gEwramData->playerState.activeBlueSoulEntities[entity->ext.blueSoul.slot] = NULL;
    EntityDelete(entity);
}

/**
 * @brief 2E960 | Alura Une blue soul: ends (BlueSoul_ReleaseActiveResources) when HP is full
 * or via BlueSoul_ShouldEnd; graphics sSkillAluraUneGfx / sSkillAluraUnePalette / sSkillAluraUneSpriteSheet in an
 * affine slot (ObjAffine_Alloc, freed like BlueSoul_DeleteEntity when none is free),
 * held 16 px behind and 26 px above the caster while its affine scale grows
 * (x by 0x1000, y by 0x2000 per frame up to 0x10000). Every 17 frames: song
 * 0x140 and Player_RestoreHP(10) (heal); at full HP 1 in 8 plays song 0x1AC.
 */
void SkillAluraUneUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;
    u16 slot;

    entity->roomActive = 1;
    if (gEwramData->playerStatus.currentHP >= gEwramData->playerStatus.maxHP)
    {
        BlueSoul_ReleaseActiveResources(entity);
        EntityDelete(entity);
        return;
    }
    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
    {
        u8 *graphics = (u8 *)sSkillAluraUneGfx;
        u8 *palette = (u8 *)sSkillAluraUnePalette;
        u8 *sprite = (u8 *)sSkillAluraUneSpriteSheet;
        LoadGfx(entity, 0, graphics, palette, sprite);
    }
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->spriteLayer = 0xD;
        entity->drawFunc = Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            EndBlueSoul(entity);
            return;
        }
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->state++;
    case 1:
    {
        s32 dx = -0x10;
        entity->flipX = parent->flipX;
        if (entity->flipX)
            dx = 0x10;
        entity->posX.part16.integer = parent->posX.part16.integer + dx;
        entity->posY.part16.integer = parent->posY.part16.integer - 0x1A;
    }
        if (entity->ext.skillGrowScale.scaleX <= 0xFFFF)
            entity->ext.skillGrowScale.scaleX += 0x1000;
        if (SKILL_ALURA_UNE_SCALE_Y(entity) <= 0xFFFF)
            SKILL_ALURA_UNE_SCALE_Y(entity) += 0x2000;
    }
    ObjAffine_Set(0, entity->ext.skillGrowScale.scaleX, SKILL_ALURA_UNE_SCALE_Y(entity), entity->flipX, 0, entity->affineIndex);
    if (entity->timer++ > 15)
    {
        entity->timer = 0;
        PlaySong(0x140);
        Player_RestoreHP(10);
        if (gEwramData->playerStatus.currentHP >= gEwramData->playerStatus.maxHP && (RandomNumberGenerator() & 0x3F) <= 7)
            PlaySong(0x1AC);
    }
}

#define PLAYER (gEwramData->playerState.player)

/**
 * @brief 2EB84 | Giant Bat blue soul (bat transformation; ends via
 * BlueSoul_ShouldEnd, or BlueSoul_ReleaseActiveResources while the player has unk_4F4 & 0x10001A0):
 * on start clamps the player's speed to +-0x40000, stops its acceleration,
 * song 0x8F; for 16 frames spawns Effect_CreateFromTable bats (type 6) at random
 * angles 16 px above the player. Keeps the player in state 8, soulStateFlags
 * bit 9 and actionLockFlags bits 0-1.
 */
void SkillGiantBatUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player;
    struct EwramData_EntityData *bat;
    s32 a, s, c, vx, vy;

    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    if (PLAYER->ext.player.actionFlags & 0x010001A0)
    {
        BlueSoul_ReleaseActiveResources(entity);
        EntityDelete(entity);
        return;
    }
    if (entity->state == 0)
    {
        if (PLAYER->velX.whole > 0x40000)
            PLAYER->velX.whole = 0x40000;
        else if (PLAYER->velX.whole < -0x40000)
            PLAYER->velX.whole = -0x40000;
        if (PLAYER->velY.whole > 0x40000)
            PLAYER->velY.whole = 0x40000;
        else if (PLAYER->velY.whole < -0x40000)
            PLAYER->velY.whole = -0x40000;
        PLAYER->accelX.whole = 0;
        PLAYER->accelY.whole = 0;
        PLAYER->ext.player.actionFlags &= ~0x20;
        PlaySong(0x8F);
        entity->state = 1;
        PLAYER->subState = 0;
        entity->timer = 0x10;
    }
    if (entity->timer != 0)
    {
        entity->timer--;
        player = PLAYER;
        a = (RandomNumberGenerator() & 0xFF) << 8;
        s = Sin(a + 0x4000);
        vx = s * 2;
        c = Sin(a);
        vy = c * 2;
        {
            s32 x = player->posX.whole + s * 4;
            s32 y = player->posY.whole - 0x100000;
            y += c * 4;
            bat = Effect_CreateFromTable(x, y, vx / 16, vy / 16, 4, 6);
        }
        if (bat != NULL)
        {
            bat->velX.whole = vx;
            bat->velY.whole = vy;
        }
    }
    PLAYER->state = 8;
    gEwramData->playerStatus.soulStateFlags |= 0x200;
    gEwramData->playerState.actionLockFlags |= 3;
}

/* Animated blue-soul effect; release its affine and active-entity slots
 * once the animation finishes. */

/**
 * @brief Single spark spawned by SkillDevilManticoreSparksUpdate: random
 * animation, moves and deletes itself (freeing its affine slot).
 */
void SkillDevilManticoreSparkUpdate(struct EwramData_EntityData *entity)
{
    if (entity->state == 0)
    {
        u8 *graphics = sSkillDevilManticoreSparkGfx;
        u8 *palette = sSkillDevilManticoreSparkPalette;
        u8 *sprite = sSkillDevilManticoreSparkSpriteSheet;
        GRAPHICS_CONTEXT->blueSoulGfx = ObjGfx_LoadResource(graphics, 0x10000);
        ObjPalette_LoadRange(palette, 0, 1, 3);
        Sprite_Attach(entity, sprite, GRAPHICS_CONTEXT->blueSoulGfx, 3);
        Anim_StartIndexed(entity, RandomNumberGenerator() & 3, 0, 0);
        entity->spriteLayer = 13;
        entity->drawFunc = Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        entity->state++;
    }
    entity->roomActive = 1;
    if (Anim_Update(entity) == 3)
    {
        ReleaseAffineSlot(entity);
        gEwramData->playerState.activeBlueSoulEntities[entity->ext.blueSoul.slot] = NULL;
        EntityDelete(entity);
    }
}

/* Devil/Manticore: emit sparks every four frames, sound every eight frames. */
void SkillDevilManticoreSparksUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    entity->roomActive = 1;
    if (!(entity->timer & 7)) PlaySong(0x90);
    if (!(entity->timer & 3))
    {
        struct EwramData_EntityData *spark = BlueSoul_CreateEntity(entity, SkillDevilManticoreSparkUpdate);
        if (spark != NULL)
        {
            spark->posX.whole = owner->posX.whole;
            spark->posY.whole = owner->posY.whole;
            spark->flipX = owner->flipX;
        }
    }
    entity->timer++;
}

/* Animated soul effect following its owner and sounding every 8 frames. */
/**
 * @brief Update of entry 2 of the blue-soul form table sBlueSoulForms used by
 * Skill_Devil_Manticore_Curly_Use (entries 0 and 1 use
 * SkillDevilManticoreSparksUpdate).
 */
void SkillBlueSoulForm2Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
    {
        u8 *graphics = sUnk_081E116C;
        u8 *palette = sUnk_0820B64C;
        u8 *sprite = sUnk_0823760C;
        GRAPHICS_CONTEXT->blueSoulGfx = ObjGfx_LoadResource(graphics, 0x10000);
        ObjPalette_LoadRange(palette, 0, 1, 3);
        Sprite_Attach(entity, sprite, GRAPHICS_CONTEXT->blueSoulGfx, 3);
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->spriteLayer = 14;
        entity->drawFunc = Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        entity->state++;
    }
    /* fall through */
    case 1:
    {
        s32 timer;
        entity->posX.whole = owner->posX.whole;
        entity->posY.whole = owner->posY.whole;
        entity->flipX = owner->flipX;
        timer = entity->timer + 1;
        entity->timer = timer;
        if ((timer & 7) == 0) PlaySong(0x93);
        break;
    }
    }
    Anim_Update(entity);
}

/* PlayerAfterimage_Update. */
/**
 * @brief 2EF28 | Update via Sprite_DrawEntity; every subState + 1 frames (counter
 * timer) run the player's draw function with the player moved to the
 * entity's position (afterimage), then restore the player's position.
 */
void PlayerAfterimage_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player = gEwramData->playerState.player;
    u16 x = player->posX.part16.integer;
    u16 y = player->posY.part16.integer;

    Sprite_DrawEntity(entity);
    if (++entity->timer > entity->subState)
        entity->timer = 0;
    if (entity->timer == 0)
    {
        struct PlayerState *p = &gEwramData->playerState;
        p->player->posX.part16.integer = entity->posX.part16.integer;
        p->player->posY.part16.integer = entity->posY.part16.integer;
        p->player->drawFunc(p->player);
        gEwramData->playerState.player->posX.part16.integer = x;
        gEwramData->playerState.player->posY.part16.integer = y;
    }
}

/**
 * @brief 2EFB0 | Devil / Manticore / Curly blue soul (transformation; ends
 * via BlueSoul_ShouldEnd, or BlueSoul_ReleaseActiveResources while the player has unk_4F4 & 0x81A2):
 * follows the caster, keeps the player in state 9 and soulStateFlags bit 12.
 * State 0 stops the player and selects form sBlueSoulInfo[equipped blue
 * soul].unk_8 (player unk_50B); when the player reaches step 3 the form's
 * entity (sBlueSoulForms) is spawned, box 0x4430C0E8 in slot 6, skillBaseAttack/skillAttackAttributes
 * from the form, its song (0x19F if Player_IsBossTargetLowHp) and a white flash.
 */
void Skill_Devil_Manticore_Curly_Use(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;
    const struct BlueSoulInfo *info = &sBlueSoulInfo[gEwramData->playerStatus.equippedBlueSoul];
    const struct BlueSoulInfo *info2 = info;
    struct PlayerState *ctx;
    struct BlueSoulForm *form;

    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    ctx = &gEwramData->playerState;
    if (ctx->player->ext.player.actionFlags & 0x81A2)
    {
        BlueSoul_ReleaseActiveResources(entity);
        EntityDelete(entity);
        return;
    }
    Follow(entity, parent, 0, 0);
    switch (entity->state)
    {
    case 0:
        ctx->player->accelX.whole = 0;
        ctx->player->accelY.whole = 0;
        ctx->player->ext.player.actionFlags &= ~0x20;
        entity->state = 1;
        ctx->player->subState = 0;
        gEwramData->playerState.player->ext.player.transformForm = info->effectValue;
        break;
    case 1:
        if (ctx->player->subState == 3)
        {
            form = &sBlueSoulForms[info2->effectValue];
            BlueSoul_CreateEntity(parent, form->func);
            Hit_Enable(entity, 6, 0, 0);
            HitBox_SetAttackRaw(entity, 0x4430C0E8);
            entity->hit.flags |= 4;
            entity->hit.flags &= ~2;
            Hit_SetTimerOverride(2, 0xF);
            {
                u16 a = form->baseAttack;
                u16 b = form->attackAttributes;
                gEwramData->playerState.skillBaseAttack = a;
                gEwramData->playerState.skillAttackAttributes = b;
            }
            if (Player_IsBossTargetLowHp())
                PlaySong(0x19F);
            else
                PlaySong(form->song);
            entity->state++;
            BgPalette_StartColorFade(0x1F, 0xF, 0xF, 0x8000, 0x10000, 0x10);
        }
        break;
    case 2:
        if (!BgPalette_GetPendingFades())
            BgPalette_StopColorFade();
        gEwramData->playerState.actionLockFlags |= 0xE;
        entity->timer++;
        break;
    }
    gEwramData->playerState.player->state = 9;
    gEwramData->playerStatus.soulStateFlags |= 0x1000;
}

static inline void SpawnBubble(struct EwramData_EntityData *entity, s32 type)
{
    s32 angle = (RandomNumberGenerator() & 0xFF) << 8;
    s32 s = Sin(angle + 0x4000);
    s32 vx = s * 2;
    s32 c = Sin(angle);
    s32 vy = c * 2;
    s32 x = entity->posX.whole + s * 16;
    s32 y = entity->posY.whole - 0x280000;
    struct EwramData_EntityData *child;
    y += c * 16;
    child = Effect_CreateFromTable(x, y, vx / 16, vy / 16, 4, type);
    if (child != NULL)
    {
        child->velX.whole = vx;
        child->velY.whole = vy;
    }
}

/**
 * @brief 2F1A4 | Sky Fish blue soul (ends via BlueSoul_CheckEndCondition or caster state
 * unk_4F4 & 0x1A2; not started while & 0x82): graphics sSkillSkyFishGfx /
 * sSkillSkyFishPalette / sSkillSkyFishSpriteSheet, song 0x99. For 16 frames two Effect_CreateFromTable
 * bubbles per frame at random angles 40 px above, then the player takes
 * animation 0x34 (actionLockFlags bit 11, song 0x1A8) and a counter spawnParam1 rises
 * to 300 (song 0x95 every 60), added to STR (/ 2) and LCK every frame
 * (Player_RecalculateAttackDefense). The end state sparkles (Effect_SpawnShared every 8 of 240
 * frames) and calls BlueSoul_ReleaseActiveResources.
 */
void SkillSkyFishUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;
    s16 *str = &gEwramData->playerState.currentStats[0];

    entity->roomActive = 1;
    if ((u8)(entity->state - 1) <= 2)
    {
        if (BlueSoul_CheckEndCondition(entity))
            entity->state = 4;
        if (parent->ext.player.actionFlags & 0x1A2)
            entity->state = 4;
    }
    Follow(entity, parent, 0, 0);
    switch (entity->state)
    {
    case 0:
        if (parent->ext.player.actionFlags & 0x82)
        {
            BlueSoul_ReleaseActiveResources(entity);
            EntityDelete(entity);
            return;
        }
    {
        u8 *graphics = (u8 *)sSkillSkyFishGfx;
        u8 *palette = (u8 *)sSkillSkyFishPalette;
        u8 *sprite = (u8 *)sSkillSkyFishSpriteSheet;
        LoadGfx(entity, 0, graphics, palette, sprite);
    }
        Anim_StartIndexed(entity, 1, 0, 1);
        entity->spriteLayer = 0xF;
        entity->drawFunc = Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        entity->timer = 0x10;
        entity->state = 1;
        PlaySong(0x99);
        break;
    case 1:
        gEwramData->playerStatus.inputOverrides |= 1;
        SpawnBubble(entity, 8);
        SpawnBubble(entity, 8);
        if (entity->timer == 0)
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->state = 2;
            entity->spawnParam0 = 1;
            gEwramData->playerState.actionLockFlags |= 0x800;
            Anim_StartIndexed(gEwramData->playerState.player, 0x34, 3, 0);
            PlaySong(0x1A8);
            break;
        }
        entity->timer--;
        break;
    case 2:
        gEwramData->playerState.actionLockFlags |= 0x800;
        gEwramData->playerStatus.inputOverrides |= 1;
        if ((s16)(entity->spawnParam1 % 60) == 0)
            PlaySong(0x95);
        if (entity->spawnParam1 <= 299)
            entity->spawnParam1++;
        else
            entity->state = 4;
        break;
    case 4:
        entity->state = 5;
        entity->timer = 0;
        if (entity->spawnParam0 != 0)
        {
            entity->timer = 0xF0;
            if (parent->ext.player.actionFlags & 0x20)
                PlaySong(0x1A2);
        }
        else
            entity->hidden = 1;
        entity->hidden = 1;
        break;
    case 5:
        if (entity->timer == 0)
            goto delete;
        if (!(entity->timer & 7))
        {
            s32 x = entity->posX.whole + (RandomSigned(0x10) << 16);
            Effect_SpawnShared(x, entity->posY.whole + ((RandomSigned(0x10) - 0x10) << 16), 0x10, 8);
        }
        entity->timer--;
        goto end;
    }
    goto end;
delete:
    BlueSoul_ReleaseActiveResources(entity);
    EntityDelete(entity);
    return;
end:
    *str += entity->spawnParam1 >> 1;
    gEwramData->playerState.currentStats[3] += entity->spawnParam1;
    Player_RecalculateAttackDefense();
    Anim_Update(entity);
}

/* Same as BlueSoul_DeleteEntity: end a blue soul entity. */

/**
 * @brief 2F548 | Shadow Knight blue soul (ends via BlueSoul_ShouldEnd): graphics
 * sSkillShadowKnightGfx / sSkillShadowKnightPalette / sSkillShadowKnightSpriteSheet, an affine slot growing for 32 frames
 * (x by 0x1000, y by 0x800 up to 0x10000), box slot 6, Hit_SetTimerOverride(2, 60),
 * skillBaseAttack/skillAttackAttributes = 0x23 / 1, song 0x1A6. Then it alternates animation 0 while
 * the caster's weaponEntities[unk_50C] slot is taken and animation 1 when that
 * ends; always at the caster, facing the other way (Anim_UpdateWithFrameAttackBox).
 */
void SkillShadowKnightUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;
    u16 slot;

    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
    {
        u8 *graphics = (u8 *)sSkillShadowKnightGfx;
        u8 *palette = (u8 *)sSkillShadowKnightPalette;
        u8 *sprite = (u8 *)sSkillShadowKnightSpriteSheet;
        LoadGfx(entity, 0, graphics, palette, sprite);
    }
        Anim_StartIndexed(entity, 1, 0, 1);
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            EndBlueSoul(entity);
            return;
        }
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->shrinkPartOffsets = 1;
        entity->spriteLayer = 0xF;
        entity->drawFunc = Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        entity->state++;
        entity->timer = 0x20;
        Hit_Enable(entity, 6, 0, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        Hit_SetTimerOverride(2, 0x3C);
        {
            u16 a = 0x23;
            u16 b = 1;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = b;
        }
        PlaySong(0x1A6);
    case 1:
        if (entity->ext.skillGrowScale.scaleX <= 0xFFFF)
            entity->ext.skillGrowScale.scaleX += 0x1000;
        if (SKILL_SHADOW_KNIGHT_SCALE_Y(entity) <= 0xFFFF)
            SKILL_SHADOW_KNIGHT_SCALE_Y(entity) += 0x800;
        ObjAffine_Set(0, entity->ext.skillGrowScale.scaleX, SKILL_SHADOW_KNIGHT_SCALE_Y(entity), entity->flipX, 0, entity->affineIndex);
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        ObjAffine_Free(entity->affineIndex);
        entity->affine = 0;
        entity->affineIndex = 0;
        entity->state++;
    case 2:
        if (gEwramData->playerState.weaponEntities[parent->ext.player.weaponSlot] != NULL)
        {
            Anim_StartIndexed(entity, 0, 0, 0);
            entity->state = 3;
        }
        break;
    case 3:
        if (entity->animFlags & 4)
        {
            entity->state = 2;
            Anim_StartIndexed(entity, 1, 0, 1);
        }
        break;
    }
    Follow(entity, parent, 0, 0);
    entity->flipX ^= 1;
    Anim_UpdateWithFrameAttackBox(entity);
}

/* Inlined BlueSoul_DeleteEntity (this file). */

/* Blue soul projectile spawned with a contact child by SkillWitchGiantGhost_OnContact: moves by
 * its velocity with effect Effect_CreateFromTable and deletes itself off screen. */
/**
 * @brief Blue soul projectile spawned on contact: move by its velocity with a
 * trail effect and delete itself off screen.
 */
void SkillWitchGiantGhostProjectileUpdate(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        Hit_Enable(entity, 6, BlueSoulProjectile_IgnoreContact, 0);
        HitBox_SetAttackRaw(entity, 0x0808FCFC);
        entity->hit.flags |= 4;
        entity->scrollWithBg = 1;
        entity->state++;
        PlaySong(0x98);
    case 1:
        Effect_CreateFromTable(entity->posX.whole, entity->posY.whole, 0, 0, 5, 6);
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        break;
    case 2:
        EndBlueSoul(entity);
        return;
    }
    entity->roomActive = 1;
    if (EntityIsOutsideScreen(entity, 2))
        entity->state = 2;
}

/* SkillWitchGiantGhost_OnContact. */

/**
 * @brief Contact callback of the Witch/Giant Ghost soul: spawn a
 * SkillWitchGiantGhostProjectileUpdate child at the contact point moving
 * away from the entity, then set palette 4.
 */
void SkillWitchGiantGhost_OnContact(struct EwramData_EntityData *entity, struct EwramData_EntityData *target)
{
    struct EwramData_EntityData *child = BlueSoul_CreateEntity(entity, SkillWitchGiantGhostProjectileUpdate);
    s16 x, y;
    s16 dx;
    u16 angle;

    if (child != NULL)
    {
        Hit_GetContactPoint(entity, target, &x, &y);
        dx = x - entity->posX.part16.integer;
        angle = ArcTan2(dx, y - (entity->posY.part16.integer - 0x10) + RandomSigned(4));
        child->posX.part16.integer = x;
        child->posY.part16.integer = y;
        child->velX.whole = Sin(angle + 0x4000) << 2;
        child->velY.whole = Sin(angle) << 2;
    }
    entity->palette = 4;
}

/* Same as BlueSoul_DeleteEntity: end a blue soul entity. */

/**
 * @brief 2F96C | Witch / Giant Ghost blue soul (unk_500 = 0 / 1, ends via
 * BlueSoul_CheckEndCondition): affine slot, box slot 7 with callback SkillWitchGiantGhost_OnContact,
 * Hit_SetTimerOverride(2, 10), skillBaseAttack/skillAttackAttributes = 0x1E / 0x20. Witch: graphics
 * sSkillWitchGfx (layer 0xD, box 0x2010E010, song 0x97); Giant Ghost: graphics
 * sSkillGiantGhostGfx drawn by BlueSoul_DrawAlternatingLayer (layer 0xE, box 0x2830D8E8, song 0x8D).
 * Grows (scale unk_50C += 0x2000 up to 0x10000) at the caster, then on the
 * end shrinks by 0x2000 per frame and calls BlueSoul_ReleaseActiveResources.
 */
void Skill_Witch_GiantGhost_Use(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;
    u16 slot;

    entity->roomActive = 1;
    {
        u8 three = 3;
        if (entity->state < three && BlueSoul_CheckEndCondition(entity))
        {
            entity->state = three;
            entity->hit.flags |= 2;
        }
    }
    switch (entity->state)
    {
    case 0:
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            EndBlueSoul(entity);
            return;
        }
        entity->scrollWithBg = 1;
        Hit_SetTimerOverride(2, 10);
        Hit_Enable(entity, 7, SkillWitchGiantGhost_OnContact, 0);
        entity->hit.flags |= 2;
        entity->hit.flags |= 4;
        if (entity->ext.blueSoul.value == 0)
        {
            u8 *graphics = (u8 *)sSkillWitchGfx;
            u8 *palette = (u8 *)sSkillWitchPalette;
            u8 *sprite = (u8 *)sSkillWitchSpriteSheet;
            LoadGfx(entity, 0, graphics, palette, sprite);
            entity->drawFunc = Sprite_DrawEntity;
            entity->spriteLayer = 0xD;
            Anim_StartIndexed(entity, 0, 0, 1);
            HitBox_SetAttackRaw(entity, 0x2010E010);
            PlaySong(0x97);
        }
        else
        {
            u8 *graphics = (u8 *)sSkillGiantGhostGfx;
            u8 *palette = (u8 *)sSkillGiantGhostPalette;
            u8 *sprite = (u8 *)sSkillGiantGhostSpriteSheet;
            LoadGfx(entity, 0, graphics, palette, sprite);
            entity->drawFunc = BlueSoul_DrawAlternatingLayer;
            entity->spriteLayer = 0xE;
            Anim_StartIndexed(entity, 1, 0, 1);
            HitBox_SetAttackRaw(entity, 0x2830D8E8);
            PlaySong(0x8D);
        }
        entity->affineIndex = slot;
        entity->shrinkPartOffsets = 1;
        entity->affine = 1;
        {
            u16 a = 0x1E;
            u16 b = 0x20;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = b;
        }
        entity->state++;
    case 1:
        if (entity->ext.skillWitch.scale <= 0xFFFF)
        {
            entity->ext.skillWitch.scale += 0x2000;
            break;
        }
        entity->affine = 0;
        entity->state++;
        entity->hit.flags &= ~2;
        break;
    case 3:
        entity->affine = 1;
        if (entity->ext.skillWitch.scale <= 0)
        {
            BlueSoul_ReleaseActiveResources(entity);
            EntityDelete(entity);
            return;
        }
        entity->ext.skillWitch.scale -= 0x2000;
        break;
    }
    entity->palette = 3;
    Follow(entity, parent, 0, 0);
    ObjAffine_Set(0, entity->ext.skillWitch.scale, entity->ext.skillWitch.scale, entity->flipX, 0, entity->affineIndex);
    Anim_Update(entity);
}

/* Final Guard soul: once the parent is idle and in the right state, load the shield graphics and keep it until the state changes. */

/* Inlined Entity_PlaceRelativeTo (this file). */
static inline void FollowParent3(struct EwramData_EntityData *entity, struct EwramData_EntityData *anchor, u16 offsetX, u16 offsetY)
{
    s16 dx = offsetX;
    s16 dy = offsetY;

    entity->flipX = anchor->flipX;
    if (entity->flipX)
        dx = -dx;
    entity->posX.part16.integer = (s32)dx + (s16)anchor->posX.part16.integer;
    entity->posY.part16.integer = (s32)dy + anchor->posY.part16.integer;
}

void SkillFinalGuardUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;

    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        if (parent->state != 0)
        {
            BlueSoul_ReleaseActiveResources(entity);
            EntityDelete(entity);
            return;
        }
        if ((parent->ext.player.actionFlags & 0x1001A2) != 0x100000)
        {
            BlueSoul_ReleaseActiveResources(entity);
            EntityDelete(entity);
            return;
        }
    {
        u8 *graphics = sSkillFinalGuardGfx;
        u8 *palette = sSkillFinalGuardPalette;
        u8 *sprite = sSkillFinalGuardSpriteSheet;
        LoadGfx(entity, 0, graphics, palette, sprite);
    }
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->spriteLayer = 0xD;
        entity->drawFunc = Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        gEwramData->playerState.player->state = 10;
        Anim_StartIndexed(gEwramData->playerState.player, 0x35, 3, 1);
        parent->ext.player.actionFlags &= 0xEFFFFBFF;
        PlaySong(0x1A7);
        entity->state++;
    case 1:
        entity->state++;
    default:
        if ((parent->ext.player.actionFlags & 0x1001A2) != 0x100000)
        {
            BlueSoul_ReleaseActiveResources(entity);
            EntityDelete(entity);
            return;
        }
        gEwramData->playerStatus.soulStateFlags |= 0x2000;
        gEwramData->playerState.actionLockFlags |= 0x820;
        FollowParent3(entity, parent, 0, 0);
        Anim_Update(entity);
    }
}

#define RADIUS(e) (*(s32 *)&(e)->spawnParam0)

/**
 * @brief Child of SkillBuerUse: animated hit sprite (box slot 6, 0x1414F6F6)
 * orbiting the parent at angle ENTITY_HALF_510 (step mp) and radius
 * parent spawnParam0, dropping a small rising effect every fourth update.
 */
void SkillBuerOrbitUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;
    struct EwramData_EntityData *flame;
    s32 x, y;

    switch (entity->state)
    {
    case 0:
    {
        u8 *graphics = sSkillBuerOrbitGfx;
        u8 *palette = sSkillBuerOrbitPalette;
        u8 *sprite = sSkillBuerOrbitSpriteSheet;
        GRAPHICS_CONTEXT->blueSoulGfx = ObjGfx_LoadResource(graphics, 0x10000);
        ObjPalette_LoadRange(palette, 0, 1, 3);
        Sprite_Attach(entity, sprite, GRAPHICS_CONTEXT->blueSoulGfx, 3);
    }
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->spriteLayer = 0xC;
        entity->drawFunc = Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        Hit_Enable(entity, 6, 0, 0);
        HitBox_SetAttackRaw(entity, 0x1414F6F6);
        entity->hit.flags |= 4;
        entity->state++;
    case 1:
        x = RADIUS(parent) * Sin(SKILL_BUER_ANGLE(entity) + 0x4000);
        y = RADIUS(parent) * Sin(SKILL_BUER_ANGLE(entity));
        entity->posX.whole = parent->posX.whole + x;
        entity->posY.whole = parent->posY.whole + y;
        SKILL_BUER_ANGLE(entity) += SKILL_BUER_SPEED(entity);
        entity->timer++;
        if ((entity->timer & 3) == 0)
        {
            y = -0x10000;
            flame = Effect_CreateFromTable(entity->posX.whole + RandomSigned(0x20000), entity->posY.whole - 0x40000, 0, y >> 4, 6, 7);
            if (flame != NULL)
            {
                flame->velY.whole = y;
                flame->spriteLayer = 0xD;
            }
        }
        break;
    }
    entity->roomActive = 1;
    Anim_Update(entity);
}

#define SIZE(e) (*(s32 *)&(e)->spawnParam0)

/**
 * @brief 2FF40 | Buer blue soul (ends via BlueSoul_CheckEndCondition): Hit_SetTimerOverride(2,
 * 0x18), skillBaseAttack/skillAttackAttributes = 12 / 2, two orbiting children SkillBuerOrbitUpdate (angles
 * 0 and 0x8000, speed 0x200 against the caster's facing), songs 0x1A5 and
 * 0x92; follows the caster with a vertical offset (velY) moving towards
 * -20 px, or -8 px while the caster has unk_4F4 bit 10, while spawnParam0
 * counts up to 28; on ending spawnParam0 counts down by 2 per frame, then
 * BlueSoul_ReleaseActiveResources and delete.
 */
void SkillBuerUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;
    struct EwramData_EntityData *child;
    s32 speed;
    u8 state;

    entity->roomActive = 1;
    {
        u8 two = 2;
        if (entity->state < two && BlueSoul_CheckEndCondition(entity))
            entity->state = two;
    }
    state = entity->state;
    switch (state)
    {
    case 0:
        Hit_SetTimerOverride(2, 0x18);
        {
            u16 a = 0xC;
            u16 b = 2;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = b;
        }
        entity->scrollWithBg = 1;
        entity->velY.whole = -0x100000;
        speed = 0x200;
        if (parent->flipX)
            speed = -speed;
        child = BlueSoul_CreateEntity(entity, SkillBuerOrbitUpdate);
        if (child != NULL)
        {
            SKILL_BUER_ANGLE(child) = state;
            SKILL_BUER_SPEED(child) = speed;
        }
        child = BlueSoul_CreateEntity(entity, SkillBuerOrbitUpdate);
        if (child != NULL)
        {
            SKILL_BUER_ANGLE(child) = 0x8000;
            SKILL_BUER_SPEED(child) = speed;
        }
        PlaySong(0x1A5);
        PlaySong(0x92);
        entity->state++;
    case 1:
        if (SIZE(entity) <= 0x1B)
            SIZE(entity)++;
        entity->posX.part16.integer = parent->posX.part16.integer;
        entity->posY.part16.integer = parent->posY.part16.integer + entity->velY.part16.integer;
        if (parent->ext.player.actionFlags & 0x400)
        {
            if (entity->velY.whole < -0x80000)
                entity->velY.whole += 0x4000;
        }
        else if (entity->velY.whole > -0x140000)
            entity->velY.whole -= 0x4000;
        break;
    case 2:
        if (SIZE(entity) > 0)
            SIZE(entity) -= 2;
        else
        {
            BlueSoul_ReleaseActiveResources(entity);
            EntityDelete(entity);
        }
        break;
    }
}

/* Same as BlueSoul_DeleteEntity: end a blue soul entity. */

/**
 * @brief 300A4 | Persephone blue soul (ends via BlueSoul_CheckEndCondition: song 0x9C
 * stopped, 16 frames, then BlueSoul_ReleaseActiveResources): graphics sSkillGiantGhostGfx / sSkillGiantGhostPalette
 * / sSkillGiantGhostSpriteSheet, box slot 6 with callback PersephoneSoul_OnHit (disabled),
 * Hit_SetTimerOverride(2, 20), skillBaseAttack/skillAttackAttributes = 10 / 0; keeps song 0x9C playing and
 * stays at the caster (Anim_UpdateWithFrameAttackBox).
 */
void SkillPersephoneUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;

    entity->roomActive = 1;
    {
        u8 three = 3;
        if (entity->state < three && BlueSoul_CheckEndCondition(entity))
        {
            entity->state = three;
            Sound_StopTracked(0x9C);
            entity->drawSkip = 1;
            entity->timer = 0x10;
        }
    }
    switch (entity->state)
    {
    case 0:
        entity->scrollWithBg = 1;
    {
        u8 *graphics = (u8 *)sSkillGiantGhostGfx;
        u8 *palette = (u8 *)sSkillGiantGhostPalette;
        u8 *sprite = (u8 *)sSkillGiantGhostSpriteSheet;
        LoadGfx(entity, 0, graphics, palette, sprite);
    }
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->spriteLayer = 0xD;
        entity->drawFunc = Sprite_DrawEntity;
        Hit_Enable(entity, 6, PersephoneSoul_OnHit, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        Hit_SetTimerOverride(2, 0x14);
        {
            u16 a = 0xA;
            u16 b = 0;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = b;
        }
        entity->state++;
        Sound_PlayTrackedIfAbsent(0x9C);
    case 1:
        Sound_PlayTrackedIfAbsent(0x9C);
        entity->state++;
    case 2:
        Sound_PlayTrackedIfAbsent(0x9C);
        Follow(entity, parent, 0, 0);
        Anim_UpdateWithFrameAttackBox(entity);
        break;
    case 3:
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        BlueSoul_ReleaseActiveResources(entity);
        EntityDelete(entity);
        break;
    }
}

/* SkillDeathSickleUpdate, decompiled from the original assembly. The second random
 * number goes into its own block variable: a block-local value ties with the
 * shifted bit in local-alloc, as in the target. */

/* Same as BlueSoul_DeleteEntity: end a blue soul entity. */

static inline void SetVelocity(struct EwramData_EntityData *entity)
{
    {
        s32 s = Sin((s16)SKILL_DEATH_SICKLE_HEADING(entity) + 0x4000);
        s32 r = entity->ext.skillDeathSickle.speed;
        entity->velX.whole = (s >> 8) * (r >> 8);
    }
    {
        s32 s = Sin((s16)SKILL_DEATH_SICKLE_HEADING(entity));
        s32 r = entity->ext.skillDeathSickle.speed;
        entity->velY.whole = (s >> 8) * (r >> 8);
    }
}

/**
 * @brief Projectile spawned by SkillDeathUse: appear with sound 0x96, pick the
 * selected or nearest target and home in on it.
 */
void SkillDeathSickleUpdate(struct EwramData_EntityData *entity)
{
    u16 slot;

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            EndBlueSoul(entity);
            return;
        }
        entity->affineIndex = slot;
        entity->affine = 1;
    {
        u8 *graphics = (u8 *)sSkillDeathGfx;
        u8 *palette = (u8 *)sSkillDeathPalette;
        u8 *sprite = (u8 *)sSkillDeathSpriteSheet;
        LoadGfx(entity, 0, graphics, palette, sprite);
    }
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->spriteLayer = 0xF;
        entity->drawFunc = Sprite_DrawEntity;
    {
        s32 r = RandomNumberGenerator();
        s32 one;
        SKILL_DEATH_SICKLE_SPIN_ANGLE(entity) = r;
        SKILL_DEATH_SICKLE_HEADING(entity) = r;
        SKILL_DEATH_SICKLE_SPIN_SPEED(entity) = 0xF700;
        { s32 r2 = RandomNumberGenerator(); s32 f = r2 & (one = 1); entity->flipX = f; }
        if (*(ENTITY_RENDER_FLAGS_ADDRESS(entity)) & 0x40)
            SKILL_DEATH_SICKLE_SPIN_SPEED(entity) = -SKILL_DEATH_SICKLE_SPIN_SPEED(entity);
        PlaySong(0x96);
        entity->state = one;
    }
    case 1:
    {
        s32 r;
        if (entity->ext.skillDeathSickle.scale <= 0xFFFF)
            entity->ext.skillDeathSickle.scale += 0x2000;
        SKILL_DEATH_SICKLE_SPIN_ANGLE(entity) += SKILL_DEATH_SICKLE_SPIN_SPEED(entity);
        r = Anim_Update(entity);
        if (r != 3)
            break;
        SKILL_DEATH_SICKLE_TARGET(entity) = Player_GetSelectedOrNearestTarget(entity);
        if (SKILL_DEATH_SICKLE_TARGET(entity) != NULL)
            entity->state = 2;
        else
            entity->state = r;
        Hit_Enable(entity, 6, 0, 0);
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->hit.flags |= 4;
        entity->timer = 0x40;
        break;
    }
    case 2:
    {
        struct EwramData_EntityData *target;
        s32 timer;
        if (entity->ext.skillDeathSickle.speed <= 0x1FFFF)
            entity->ext.skillDeathSickle.speed += 0x2000;
        target = SKILL_DEATH_SICKLE_TARGET(entity);
        SKILL_DEATH_SICKLE_HEADING(entity) = Entity_StepAngleTowardPoint(entity, target->posX.whole, target->posY.whole, (s16)SKILL_DEATH_SICKLE_HEADING(entity), 0x555);
        SKILL_DEATH_SICKLE_SPIN_ANGLE(entity) += SKILL_DEATH_SICKLE_SPIN_SPEED(entity);
        SetVelocity(entity);
        EntityAccelerateAndMove(entity, 0x80000);
        timer = entity->timer - 1;
        entity->timer = timer;
        if ((s16)target->hp <= 0 || (u8)timer == 0)
            entity->state = 3;
        break;
    }
    case 3:
        if (entity->ext.skillDeathSickle.speed <= 0x1FFFF)
            entity->ext.skillDeathSickle.speed += 0x2000;
        SKILL_DEATH_SICKLE_HEADING(entity) += SKILL_DEATH_SICKLE_SPIN_SPEED(entity) / 16;
        SKILL_DEATH_SICKLE_SPIN_ANGLE(entity) += SKILL_DEATH_SICKLE_SPIN_SPEED(entity);
        SetVelocity(entity);
        EntityAccelerateAndMove(entity, 0x80000);
        break;
    case 4:
        EndBlueSoul(entity);
        return;
    }
    ObjAffine_Set(SKILL_DEATH_SICKLE_SPIN_ANGLE(entity), entity->ext.skillDeathSickle.scale, entity->ext.skillDeathSickle.scale, entity->flipX, 0, entity->affineIndex);
    if (EntityIsOutsideScreen(entity, 1))
        entity->state = 4;
}

/* SkillDeathUse; graphics
 * loading as in SkillBlueSoulForm2Update. */

/**
 * @brief 30518 | Death blue soul (ends via BlueSoul_ShouldEnd): load its graphics,
 * hidden, Hit_SetTimerOverride(2, 0x14), skillBaseAttack/skillAttackAttributes = 0x14/1; follow 0x20 px above
 * the caster and every 0x38 frames (first after 8) spawn a SkillDeathSickleUpdate
 * child (BlueSoul_CreateEntity) at a random offset RandomSigned(0x40) per axis.
 */
void SkillDeathUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;

    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
    {
        u8 *graphics = sSkillDeathGfx;
        u8 *palette = sSkillDeathPalette;
        u8 *sprite = sSkillDeathSpriteSheet;
        GRAPHICS_CONTEXT->blueSoulGfx = ObjGfx_LoadResource(graphics, 0x10000);
        ObjPalette_LoadRange(palette, 0, 1, 3);
        Sprite_Attach(entity, sprite, GRAPHICS_CONTEXT->blueSoulGfx, 3);
    }
        entity->hidden = 1;
        Hit_SetTimerOverride(2, 0x14);
        {
            u16 a = 0x14;
            u16 b = 1;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = b;
        }
        entity->timer = 8;
        entity->state++;
    case 1:
        entity->flipX = parent->flipX;
        entity->posX.part16.integer = parent->posX.part16.integer;
        entity->posY.part16.integer = parent->posY.part16.integer - 0x20;
        if (--entity->timer == 0)
        {
            struct EwramData_EntityData *child = BlueSoul_CreateEntity(parent, SkillDeathSickleUpdate);
            if (child != NULL)
            {
                child->posX.part16.integer = entity->posX.part16.integer + RandomSigned(0x40);
                child->posY.part16.integer = entity->posY.part16.integer + RandomSigned(0x40);
                child->scrollWithBg = 1;
            }
            entity->timer = 0x38;
        }
    }
}

/* SkillCreakingSkullHitBoxUpdate. */
/**
 * @brief Child of SkillCreakingSkullUse: orbiting hit box (slot 6,
 * 0x2020F0F0) active while the parent flag is set, radius alternating
 * 0x30/0x10 around the parent at its angle, mirrored by flipX.
 */
void SkillCreakingSkullHitBoxUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target = &gEwramData->entityData[SKILL_CREAKING_SKULL_SOUL_INDEX(entity)];
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;
    u16 angle;
    s32 radius;
    s32 timer;

    entity->roomActive = 1;
    if (entity->state == 0)
    {
        Hit_Enable(entity, 6, 0, 0);
        HitBox_SetAttackRaw(entity, 0x2020F0F0);
        entity->hit.flags |= 4;
        entity->state++;
        entity->scrollWithBg = 1;
    }
    if (SKILL_CREAKING_SKULL_FRAME(target) == 0)
    {
        entity->hit.flags |= 2;
        return;
    }
    entity->hit.flags &= ~2;
    timer = entity->timer + 1;
    entity->timer = timer;
    if (timer & 1)
        entity->spawnParam1 = 0x30;
    else
        entity->spawnParam1 = 0x10;
    if (!parent->flipX)
        angle = -0x8000 + SKILL_CREAKING_SKULL_ANGLE(parent);
    else
        angle = -SKILL_CREAKING_SKULL_ANGLE(parent);
    radius = entity->spawnParam1;
    entity->posX.whole = parent->posX.whole + radius * Sin((s16)angle + 0x4000);
    entity->posY.whole = parent->posY.whole + radius * Sin((s16)angle);
}

#define TO_PIXELS(x) ((x) < 0 ? -(-(x) >> 16) : (x) >> 16)

static inline void FreeMatrix(struct EwramData_EntityData *entity)
{
    if (ENTITY_RENDER_FLAGS16(entity) & 0x101F)
        ObjAffine_Free(entity->affineIndex);
}

static inline void Destroy(struct EwramData_EntityData *entity)
{
    gEwramData->playerState.activeBlueSoulEntities[entity->ext.blueSoul.slot] = NULL;
    EntityDelete(entity);
}

/* Blue soul segment: turns towards its pose angle and orbits its parent. */
/**
 * @brief Body segment of the Creaking Skull soul: turn towards its pose angle
 * and orbit its parent.
 */
void SkillCreakingSkullSegmentUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *soul = &gEwramData->entityData[SKILL_CREAKING_SKULL_SOUL_INDEX(entity)];
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;
    u16 *angles = (u16 *)SKILL_CREAKING_SKULL_ANGLES(soul);
    u16 count = *angles;
    u16 matrix;
    s32 radius;
    u16 angle;
    s16 a;

    angles += 2;
    entity->roomActive = 1;
    if (entity->state != 0 && SKILL_CREAKING_SKULL_ACTIVE(soul) != 0)
        entity->state = 2;
    entity->flipX = soul->flipX;
    switch (entity->state)
    {
    case 0:
        matrix = ObjAffine_Alloc(0);
        if ((s16)matrix < 0)
        {
            FreeMatrix(entity);
            /* Dead store; it keeps a block boundary after the call so that
             * cross-jumping merges this copy with the one in case 4. */
            radius = 0;
            goto remove;
        }
        {
            u8 *graphics = sUnk_081E116C + 0x804;
            u8 *palette = sUnk_0820B64C + 0x24;
            u8 *sprite = sUnk_0823760C + 0x70;
            GRAPHICS_CONTEXT->blueSoulGfx = ObjGfx_LoadResource(graphics, 0x10000);
            ObjPalette_LoadRange(palette, 0, 1, 3);
            Sprite_Attach(entity, sprite, GRAPHICS_CONTEXT->blueSoulGfx, 3);
        }
        entity->animFrame = sUnk_080E1AF8[SKILL_CREAKING_SKULL_SEGMENT(entity)];
        entity->spriteLayer = 0xF;
        entity->drawFunc = Sprite_DrawEntity;
        entity->affineIndex = matrix;
        entity->affine = 1;
        entity->ext.skillCreakingSkullSegment.unused = 0x200000;
        SKILL_CREAKING_SKULL_SEGMENT_ANGLE(entity) = angles[SKILL_CREAKING_SKULL_SEGMENT(entity)];
        entity->scrollWithBg = 1;
        entity->state = 1;
        break;
    case 2:
    {
        s16 delta;
        a = angles[SKILL_CREAKING_SKULL_SEGMENT(entity)];
        AngleTurnDirection(SKILL_CREAKING_SKULL_SEGMENT_ANGLE(entity), a);
        delta = a - SKILL_CREAKING_SKULL_SEGMENT_ANGLE(entity);
        SKILL_CREAKING_SKULL_SEGMENT_TURN(entity) = delta / (s16)count;
        entity->spawnParam1 = count;
        entity->state = 3;
    }
    case 3:
    {
        a = angles[SKILL_CREAKING_SKULL_SEGMENT(entity)];
        SKILL_CREAKING_SKULL_SEGMENT_ANGLE(entity) += SKILL_CREAKING_SKULL_SEGMENT_TURN(entity);
        if (entity->spawnParam1 != 0)
        {
            entity->spawnParam1--;
            break;
        }
        SKILL_CREAKING_SKULL_SEGMENT_ANGLE(entity) = a;
        entity->state = 1;
        break;
    }
    case 1:
        break;
    case 4:
        if (ENTITY_RENDER_FLAGS16(entity) & 0x101F)
            ObjAffine_Free(entity->affineIndex);
    remove:
        Destroy(entity);
        return;
    }
    if (SKILL_CREAKING_SKULL_SEGMENT(entity) != 0)
    {
        radius = TO_PIXELS(sUnk_080E1AFC[SKILL_CREAKING_SKULL_SEGMENT(entity)] * soul->ext.skillCreakingSkull.scale);
        angle = -0x8000 + SKILL_CREAKING_SKULL_ANGLE(parent);
        if (entity->flipX)
            angle = -SKILL_CREAKING_SKULL_ANGLE(parent);
        entity->posX.whole = parent->posX.whole + radius * Sin((s16)angle + 0x4000);
        entity->posY.whole = parent->posY.whole + radius * Sin((s16)angle);
    }
    else
    {
        entity->posX.whole = parent->posX.whole;
        entity->posY.whole = parent->posY.whole;
    }
    angle = SKILL_CREAKING_SKULL_SEGMENT_ANGLE(entity);
    if (entity->flipX)
        angle = -(angle << 16) >> 16; /* negated in the upper half */
    ObjAffine_Set((s16)angle, soul->ext.skillCreakingSkull.scale, soul->ext.skillCreakingSkull.scale, entity->flipX, 0, entity->affineIndex);
}

struct SkullFrame {
    u8 duration;
    u8 pad_1;
    u16 frame;
};

/**
 * @brief 30A24 | Creaking Skull blue soul (ends via BlueSoul_ShouldEnd): graphics
 * sSkillCreakingSkullGfx / sSkillCreakingSkullPalette / sSkillCreakingSkullSpriteSheet, hidden, a chain of four children
 * (three SkillCreakingSkullSegmentUpdate segments and the SkillCreakingSkullHitBoxUpdate head, each spawned from
 * the previous one, spawnParam0 = caster byte / index), Hit_SetTimerOverride(2, 30),
 * skillBaseAttack/skillAttackAttributes = 0x28 / 0. Grows, then plays frame scripts (hp: idle
 * sSkillCreakingSkullFrames, bite sSkillCreakingSkullBiteFrames while the caster's weaponEntities slot is taken;
 * entries {duration, frame}, NULL ends; song 0x9B on bite step 3) 20 px
 * above the caster.
 */
void SkillCreakingSkullUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;
    struct EwramData_EntityData *prev, *child;
    struct SkullFrame *frame;
    s32 i;

    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    SKILL_CREAKING_SKULL_ACTIVE(entity) = 0;
    if (entity->stage != 0 && gEwramData->playerState.weaponEntities[parent->ext.player.weaponSlot] != NULL)
    {
        SKILL_CREAKING_SKULL_FRAMES(entity) = (s32)sSkillCreakingSkullBiteFrames;
        entity->state = 3;
        entity->stage = 0;
    }
    switch (entity->state)
    {
    case 0:
    {
        u8 *graphics = (u8 *)sSkillCreakingSkullGfx;
        u8 *palette = (u8 *)sSkillCreakingSkullPalette;
        u8 *sprite = (u8 *)sSkillCreakingSkullSpriteSheet;
        LoadGfx(entity, 0, graphics, palette, sprite);
    }
        entity->hidden = 1;
        prev = entity;
        for (i = 0; i < 4; i++)
        {
            if (i < 3)
                child = BlueSoul_CreateEntity(prev, SkillCreakingSkullSegmentUpdate);
            else
                child = BlueSoul_CreateEntity(prev, SkillCreakingSkullHitBoxUpdate);
            if (child != NULL)
            {
                SKILL_CREAKING_SKULL_SOUL_INDEX(child) = ENTITY_INDEX(entity);
                SKILL_CREAKING_SKULL_SEGMENT(child) = i;
                prev = child;
            }
        }
        Hit_SetTimerOverride(2, 0x1E);
        {
            u16 a = 0x28;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = 0;
        }
        SKILL_CREAKING_SKULL_ANGLES(entity) = (s32)sSkillCreakingSkullAngles;
        SKILL_CREAKING_SKULL_FRAMES(entity) = (s32)sSkillCreakingSkullFrames;
        entity->flipX = 1 ^ parent->flipX;
        entity->state = 1;
    case 1:
        if (entity->ext.skillCreakingSkull.scale <= 0xFFFF)
            entity->ext.skillCreakingSkull.scale += 0x1000;
        else
            entity->state = 2;
        break;
    case 2:
        SKILL_CREAKING_SKULL_FRAMES(entity) = (s32)sSkillCreakingSkullFrames;
        entity->state = 3;
        entity->stage = 1;
        break;
    case 3:
        {
            struct SkullFrame **list = (struct SkullFrame **)SKILL_CREAKING_SKULL_FRAMES(entity);
            frame = list[entity->subState];
        }
        if (frame == NULL)
        {
            entity->state = 2;
            entity->subState = 0;
            break;
        }
        SKILL_CREAKING_SKULL_ANGLES(entity) = (s32)frame;
        SKILL_CREAKING_SKULL_ACTIVE(entity) = 1;
        entity->timer = frame->duration - 1;
        SKILL_CREAKING_SKULL_FRAME(entity) = frame->frame;
        entity->state = 4;
    case 4:
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        if (SKILL_CREAKING_SKULL_FRAMES(entity) == (s32)sSkillCreakingSkullBiteFrames && entity->subState == 3)
            PlaySong(0x9B);
        entity->state = 3;
        entity->subState++;
        break;
    }
    entity->posX.part16.integer = parent->posX.part16.integer;
    entity->posY.part16.integer = parent->posY.part16.integer - 0x14;
}

void SkillMedusaHeadUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;
    s32 done;
    s32 velocity;
    if (entity->state == 0)
    {
        if ((parent->ext.player.actionFlags & 0x01008082) != 2)
        {
            BlueSoul_ReleaseActiveResources(entity);
            EntityDelete(entity);
            return;
        }
        velocity = parent->velY.whole;
        entity->velY.whole = velocity;
        entity->accelY.whole = parent->accelY.whole;
        if (velocity < 0) entity->velY.whole = 0;
        entity->accelY.whole = 0x2800;
        PlaySong(0x8C);
        entity->state = 1;
    }
    done = BlueSoul_ShouldEnd(entity);
    if (done)
    {
        parent->velY.whole = entity->velY.whole;
        parent->accelY.whole = entity->accelY.whole;
        EntityDelete(entity);
        return;
    }
    if ((parent->ext.player.actionFlags & 0x01008082) != 2)
    {
        parent->velY.whole = entity->velY.whole;
        parent->accelY.whole = entity->accelY.whole;
        BlueSoul_ReleaseActiveResources(entity);
        EntityDelete(entity);
        return;
    }
    gEwramData->playerState.actionLockFlags |= 0x20;
    gEwramData->playerState.player->velY.whole = done;
    gEwramData->playerState.player->accelY.whole = done;
}

/* SkillCatoblepasProjectileUpdate (blue soul projectile). */

/* Same as BlueSoul_DeleteEntity: end a blue soul entity. */

/**
 * @brief Child of SkillCatoblepasUse: same projectile behavior as
 * SkillBonePillarProjectileUpdate (box 0x1414F6F6, 26-update timer).
 */
void SkillCatoblepasProjectileUpdate(struct EwramData_EntityData *entity)
{
    entity->roomActive = 1;
    switch (entity->state)
    {
        case 0:
            Sprite_Attach(entity, sSkillCatoblepasProjectileSpriteSheet, gEwramData->playerState.blueSoulGfx, 3);
            Anim_StartIndexed(entity, 0, 0, 0);
            entity->spriteLayer = 0xD;
            entity->drawFunc = Sprite_DrawEntity;
            Hit_Enable(entity, 6, NULL, 0);
            HitBox_SetAttackRaw(entity, 0x1414F6F6);
            entity->hit.flags |= 4;
            entity->timer = 0x1A;
            entity->scrollWithBg = 1;
            entity->state++;
        case 1:
            entity->spriteLayer = (entity->timer >> 2) + 7;
            EntityAccelerateAndMove(entity, 0x80000);
            if (--entity->timer == 0)
            {
                BlueSoulProjectile_Delete(entity);
                return;
            }
            break;
    }
    Anim_Update(entity);
}

/**
 * @brief 30E64 | Catoblepas blue soul, same frame as SkillBonePillarUse:
 * graphics sSkillCatoblepasGfx / sSkillCatoblepasPalette / sSkillCatoblepasProjectileSpriteSheet, Hit_SetTimerOverride(2, 30),
 * skillBaseAttack/skillAttackAttributes = 10 / 0x100. Every 5 frames breathes a SkillCatoblepasProjectileUpdate
 * projectile (song 0x8E every other one) at the head, speed 0x18000 +
 * caster velX, deceleration 0x400, vertical speed sSkillProjectileLaunchVelocities[unk_50A]
 * + 0x8000 and vertical acceleration 0x200.
 */
void SkillCatoblepasUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;

    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    if ((parent->ext.player.actionFlags & 0x1001A2) != 0x100000)
    {
        BlueSoul_ReleaseActiveResources(entity);
        EntityDelete(entity);
        return;
    }
    gEwramData->playerState.actionLockFlags |= 0x820;
    switch (entity->state)
    {
    case 0:
    {
        u8 *graphics = (u8 *)sSkillCatoblepasGfx;
        u8 *palette = (u8 *)sSkillCatoblepasPalette;
        u8 *sprite = (u8 *)sSkillCatoblepasProjectileSpriteSheet;
        LoadGfx(entity, 0, graphics, palette, sprite);
    }
        entity->hidden = 1;
        Hit_SetTimerOverride(2, 0x1E);
        {
            u16 a = 0xA;
            u16 b = 0x100;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = b;
        }
        gEwramData->playerState.player->state = 0xA;
        Anim_StartIndexed(gEwramData->playerState.player, 0x35, 3, 1);
        parent->ext.player.actionFlags &= 0xEFFFFBFF;
        entity->state++;
    case 1:
        Follow(entity, parent, 0x10, -0x1C);
        if (entity->timer == 0)
        {
            struct EwramData_EntityData *child = BlueSoul_CreateEntity(entity, SkillCatoblepasProjectileUpdate);
            if (child != NULL)
            {
                s32 vx, vy;
                if (entity->subState)
                    PlaySong(0x8E);
                entity->subState ^= 1;
                Follow(child, entity, 0, 0);
                vx = 0x18000;
                child->velX.whole = vx;
                vy = 0x400;
                child->accelX.whole = vy;
                if (child->flipX)
                {
                    child->velX.whole = -vx;
                    child->accelX.whole = -vy;
                }
                child->velX.whole += parent->velX.whole;
                child->flipX = parent->flipX;
                child->velY.whole = sSkillProjectileLaunchVelocities[entity->ext.skillCatoblepas.shotIndex] + 0x8000;
                child->accelY.whole = 0x200;
            }
            if (++entity->ext.skillCatoblepas.shotIndex > 2)
                entity->ext.skillCatoblepas.shotIndex = 0;
            entity->timer = 5;
        }
        entity->timer--;
    }
}

/**
 * @brief 310AC | Imp blue soul (familiar, ends via BlueSoul_CheckEndCondition): graphics
 * sSkillImpGfx / palette sSkillImpPalette / sprite sSkillImpSpriteSheet (slot 3), layer 0xF,
 * box slot 6, Hit_SetTimerOverride(2, 60), skillBaseAttack/skillAttackAttributes = 15 / 0x11, song 0x19E.
 * Rises from the caster, then flies towards its anchor point
 * (Target_GetX / Target_GetY - 40 px), circles it (sin/cos by timer)
 * and now and then (1-2 in 256) picks the nearest target (Player_GetSelectedOrNearestTarget) to
 * attack at a random height of 16-23 px above it; state 6 waits 16 frames
 * and ends via BlueSoul_ReleaseActiveResources.
 */

void SkillImpUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;
    s32 x = Target_GetX(entity);
    s32 y = Target_GetY(entity) - 0x280000;
    s32 dist;

    entity->roomActive = 1;
    {
    u8 six = 6;
    if (entity->state < six && BlueSoul_CheckEndCondition(entity))
    {
        entity->state = six;
        entity->drawSkip = 1;
        entity->timer = 0x10;
    }
    }
    switch (entity->state)
    {
    case 0:
    {
        u8 *graphics = (u8 *)sSkillImpGfx;
        u8 *palette = (u8 *)sSkillImpPalette;
        u8 *sprite = (u8 *)sSkillImpSpriteSheet;
        LoadGfx(entity, 0, graphics, palette, sprite);
    }
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->spriteLayer = 0xF;
        entity->drawFunc = Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        entity->flipX = parent->flipX;
        entity->posX.part16.integer = parent->posX.part16.integer;
        entity->posY.part16.integer = parent->posY.part16.integer - 0x10;
        entity->velY.whole = -0x4000;
        Hit_Enable(entity, 6, 0, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        Hit_SetTimerOverride(2, 0x3C);
        {
            u16 a = 0xF;
            u16 b = 0x11;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = b;
        }
        entity->state = 1;
        PlaySong(0x19E);
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->state = 2;
        break;
    case 2:
        if ((RandomNumberGenerator() & 0xFF) == 0)
        {
            entity->state = 4;
            break;
        }
        dist = Entity_StepTowardPoint(entity, x, y, 5);
        if (entity->velX.whole >= 0)
            entity->flipX = 1;
        else
            entity->flipX = 0;
        if (dist <= 0x7FFF)
            entity->state = 3;
        break;
    case 3:
        if ((RandomNumberGenerator() & 0xFF) <= 1)
        {
            entity->state = 4;
            break;
        }
        entity->timer++;
        dist = entity->timer << 8;
        entity->velX.whole = Sin(dist) / 4;
        dist = entity->timer << 9;
        entity->velY.whole = Sin(dist + 0xC000) / 4;
        EntityAccelerateAndMove(entity, 0x20000);
        if (Entity_DistanceToPoint(entity, x, y) > 0x300000)
            entity->state = 2;
        break;
    case 4:
        SKILL_IMP_TARGET(entity) = Player_GetSelectedOrNearestTarget(entity);
        if (SKILL_IMP_TARGET(entity) == NULL)
        {
            entity->state = 2;
            entity->timer = 0x80;
            break;
        }
        SKILL_IMP_AIM_Y(entity) = -(((RandomNumberGenerator() & 7) + 0x10) << 16);
        entity->state = 5;
    case 5:
    {
        struct EwramData_EntityData *target = SKILL_IMP_TARGET(entity);
        s32 timer = entity->timer - 1;
        entity->timer = timer;
        if ((s16)target->hp > 0 && (u8)timer != 0)
        {
            dist = Entity_StepTowardPoint(entity, target->posX.whole, target->posY.whole + SKILL_IMP_AIM_Y(entity), 4);
            if (entity->velX.whole >= 0)
                entity->flipX = 1;
            else
                entity->flipX = 0;
            if (dist > 0x3FFF)
                break;
        }
        entity->state = 2;
        break;
    }
    case 6:
        if (entity->timer == 0)
        {
            BlueSoul_ReleaseActiveResources(entity);
            EntityDelete(entity);
            return;
        }
        entity->timer--;
        break;
    }
    if (entity->state != 6)
        Anim_UpdateWithFrameAttackBox(entity);
}

/* Short-lived blue-soul companion; cancel if its active soul slot is gone. */
static inline s32 IsSlotEmpty(struct EwramData_EntityData *entity)
{
    return entity == NULL;
}

static inline struct EwramData_EntityData *GetActiveBlueSoulEntity(struct EwramData *data, u32 index)
{
    u32 scaled = index * 4;
    u32 offset = OFFSET_OF(struct EwramData, playerState.activeBlueSoulEntities);
    u8 *slots = (u8 *)data + offset;
    slots += scaled;
    return *(struct EwramData_EntityData **)slots;
}
/* Keep the separate offset lifetimes for the first and indexed slot reads.
 * They reproduce agbcc's reload and allocation. The countdown copy similarly
 * retains the original compare/decrement registers. These are compiler
 * constraints, not evidence for the original source spelling. */
/**
 * @brief Delete the afterimage when its blue soul slot is empty, else count
 * down its timer and delete it at the end.
 */
void BlueSoulAfterimage_Update(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram = gEwramData;
    s32 empty;
    {
        s8 cancelled = -1;
        ewram->playerState.blueSoulGfx = cancelled;
    }
    {
        s32 offset = OFFSET_OF(struct EwramData, playerState.activeBlueSoulEntities);
        s16 missing = IsSlotEmpty(*(struct EwramData_EntityData **)((u8 *)ewram + offset));
        empty = missing;
    }
    if (empty || GetActiveBlueSoulEntity(ewram, entity->stage) == NULL)
    {
        do { EntityDelete(entity); } while (0); /* FAKEMATCH: loop note weights entity (global-alloc order) */
        return;
    }
    {
        struct EwramData_EntityData *timerEntity = entity;
        u8 timerByte = timerEntity->timer;
        s16 timer = timerByte;
        if (timer == 4) timerEntity->drawSkip = 1;
        if (timer != 0) timerEntity->timer = timerByte - 1;
        else EntityDelete(timerEntity);
    }
}

/**
 * @brief Unreferenced: create a copy of the owner sprite frame one layer above
 * it (BlueSoulAfterimage_Update, 8 frames, blue soul slot from unk_508);
 * return it or NULL.
 */
struct EwramData_EntityData *BlueSoul_CreateAfterimage(struct EwramData_EntityData *owner)
{
    s32 graphics = Sprite_GetTileSlot(owner);
    u8 *index = &owner->spriteSlot;
    struct SpriteSheet *sprite = gEwramData->spriteSlots[*index].sheet;
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, BlueSoulAfterimage_Update);
    if (entity != NULL)
    {
        if (sprite != NULL)
        {
            Sprite_Attach(entity, sprite, graphics, owner->palette);
            entity->animFrame = owner->animFrame;
            entity->spriteLayer = owner->spriteLayer + 1;
            entity->flipX = owner->flipX;
            entity->timer = 8;
            entity->stage = owner->ext.raw.unk_508.part16.unk_508;
            entity->scrollWithBg = 1;
            return entity;
        }
        EntityDelete(entity);
    }
    return NULL;
}

/* SkillAlastorUse: Alastor familiar attack. Only matches without -f2003-patch
 * (NO_2003_PATCH_SRCS in ../Makefile). (flipX test mask kept in a variable across the
 * position copies, explicit u32 negation of dx). */

#define FIXED_TO_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)

void SkillAlastorUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;
    s32 x = Target_GetX(entity);
    s32 y = Target_GetY(entity) - 0x280000;
    s32 dist;
    u16 slot;

    entity->roomActive = 1;
    {
    u8 six = 6;
    if (entity->state < six && BlueSoul_CheckEndCondition(entity))
        entity->state = six;
    }
    switch (entity->state)
    {
    case 0:
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            if (entity->affineIndex || entity->affine)
                ObjAffine_Free(entity->affineIndex);
            gEwramData->playerState.activeBlueSoulEntities[entity->ext.blueSoul.slot] = NULL;
            EntityDelete(entity);
            return;
        }
        entity->affineIndex = slot;
        entity->affine = 1;
    {
        u8 *graphics = (u8 *)sSkillAlastorGfx;
        u8 *palette = (u8 *)sSkillAlastorPalette;
        u8 *sprite = (u8 *)sSkillAlastorSpriteSheet;
        LoadGfx(entity, 0, graphics, palette, sprite);
    }
        entity->spriteLayer = 0xF;
        entity->animFrame = 0;
        entity->drawFunc = Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        {
            u32 m;
            entity->flipX = parent->flipX;
            m = 0x40;
            entity->posX.part16.integer = parent->posX.part16.integer;
            entity->posY.part16.integer = parent->posY.part16.integer - 0x10;
            SKILL_ALASTOR_SPIN(entity) = 0x100;
            m &= ((u8 *)entity)[0x58];
        if (m)
            SKILL_ALASTOR_SPIN(entity) = -SKILL_ALASTOR_SPIN(entity);
        }
        entity->flipX ^= 1;
        Hit_Enable(entity, 6, 0, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        Hit_SetTimerOverride(2, 0x3C);
        {
            u16 a = 0x19;
            u16 b = 1;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = b;
        }
        entity->state = 1;
        PlaySong(0x19E);
    case 1:
        SKILL_ALASTOR_ANGLE(entity) += SKILL_ALASTOR_SPIN(entity);
        if (entity->ext.skillAlastor.scale > 0xFFFF)
        {
            entity->state = 2;
            break;
        }
        entity->ext.skillAlastor.scale += 0x1000;
        break;
    case 2:
    {
        s32 a;
        entity->hit.flags |= 2;
        if ((RandomNumberGenerator() & 0x7F) == 0)
        {
            entity->state = 4;
            break;
        }
        dist = Entity_StepTowardPoint(entity, x, y, 5);
        entity->timer++;
        SKILL_ALASTOR_ANGLE(entity) += Sin(entity->timer << 9) / 512;
        if (entity->velX.whole >= 0)
        {
            if (SKILL_ALASTOR_ANGLE(entity) <= 0x1FFF)
                SKILL_ALASTOR_ANGLE(entity) += (entity->velX.whole >> 8);
        }
        else
        {
            if (SKILL_ALASTOR_ANGLE(entity) > -0x2000)
                SKILL_ALASTOR_ANGLE(entity) += ((entity->velX.whole + 0xFF) >> 8);
        }
        if (dist <= 0x7FFF)
            entity->state = 3;
        break;
    }
    case 3:
        entity->hit.flags |= 2;
        if ((RandomNumberGenerator() & 0x7F) <= 1)
        {
            entity->state = 4;
            break;
        }
        entity->timer++;
        dist = entity->timer << 8;
        entity->velY.whole = Sin(dist + 0x4000) / 4;
        EntityDampVelocityX(entity, 0x40);
        EntityAccelerateAndMove(entity, 0x20000);
        if (Entity_DistanceToPoint(entity, x, y) > 0x300000)
            entity->state = 2;
        break;
    case 4:
        SKILL_ALASTOR_TARGET(entity) = Player_GetSelectedOrNearestTarget(entity);
        if (SKILL_ALASTOR_TARGET(entity) == NULL)
        {
            entity->state = 2;
            entity->timer = 0x80;
            break;
        }
        SKILL_ALASTOR_AIM_Y(entity) = -((RandomNumberGenerator() & 0xF) << 16);
        entity->state = 5;
    case 5:
    {
        struct EwramData_EntityData *target;
        s32 timer;
        entity->hit.flags &= ~2;
        target = SKILL_ALASTOR_TARGET(entity);
        timer = entity->timer - 1;
        entity->timer = timer;
        if ((s16)target->hp > 0 && (u8)timer != 0)
        {
            dist = Entity_StepTowardPoint(entity, target->posX.whole, target->posY.whole + SKILL_ALASTOR_AIM_Y(entity), 5);
            SKILL_ALASTOR_ANGLE(entity) = entity->velX.whole / 64 + SKILL_ALASTOR_ANGLE(entity) + SKILL_ALASTOR_SPIN(entity) * 4;
            if (dist > 0x1FFF)
                break;
        }
        entity->state = 2;
        break;
    }
    case 6:
        SKILL_ALASTOR_ANGLE(entity) = SKILL_ALASTOR_SPIN(entity) * 16 + SKILL_ALASTOR_ANGLE(entity);
        if (entity->ext.skillAlastor.scale <= 0)
        {
            BlueSoul_ReleaseActiveResources(entity);
            EntityDelete(entity);
            return;
        }
        entity->ext.skillAlastor.scale -= 0x1000;
        break;
    }
    if (entity->state == 5)
    {
        u16 a = SKILL_ALASTOR_ANGLE(entity) - 0x4000;
        u16 dx = FIXED_TO_INT(Sin((s16)a + 0x4000) << 4);
        u16 dy = FIXED_TO_INT(Sin((s16)a) << 4);
        if (entity->flipX)
            dx = (u32)-(s32)(dx << 16) >> 16;
        HitBox_SetAttackRaw(entity, ((((s16)dx - 10) & 0xFF) + ((((s16)dy - 10) & 0xFF) << 8)) + 0x14140000);
    }
    ObjAffine_Set(SKILL_ALASTOR_ANGLE(entity), entity->ext.skillAlastor.scale, entity->ext.skillAlastor.scale, entity->flipX, 0, entity->affineIndex);
}

/* SkillGreatArmorUse: Great Armor soul effect. Local register bindings in the
   stat multiply are compiler constraints, not evidence of original spelling. */

/* Inlined Entity_PlaceRelativeTo (this file). */

void SkillGreatArmorUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;
    s16 *stats = gEwramData->playerState.currentStats;
    u32 t;

    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
    {
        u8 *graphics = sUnk_081E116C;
        u8 *palette = sUnk_0820B64C;
        u8 *sprite = sUnk_0823760C;
        GRAPHICS_CONTEXT->blueSoulGfx = ObjGfx_LoadResource(graphics, 0x10000);
        ObjPalette_LoadRange(palette, 0, 1, 3);
        Sprite_Attach(entity, sprite, GRAPHICS_CONTEXT->blueSoulGfx, 3);
    }
        Anim_StartIndexed(entity, 1, 0, 1);
        entity->spriteLayer = 0xD;
        entity->drawFunc = Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        entity->state++;
    case 1:
        FollowParent3(entity, parent, 0, 0);
        Anim_Update(entity);
        t = entity->timer + 1;
        entity->timer = t;
        if ((t & 0x1F) == 0)
            PlaySong(0x94);
        else if ((t & 0xF) == 0 && (RandomNumberGenerator() & 1))
            PlaySong(0x94);
        if ((entity->timer & 7) == 0)
        {
            s32 x = parent->posX.whole + (RandomSigned(0x10) << 16);
            Effect_SpawnShared(x, parent->posY.whole + ((RandomSigned(0x10) - 0x10) << 16), 0x10, 7);
        }
        break;
    }
    stats += entity->ext.blueSoul.value;
    { register s32 product asm("r0") = *stats * 0x133; /* FAKEMATCH: pinned register */
      register s32 result asm("r1"); /* FAKEMATCH: pinned register */
      if (product < 0) product += 255;
      result = product >> 8;
      { s16 *destination = stats;
      *destination = result; } }
    Player_RecalculateAttackDefense();
}

/* Same as BlueSoul_DeleteEntity: end a blue soul entity. */

/**
 * @brief 31ACC | Big Golem blue soul, same frame as SkillShadowKnightUse:
 * graphics sSkillBigGolemGfx / sSkillBigGolemPalette / sSkillBigGolemSpriteSheet, skillBaseAttack/skillAttackAttributes = 0x1E / 0,
 * song 0x1A6; animation 1 while the caster's weaponEntities[unk_50C] slot is
 * taken (song 0xC5 on frame 4), back to animation 0 when it ends; held
 * 12 px behind and 4 px above the caster.
 */
void SkillBigGolemUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;
    u16 slot;

    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
    {
        u8 *graphics = (u8 *)sSkillBigGolemGfx;
        u8 *palette = (u8 *)sSkillBigGolemPalette;
        u8 *sprite = (u8 *)sSkillBigGolemSpriteSheet;
        LoadGfx(entity, 0, graphics, palette, sprite);
    }
        Anim_StartIndexed(entity, 0, 0, 1);
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            EndBlueSoul(entity);
            return;
        }
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->shrinkPartOffsets = 1;
        entity->spriteLayer = 0xF;
        entity->drawFunc = Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        entity->state++;
        entity->timer = 0x20;
        Hit_Enable(entity, 6, 0, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        Hit_SetTimerOverride(2, 0x3C);
        {
            u16 a = 0x1E;
            u16 b = 0;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = b;
        }
        PlaySong(0x1A6);
    case 1:
        if (entity->ext.skillGrowScale.scaleX <= 0xFFFF)
            entity->ext.skillGrowScale.scaleX += 0x1000;
        if (SKILL_BIG_GOLEM_SCALE_Y(entity) <= 0xFFFF)
            SKILL_BIG_GOLEM_SCALE_Y(entity) += 0x800;
        ObjAffine_Set(0, entity->ext.skillGrowScale.scaleX, SKILL_BIG_GOLEM_SCALE_Y(entity), entity->flipX, 0, entity->affineIndex);
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        ObjAffine_Free(entity->affineIndex);
        entity->affine = 0;
        entity->affineIndex = 0;
        entity->state++;
    case 2:
        if (gEwramData->playerState.weaponEntities[parent->ext.player.weaponSlot] != NULL)
        {
            Anim_StartIndexed(entity, 1, 0, 0);
            entity->state = 3;
        }
        break;
    case 3:
        if (*(u16 *)&entity->animStep == 4)
            PlaySong(0xC5);
        if (entity->animFlags & 4)
        {
            entity->state = 2;
            Anim_StartIndexed(entity, 0, 0, 1);
        }
        break;
    }
    {
        s32 dx = -0xC;
        entity->flipX = parent->flipX;
        if (entity->flipX)
            dx = 0xC;
        entity->posX.part16.integer = parent->posX.part16.integer + dx;
        entity->posY.part16.integer = parent->posY.part16.integer - 4;
    }
    Anim_UpdateWithFrameAttackBox(entity);
}

/* Blue and yellow soul helpers 0x08031D6C..0x08032443. */

/* Blue soul info, indexed by equippedBlueSoul (12-byte records). */

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
    gEwramData->playerState.activeBlueSoulEntities[entity->ext.blueSoul.slot] = NULL;
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
    child->ext.yellowSoul.param = yellowSoul->effectParam;
    child->ext.yellowSoul.value = yellowSoul->effectValue;
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
    child->ext.yellowSoul.owner = gEwramData->playerState.player;
    child->ext.yellowSoul.param = yellowSoul->effectParam;
    child->ext.yellowSoul.value = yellowSoul->effectValue;
    gEwramData->playerState.activeYellowSoulEntity = child;
    if (soul == 0xC && (gEwramData->playerStatus.statusEffects & 1))
    {
        gEwramData->playerState.currentStats[0] >>= 2;
        gEwramData->playerState.currentStats[1] >>= 2;
        gEwramData->playerState.currentStats[2] >>= 2;
    }
    child->updateFunc(child);
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
    if (player->velX.whole != 0 || (player->ext.player.actionFlags & 0x360))
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
    gEwramData->playerStatus.soulStateFlags |= entity->ext.yellowSoul.value;
}

/**
 * @brief Mark the entity active and double current attack when the stored selector is zero, otherwise double current defense.
 */
void YellowSoul_DoubleAttackOrDefense(struct EwramData_EntityData *entity)
{
    entity->roomActive = 1;
    if (entity->ext.yellowSoul.value == 0)
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

    stat += (u16)entity->ext.yellowSoul.param;
    *stat += entity->ext.yellowSoul.value;
    entity->roomActive = 1;
    Player_RecalculateAttackDefense();
}

/**
 * @brief 322A0 | Souls giving resistances: resistances |= unk_500 (and
 * Player_ClearStatusEffects(unk_500 >> 6) when statusEffects is set).
 */
void Skill_PoisonWorm_Ectoplasm_Gargoyle_Use(struct EwramData_EntityData *entity)
{
    s32 resist = entity->ext.yellowSoul.value;

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
