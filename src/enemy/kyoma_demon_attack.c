/*
 * Kyoma Demon: attack, hits, drawing, graphics and death (after the lurk state, own CFLAGS).
 */
#include "kyoma_demon_attack.h"
#include "core.h"
#include "sound.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "player_core.h"
#include "entity_common.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "damage_numbers.h"
#include "enemy_common.h"

#define FRAME(e) (*(u16 *)&(e)->animStep)

/**
 * @brief A04B0 | Enemy sub-state (file 0x080A02AC): animation 2 facing the
 * player, layer 0x11, 16 px higher; song 0x85 on frame 4, then animation 3
 * and 40 frames of blinking (Entity_GetVisibilityPattern); then back to state 0 hidden
 * behind (layer 0x1D) with a random 64..127 timer in unk_4FE and unk_4FC =
 * 1.
 */
void KyomaDemon_UpdateAttack(struct EwramData_EntityData *entity)
{
    u32 r;

    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->flipX = Entity_IsTargetRight(entity);
        entity->ext.kyomaDemon.hidden = 0;
        entity->scrollWithBg = 1;
        entity->subState++;
        entity->spriteLayer = 0x11;
        entity->posY.whole -= 0x100000;
        break;
    case 1:
        if (FRAME(entity) == 4)
            PlaySong(0x85);
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 3, 0, 0);
            entity->subState++;
            entity->timer = 0x28;
        }
        break;
    case 2:
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0)
        {
            entity->state = 0;
            entity->subState = 0;
            entity->scrollWithBg = 0;
            entity->hidden = 0;
            r = (RandomNumberGenerator() & 0x3F) + 0x40;
            entity->ext.kyomaDemon.lurkTimer = r;
            entity->ext.kyomaDemon.hidden = 1;
            entity->spriteLayer = 0x1D;
        }
        break;
    }
}

void KyomaDemon_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 damage = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(entity, attacker, hit, damage);
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
        Anim_Start(entity, &sKyomaDemonHitAnimScript, 0, 0);
        entity->palette = entity->basePalette;
        entity->timer = 0x3C;
        entity->state = 2;
        entity->subState = 0;
    }
}

/* Draw the player inside this effect's horizontal window, with a temporary
 * position, priority and facing, then restore the saved player state. */
static inline struct EwramData_EntityData *CtxV(void) { return *(struct EwramData_EntityData * volatile *)&gEwramData->playerState.player; }
void KyomaDemon_Draw(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram = gEwramData;
    struct BgLayerState *bg = &ewram->bgInfo[1];
    struct EwramData_EntityData *player = ewram->playerState.player;
    if (entity->state == 2)
    {
        s32 center = entity->ext.kyomaDemon.homeX;
        s32 x = CtxV()->posX.whole;
        s32 b_ = bg->xPos.whole;
        s32 offset = 0x80000;
        s32 screenX;
        b_ += offset;
        screenX = (x + b_) >> 16;
        if (screenX > (s16)center - 32 && screenX < (s16)center + 32)
        {
            s32 oldX = player->posX.whole;
            s32 oldY = player->posY.whole;
            s32 oldPriority = player->spriteLayer;
            s32 oldFlip = player->flipX;
            u8 old2 = player->drawSkipCounter;
            u8 old3 = player->drawSkip;
            player->posX.whole = x + offset;
            player->posY.whole = CtxV()->posY.whole;
            player->spriteLayer = 29;
            if (gEwramData->playerStatus.currentCharacter == 1)
                player->flipX ^= 1;
            Player_DrawWithOffset(player);
            player->posX.whole = oldX;
            player->posY.whole = oldY;
            player->spriteLayer = oldPriority;
            player->flipX = oldFlip;
            player->drawSkipCounter = old2;
            player->drawSkip = old3;
        }
        Sprite_DrawEntity(entity);
    }
    else Sprite_DrawEntity(entity);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   KyomaDemon_LoadGraphics = LegionWalkingCorpse_LoadGraphics */

/**
 * @brief A0744 | Same as LegionWalkingCorpse_LoadGraphics.
 */
u8 KyomaDemon_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sKyomaDemonGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sKyomaDemonPalette, 0);
    Sprite_Attach(entity, sKyomaDemonSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = KyomaDemon_Draw;
    return 1;
}

/* KyomaDemon_UpdateDeath. */
/**
 * @brief A0790 | Substate 0: blink while the timer timer runs, then
 * advance the substate.
 */
void KyomaDemon_UpdateDeath(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0)
            entity->subState++;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   KyomaDemon_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */

/**
 * @brief A07D0 | Same as Enemy_DamagePlayerOnContact.
 */
void KyomaDemon_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}
