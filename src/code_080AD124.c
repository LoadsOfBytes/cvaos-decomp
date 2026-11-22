/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   BigGolemRock_OnKilled = GolemDebris_OnKilled */
#include "code_08009A0.h"
#include "structs/ewram.h"

extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern void HitEffect_Spawn(s32 x, s32 y, s32 type);
extern void Effect_SpawnFallingPiece(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 piece);

/**
 * @brief Same as GolemDebris_OnKilled: hit effect, two falling pieces, delete.
 */
void BigGolemRock_OnKilled(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
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
