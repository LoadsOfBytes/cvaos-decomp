/* Hit callback with an equipped-souls check. */
#include "code_080D73B8.h"
#include "graham_entity.h"
#include "structs/ewram.h"

extern s32 GrahamPart_TakeDamage(struct EwramData_EntityData *, s32, u16 *);

#define SET_STATE(e, s)                     \
    {                                       \
        (e)->state = (s);                 \
        (e)->stage = 0;                   \
        (e)->subState = 0;                   \
    }

/* 0 when exactly this red/blue/yellow soul set is equipped */
static inline s32 SoulsDiffer(u8 red, u8 blue, u8 yellow)
{
    struct PlayerStatus *equip = &gEwramData->playerStatus;
    s32 differ = 0;

    if (equip->equippedRedSoul != red)
        return 1;
    if (equip->equippedBlueSoul != blue)
        return 1;
    if (equip->equippedYellowSoul != yellow)
        differ = 1;
    return differ;
}

/**
 * @brief Apply linked-part damage. First HP exhaustion sets flag 2 and
 * restarts teleport; later exhaustion starts transition state 5. On
 * that transition, equipment soul IDs 0x2C/2/7 set root flag 8.
 */
void GrahamFirstForm_OnHit(struct EwramData_EntityData *entity, s32 hit, u16 *data)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[(u8)ENTITY_PARENT_INDEX(entity)];

    if (GrahamPart_TakeDamage(entity, hit, data))
    {
        if (!(entity->ext.graham.flags & 2))
        {
            entity->ext.graham.flags |= 2;
            SET_STATE(entity, 3);
        }
        else
        {
            SET_STATE(entity, 5);
            if (!SoulsDiffer(0x2C, 2, 7))
                owner->ext.graham.flags |= 8;
        }
    }
    PlaySong(0x1DA);
}
