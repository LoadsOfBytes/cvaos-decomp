#include "code_08009A0.h"
#include "bg_particle_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void BgParticle_Update(struct EwramData_EntityData *entity);

extern const s32 sBgParticleCounts[];

/**
 * @brief 4F374 | Spawn sBgParticleCounts[unk_516] particles of four sizes with
 * random positions and velocities (size 3 is placed relative to the scroll).
 * Returns no value; r0 is left as the original leaves it.
 */
s32 BgParticle_SpawnAll(struct EwramData_EntityData *owner)
{
    s32 i;
    struct EwramData_EntityData *entity;

    for (i = 0; i < sBgParticleCounts[owner->unk_514.part16.spawnParam1]; i++)
    {
                entity = EntityCreateInRange(0x49, 0xCF, (u32 *)BgParticle_Update);
        if (!entity)
            continue;
        BG_PARTICLE_OWNER_INDEX(entity) = ENTITY_INDEX(owner);
        BG_PARTICLE_KIND(entity) = RandomNumberGenerator() & 0xF;
        if (BG_PARTICLE_KIND(entity) > 8)
        {
            BG_PARTICLE_KIND(entity) = 2;
            entity->spriteLayer = 0x1D;
        }
        else if (BG_PARTICLE_KIND(entity) > 5)
        {
            BG_PARTICLE_KIND(entity) = 1;
            entity->spriteLayer = 0x1A;
        }
        else if (BG_PARTICLE_KIND(entity) > 1)
        {
            BG_PARTICLE_KIND(entity) = 0;
            entity->spriteLayer = 0x19;
        }
        else
        {
            BG_PARTICLE_KIND(entity) = 3;
            entity->spriteLayer = 0x1D;
        }
        if (BG_PARTICLE_KIND(entity) != 3)
        {
            entity->posX.part16.integer = (RandomNumberGenerator() & 0x3F) * 4 + 0xFFE0;
            entity->posY.part16.integer = (RandomNumberGenerator() & 0x7F) + 1;
            entity->accelX.whole = ((RandomNumberGenerator() & 0x3FFF) + 0xC000) >> BG_PARTICLE_KIND(entity);
            entity->accelY.whole = ((RandomNumberGenerator() & 0x1FFF) + 0x8000) >> BG_PARTICLE_KIND(entity);
        }
        else
        {
            entity->posX.part16.integer = 0x8C - ((s16)(gEwramData->bgInfo + 1)->xPos.part16.integer >> 2) + (RandomNumberGenerator() & 0x3F);
            entity->posY.part16.integer = (RandomNumberGenerator() & 0x3F) + 0x21;
            entity->accelX.whole = ((RandomNumberGenerator() & 0x7F) << 4) + 0x800;
            entity->accelY.whole = ((RandomNumberGenerator() & 0x7F) << 4) + 0x800;
            BG_PARTICLE_ANGLE_Y(entity) = (RandomNumberGenerator() & 0x1F) << 12;
        }
        if (entity->flipX)
            entity->accelX.whole = -entity->accelX.whole;
        BG_PARTICLE_ANGLE_X(entity) = (RandomNumberGenerator() & 0x1F) << 12;
    }
}
