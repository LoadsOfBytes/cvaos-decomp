#include "code_08009A0.h"
#include "death_entity.h"
#include "code_08013960.h"
#include "code_08033CAC.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *update);
extern s32 Hit_EnableContact(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Enemy_OnContact(struct EwramData_EntityData *entity);
extern void Death_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void DeathOverlay_Update(struct EwramData_EntityData *entity);
extern void DeathIntroEffect_Update(struct EwramData_EntityData *entity);
extern void DeathFloatingWeapon_Update(struct EwramData_EntityData *entity);
extern u8 sDeathGfx[], sDeathPalette[], sDeathSpriteSheet[];

/**
 * @brief BF2E8 | Death create: the boss version (unk_514) is deleted once
 * beaten (bossEventFlags bit 1) unless in the boss rush mode; sprite layer 0x13,
 * animation 1, child DeathOverlay_Update; bestiary preview gives state 8; otherwise
 * box slot 8 (Hit_EnableContact) with box 0x3230CEE8, boxes off; the boss
 * version starts in state 0 at room position (128, 32) on screen with song
 * 0x1010 and child DeathIntroEffect_Update, else state 1 with song 0x10; HP 0x98C,
 * child DeathFloatingWeapon_Update and the boss HP bar.
 */
void EnemyDeathCreate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;
    struct EwramData_EntityData *first;
    struct BgLayerState *bg;
    u8 frozen;

    if (entity->unk_514.part16.spawnParam0 != 0 && !gEwramData->gameState.bossRushMode
        && (gEwramData->gameState.bossEventFlags & 2))
        goto delete;
    if (!Entity_LoadGraphics(entity, sDeathGfx, sDeathPalette, 0, sDeathSpriteSheet))
    {
    delete:
        EntityDelete(entity);
        return;
    }
    entity->flipX = 0;
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x13;
    Anim_StartIndexed(entity, 1, 2, 1);
    Anim_Update(entity);
    first = Entity_CreateObject((u32 *)DeathOverlay_Update);
    if (first != NULL)
    {
        DEATH_OWNER_INDEX(first) = ENTITY_INDEX(entity);
        first->scrollWithBg = 1;
    }
    frozen = *((u8 *)gEwramData + 0x8E) & 0x40;
    if (frozen)
    {
        entity->state = 8;
        return;
    }
    Hit_EnableContact(entity, 8, Enemy_OnContact, (s32)Death_OnHit);
    HitBox_SetBodyRaw(entity, 0x3230CEE8);
    HitBox_SetAttackRaw(entity, 0x3230CEE8);
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    if (entity->unk_514.part16.spawnParam0 != 0)
    {
        entity->state = 0;
        bg = &gEwramData->bgInfo[1];
        entity->posX.part16.integer = 0x80 - bg->xPos.part16.integer;
        entity->posY.part16.integer = 0x20 - bg->yPos.part16.integer;
        PlaySong(0x1010);
        child = Entity_CreateObject((u32 *)DeathIntroEffect_Update);
        if (child != NULL)
        {
            DEATH_OWNER_INDEX(child) = ENTITY_INDEX(entity);
            child->scrollWithBg = 1;
        }
    }
    else
    {
        entity->state = 1;
        Music_SetCurrent(0x10);
    }
    entity->objMode = 1;
    entity->unk_518.part16.hp = 0x98C;
    child = Entity_CreateObject((u32 *)DeathFloatingWeapon_Update);
    if (child != NULL)
    {
        DEATH_OWNER_INDEX(child) = ENTITY_INDEX(entity);
        child->scrollWithBg = 1;
    }
    Player_SetSelectedTarget(entity);
    gEwramData->gameState.stateFlags |= 2;
}
