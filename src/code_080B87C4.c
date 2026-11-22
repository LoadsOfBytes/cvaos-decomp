#include "structs/ewram.h"
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern void WitchGroundProjectile_Update(struct EwramData_EntityData *);
static inline s32 Add16(s16 a, u16 b) { return a + b; }

static inline void FollowParent(struct EwramData_EntityData *child, struct EwramData_EntityData *owner, u16 x_, u16 y_)
{
    u16 x = x_;
    s16 y = y_;
    child->flipX = owner->flipX;
    if (child->flipX) x = (-((u32)x << 16)) >> 16;
    child->posX.part16.integer = Add16(x, owner->posX.part16.integer);
    child->posY.part16.integer = (s32)y + owner->posY.part16.integer;
}

/* Spawn an attached effect at a mirrored offset, with half the owner value. */
/**
 * @brief Spawn the shared ground projectile at a mirrored offset with half
 * the owner damage value; copy the supplied mode into its substate.
 */
void Witch_SpawnGroundProjectile(struct EwramData_EntityData *owner, u16 x, u16 y, s32 state)
{
    struct EwramData_EntityData *child = Entity_CreateObject((u32 *)WitchGroundProjectile_Update);
    if (child != NULL)
    {
        child->attack = owner->attack >> 1;
        FollowParent(child, owner, x, y);
        child->subState = state;
        child->scrollWithBg = 1;
    }
}


