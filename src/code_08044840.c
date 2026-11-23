#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
#include "entity_hit.h"

extern s32 RandomSigned(s32);
extern struct EwramData_EntityData *Effect_CreateFromTable(s32, s32, s32, s32, s32, u8);
extern struct EwramData_EntityData *SoulPickup_Create(s32 x, s32 y, s32 palette, s32 soulId, s32 stationary);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);

/**
 * @brief Emit particles and position the child animation; on a qualifying hit create the soul pickup, then hide and expire with the existing particle sequence.
 */
void SoulPickup_UpdateContainer(struct EwramData_EntityData *entity)
{
    struct HitResult res;
    u16 hit;
    struct EwramData_EntityData *other;

    hit = Hit_CheckWithPlayer(&res, (u8 *)entity);
    switch (entity->state)
    {
    case 0:
        entity->timer++;
        if ((entity->timer & 7) == 0)
        {
            s32 x = entity->posX.whole + (RandomSigned(8) << 16);
            s32 y = entity->posY.whole - (((RandomNumberGenerator() & 7) + 0x14) << 16);
            other = Effect_CreateFromTable(x, y, RandomSigned(0x1000), 0, 4, entity->ext.soulContainer.palette);
            if (other != NULL)
            {
                other->velY.whole = RandomSigned(0x4000) - 0x10000;
                other->accelY.whole = -other->velY.whole / 32;
            }
        }
        if (entity->ext.soulContainer.soul != NULL)
        {
            other = entity->ext.soulContainer.soul;
            other->posX.whole = entity->posX.whole;
            other->posY.whole = entity->posY.whole;
        }
        if (hit & 0x1E)
        {
            other = SoulPickup_Create(entity->posX.part16.integer, (u16)entity->posY.part16.integer,
                entity->enemyId, entity->spawnParam1, 0);
            if (other != NULL)
            {
                PlaySong(0xA2);
                other->spawnParam0 = entity->spawnParam0;
                entity->state = 1;
                entity->timer = 0x10;
                entity->hidden = 1;
                if (entity->ext.soulContainer.soul != NULL)
                    entity->ext.soulContainer.soul->deleteRequested = 1;
            }
        }
        Anim_Update(entity);
        break;
    case 1:
        if (!(entity->timer & 1))
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x180000, 0x80000, 0x80000, entity->spriteLayer - 1);
        if (entity->timer != 0)
            entity->timer--;
        else
            entity->deleteRequested = 1;
        break;
    }
}
