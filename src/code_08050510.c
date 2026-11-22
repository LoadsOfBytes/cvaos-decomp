/* Object0C_LaunchPlayerTrigger, decompiled from the original assembly. The store to
 * hitSourceX goes through a pointer type: with strict aliasing it may alias
 * the player pointer, which is then reloaded as in the target. */
/* Object 0x0C: rectangular interaction regions with a callback table. */
#include "structs/ewram.h"
#include "player_entity.h"
#include "code_08001194.h"
#include "code/code_080211F0.h"

struct Object0CTrigger {
    u16 type;
    u16 unk_2;
    s32 value;
    s16 x;
    s16 y;
    u16 width;
    u16 height;
};

extern struct Object0CTrigger *sObject0CTriggerLists[];
extern void (*sObject0CTriggerFuncs[])(struct EwramData_EntityData *, struct Object0CTrigger *);

/**
 * @brief Trigger type 2: when bit 9 of soulStateFlags or player flag 0x400000 is set,
 * set player flag 0x4000000 and either mark unk_502 or start the scripted
 * player motion (hitReaction.. fields) with speed 0x30000 left (value 0) or
 * right, copying that speed to the entity velX.
 */
void Object0C_LaunchPlayerTrigger(struct EwramData_EntityData *entity, struct Object0CTrigger *trigger)
{
    s32 speed = 0x30000;
    struct EwramData *ewram = gEwramData;
    if ((ewram->playerStatus.soulStateFlags & 0x200)
        || (PLAYER_ACTION_FLAGS(ewram->playerState.player) & 0x400000))
    {
        struct EwramData_EntityData **player = &ewram->playerState.player;
        PLAYER_ACTION_FLAGS((*player)) |= 0x4000000;
        if ((*player)->unk_500.part8.unk_502 != 0)
            (*player)->unk_500.part8.unk_502 = 1;
        else
        {
            ewram->playerState.pendingHitDamage = 1;
            ewram->playerState.hitReaction = 3;
            *(struct EwramData_EntityData **)&ewram->playerState.hitSourceX = (void *)((*player)->posX.whole + 0x80000);
            ewram->playerState.hitSourceY = (*player)->posY.whole;
            if (trigger->value == 0)
            {
                s32 negative = -speed;
                Player_SetKnockbackVelocity(negative, negative);
                entity->velX.whole = negative;
            }
            else
            {
                Player_SetKnockbackVelocity(speed, -0x30000);
                entity->velX.whole = speed;
            }
        }
    }
}
