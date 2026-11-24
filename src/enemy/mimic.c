/*
 * Mimic: body and trigger.
 */
#include "graphics.h"
#include "structs/ewram.h"
#include "core.h"
#include "gameplay.h"
#include "sound.h"
#include "player_core.h"
#include "effects.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "hitbox.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "damage_numbers.h"
#include "entity_hit.h"

extern s32 Target_IsInRect(s16 x, s16 y, s32 w, s32 h);
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);

void Mimic_UpdateWait(struct EwramData_EntityData *);
void Mimic_UpdatePreview(struct EwramData_EntityData *);
u8 Mimic_LoadGraphics(struct EwramData_EntityData *);
void Mimic_DamagePlayerOnContact(struct EwramData_EntityData *);
void Mimic_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
void Mimic_SpawnTrigger(struct EwramData_EntityData *, s32, s32);
void MimicTrigger_Update(struct EwramData_EntityData *);
void MimicTrigger_OnHit(struct EwramData_EntityData *, s32, u16 *);

/* Mimic: dispatch state, refresh the box on frame changes and run collision. */
void EnemyMimicUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    union MimicAnimation animation = sMimicAnimation;
    u8 frame;
    entity->ext.mimic.flags = 0;
    if (Enemy_PreUpdate(entity)) return;
    switch (entity->state)
    {
    case 0: Mimic_UpdateWait(entity); break;
    case 1: Enemy_UpdateFragmentDeath(entity); break;
    case 2:
        Mimic_UpdatePreview(entity);
        Anim_Update(entity);
        return;
    }
    frame = entity->animFrame;
    if (entity->ext.mimic.flags & 2)
    {
        Anim_Start(entity, (struct AnimScript *)&animation, 0, 1);
        animation.bytes[4] = entity->ext.mimic.heldFrame;
        Anim_Update(entity);
    }
    else Anim_Update(entity);
    if (frame != entity->animFrame) Entity_UpdateBoxes(entity);
    if (entity->state != 1 && !(entity->ext.mimic.flags & 1)) Hit_CheckWithPlayer(&result, entity);
}

/* Mimic initialization, including the dormant and alternate room-start paths. */
void EnemyMimicCreate(struct EwramData_EntityData *entity)
{
    u32 box = 0x0E10F2F8;
    if (!Mimic_LoadGraphics(entity)) { EntityDelete(entity); return; }
    Enemy_ResetFromTemplate(entity);
    entity->flipX = 0;
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 2;
        Anim_StartIndexed(entity, 0, 0, 0);
        Anim_Update(entity);
        return;
    }
    if (Anim_GetMaxBoxCount(entity) > 1) Hit_EnableContact(entity, 8, Mimic_DamagePlayerOnContact, Mimic_OnHit);
    else Hit_Enable(entity, 8, Mimic_DamagePlayerOnContact, Mimic_OnHit);
    if (entity->spawnParam0 != 0)
    {
        entity->ext.mimic.flags |= 2;
        entity->ext.mimic.heldFrame = 0;
        entity->animFrame = 0;
        HitBox_SetBody(entity, &box);
        HitBox_SetAttack(entity, &box);
        entity->hit.flags &= ~4;
        entity->hit.flags |= 2;
        entity->state = 0;
        entity->subState = 3;
        Entity_SnapToGround(entity);
    }
    else
    {
        Anim_StartIndexed(entity, 0, 0, 1);
        Anim_Update(entity);
        entity->hidden = 1;
        Mimic_SpawnTrigger(entity, 0, 0);
        Entity_UpdateBoxes(entity);
        entity->state = 0;
        entity->subState = 0;
    }
}

/**
 * @brief 990D8 | Enemy sub-state machine (subState) with flags in unk_4FC:
 * 0 hidden; 1-2 falls (Entity_MoveWithCollisionAlt, accelY 0x2000) until it lands (bit
 * 2), then the 0x0E10F2F8 box with contact on; 3 waits for the player
 * within a 32x20 area above it (Target_IsInRect), then animation 0; 4
 * animation 1 after the current one ends; 5 hidden off screen.
 */
void Mimic_UpdateWait(struct EwramData_EntityData *entity)
{
    u32 box = 0x0E10F2F8;

    switch (entity->subState)
    {
    case 0:
        entity->ext.mimic.flags |= 1;
        entity->hidden = 1;
        break;
    case 1:
        entity->subState++;
        entity->hidden = 0;
        entity->accelY.whole = 0x2000;
    case 2:
        entity->ext.mimic.flags |= 3;
        entity->ext.mimic.heldFrame = 0;
        entity->animFrame = 0;
        if (Entity_MoveWithCollisionAlt(entity, 0x10000, -0x100000, 0x40000) & 4)
        {
            entity->subState++;
            HitBox_SetBody(entity, &box);
            HitBox_SetAttack(entity, &box);
            entity->hit.flags &= 0xFB;
            entity->hit.flags |= 2;
        }
        break;
    case 3:
        entity->ext.mimic.flags |= 2;
        entity->ext.mimic.heldFrame = 0;
        entity->animFrame = 0;
        if (Target_IsInRect(entity->posX.part16.integer - 0x10, entity->posY.part16.integer - 0x14, 0x20, 0x14))
        {
            entity->ext.mimic.flags = 0;
            Anim_StartIndexed(entity, 0, 0, 0);
            entity->subState++;
        }
        break;
    case 4:
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->subState++;
        }
        break;
    case 5:
        if (EntityIsOutsideScreen(entity, 4))
            entity->hidden = 1;
        else
            entity->hidden = 0;
        break;
    }
}

void Mimic_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 damage = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(entity, (s32)attacker, hit, (s16)damage);
    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;
        damage <<= 16;
        damage >>= 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, (s16)damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x72);
        if (entity->subState == 3) Anim_StartIndexed(entity, 3, 0, 0);
        else Anim_StartIndexed(entity, 2, 0, 0);
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->state = 1;
        entity->subState = 0;
    }
}

/* Spawn a fragment at owner + (x, y): graphics 081C15F4 / 0820ED60
 * (palette 7, animation 3, priority 20), hurt box 0x100CF8FA (bytes x -6,
 * y -8, w 12, h 16; not an address), hit callback MimicTrigger_OnHit; it keeps the
 * owner (unk_500) and its type byte. */

/**
 * @brief Spawn the decoy at owner + (x, y): an effect sprite (animation 3) drawn
 * like a pickup source (PickupSource_Draw); hitting it wakes the
 * Mimic (MimicTrigger_OnHit).
 */
void Mimic_SpawnTrigger(struct EwramData_EntityData *owner, s32 x, s32 y)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, MimicTrigger_Update);
    u32 box = 0x100CF8FA;
    s32 graphics;

    if (child == NULL)
        return;
    child->posX.whole = owner->posX.whole + x;
    child->posY.whole = owner->posY.whole + y;
    child->spriteLayer = 0x14;
    graphics = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(child, sCommonObjectSpriteSheet, graphics, 7);
    Anim_StartIndexed(child, 3, 0, 1);
    Anim_Update(child);
    Hit_Enable(child, 8, NULL, MimicTrigger_OnHit);
    HitBox_SetBody(child, &box);
    child->hit.flags &= ~4;
    child->hit.flags |= 2;
    child->ext.mimicTrigger.owner = owner;
    child->enemyId = owner->enemyId;
    child->scrollWithBg = 1;
    child->drawFunc = PickupSource_Draw;
}

/* MimicTrigger_Update and MimicTrigger_OnHit. */
/* state and subState read as one halfword (as in enemy/graham.c). */
#define STATE_WORD(e) (*(u16 *)&(e)->state)

/**
 * @brief 9937C | State 0: hitbox. State 1: hidden; while the timer timer
 * is above 0x1E spawn Effect_SpawnRandom particles every 4 frames, and set
 * bit 3 of 0x53D when it runs out.
 */
void MimicTrigger_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;

    switch (entity->state)
    {
        case 0:
            Hit_CheckWithPlayer(&result, entity);
            break;
        case 1:
            entity->hidden = 1;
            if (entity->timer > 0x1E && !(entity->timer & 3))
                Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0x80000, 0x80000, entity->spriteLayer - 1);
            if (--entity->timer == 0xFF)
                entity->deleteRequested = 1;
            break;
    }
}

/**
 * @brief 993EC | Hit callback: Player_CalculateAttackPower(hit[2]), go to state 1
 * with timer 0x28 and advance the substate of the parent (unk_500_entity)
 * when it is alive, of the same type (enemyId) and in state 0 / substate 0.
 */
void MimicTrigger_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    struct EwramData_EntityData *mimic = entity->ext.mimicTrigger.owner;

    Player_CalculateAttackPower(hit[2]);
    entity->state = 1;
    entity->subState = 0;
    entity->timer = 0x28;
    if (mimic != NULL && mimic->updateFunc != NULL
        && mimic->enemyId == entity->enemyId
        && STATE_WORD(mimic) == 0)
        mimic->subState++;
}

/* Helpers 0x08099430..0x080994AF (end of this file.s; MimicTrigger_OnHit
 * before them reads state/subState of its parent as one u16 and stays
 * asm). */

/**
 * @brief 99430 | Load the graphics (sMimicGfx), palette from
 * sMimicPalette for (u8)spawnParam1 and start animation set sMimicSpriteSheet;
 * returns 0 when ObjGfx_LoadResource fails.
 */
u8 Mimic_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sMimicGfx, 0);

    if (gfx < 0)
        return 0;

    entity->basePalette = ObjPalette_FindOrLoadVariant(sMimicPalette, (u8)entity->spawnParam1);
    Sprite_Attach(entity, sMimicSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 99480 | Substate 0: once bit 0 of 0x53D is set, start animation 1
 * and advance the substate.
 */
void Mimic_UpdatePreview(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0 && entity->animDone)
    {
        Anim_StartIndexed(entity, 1, 0, 1);
        entity->subState++;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Mimic_DamagePlayerOnContact = Object28_OnPlayerContact */

/**
 * @brief 994B0 | Same as Object28_OnPlayerContact.
 */
void Mimic_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}
