/* Julius_TryStartAbilityDash. */
#include "code_08009A0.h"
#include "code_080D73B8.h"

void Anim_StartIndexed(struct EwramData_EntityData *entity, s32 script, s32 mode, s32 flags);
extern void Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);

/**
 * @brief When flags 2, 0x20 and 0x100 are clear and the ability button is
 * held, enter state 11 with animation 3, facing velocity +-0x28000
 * and opposing deceleration 0x800, two afterimages and sound 0x12B.
 */
void Julius_TryStartAbilityDash(struct EwramData_EntityData *entity)
{
    s32 bits = entity->unk_4F4.whole & 0x122;

    if (bits == 0 && (gEwramData->inputData.playerHeldInput & gEwramData->playerStatus.abilityButton))
    {
        entity->state = 0xB;
        entity->subState = bits;
        entity->unk_504.part16.unk_504 = 0xFF;
        if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != 3)
        {
            Anim_StartIndexed(entity, 3, 1, 0);
            entity->unk_504.part16.unk_504 = 0xFF;
            entity->unk_4F4.whole &= 0xFFDFFFFF;
        }
        if (!entity->flipX)
            entity->velX.whole = 0x28000;
        else
            entity->velX.whole = -0x28000;
        entity->accelX.whole = (entity->velX.whole < 0) ? 0x800 : -0x800;
        entity->timer = 0;
        Afterimage_Create(entity, 0, 0xC, 0xD5);
        Afterimage_Create(entity, 0, 6, 0xD4);
        PlaySong(0x12B);
    }
}
