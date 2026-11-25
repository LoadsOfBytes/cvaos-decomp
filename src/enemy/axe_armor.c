/*
 * Axe Armor (the axe update is in the previous, separately compiled file).
 */
#include "axe_armor.h"
#include "core.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "math_anim.h"
#include "minotaur.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "player_core.h"
#include "sound.h"
#include "effects.h"
#include "constants/songs.h"


void AxeArmor_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hitData);

/* AxeArmor_UpdateAttack: Axe Armor attack substates (throws the axe AxeArmorAxe_Update); matched. */

/* States of AxeArmor (entity->state), named after the update function the dispatcher calls. */
enum {
    AXE_ARMOR_STATE_WALKING = 0,
    AXE_ARMOR_STATE_ATTACK = 1,
    AXE_ARMOR_STATE_ANIM_ONLY = 3,
};

/**
 * @brief Choose a throw animation, create an axe on its event, then retreat or resume walking.
 */
void AxeArmor_UpdateAttack(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *axe;
    u16 dx;
    s32 distance;
    s32 m1;
    u8 frame;
    s32 speed;
    s32 attack;

    m1 = -1;
    switch (entity->subState)
    {
    case 0:
        if (RandomNumberGenerator() & 1)
        {
            Anim_StartIndexed(entity, 1, 0, 0);
            entity->stage = 0;
        }
        else
        {
            Anim_StartIndexed(entity, 2, 0, 0);
            entity->stage = 1;
        }
        entity->subState = 1;
        break;
    case 1:
        if (*(u16 *)&entity->animStep == 0x20C)
        {
            axe = Entity_CreateObject(AxeArmorAxe_Update);
            if (axe != NULL)
            {
                dx = -0x18;
                axe->posX.part16.integer = entity->posX.part16.integer;
                axe->posY.part16.integer = entity->posY.part16.integer - 0x10;
                axe->flipX = entity->flipX;
                if (axe->flipX)
                    dx = 0x18;
                {
                    s32 t = (s16)dx;
                    axe->posX.part16.integer += t;
                }
                axe->attack = entity->attack;
                axe->stage = entity->stage;
            }
        }
        if (!(entity->animFlags & 4))
            break;
        if ((entity->flipX && (s8)Entity_GetTargetDirection(entity) == 1)
            || (!entity->flipX && (s8)Entity_GetTargetDirection(entity) == m1))
        {
            distance = Entity_GetTargetDistance(entity);
            attack = 1;
            if (distance <= 99)
                goto test;
        }
        attack = 0;
    test:
        if (attack)
        {
            Anim_StartIndexed(entity, 0, 0, 0);
            entity->subState = 2;
        }
        else
            entity->subState = 3;
        break;
    case 2:
        frame = entity->animStep % 9;
        speed = frame * 0x2800 + 0x1000;
        if (frame > 6)
            speed = 0;
        Entity_Walk(entity, -speed, sAxeArmorWalkProfile, 0x14);
        if (entity->animFlags & 4)
            entity->subState = 0;
        break;
    case 3:
        entity->state = AXE_ARMOR_STATE_WALKING;
        entity->stage = 0;
        entity->subState = 0;
        Anim_StartIndexed(entity, 0, 0, 1);
        break;
    }
}

/* Axe Armor death. */

/* State 0 starts animation 4 and an upward velocity; state 1 runs for at most
 * 32 frames, emitting rising debris on even frames and a sound plus a second
 * effect every eighth frame.  Returns 1 once the animation reports flag 4. */
/**
 * @brief Start the death animation and emit debris; return 1 when the animation finishes, otherwise 0.
 */
s32 AxeArmor_UpdateDeath(struct EwramData_EntityData *entity)
{
    u8 state = entity->subState;
    switch (state)
    {
    case 0:
        Anim_StartIndexed(entity, 4, 0, 0);
        entity->subState = 1;
        entity->velY.whole = -0x280000;
        entity->velX.whole = state;
        PlaySong(0x6F);
        break;
    case 1:
    {
        u32 timer;
        s32 y, phase, x;
        timer = entity->timer + 1;
        entity->timer = timer;
        y = entity->posY.whole + entity->velY.whole;
        entity->velY.whole += 0x12000;
        phase = entity->velX.whole;
        entity->velX.whole = phase + 0x5000;
        if (entity->animFlags & 4)
            return 1;
        if ((u8)timer > 0x20)
            break;
        if ((timer & state) == 0)
        {
            struct EwramData_EntityData *debris;
            s32 offset = Sin(phase + 0x9000);
            x = entity->posX.whole + (offset << 4) + RandomSigned(0x40000);
            debris = Effect_SpawnShared(x, y, 1, 8);
            if (debris != NULL)
            {
                debris->velY.whole = 0x8000;
                debris->accelY.whole = -0x1000;
            }
        }
        if ((entity->timer & 7) == 0)
        {
            PlaySong(SE_ENEMY_DEATH);
            x = entity->posX.whole + (RandomSigned(16) << 16);
            y += RandomSigned(4) << 16;
            Effect_SpawnShared(x, y, 16, 6);
        }
        break;
    }
    }
    return 0;
}

/* EnemyAxeArmorCreate. */

/**
 * @brief B12C4 | Axe Armor create: load the graphics (delete the enemy
 * when that fails), hit boxes, layer 0x13, sprite 0; state 3 when
 * bestiaryPreview is set, otherwise collision callbacks, boxes, one pixel lower,
 * 0x53D bit 1 and state 0.
 */
void EnemyAxeArmorCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sAxeArmorGfx, sAxeArmorPalette, 0, sAxeArmorSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x13;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = AXE_ARMOR_STATE_ANIM_ONLY;
        return;
    }
    Entity_EnableBoxes(entity, Enemy_OnContact, AxeArmor_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->posY.part16.integer++;
    entity->scrollWithBg = 1;
    entity->state = AXE_ARMOR_STATE_WALKING;
}

/* AxeArmor_OnHit. */

/**
 * @brief Face the target and apply damage; on death drop rewards, restore the palette and enter state 2.
 */
void AxeArmor_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hitData)
{
    Entity_FaceTarget(entity);
    if (Enemy_TakeDamage(entity, attacker, hitData))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/**
 * @brief Clear axe health so its next update starts destruction.
 */
void AxeArmorAxe_ClearHealth(struct EwramData_EntityData *entity)
{
    entity->hp = 0;
}

/* EnemyAxeArmorUpdate. */

/**
 * @brief B13C4 | Axe Armor update: unless Enemy_PreUpdate handles the frame,
 * run the state (0 AxeArmor_UpdateWalking, 1 AxeArmor_UpdateAttack, 2 AxeArmor_UpdateDeath - delete
 * when it returns nonzero, 3 only animate) and then Enemy_CheckHitPlayer /
 * Entity_AnimateWithBoxes.
 */
void EnemyAxeArmorUpdate(struct EwramData_EntityData *entity)
{
    if (Enemy_PreUpdate(entity))
        return;
    switch (entity->state)
    {
        case AXE_ARMOR_STATE_WALKING:
            AxeArmor_UpdateWalking(entity);
            break;
        case AXE_ARMOR_STATE_ATTACK:
            AxeArmor_UpdateAttack(entity);
            break;
        case 2:
            if (AxeArmor_UpdateDeath(entity))
            {
                EntityDelete(entity);
                return;
            }
            break;
        case AXE_ARMOR_STATE_ANIM_ONLY:
            Anim_Update(entity);
            return;
    }
    Enemy_CheckHitPlayer(entity);
    Entity_AnimateWithBoxes(entity);
}
