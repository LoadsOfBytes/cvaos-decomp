/* Julius_HandleWhipAttackInput. */
#include "code_08009A0.h"
#include "player_entity.h"
#include "code_080D73B8.h"

void Anim_StartIndexed(struct EwramData_EntityData *entity, s32 script, s32 mode, s32 flags);
void Julius_CreateWhipAttackEntity(struct EwramData_EntityData *entity);

/* Same block as written out in Julius_UpdateKnockbackFromImpulse / Julius_TryStartAbilityDash / Julius_StartDeath. */
static inline void SetPlayerAnim(struct EwramData_EntityData *entity, s32 anim)
{
    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != anim)
    {
        Anim_StartIndexed(entity, anim, 1, 0);
        entity->ext.player.requestedAnim = 0xFF;
        entity->ext.player.actionFlags &= 0xFFDFFFFF;
    }
}

/**
 * @brief On a new attack press (or playerStatus.inputOverrides bit 1), unless actionLockFlags bit
 * 0x80 or flags 0x160 block it, select animation 0x18 (0x1A when
 * crouching flag 0x400 is set), play sound 0x122, set flag 0x20,
 * enter state 1 and start the whip helper.
 */
void Julius_HandleWhipAttackInput(struct EwramData_EntityData *entity)
{
    if (gEwramData->playerState.actionLockFlags & 0x80)
        return;
    if (!(gEwramData->inputData.playerNewInput & gEwramData->playerStatus.attackButton)
        && !(gEwramData->playerStatus.inputOverrides & 2))
        return;
    if (entity->ext.player.actionFlags & 0x160)
        return;
    if (!(entity->ext.player.actionFlags & 0x400))
        SetPlayerAnim(entity, 0x18);
    else
        SetPlayerAnim(entity, 0x1A);
    PlaySong(0x122);
    entity->ext.player.actionFlags |= 0x20;
    entity->state = 1;
    entity->subState = 0;
    Julius_CreateWhipAttackEntity(entity);
}
