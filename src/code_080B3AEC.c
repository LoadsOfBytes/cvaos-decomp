/* Update of a stacked column member. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void HeadStackSegment_Settle(struct EwramData_EntityData *);
extern s32 Enemy_IsStunned(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern s32 Target_IsInRect(s16, s16, s16, s16);
extern void HeadStackSegment_UpdateAttack(struct EwramData_EntityData *);
extern s32 HeadStackSegment_UpdateDeath(struct EwramData_EntityData *);

/**
 * @brief Settle a stack segment and dispatch proximity wait, attack, death or animation-only state.
 */
void HeadStackSegment_Update(struct EwramData_EntityData *entity)
{
    u32 buf[4];
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);

    if (entity->state != 2)
    {
        if (owner->ext.headStack.onScreen == 0)
            return;
        HeadStackSegment_Settle(entity);
    }
    if (Enemy_IsStunned(entity) != 0)
    {
        if (entity->state != 2)
            Hit_CheckWithPlayer(buf, entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        if (entity->subState == 0
            && Target_IsInRect(entity->posX.part16.integer - 0x50, entity->posY.part16.integer - 0x3C, 0xA0, 0x78))
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 1:
        HeadStackSegment_UpdateAttack(entity);
        break;
    case 2:
        if (HeadStackSegment_UpdateDeath(entity) != 0)
        {
            EntityDelete(entity);
            return;
        }
        break;
    case 3:
        Anim_Update(entity);
        return;
    }
    if (entity->state != 2)
        Hit_CheckWithPlayer(buf, entity);
    Anim_Update(entity);
}
