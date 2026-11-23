/*
 * Kyoma Demon: update, create and the lurk state (attack, hits, drawing, death: kyoma_demon_b.c).
 * Built with -fno-cse-follow-jumps (KyomaDemon_UpdateLurk needs it; update / create compile the same either way).
 */
#include "core.h"
#include "graphics.h"
#include "entity_util.h"
#include "player_core.h"
#include "gameplay.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "kyoma_demon_b.h"
#include "math_anim.h"

extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void KyomaDemon_UpdateLurk(struct EwramData_EntityData *);

void EnemyKyomaDemonUpdate(struct EwramData_EntityData *entity)
{
    u32 box[4];
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    u8 frame;
    if (entity->state != 2)
    {
        if (Entity_IsInRect(entity, -16, 0, 0x110, 0xB0)) Player_TrackEntity(entity);
    }
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 2 && entity->ext.kyomaDemon.hidden == 0)
            Hit_CheckWithPlayer(box, entity);
        return;
    }
    switch (entity->state)
    {
    case 0: KyomaDemon_UpdateLurk(entity); break;
    case 1: KyomaDemon_UpdateAttack(entity); break;
    case 2: KyomaDemon_UpdateDeath(entity); break;
    case 3: Anim_Update(entity); return;
    }
    frame = entity->animFrame;
    Anim_Update(entity);
    if (frame != entity->animFrame) Entity_UpdateBoxes(entity);
    if (entity->state != 2 && entity->ext.kyomaDemon.hidden == 0)
        Hit_CheckWithPlayer(box, entity);
    entity->ext.kyomaDemon.roomX = entity->posX.whole + background->xPos.whole;
    entity->ext.kyomaDemon.roomY = entity->posY.whole + background->yPos.whole;
}

extern s32 Hit_EnableContact(struct EwramData_EntityData *, s32, void *, s32);
extern void KyomaDemon_OnHit(struct EwramData_EntityData *);
void EnemyKyomaDemonCreate(struct EwramData_EntityData *entity)
{
    struct EwramData **ewram = &gEwramData;
    struct BgLayerState *camera = &(*ewram)->bgInfo[1];
    s32 random;
    if (!KyomaDemon_LoadGraphics(entity)) { EntityDelete(entity); return; }
    Anim_StartIndexed(entity, 1, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if ((*ewram)->gameState.bestiaryPreview) { entity->state = 3; return; }
    if (Anim_GetMaxBoxCount(entity) > 1) Hit_EnableContact(entity, 8, KyomaDemon_DamagePlayerOnContact, (s32)KyomaDemon_OnHit);
    else Hit_Enable(entity, 8, KyomaDemon_DamagePlayerOnContact, (s32)KyomaDemon_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->spriteLayer = 29;
    entity->state = 0;
    entity->ext.kyomaDemon.hidden = 1;
    entity->scrollWithBg = 0;
    random = RandomNumberGenerator() % 32;
    random += 32;
    entity->ext.kyomaDemon.lurkTimer = random;
    entity->ext.kyomaDemon.homeX = entity->posX.part16.integer + camera->xPos.part16.integer;
    entity->ext.kyomaDemon.homeY = entity->posY.part16.integer + camera->yPos.part16.integer;
    if (entity->spawnParam0 != 0)
    {
        entity->state = 2;
        entity->subState = 1;
        entity->hidden = 1;
    }
}

/* Entity that follows the player at a fixed offset (KyomaDemon_UpdateLurk). Needs
 * -fno-cse-follow-jumps (Makefile): the target recomputes ew + 0x13110 after
 * the range check instead of reusing it across the jump. */

extern u8 sUnk_08528340[];

/* Byte 0x58 of an entity (unk_53C) viewed as plain u8 bitfields. */
struct EntityFlags58
{
    u8 low:6;
    u8 flipX:1;
    u8 flipY:1;
};

/**
 * @brief Lurk state (state 0): hide behind the scenery and wait (random timer in
 * unk_4FE), then appear near the target and start the attack (state 1).
 */
void KyomaDemon_UpdateLurk(struct EwramData_EntityData *entity)
{
    struct EwramData *ew = gEwramData;
    struct BgLayerState *bg = &ew->bgInfo[1];
    s32 px;
    s32 x;
    s32 one;
    s16 *ppx;
    u8 state = entity->subState;

    switch (state)
    {
    case 0:
    {
        s32 playerFlags = ((u8 *)ew->playerState.player)[0x58] >> 6;
        struct EntityFlags58 *flags;

        one = 1;
        flags = (struct EntityFlags58 *)(ENTITY_RENDER_FLAGS_ADDRESS(entity));
        flags->flipX = ~playerFlags & one;
    }
        entity->posX.whole = ew->playerState.player->posX.whole + 0x100000;
        entity->posY.whole = ew->playerState.player->posY.whole;
        ppx = &entity->posX.part16.integer;
        px = *ppx + (s16)bg->xPos.part16.integer;
        x = (s16)entity->ext.kyomaDemon.homeX;
        if (px < x - 0x20 || px > x + 0x20)
        {
            entity->hidden = 1;
            break;
        }
        entity->hidden = 0;
        if (ew->playerState.player->ext.player.actionFlags & 2)
        {
            Anim_Start(entity, (struct AnimScript *)(sUnk_08528340 + 0x24), 0, 0);
            entity->subState = one;
            break;
        }
        if (entity->ext.kyomaDemon.lurkTimer <= 0)
        {
            entity->ext.kyomaDemon.lurkTimer = RandomNumberGenerator() % 64 + 64;
            px = *ppx + (s16)bg->xPos.part16.integer;
            x = (s16)entity->ext.kyomaDemon.homeX;
            if (px > x - 10 && px < x + 10)
            {
                entity->state = one;
                entity->subState = 0;
                break;
            }
        }
        else
        {
            entity->ext.kyomaDemon.lurkTimer--;
        }
        if (entity->posX.whole + bg->xPos.whole == entity->ext.kyomaDemon.roomX
            && entity->posY.whole + bg->yPos.whole == entity->ext.kyomaDemon.roomY)
        {
            if (entity->animScriptIndex)
                Anim_StartIndexed(entity, 0, 0, 1);
        }
        else if (entity->animScriptIndex != 1)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            Anim_Update(entity);
            entity->animTimer = 0x7F;
            Anim_Update(entity);
        }
        break;
    case 1:
        entity->flipX = state & ~(((u8 *)ew->playerState.player)[0x58] >> 6);
        entity->posX.whole = ew->playerState.player->posX.whole + 0x100000;
        entity->posY.whole = ew->playerState.player->posY.whole;
        px = (s16)entity->posX.part16.integer + (s16)bg->xPos.part16.integer;
        x = (s16)entity->ext.kyomaDemon.homeX;
        if (px < x - 0x20 || px > x + 0x20)
            entity->hidden = 1;
        else
            entity->hidden = 0;
        if (!(gEwramData->playerState.player->ext.player.actionFlags & 2))
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->subState = 0;
        }
        break;
    }
}
