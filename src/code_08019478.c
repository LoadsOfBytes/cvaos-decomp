/* Soma_CastEquippedRedSoul; soul spawn
 * as in Soma_TryCastRedSoulUnused (src/code_0801F2C8.c). */
#include "code_08009A0.h"
#include "player_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"

struct PlayerBodyBox {
    u8 x;
    u8 y;
    u8 width;
    u8 height;
};

extern struct PlayerBodyBox sSomaStandingBodyBox[];
extern struct PlayerBodyBox sSomaAirborneBodyBox[];
extern u8 sSomaAnimPalettes[];
extern u8 sSomaPalette[];
extern u8 RedSoul_GetCastAnim(s32 pose);

/* Animation switch of the player states (as SetPlayerAnim in
 * src/code_08019BB0.c) with the hitbox copied from table[0]. */
static inline void SetPlayerAnimBox(struct EwramData_EntityData *entity, u8 anim, struct PlayerBodyBox *table)
{
    struct PlayerBodyBox box = table[0];
    s32 pal;

    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != anim)
    {
        Anim_StartIndexed(entity, anim, 3, 0);
        HitBox_SetBody(entity, &box);
        entity->unk_504.part16.unk_504 = 0xFF;
        entity->unk_4F4.whole &= ~0x200000;
        pal = sSomaAnimPalettes[anim];
        if (PLAYER_ANIM_PALETTE(entity) != pal)
        {
            PLAYER_ANIM_PALETTE(entity) = pal;
            ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
        }
    }
}

/**
 * @brief Validate equipped red soul, MP, active limit and entity slot,
 * create and register its entity and deduct MP. Unless noAnim,
 * apply cast animation/motion and enter casting state 2.
 */
void Soma_CastEquippedRedSoul(struct EwramData_EntityData *entity, s32 noAnim)
{
    struct RedSoulActivationView *info = RedSoul_GetInfo(gEwramData->playerStatus.equippedRedSoul);
    struct EwramData_EntityData *soul;
    s8 slot;
    u8 anim;

    if (gEwramData->playerState.actionLockFlags & 0x100)
        return;
    if (gEwramData->gameState.gameFlagsTail[0] & 0x200000)
        return;
    if (gEwramData->playerState.nbrActiveRedSoulEntities == 0)
    {
        if (gEwramData->playerState.lastUsedRedSoul != gEwramData->playerStatus.equippedRedSoul)
        {
            gEwramData->playerState.lastUsedRedSoul = gEwramData->playerStatus.equippedRedSoul;
            if (gEwramData->playerState.redSoulGfx != -1)
            {
                ObjGfx_Unload(gEwramData->playerState.redSoulGfx);
                gEwramData->playerState.redSoulGfx = -1;
            }
        }
    }
    if (gEwramData->playerState.lastUsedRedSoul != gEwramData->playerStatus.equippedRedSoul)
        return;
    if (info->updateFunc == 0)
        return;
    if (info->mpCost > gEwramData->playerStatus.currentMP)
        return;
    if (RedSoul_GetActiveLimit() <= gEwramData->playerState.nbrActiveRedSoulEntities)
        return;
    slot = RedSoul_FindFreeEntitySlot();
    if (slot < 0)
        return;
    soul = EntityCreateInRange(2, 0x10, info->updateFunc);
    if (soul == NULL)
        return;
    gEwramData->playerStatus.currentMP -= info->mpCost;
    gEwramData->playerState.activeRedSoulEntities[slot] = soul;
    soul->unk_4F8.unk_4F8_entity = entity;
    soul->unk_4FC.part16.unk_4FE[0] = slot;
    soul->unk_500.part16.unk_500 = gEwramData->playerStatus.equippedRedSoul;
    soul->unk_500.part16.unk_502 = info->unk_E;
    soul->roomActive = 1;
    if (noAnim)
        return;
    entity->subState = 0;
    if (!(entity->unk_4F4.whole & 2))
    {
        anim = RedSoul_GetCastAnim(0);
        SetPlayerAnimBox(entity, anim, sSomaStandingBodyBox);
        switch (info->castAnimKind)
        {
        case 1:
        {
            u8 v = 0x10;
            PLAYER_INVULNERABLE_TIMER(gEwramData->playerState.player) = v;
            entity->unk_4F4.whole = (entity->unk_4F4.whole | 0xA) & ~0x100400;
            entity->velY.whole = -0x38000;
            entity->accelY.whole = -0x1000;
            break;
        }
        case 2:
        {
            u8 v = 0x10;
            PLAYER_INVULNERABLE_TIMER(gEwramData->playerState.player) = v;
            if (!entity->flipX)
                entity->velX.whole = 0x30000;
            else
                entity->velX.whole = -0x30000;
            break;
        }
        case 3:
        {
            u8 v = 0x32;
            PLAYER_INVULNERABLE_TIMER(gEwramData->playerState.player) = v;
            entity->subState = 1;
            break;
        }
        }
    }
    else
    {
        anim = RedSoul_GetCastAnim(1);
        SetPlayerAnimBox(entity, anim, sSomaAirborneBodyBox);
        if (info->castAnimKind == 3)
        {
            u8 v = 0x14;
            PLAYER_INVULNERABLE_TIMER(gEwramData->playerState.player) = v;
        }
    }
    entity->unk_4F4.whole = (entity->unk_4F4.whole & ~0x200060) | 0x100;
    entity->state = 2;
}
