#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 GhostDancer_LoadGraphics(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void GhostDancer_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void GhostDancer_OnHit(struct EwramData_EntityData *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void (*)(struct EwramData_EntityData *), void (*)(struct EwramData_EntityData *));

void EnemyGhostDancerCreate(struct EwramData_EntityData *input)
{
    /* agbcc otherwise assigns r4 to the zero kept across Enemy_ResetFromTemplate,
     * exchanging r4/r5 throughout this function. Preserve the ROM allocation. */
    register struct EwramData_EntityData *entity asm("r4") = input; /* FAKEMATCH: pinned register */
    if (GhostDancer_LoadGraphics(entity))
    {
        u8 flags;
        entity->spriteLayer = 3;
        Enemy_ResetFromTemplate(entity);
        entity->state = 1;
        entity->stage = 0;
        entity->subState = 0;
        /* The byte temporary retains the original narrowing and provides the
         * zero halfword written below when this test is false. */
        flags = ((u8 *)gEwramData)[0x8E] & 0x40;
        if (flags)
            entity->state = 5;
        else
        {
            entity->ext.ghostDancer.phase = flags;
            entity->ext.ghostDancer.cycles = 0;
            entity->ext.ghostDancer.side = 1;
            entity->objMode = 1;
            entity->ext.ghostDancer.mode = 0;
            entity->ext.ghostDancer.timer = 0;
            Entity_EnableBoxes(entity, GhostDancer_DamagePlayerOnContact, GhostDancer_OnHit);
        }
    }
}
