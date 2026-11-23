/* Big Golem update (like EnemyGolemUpdate): screen/hit checks unless dying;
 * states 0 / 1 (BigGolem_UpdateWalk / BigGolem_UpdateAttack), 2 dying: dust every 4th frame
 * while timer is in 0x79..0xD6, then frees the parts; otherwise
 * Entity_ClampXToRoom and a 1-in-4 chance to clear unk_4FE. */
#include "code_08009A0.h"
#include "big_golem_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *entity);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *entity);
extern void Heap_Free(void *parts);
extern void Effect_SpawnRisingParticle(s32 x, s32 y, s32 layer, s32 mode);
extern s32 Entity_ClampXToRoom(struct EwramData_EntityData *entity, s32 inset);
extern void BigGolem_UpdateWalk(struct EwramData_EntityData *entity);
extern void BigGolem_UpdateAttack(struct EwramData_EntityData *entity);

void EnemyBigGolemUpdate(struct EwramData_EntityData *entity)
{
    u8 unused[48];

    if (entity->state != 2)
    {
        if (Entity_IsInRect(entity, -16, 0, 272, 176))
            Player_TrackEntity(entity);
        if (entity->state != 2)
            Enemy_UpdateHitFlash(entity);
    }
    if (Entity_IsTimeStopped(entity) && entity->state != 2)
        goto idle;
    switch (entity->state)
    {
    case 0:
        BigGolem_UpdateWalk(entity);
        break;
    case 1:
        BigGolem_UpdateAttack(entity);
        break;
    case 2:
        if (entity->timer > 0x78 && entity->timer < 0xD7 && (entity->timer & 3) == 0)
        {
            s32 x = RandomNumberGenerator() % 0x280000 - 0x140000;
            RandomNumberGenerator();
            Effect_SpawnRisingParticle(entity->posX.whole + x, entity->posY.whole, entity->spriteLayer - 1, 1);
        }
        if ((u8)--entity->timer <= 1)
            entity->deleteRequested = 1;
        if (entity->deleteRequested && entity->unk_500.golemParts != NULL)
            Heap_Free(entity->unk_500.golemParts);
        return;
    case 3:
        return;
    }
idle:
    Entity_ClampXToRoom(entity, 1);
    if ((RandomNumberGenerator() & 0x7F) < 0x20)
        BIG_GOLEM_HIT(entity) = 0;
}
