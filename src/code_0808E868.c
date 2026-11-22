#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern s32 Entity_MoveWithCollisionDefault(struct EwramData_EntityData *, s32, s32);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Effect_SpawnFallingPiece(s32, s32, s32, s32, s32, s32);
extern void GreatArmor_SpawnParticle(s32, s32, s32, s32, s32);
/**
 * @brief Shockwave update: move along the ground and delete when it leaves the
 * ground, stops or leaves the screen; contact test, debris pieces with sound
 * 0x80 every 8 frames and particles (GreatArmor_SpawnParticle) every 4.
 */
void GreatArmorShockwave_Update(struct EwramData_EntityData *entity)
{
    s32 buffer[4];
    s32 vx, vy;
    s32 oldX = (s16)entity->posX.part16.integer;
    if (!(Entity_MoveWithCollisionDefault(entity, 0x40000, -0x200000) & 4) || oldX == (s16)entity->posX.part16.integer || EntityIsOutsideScreen(entity, 6))
    {
        EntityDelete(entity);
        return;
    }
    Hit_CheckWithPlayer(buffer, entity);
    if ((entity->timer++ & 7) == 0)
    {
        s32 variant;
        vx = (RandomNumberGenerator() & 0xFFFF) - 0x8000;
        vy = -0x10000 - RandomNumberGenerator() % 0x30000;
        variant = RandomNumberGenerator() & 1;
        Effect_SpawnFallingPiece(entity->posX.whole, entity->posY.whole, vx, vy, entity->spriteLayer, variant);
        PlaySong(0x80);
    }
    if ((entity->timer & 3) == 2)
    {
        vx = (RandomNumberGenerator() & 0xFFFF) - 0x8000;
        vy = -0x10000 - RandomNumberGenerator() % 0x30000;
        GreatArmor_SpawnParticle(entity->posX.whole, entity->posY.whole, vx, vy, 9);
    }
}
