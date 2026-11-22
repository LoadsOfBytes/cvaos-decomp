/* Soma_TryCastRedSoulUnused (Soma variant
 * of Julius_HandleSubweaponInput in src/code_08033CAC.c). */
#include "code_08009A0.h"
#include "red_soul_entity.h"
#include "player_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"


/**
 * @brief 1F2C8 | Cast the equipped red soul on up + attack: switching soul
 * frees the old graphics (redSoulGfx); checks MP, the soul limit and a free
 * slot, spawns the soul entity, pays MP and plays animation 0x17 (state 2).
 */
void Soma_TryCastRedSoulUnused(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;
    s8 slot;
    struct RedSoulActivationView *info;

    if (PLAYER_ACTION_FLAGS(entity) & 0x540)
        return;
    if (!(gEwramData->inputData.playerHeldInput & 0x40))
        return;
    if (!(gEwramData->inputData.playerNewInput & gEwramData->playerStatus.attackButton))
        return;
    info = RedSoul_GetInfo(gEwramData->playerStatus.equippedRedSoul);
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
    if (info->mpCost >= gEwramData->playerStatus.currentMP)
        return;
    if (RedSoul_GetActiveLimit() <= gEwramData->playerState.nbrActiveRedSoulEntities)
        return;
    slot = RedSoul_FindFreeEntitySlot();
    if (slot < 0)
        return;
    child = EntityCreateInRange(2, 0x10, info->updateFunc);
    if (child == NULL)
        return;
    gEwramData->playerStatus.currentMP -= info->mpCost;
    gEwramData->playerState.activeRedSoulEntities[slot] = child;
    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != 0x17)
    {
        Anim_StartIndexed(entity, 0x17, 1, 0);
        PLAYER_REQUESTED_ANIM(entity) = 0xFF;
        PLAYER_ACTION_FLAGS(entity) &= 0xFFDFFFFF;
    }
    PLAYER_ACTION_FLAGS(entity) |= 0x100;
    entity->state = 2;
    RED_SOUL_OWNER(child) = entity;
    RED_SOUL_SLOT(child) = slot;
    RED_SOUL_ID(child) = gEwramData->playerStatus.equippedRedSoul;
    RED_SOUL_PARAM(child) = info->param;
    child->roomActive = 1;
    PLAYER_ACTION_FLAGS(entity) &= ~0x200060;
}
