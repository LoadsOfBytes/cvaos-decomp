/* Soma_HandleWeaponAttackInput. */
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
extern struct PlayerBodyBox sSomaCrouchBodyBoxes[];
extern u8 sSomaAnimPalettes[];
extern u8 sSomaPalette[];

/* Animation switch as in Soma_UpdateStatusReaction with the hitbox table as parameter; the
 * u8 anim parameter is needed for the original register allocation. */
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
 * @brief On a permitted attack request with no active attack entity,
 * start the weapon helper and choose standing/crouching/air animation
 * and box, then enter weapon attack state 1 under existing guards.
 */
void Soma_HandleWeaponAttackInput(struct EwramData_EntityData *entity)
{

    if (gEwramData->playerState.actionLockFlags & 0x80)
        return;
    if (!(gEwramData->inputData.playerNewInput & gEwramData->playerStatus.attackButton) && !(gEwramData->playerStatus.inputOverrides & 2))
        return;
    if (entity->unk_4F4.whole & 0x160)
        return;
    if (gEwramData->playerState.weaponEntities[0] != NULL)
        return;
    if (Weapon_CreateAttackEntity(entity) == NULL)
        return;
    if (Weapon_GetEquippedInfo()->attackAnimKind == 5)
        return;
    if (entity->unk_4F4.whole & 2)
    {
        entity->unk_4F4.whole |= 0x40;
        SetPlayerAnimBox(entity, Weapon_GetPlayerAttackAnim(2), sSomaAirborneBodyBox);
    }
    else if (!(entity->unk_4F4.whole & 0x400))
    {
        SetPlayerAnimBox(entity, Weapon_GetPlayerAttackAnim(0), sSomaStandingBodyBox);
    }
    else
    {
        SetPlayerAnimBox(entity, Weapon_GetPlayerAttackAnim(1), sSomaCrouchBodyBoxes);
    }
    entity->unk_4F4.whole |= 0x20;
    entity->state = 1;
}
