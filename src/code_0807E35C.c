#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *entity);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *entity);
extern void Heap_Free(void *parts);
extern void Effect_SpawnRisingParticle(s32 x, s32 y, s32 layer, s32 mode);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern void Golem_UpdateMovement(struct EwramData_EntityData *entity);
extern void Golem_UpdateAttack(struct EwramData_EntityData *entity);
extern void Golem_UpdateBestiaryPreview(struct EwramData_EntityData *entity);

struct GolemBox {
    s32 x:8;
    s32 y:8;
    u32 w:8;
    u32 h:8;
};

#define TO_PIXELS(d) ((d) < 0 ? -(-(d) >> 16) : (d) >> 16)

/**
 * @brief 7E35C | Golem update (as EnemyGreatArmorUpdate): skipped off
 * screen unless dying; states 0 / 1 (Golem_UpdateMovement / Golem_UpdateAttack), 2
 * dying: dust every 4th frame within +-20 px (Effect_SpawnRisingParticle), frees the
 * parts (Heap_Free) when done, 3 Golem_UpdateBestiaryPreview; the 16 px hit box
 * follows part 4 (mirrored by flipX).
 */
void EnemyGolemUpdate(struct EwramData_EntityData *entity)
{
    u8 unused[48];
    struct GolemBox box;
    s32 *parts; /* GolemPart viewed as 8 words (needed for the addressing) */

    if (entity->state != 2)
    {
        if (EntityIsOutsideScreen(entity, 6))
            return;
        if (Entity_IsInRect(entity, -16, 0, 272, 176))
            Player_TrackEntity(entity);
        if (entity->state != 2)
            Enemy_UpdateHitFlash(entity);
    }
    if (Entity_IsTimeStopped(entity) && entity->state != 2)
        goto boxes;
    switch (entity->state)
    {
    case 0:
        Golem_UpdateMovement(entity);
        break;
    case 1:
        Golem_UpdateAttack(entity);
        break;
    case 2:
        if ((entity->timer & 3) == 0)
        {
            s32 x = RandomNumberGenerator() % 0x280000 - 0x140000;
            RandomNumberGenerator();
            Effect_SpawnRisingParticle(entity->posX.whole + x, entity->posY.whole, entity->spriteLayer - 1, 1);
        }
        if ((s8)--entity->timer <= 0)
            entity->deleteRequested = 1;
        if (entity->deleteRequested && entity->unk_500.golemParts != NULL)
            Heap_Free(entity->unk_500.golemParts);
        break;
    case 3:
        Golem_UpdateBestiaryPreview(entity);
        return;
    }
boxes:
    parts = (s32 *)entity->unk_500.golemParts;
    if (entity->flipX)
        box.x = -TO_PIXELS(parts[4 * 8 + 0]) - 8;
    else
        box.x = TO_PIXELS(parts[4 * 8 + 0]) - 8;
    box.y = TO_PIXELS(parts[4 * 8 + 1]) - 8;
    box.w = 0x10;
    box.h = box.y < 0 ? -box.y : box.y;
    HitBox_SetAttack(entity, &box);
    HitBox_SetBody(entity, &box);
    Enemy_CheckHitPlayer(entity);
}
