#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
/* Cleared at the start of every update; never read. */
#define CREAKING_SKULL_UNUSED_FLAG(e) e->unk_504.part8.unk_505

extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *entity);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *entity);
extern void Heap_Free(void *parts);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern void CreakingSkull_UpdateMovement(struct EwramData_EntityData *entity);
extern void CreakingSkull_UpdateAttack(struct EwramData_EntityData *entity);
extern void CreakingSkull_UpdateBestiaryPreview(struct EwramData_EntityData *entity);

struct SkullBox {
    s32 x:8;
    s32 y:8;
    u32 w:8;
    u32 h:8;
};

#define TO_PIXELS(d) ((d) < 0 ? -(-(d) >> 16) : (d) >> 16)

/**
 * @brief 95320 | Creaking Skull update (as EnemyGreatArmorUpdate): clears
 * unk_505, skipped off screen unless dying; states 0 / 1 (CreakingSkull_UpdateMovement /
 * CreakingSkull_UpdateAttack), 2 dying frees the parts (Heap_Free) when done, 3
 * CreakingSkull_UpdateBestiaryPreview; the hit box (0x2C wide) follows part 5 of the 13 parts
 * (mirrored by flipX).
 */
void EnemyCreakingSkullUpdate(struct EwramData_EntityData *entity)
{
    u8 unused[48];
    struct SkullBox box;
    s32 *parts; /* EntityJointPart viewed as 12 words (needed for the addressing) */

    CREAKING_SKULL_UNUSED_FLAG(entity) = 0;
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
        CreakingSkull_UpdateMovement(entity);
        break;
    case 1:
        CreakingSkull_UpdateAttack(entity);
        break;
    case 2:
        if ((s8)--entity->timer <= 0)
            entity->deleteRequested = 1;
        if (entity->deleteRequested && entity->unk_500.jointParts != NULL)
            Heap_Free(entity->unk_500.jointParts);
        return;
    case 3:
        CreakingSkull_UpdateBestiaryPreview(entity);
        return;
    }
boxes:
    parts = (s32 *)entity->unk_500.jointParts;
    if (entity->flipX)
        box.x = -TO_PIXELS(parts[5 * 12 + 0]) - 0x14;
    else
        box.x = TO_PIXELS(parts[5 * 12 + 0]) - 0x14;
    box.y = TO_PIXELS(parts[5 * 12 + 1]) - 0x14;
    box.w = 0x2C;
    box.h = box.y < 0 ? -box.y : box.y;
    HitBox_SetAttack(entity, &box);
    HitBox_SetBody(entity, &box);
    Enemy_CheckHitPlayer(entity);
}
