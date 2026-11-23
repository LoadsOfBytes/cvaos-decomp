/* MimicTrigger_Update and MimicTrigger_OnHit. */
#include "code/code_080211F0.h"
#include "structs/ewram.h"

/* state and subState read as one halfword (as in code_080D34FC.c). */
#define STATE_WORD(e) (*(u16 *)&(e)->state)

extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern void Effect_SpawnRandom(s32 x, s32 y, s32 width, s32 height, s32 layer);

/**
 * @brief 9937C | State 0: hitbox. State 1: hidden; while the timer timer
 * is above 0x1E spawn Effect_SpawnRandom particles every 4 frames, and set
 * bit 3 of 0x53D when it runs out.
 */
void MimicTrigger_Update(struct EwramData_EntityData *entity)
{
    u8 collision[0x10];

    switch (entity->state)
    {
        case 0:
            Hit_CheckWithPlayer(collision, entity);
            break;
        case 1:
            entity->hidden = 1;
            if (entity->timer > 0x1E && !(entity->timer & 3))
                Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0x80000, 0x80000, entity->spriteLayer - 1);
            if (--entity->timer == 0xFF)
                entity->deleteRequested = 1;
            break;
    }
}

/**
 * @brief 993EC | Hit callback: Player_CalculateAttackPower(hit[2]), go to state 1
 * with timer 0x28 and advance the substate of the parent (unk_500_entity)
 * when it is alive, of the same type (unk_51A) and in state 0 / substate 0.
 */
void MimicTrigger_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    struct EwramData_EntityData *mimic = entity->ext.mimicTrigger.owner;

    Player_CalculateAttackPower(hit[2]);
    entity->state = 1;
    entity->subState = 0;
    entity->timer = 0x28;
    if (mimic != NULL && mimic->updateFunc != NULL
        && mimic->unk_518.part8.enemyId == entity->unk_518.part8.enemyId
        && STATE_WORD(mimic) == 0)
        mimic->subState++;
}
