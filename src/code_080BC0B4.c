/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   CockatriceFamily_OnHit = HeadStackSegment_OnHit */
#include "code_08009A0.h"
#include "cockatrice_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Enemy_TakeDamage(struct EwramData_EntityData *param_0);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief Apply enemy damage; on defeat drop rewards, restore the palette
 * and enter death state 2 with substate counters reset.
 */
void CockatriceFamily_OnHit(struct EwramData_EntityData *entity)
{
    if (Enemy_TakeDamage(entity))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/* Cockatrice/Basilisk hit callback, slash trail and Cockatrice create
 * 0x080BC0F4..0x080BC163. */
extern void CockatriceFamily_Create(struct EwramData_EntityData *entity, s32 paletteSlot);

/**
 * @brief Apply the shared attack damage at the effect position with
 * effect type 0x100 and zero effect flags.
 */
void CockatriceFamilyAttack_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 0x100;
    contactDamage.unk_4 = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief Draw 16 sprite copies stepped by the stored direction vector,
 * then restore both full position values.
 */
void CockatriceFamilyAttack_Draw(struct EwramData_EntityData *entity)
{
    s32 x = entity->posX.whole;
    s32 y = entity->posY.whole;
    s32 i;

    for (i = 16; i != 0; i--)
    {
        Sprite_Draw(entity);
        entity->posX.whole += COCKATRICE_BEAM_STEP_X(entity);
        entity->posY.whole += COCKATRICE_BEAM_STEP_Y(entity);
    }
    entity->posX.whole = x;
    entity->posY.whole = y;
}

/**
 * @brief BC158 | Cockatrice create: Cockatrice/Basilisk with palette row 0.
 */
void EnemyCockatriceCreate(struct EwramData_EntityData *entity)
{
    CockatriceFamily_Create(entity, 0);
}
