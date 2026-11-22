#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern void GrahamPlayerEffect_Update(struct EwramData_EntityData *);
extern struct EwramData_EntityData *EntityCreateInRange(s32, s32, void (*)(struct EwramData_EntityData *));

/* Remove this effect and spawn a follower above the current player. */
/**
 * @brief Request projectile deletion and create the animation-12 effect
 * linked to the current player at a fixed upward offset, if allocation
 * and the player pointer permit it.
 */
void GrahamSceneProjectile_OnContact(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player;
    struct EwramData_EntityData *effect;
    s32 kind, offset;
    s32 zero;
    entity->deleteRequested = 1;
    player = gEwramData->playerState.player;
    zero = 0;
    kind = 12;
    offset = -0x100000;
    if (player != 0)
    {
        effect = EntityCreateInRange(0x49, 0xCF, GrahamPlayerEffect_Update);
        if (effect != 0)
        {
            effect->unk_4F8.part8.unk_4F8 = kind;
            effect->unk_4FC.unk_4FC_entity = player;
            effect->palette = 5;
            effect->accelX.whole = zero;
            effect->accelY.whole = offset;
            effect->posX.whole = player->posX.whole;
            effect->posY.whole = player->posY.whole + offset;
            effect->spriteLayer = kind;
        }
    }
}
