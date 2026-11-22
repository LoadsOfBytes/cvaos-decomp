/* GolemDebris_OnKilled (death burst). */
#include "code_08009A0.h"
#include "structs/ewram.h"

extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern void HitEffect_Spawn(s32 x, s32 y, s32 type);
extern void Effect_SpawnFallingPiece(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 piece);
extern void Golem_UpdatePart(struct EwramData_EntityData *entity, s32 index);

/**
 * @brief 818D0 | Killed: HitEffect_Spawn at the position, two Effect_SpawnFallingPiece
 * pieces with the velocity +- 1 px random per axis, then delete.
 */
void GolemDebris_OnKilled(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 jitterX;
    s32 jitterY;

    Player_CalculateAttackPower(hit[2]);
    HitEffect_Spawn((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer, 0);
    jitterX = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
    jitterY = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
    Effect_SpawnFallingPiece(entity->posX.whole, entity->posY.whole, entity->velX.whole + jitterX,
                 entity->velY.whole + jitterY, entity->spriteLayer, 0);
    jitterX = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
    jitterY = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
    Effect_SpawnFallingPiece(entity->posX.whole, entity->posY.whole, entity->velX.whole + jitterX,
                 entity->velY.whole + jitterY, entity->spriteLayer, 0);
    EntityDelete(entity);
}

/* 8197C | Unless timer is 0, run Golem_UpdatePart on parts 0..10. */
void Golem_UpdateAllParts(struct EwramData_EntityData *entity)
{
    s32 i;

    if (entity->timer != 0)
    {
        for (i = 0; i <= 10; i++)
            Golem_UpdatePart(entity, i);
    }
}
